#ifndef PARSER_H
#define PARSER_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// Define dr_libs
#include "dr_wav.h"
#include "dr_flac.h"
#include "dr_mp3.h"

#include "api.h"

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
struct CHORDEC_API AudioBuffer {
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
CHORDEC_API AudioBuffer loadAudioFile(const std::string& filePath);

#endif