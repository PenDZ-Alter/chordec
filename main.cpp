#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <string_view>
#include <cstdlib>
#include <iomanip>
#include <utility>

#include "src/parser.cpp"
#include "src/chord.cpp"
#include "src/ca_math.cpp"
#include "src/headers/params.h"
#include "src/info.cpp"
#include "src/export.cpp"

#include "version.h"

int main(int argc, char *argv[])
{
    std::string file;
    std::string outputFile = "";

    AudioBuffer audio;
    size_t FFT_SIZE = 8192;
    size_t HOP_SIZE = FFT_SIZE / 4;
    size_t SMOOTHING_WINDOW_SIZE = 20;
    double tuning = 440.0; // reference of A4 freq
    double weighted7thSize = 0.6;
    bool usingChromaOptimization = false;
    bool usingWeightedTemplates = false;
    bool using7thChords = true;
    bool usingLogCompression = false;
    bool usingL2Normalization = false;
    std::string signature = "default";

    // Loop through arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        // Helper lambda to extract value after '='
        auto getValue = [](std::string_view a, size_t prefixLen)
        {
            return std::string(a.substr(prefixLen));
        };

        if (arg == "-v" || arg == "--version")
        {
            printVersion();
            return 0;
        }
        else if (arg == "-h" || arg == "--help")
        {
            printHelp();
            return 0;
        }
        else if (arg.starts_with("--path=") || arg.starts_with("-p="))
        {
            size_t len = arg.starts_with("--path=") ? 7 : 3;
            file = getValue(arg, len);
        }
        else if (arg.starts_with("--fft-size="))
        {
            FFT_SIZE = std::stoul(getValue(arg, 11));
        }
        else if (arg == "--use-chroma")
        {
            usingChromaOptimization = true;
        }
        else if (arg == "--use-weighted")
        {
            if (!using7thChords)
            {
                std::cerr << "For using weighted 7th chords, you need to enable 7th chords!" << std::endl;
                return 1;
            }
            usingWeightedTemplates = true;
        }
        else if (arg == "--disable-7th-chords" || arg == "--disable-7th")
        {
            using7thChords = false;
        }
        else if (arg == "--use-log-compression" || arg == "--use-log") { 
            usingLogCompression = true;
        }
        else if (arg == "--use-l2-normalization" || arg == "--use-l2") {
            usingL2Normalization = true;
        }
        else if (arg.starts_with("--hop-size=")) {
            HOP_SIZE = std::stoul(getValue(arg, 11));
        }
        else if (arg.starts_with("--weight=") || arg.starts_with("-w="))
        {
            size_t len = arg.starts_with("--weight=") ? 9 : 3;
            double weightValue = std::stod(getValue(arg, len));
            if (weightValue < 0.0 || weightValue > 1.0)
            {
                std::cerr << "Error: Weight value must be between 0.0 and 1.0\n";
                return 1;
            }
            weighted7thSize = weightValue;
        }
        else if (arg.starts_with("--tuning=") || arg.starts_with("-t=")) {
            size_t len = arg.starts_with("--tuning=") ? 9 : 3;
            double tuningValue = std::stod(getValue(arg, len));
            tuning = tuningValue;
        }
        else if (arg.starts_with("-swz="))
        {
            SMOOTHING_WINDOW_SIZE = std::stoul(getValue(arg, 5));
        }
        else if (arg.starts_with("--output=") || arg.starts_with("-o="))
        {
            size_t len = arg.starts_with("--output=") ? 9 : 3;
            outputFile = getValue(arg, len);
        }
        else if (arg.starts_with("--signature=") || arg.starts_with("-sign="))
        {
            size_t len = arg.starts_with("--signature=") ? 12 : 6;
            std::string_view sign = arg.substr(len);
            if (sign != "default" && sign != "sharp" && sign != "flat")
            {
                std::cerr << "Unknown signature: Only supported 'sharp', 'flat', and 'default'\n";
                printHelp();
                return 1;
            }
            signature = std::string(sign);
        }
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            printHelp();
            return 1;
        }
    }

    if (file.empty())
    {
        std::cerr << "Error: No audio file specified. Use --path=<file> or -p=<file> to specify the audio file.\n";
        printHelp();
        return 1;
    }

    try
    {
        audio = loadAudioFile(file);

        std::cout << "--- Audio Loaded Successfully ---\n";
        std::cout << "Sample Rate : " << audio.sampleRate << " Hz\n";
        std::cout << "Channels    : " << audio.channels << "\n";
        std::cout << "Total Sample: " << audio.samples.size() << " frame\n";
        std::cout << "Duration    : " << static_cast<double>(audio.samples.size()) / audio.sampleRate << " seconds\n";
        std::cout << "Format      : " << (audio.format == AudioFormat::WAV ? "WAV" : (audio.format == AudioFormat::FLAC ? "FLAC" : (audio.format == AudioFormat::MP3 ? "MP3" : "UNKNOWN"))) << "\n";
        std::cout << "FFT Size    : " << FFT_SIZE << "\n";
        std::cout << "HOP Size    : " << HOP_SIZE << "\n";
        std::cout << "Signature   : " << signature << "\n";
        std::cout << "Smoothing Window Size: " << SMOOTHING_WINDOW_SIZE << "\n";
        std::cout << "Using Chroma Optimization: " << (usingChromaOptimization ? "Yes" : "No") << "\n";
        std::cout << "Using Weighted 7th Chord Templates: " << (usingWeightedTemplates ? "Yes" : "No") << "\n";
        std::cout << "---------------------------------\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    try
    {
        auto chordTemplates = usingWeightedTemplates ? generateWeightedChordTemplates(weighted7thSize, signature, using7thChords) : generateChordTemplates(signature, using7thChords);
        size_t totalSamples = audio.samples.size();

        // Raw buffer structure for pass 1
        std::vector<std::string> rawChords;
        std::vector<double> frameTimes;

        std::cout << "Processing audio frames...\n";

        std::ofstream debugFile("/home/abx41/code/chordec/debug/chroma.txt");
        std::ofstream compareFile("/home/abx41/code/chordec/debug/candidates.txt");
        std::ofstream rawChordsFile("/home/abx41/code/chordec/debug/raw_chords.txt");

        if (!debugFile.is_open() || !compareFile.is_open() || !rawChordsFile.is_open())
        {
            std::cerr << "Error opening file!" << std::endl;
            return 0;
        }

        // =========================================================================
        // PASS 1: Extract FFT, Chroma, and Raw Chords per Frame
        // =========================================================================
        for (size_t startIdx = 0; startIdx + FFT_SIZE <= totalSamples; startIdx += HOP_SIZE)
        {
            debugFile << "\n==== STEP INFORMATION ====\n";
            debugFile << "Frame StartIdx: " << startIdx << "\n";
            debugFile << "Frame Time: "
                      << static_cast<double>(startIdx) / audio.sampleRate
                      << " seconds\n";

            std::vector<Complex> buffer(FFT_SIZE);

            for (size_t i = 0; i < FFT_SIZE; ++i)
            {
                double sample = audio.samples[startIdx + i];
                double window = 0.5 * (1.0 - std::cos(2.0 * PI * i / (FFT_SIZE - 1))); // Hann window
                buffer[i] = Complex(sample * window, 0.0);
            }

            fft(buffer);

            std::vector<double> frameChroma(12, 0.0);
            double totalMagnitude = 0.0;

            for (size_t i = 1; i < (FFT_SIZE / 2) - 1; ++i)
            {
                double freq = static_cast<double>(i) * audio.sampleRate / FFT_SIZE;
                double magnitude = std::abs(buffer[i]);

                int pitchClass = freqToPitchClass(freq, tuning);
                if (usingChromaOptimization)
                {
                    if (pitchClass >= 0 && magnitude > std::abs(buffer[i - 1]) && magnitude > std::abs(buffer[i + 1]))
                    {
                        frameChroma[pitchClass] += magnitude;
                        totalMagnitude += magnitude;
                    }
                }
                else
                {
                    if (pitchClass >= 0)
                    {
                        frameChroma[pitchClass] += magnitude;
                        totalMagnitude += magnitude;
                    }
                }
            }

            // Usually used for chroma optimization
            if (usingLogCompression || usingL2Normalization)
            {
                auto rawChroma = frameChroma;

                debugFile << "\n=== RAW CHROMA ===\n";
                for (size_t i = 0; i < rawChroma.size(); i++)
                {
                    debugFile << NOTE_NAMES[i] << ": "
                              << rawChroma[i] << "\n";

                    if (i % 12 == 11)
                    {
                        debugFile << "\n";
                    }
                }

                auto logChroma = frameChroma;
                for (auto &value : logChroma)
                {
                    value = std::log1p(10.0 * value);
                }

                if (usingLogCompression) {
                    debugFile << "\n=== LOG CHROMA ===\n";
    
                    for (size_t i = 0; i < logChroma.size(); i++)
                    {
                        debugFile << NOTE_NAMES[i] << ": "
                                  << logChroma[i] << "\n";
    
                        if (i % 12 == 11)
                        {
                            debugFile << "\n";
                        }
                    }

                    for (int p = 0; p < frameChroma.size(); ++p)
                    {
                        frameChroma[p] = std::log1p(10.0 * frameChroma[p]);
                    }
                }

                if (usingL2Normalization)
                {

                    double norm = 0.0;
                    for (int i = 0; i < frameChroma.size(); ++i)
                    {
                        norm += frameChroma[i] * frameChroma[i];
                    }
                    norm = std::sqrt(norm);
                    if (norm > 1e-6)
                    {
                        for (int i = 0; i < frameChroma.size(); ++i)
                        {
                            frameChroma[i] /= norm;
                        }
                    }
                    debugFile << "\n=== L2 NORMALIZED CHROMA ===\n";
                    for (size_t i = 0; i < frameChroma.size(); i++)
                    {
                        debugFile << NOTE_NAMES[i] << ": " << frameChroma[i] << std::endl;

                        if (i % 12 == 11)
                        {
                            debugFile << "\n";
                        }
                    }
                }
            }

            debugFile << "\n=== FRAME CHROMA (FINAL) ===\n";
            for (size_t i = 0; i < frameChroma.size(); i++)
            {
                debugFile << NOTE_NAMES[i] << ": " << frameChroma[i] << std::endl;

                if (i % 12 == 11)
                {
                    debugFile << "\n";
                }
            }

            std::string rawChord = "N/C";

            if (totalMagnitude > 0.1)
            {

                std::vector<std::pair<double, std::string>> candidateScores;

                candidateScores.reserve(chordTemplates.size());

                for (const auto &tmpl : chordTemplates)
                {
                    double score = calculateCosineSimilarity(
                        frameChroma,
                        tmpl.profile);

                    candidateScores.emplace_back(score, tmpl.name);
                }

                std::sort(
                    candidateScores.begin(),
                    candidateScores.end(),
                    [](const auto &a, const auto &b)
                    {
                        return a.first > b.first;
                    });

                if (!candidateScores.empty())
                {
                    rawChord = candidateScores.front().second;

                    debugFile << "\n=== COSINE SIMILARITY ===\n";
                    debugFile << "Score: "
                              << candidateScores.front().first << "\n";
                    debugFile << "Raw Chord: " << rawChord << "\n";

                    compareFile
                        << "\nFrame StartIdx: " << startIdx
                        << "\nFrame Time: "
                        << static_cast<double>(startIdx) / audio.sampleRate
                        << " seconds\n";

                    compareFile << "=== TOP 5 CHORD CANDIDATES ===\n";

                    for (size_t i = 0;
                         i < std::min(size_t(5), candidateScores.size());
                         ++i)
                    {

                        compareFile
                            << i + 1 << ". "
                            << candidateScores[i].second
                            << " : "
                            << std::fixed
                            << std::setprecision(6)
                            << candidateScores[i].first
                            << "\n";
                    }
                }
            }

            // Save RAW result and its time
            rawChords.push_back(rawChord);
            frameTimes.push_back(static_cast<double>(startIdx) / audio.sampleRate);
        }

        for (int i = 0; i < rawChords.size(); i++) {
            rawChordsFile << rawChords[i] << "\n";
        }

        debugFile.close();
        compareFile.close();
        rawChordsFile.close();

        // =========================================================================
        // PASS 2: Centered Smoothing & Timeline Generation
        // =========================================================================
        std::cout << "\n--- CHORD TIMELINE DETECTOR (CENTERED SMOOTHED) ---\n";
        std::cout << "Time (s)\tDetected Chord\n";
        std::cout << "--------------------------------------------------\n";

        std::vector<ChordEvent> chordEvents;
        std::string lastPrintedChord = "";
        int halfWindow = static_cast<int>(SMOOTHING_WINDOW_SIZE / 2);

        for (size_t i = 0; i < rawChords.size(); ++i)
        {
            // Ambil window simetris di sekitar frame i (melihat ke belakang DAN ke depan)
            int startWin = std::max(0, static_cast<int>(i) - halfWindow);
            int endWin = std::min(static_cast<int>(rawChords.size()) - 1, static_cast<int>(i) + halfWindow);

            std::vector<std::string> windowSlice(rawChords.begin() + startWin, rawChords.begin() + endWin + 1);
            std::string smoothedChord = getMajorityChord(windowSlice);
            double currentTime = frameTimes[i];

            // Print to terminal
            if (smoothedChord != lastPrintedChord)
            {
                std::cout << "[" << currentTime << "s]\t\t" << smoothedChord << "\n";
                lastPrintedChord = smoothedChord;

                if (!chordEvents.empty())
                {
                    chordEvents.back().endTime = currentTime;
                }

                if (smoothedChord != "N/C")
                {
                    chordEvents.push_back({currentTime, 0.0, smoothedChord});
                }
            }
        }

        // Set endTime untuk chord terakhir sesuai durasi total audio
        if (!chordEvents.empty())
        {
            chordEvents.back().endTime = static_cast<double>(totalSamples) / audio.sampleRate;
        }

        // =========================================================================
        // EXPORT TO SRT (Hanya dipanggil SEKALI di luar loop)
        // =========================================================================
        if (!outputFile.empty() && !chordEvents.empty())
        {
            if (exportToSRT(chordEvents, outputFile))
            {
                std::cout << "\n[Success] Subtitle chord exported to: " << outputFile << "\n";
            }
            else
            {
                std::cerr << "\n[Error] Failed to export subtitle to: " << outputFile << "\n";
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}