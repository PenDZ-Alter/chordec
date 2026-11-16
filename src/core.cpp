
#include "core.h"

#include "cli.h"
#include "pipeline.h"
#include "parser.h"
#include "info.h"
#include "export.h"

#include <iostream>
#include <exception>

int runApplication(int argc, char *argv[])
{
    CliOptions options;

    // 1. Parse CLI
    CliResult cliResult = parseArguments(
        argc,
        argv,
        options);

    if (cliResult == CliResult::ExitSuccess)
        return 0;

    if (cliResult == CliResult::ExitFailure)
        return 1;
    
    try
    {
        // 2. Load audio
        AudioBuffer audio = loadAudioFile(options.file);
        std::cout << "Getting audio buffer ...\n";

        // 3. Print audio information
        if (options.usingVerboseMode)
        {
            std::cout << "------- Audio Information -------\n";
            std::cout << "Sample Rate : " << audio.sampleRate << " Hz\n";
            std::cout << "Channels    : " << audio.channels << "\n";
            std::cout << "Total Sample: " << audio.samples.size() << " frame\n";
            std::cout << "Duration    : " << static_cast<double>(audio.samples.size()) / audio.sampleRate << " seconds\n";
            std::cout << "Format      : " << (audio.format == AudioFormat::WAV ? "WAV" : (audio.format == AudioFormat::FLAC ? "FLAC" : (audio.format == AudioFormat::MP3 ? "MP3" : "UNKNOWN"))) << "\n";
            std::cout << "FFT Size    : " << options.fftSize << "\n";
            std::cout << "HOP Size    : " << options.hopSize << "\n";
            std::cout << "Tuning      : " << options.tuning << "\n";
            std::cout << "Signature   : " << options.signature << "\n";
            std::cout << "Smoothing Window Size: " << options.smoothingWindowSize << "\n";
            std::cout << "Using Chroma Optimization: " << (options.usingChromaOptimization ? "Yes" : "No") << "\n";
            std::cout << "Using Weighted 7th Chord Templates: " << (options.usingWeightedTemplates ? "Yes" : "No") << "\n";
            std::cout << "---------------------------------\n";
        }

        // 4. Run analysis pipeline
        auto chordEvents = runAnalysisPipeline(
            audio,
            options);

        // 5. Export results
        if (!options.outputFile.empty() &&
            !chordEvents.empty())
        {
            if (exportToSRT(
                    chordEvents,
                    options.outputFile))
            {
                std::cout
                    << "\n[Success] Subtitle chord exported to: "
                    << options.outputFile << "\n";
            }
            else
            {
                std::cerr
                    << "\n[Error] Failed to export subtitle to: "
                    << options.outputFile << "\n";
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
