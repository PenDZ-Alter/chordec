#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <string_view>
#include <cstdlib>

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
    size_t HOP_SIZE;
    size_t SMOOTHING_WINDOW_SIZE = 25;
    double weighted7thSize = 0.6;
    bool usingChromaOptimization = false;
    bool usingWeightedTemplates = false;
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
            usingWeightedTemplates = true;
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

    HOP_SIZE = FFT_SIZE / 4;

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
        auto chordTemplates = usingWeightedTemplates ? generateWeightedChordTemplates(weighted7thSize, signature) : generateChordTemplates(signature);
        size_t totalSamples = audio.samples.size();

        // Structure penampung data mentah untuk Pass 1
        std::vector<std::string> rawChords;
        std::vector<double> frameTimes;

        std::cout << "Processing audio frames...\n";

        // =========================================================================
        // PASS 1: Extract FFT, Chroma, and Raw Chords per Frame
        // =========================================================================
        for (size_t startIdx = 0; startIdx + FFT_SIZE <= totalSamples; startIdx += HOP_SIZE)
        {
            std::vector<Complex> buffer(FFT_SIZE);

            for (size_t i = 0; i < FFT_SIZE; ++i)
            {
                double sample = audio.samples[startIdx + i];
                double window = 0.5 * (1.0 - std::cos(2.0 * PI * i / (FFT_SIZE - 1)));
                buffer[i] = Complex(sample * window, 0.0);
            }

            fft(buffer);

            std::vector<double> frameChroma(12, 0.0);
            double totalMagnitude = 0.0;

            for (size_t i = 1; i < (FFT_SIZE / 2) - 1; ++i)
            {
                double freq = static_cast<double>(i) * audio.sampleRate / FFT_SIZE;
                double magnitude = std::abs(buffer[i]);

                int pitchClass = freqToPitchClass(freq);
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

            if (usingChromaOptimization)
            {
                for (int p = 0; p < frameChroma.size(); ++p)
                {
                    frameChroma[p] = std::log1p(10.0 * frameChroma[p]);
                }

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
            }

            std::string rawChord = "N/C";

            if (totalMagnitude > 0.1)
            {
                double maxScore = -1.0;
                for (const auto &tmpl : chordTemplates)
                {
                    double score = calculateCosineSimilarity(frameChroma, tmpl.profile);
                    if (score > maxScore)
                    {
                        maxScore = score;
                        rawChord = tmpl.name;
                    }
                }
            }

            // Simpan hasil mentah & waktu frame-nya
            rawChords.push_back(rawChord);
            frameTimes.push_back(static_cast<double>(startIdx) / audio.sampleRate);
        }

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

            // Cetak ke terminal & catat event jika terjadi perubahan chord
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