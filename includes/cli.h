
#ifndef CLI_H
#define CLI_H

#include <cstddef>
#include <filesystem>
#include <string>

#include "api.h"

/**
 * CLI Options
 * 
 * This is the options of application
 */
struct CliOptions
{
    // File Options
    std::string file;
    std::string outputFile;
    std::filesystem::path debugDir = "./debug";

    size_t fftSize = 8192;
    size_t hopSize = 2048;
    size_t smoothingWindowSize = 20;

    double tuning = 440.0;
    double weighted7thSize = 0.6;

    bool usingChromaOptimization = false;
    bool usingWeightedTemplates = false;
    bool using7thChords = true;
    bool usingLogCompression = false;
    bool usingL2Normalization = false;
    bool usingDebugMode = false;
    bool usingVerboseMode = false;

    std::string signature = "default";
};

/**
 * Status after parsing the Argument of CLI
 */
enum class CliResult
{
    Run,
    ExitSuccess,
    ExitFailure
};

/**
 * Parsing the Arguments
 * 
 * @param argc Argument Count
 * @param argv Argument Character
 * @param options CLI Options
 * @return Structure of CliResult
 */
CHORDEC_API CliResult parseArguments(
    int argc,
    char* argv[],
    CliOptions& options
);

#endif
