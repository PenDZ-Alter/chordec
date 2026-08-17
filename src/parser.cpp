#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

#include "parser.h"

AudioBuffer loadAudioFile(const std::string& filePath) {
    AudioBuffer audio;
    float* pSampleData = nullptr;
    drwav_uint64 totalFrameCount = 0;
    unsigned int channels = 0;
    unsigned int sampleRate = 0;

    // Try decode to WAV first
    pSampleData = drwav_open_file_and_read_pcm_frames_f32(filePath.c_str(), &channels, &sampleRate, &totalFrameCount, NULL);

    // if not, try decode as FLAC
    if (!pSampleData) {
        pSampleData = drflac_open_file_and_read_pcm_frames_f32(filePath.c_str(), &channels, &sampleRate, &totalFrameCount, NULL);
    }

    // if not, try decode as MP3
    if (!pSampleData) {
        drmp3_config config;
        pSampleData = drmp3_open_file_and_read_pcm_frames_f32(filePath.c_str(), &config, &totalFrameCount, NULL);
        if (pSampleData) {
            channels = config.channels;
            sampleRate = config.sampleRate;
        }
    }

    if (!pSampleData) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    audio.sampleRate = sampleRate;
    audio.channels = channels;
    audio.samples.reserve(totalFrameCount);

    // Downmix Stereo to Mono & copy to vector<double>
    for (drwav_uint64 i = 0; i < totalFrameCount; ++i) {
        if (channels == 1) {
            audio.samples.push_back(pSampleData[i]);
        } else {
            // Average left and right channels for stereo
            float left = pSampleData[i * channels];
            float right = pSampleData[i * channels + 1];
            audio.samples.push_back(static_cast<double>(left + right) / 2.0);
        }
    }

    // Free the sample data allocated by dr_libs
    drwav_free(pSampleData, NULL); 

    return audio;
}