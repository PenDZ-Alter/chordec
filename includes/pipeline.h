
#ifndef PIPELINE_H
#define PIPELINE_H

#include "cli.h"
#include "parser.h"
#include "export.h"

#include "api.h"

#include <vector>

/**
 * Data type structure to return Pass 1 Pipeline
 */
typedef struct PipelinePass1 {
    std::vector<std::string> rawChords;
    std::vector<double> frameTimes;
} PipelinePass1;

/**
 * Process of Pass 1 Pipeline
 * 
 * @param options CLI options arguments
 * @param audio Audio Buffer
 * 
 * @return The buffer of raw chords and frame time data
 */
CHORDEC_API PipelinePass1 runPass1(
    const CliOptions& options, 
    const AudioBuffer& audio
);

/**
 * Process of Pass 2 Pipeline
 * 
 * @param options CLI Options arguments
 * @param audio Audio Buffer
 * @param rawChords Raw Chords buffer
 * @param frameTimes Frame Time buffer
 * 
 * @return Vector of Chord Event
 */
CHORDEC_API std::vector<ChordEvent> runPass2(
    const CliOptions& options, 
    const AudioBuffer& audio, 
    std::vector<std::string> rawChords, 
    std::vector<double> frameTimes
);

/**
 * Analysis Pipeline Runner
 *
 * @param audio Audio buffer
 * @param options CLI Options
 * @return Vector of Chord Event
 */
CHORDEC_API std::vector<ChordEvent> runAnalysisPipeline(
    const AudioBuffer& audio,
    const CliOptions& options
);

#endif
