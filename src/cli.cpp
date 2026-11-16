#include <iostream>

#include "cli.h"
#include "info.h"

CliResult parseArguments(
    int argc,
    char* argv[],
    CliOptions& options
) {
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
            return CliResult::ExitSuccess;
        }
        else if (arg == "-h" || arg == "--help")
        {
            printHelp();
            return CliResult::ExitSuccess;
        }
        else if (arg.starts_with("--path=") || arg.starts_with("-p="))
        {
            size_t len = arg.starts_with("--path=") ? 7 : 3;
            options.file = getValue(arg, len);
        }
        else if (arg.starts_with("--fft-size="))
        {
            size_t fft_size = std::stoul(getValue(arg, 11));
            if (fft_size <= 0)
            {
                std::cerr << "FFT Size must bigger than 0! It's really recommended to use 2^n result\n";
                return CliResult::ExitFailure;
                
            }
            options.fftSize = fft_size;
        }
        else if (arg == "--use-chroma")
        {
            options.usingChromaOptimization = true;
        }
        else if (arg == "--debug")
        {
            options.usingDebugMode = true;
        }
        else if (arg.starts_with("--debug-dir="))
        {
            if (!options.usingDebugMode)
            {
                printHelp();
                std::cerr << "Debug dir, should be in debug mode!\n";
                return CliResult::ExitFailure;
            }
            options.debugDir = getValue(arg, 12);
        }
        else if (arg == "--verbose")
        {
            options.usingVerboseMode = true;
        }
        else if (arg == "--disable-7th-chords" || arg == "--disable-7th")
        {
            options.using7thChords = false;
        }
        else if (arg == "--use-weighted")
        {
            if (!options.using7thChords)
            {
                std::cerr << "For using weighted 7th chords, you need to enable 7th chords!\n";
                printHelp();
                return CliResult::ExitFailure;
            }
            options.usingWeightedTemplates = true;
        }
        else if (arg == "--use-log-compression" || arg == "--use-log")
        {
            options.usingLogCompression = true;
        }
        else if (arg == "--use-l2-normalization" || arg == "--use-l2")
        {
            options.usingL2Normalization = true;
        }
        else if (arg.starts_with("--hop-size="))
        {
            size_t hop_size = std::stoul(getValue(arg, 11));
            if (options.hopSize <= 0)
            {
                std::cerr << "HOP Size must bigger than 0! It's really recommended to use 2^n result\n";
                return CliResult::ExitFailure;
            }
            options.hopSize = hop_size;
        }
        else if (arg.starts_with("--weight=") || arg.starts_with("-w="))
        {
            size_t len = arg.starts_with("--weight=") ? 9 : 3;
            double weightValue = std::stod(getValue(arg, len));
            if (weightValue < 0.0 || weightValue > 1.0)
            {
                std::cerr << "Error: Weight value must be between 0.0 and 1.0\n";
                return CliResult::ExitFailure;
            } else if (!options.usingWeightedTemplates || !options.using7thChords)
            {
                std::cerr << "Error: Weighted 7th and 7th chords must be enabled!\n";
                return CliResult::ExitFailure;
            }
            options.weighted7thSize = weightValue;
        }
        else if (arg.starts_with("--tuning=") || arg.starts_with("-t="))
        {
            size_t len = arg.starts_with("--tuning=") ? 9 : 3;
            double tuningValue = std::stod(getValue(arg, len));
            options.tuning = tuningValue;
        }
        else if (arg.starts_with("-swz="))
        {
            options.smoothingWindowSize = std::stoul(getValue(arg, 5));
        }
        else if (arg.starts_with("--output=") || arg.starts_with("-o="))
        {
            size_t len = arg.starts_with("--output=") ? 9 : 3;
            options.outputFile = getValue(arg, len);
        }
        else if (arg.starts_with("--signature=") || arg.starts_with("-sign="))
        {
            size_t len = arg.starts_with("--signature=") ? 12 : 6;
            std::string_view sign = arg.substr(len);
            if (sign != "default" && sign != "sharp" && sign != "flat")
            {
                std::cerr << "Unknown signature: Only supported 'sharp', 'flat', and 'default'\n";
                printHelp();
                return CliResult::ExitFailure;
            }
            options.signature = std::string(sign);
        }
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            printHelp();
            return CliResult::ExitFailure;
        }
    }
    return CliResult::Run;
}