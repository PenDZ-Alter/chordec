
#ifndef PIPELINE_H
#define PIPELINE_H

#include "cli.h"
#include "parser.h"
#include "export.h"

#include "api.h"

#include <vector>

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
