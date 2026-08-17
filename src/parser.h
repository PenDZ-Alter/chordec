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
 * Struct to hold audio data after loading from a file.
 * Contains sample rate, number of channels, and normalized samples.
 */
struct AudioBuffer {
    uint32_t sampleRate;
    uint32_t channels;
    std::vector<double> samples; // Normalized (-1.0 to 1.0)
};