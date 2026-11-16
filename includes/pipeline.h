
#ifndef PIPELINE_H
#define PIPELINE_H

#include "cli.h"
#include "parser.h"
#include "export.h"

#include "api.h"

#include <vector>

CHORDEC_API std::vector<ChordEvent> runAnalysisPipeline(
    const AudioBuffer& audio,
    const CliOptions& options
);

#endif
