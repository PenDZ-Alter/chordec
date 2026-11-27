#include <filesystem>
#include <fstream>

#include "pipeline.h"
#include "chord.h"
#include "params.h"
#include "fft.h"

PipelinePass1 runPass1(
    const CliOptions& options, 
    const AudioBuffer& audio)
{
    PipelinePass1 rawData;

    // Debug file
    std::ofstream debugFile;
    std::ofstream compareFile;
    std::ofstream rawChordsFile;

    if (options.usingDebugMode)
    {
        std::filesystem::create_directories(options.debugDir);

        debugFile.open(options.debugDir / "chroma.txt");
        compareFile.open(options.debugDir / "candidates.txt");
        rawChordsFile.open(options.debugDir / "raw_chords.txt");

        if (!debugFile.is_open() ||
            !compareFile.is_open() ||
            !rawChordsFile.is_open())
        {
            throw std::runtime_error("Error opening debug files!\n");
        }

        std::cout << "[Debug] Output directory: " << options.debugDir << "\n";
    }

    auto chordTemplates = options.usingWeightedTemplates ? generateWeightedChordTemplates(options.weighted7thSize, options.signature, options.using7thChords) : generateChordTemplates(options.signature, options.using7thChords);
    size_t totalSamples = audio.samples.size();

    for (size_t startIdx = 0; startIdx + options.fftSize <= totalSamples; startIdx += options.hopSize)
    {
        if (options.usingDebugMode)
        {
            debugFile << "\n==== STEP INFORMATION ====\n";
            debugFile << "Frame StartIdx: " << startIdx << "\n";
            debugFile << "Frame Time: "
                      << static_cast<double>(startIdx) / audio.sampleRate
                      << " seconds\n";
        }

        std::vector<Complex> buffer(options.fftSize);

        for (size_t i = 0; i < options.fftSize; ++i)
        {
            double sample = audio.samples[startIdx + i];
            double window = 0.5 * (1.0 - std::cos(2.0 * PI * i / (options.fftSize - 1))); // Hann window
            buffer[i] = Complex(sample * window, 0.0);
        }

        fft(buffer);

        std::vector<double> frameChroma(12, 0.0);
        double totalMagnitude = 0.0;

        for (size_t i = 1; i < (options.fftSize / 2) - 1; ++i)
        {
            double freq = static_cast<double>(i) * audio.sampleRate / options.fftSize;
            double magnitude = std::abs(buffer[i]);

            int pitchClass = freqToPitchClass(freq, options.tuning);
            if (options.usingChromaOptimization)
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
        if (options.usingLogCompression || options.usingL2Normalization)
        {
            auto rawChroma = frameChroma;

            if (options.usingDebugMode)
            {
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
            }

            auto logChroma = frameChroma;
            for (auto &value : logChroma)
            {
                value = std::log1p(10.0 * value);
            }

            if (options.usingLogCompression)
            {
                if (options.usingDebugMode)
                {
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
                }
                for (int p = 0; p < frameChroma.size(); ++p)
                {
                    frameChroma[p] = std::log1p(10.0 * frameChroma[p]);
                }
            }

            if (options.usingL2Normalization)
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
                if (options.usingDebugMode)
                {
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
        }

        if (options.usingDebugMode)
        {
            debugFile << "\n=== FRAME CHROMA (FINAL) ===\n";
            for (size_t i = 0; i < frameChroma.size(); i++)
            {
                debugFile << NOTE_NAMES[i] << ": " << frameChroma[i] << std::endl;

                if (i % 12 == 11)
                {
                    debugFile << "\n";
                }
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

                if (options.usingDebugMode)
                {
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
        }

        // Save RAW result and its time
        rawData.rawChords.push_back(rawChord);
        rawData.frameTimes.push_back(static_cast<double>(startIdx) / audio.sampleRate);
    }

    if (options.usingDebugMode) 
    {
        rawChordsFile << "=== RAW CHORDS ===\n";

        for (int i = 0; i < rawData.rawChords.size(); i++)
        {
            rawChordsFile << rawData.rawChords[i] << std::endl;
        }
    }

    return rawData;
}

std::vector<ChordEvent> runPass2(
    const CliOptions& options, 
    const AudioBuffer& audio, 
    std::vector<std::string> rawChords, 
    std::vector<double> frameTimes)
{
    std::vector<ChordEvent> chordEvents;
    std::string lastPrintedChord = "";
    int halfWindow = static_cast<int>(options.smoothingWindowSize / 2);
    size_t totalSamples = audio.samples.size();

    for (size_t i = 0; i < rawChords.size(); ++i)
    {
        // Take a symmetrical window around frame i (looking back AND forward)
        int startWin = std::max(0, static_cast<int>(i) - halfWindow);
        int endWin = std::min(static_cast<int>(rawChords.size()) - 1, static_cast<int>(i) + halfWindow);

        std::vector<std::string> windowSlice(rawChords.begin() + startWin, rawChords.begin() + endWin + 1);
        std::string smoothedChord = getMajorityChord(windowSlice);
        double currentTime = frameTimes[i];

        // Print to terminal
        if (smoothedChord != lastPrintedChord)
        {
            if (options.usingVerboseMode)
            {
                std::cout << "[" << currentTime << "s]\t\t" << smoothedChord << "\n";
            }
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

    // Set endTime for last chords
    if (!chordEvents.empty())
    {
        chordEvents.back().endTime = static_cast<double>(totalSamples) / audio.sampleRate;
    }

    return chordEvents;
}

std::vector<ChordEvent> runAnalysisPipeline(
    const AudioBuffer &audio,
    const CliOptions &options)
{
    // Initialize rawChords and frameTimes
    std::vector<std::string> rawChords;
    std::vector<double> frameTimes;

    std::cout << "Processing audio frames...\n";

    // PASS 1
    // =========================================================================
    // PASS 1: Extract FFT, Chroma, and Raw Chords per Frame
    // =========================================================================
    auto rawData = runPass1(options, audio);
    rawChords = rawData.rawChords;
    frameTimes = rawData.frameTimes;

    // PASS 2
    // =========================================================================
    // PASS 2: Centered Smoothing & Timeline Generation
    // =========================================================================
    if (options.usingVerboseMode)
    {
        std::cout << "\n--- CHORD TIMELINE DETECTOR (CENTERED SMOOTHED) ---\n";
        std::cout << "Time (s)\tDetected Chord\n";
        std::cout << "--------------------------------------------------\n";
    }
    
    auto chordEvents = runPass2(options, audio, rawChords, frameTimes);

    // Return chordEvents
    return chordEvents;
}