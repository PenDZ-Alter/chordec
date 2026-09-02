#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

// Define dr_libs implementations
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

/**
 * Enum to represent supported audio formats.
 * Used for determining which decoder to use when loading audio files.
 */
enum AudioFormat {
    WAV,
    FLAC,
    MP3,
    UNKNOWN
};

/**
 * Struct to hold audio data after loading from a file.
 * Contains sample rate, number of channels, and normalized samples.
 */
struct AudioBuffer {
    uint32_t sampleRate;
    uint32_t channels;
    std::vector<double> samples; // Normalized (-1.0 to 1.0)
    AudioFormat format;
};

/**
 * Loads an audio file and returns its data in a normalized format.
 * @param filePath Path to the audio file.
 * @return AudioBuffer containing the audio data.
 */
AudioBuffer loadAudioFile(const std::string& filePath);

#endif