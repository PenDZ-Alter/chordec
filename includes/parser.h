
#ifndef PARSER_H
#define PARSER_H

#include <cstdint>
#include <string>
#include <vector>

#include "api.h"

/**
 * Supported audio formats.
 */
enum AudioFormat {
    WAV,
    FLAC,
    MP3,
    UNKNOWN
};

/**
 * Audio data after decoding.
 */
struct CHORDEC_API AudioBuffer {
    uint32_t sampleRate;
    uint32_t channels;
    std::vector<double> samples;
    AudioFormat format;
};

/**
 * Load and decode an audio file.
 */
CHORDEC_API AudioBuffer loadAudioFile(
    const std::string& filePath
);

#endif
