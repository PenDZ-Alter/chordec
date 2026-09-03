#include <cstdint>
#include <cctype>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#include "parser.h"

AudioBuffer loadAudioFile(const std::string& filePath) 
{
    AudioBuffer audio;
    float* pSampleData = nullptr;
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    uint64_t totalFrameCount = 0;

    // Get extension & convert to lowercase
    size_t dotPos = filePath.find_last_of(".");
    if (dotPos == std::string::npos) {
        throw std::runtime_error("File has no extension: " + filePath);
    }
    
    std::string extension = filePath.substr(dotPos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), 
                   [](unsigned char c){ return std::tolower(c); });

    AudioFormat format = AudioFormat::UNKNOWN;

    // Decoding based on extension
    if (extension == "wav") 
    {
        drwav_uint64 frameCount = 0;
        pSampleData = drwav_open_file_and_read_pcm_frames_f32(filePath.c_str(), &channels, &sampleRate, &frameCount, NULL);
        totalFrameCount = frameCount;
        format = AudioFormat::WAV;
    } 
    else if (extension == "flac") 
    {
        drflac_uint64 frameCount = 0;
        pSampleData = drflac_open_file_and_read_pcm_frames_f32(filePath.c_str(), &channels, &sampleRate, &frameCount, NULL);
        totalFrameCount = frameCount;
        format = AudioFormat::FLAC;
    } 
    else if (extension == "mp3") 
    {
        drmp3_config config;
        drmp3_uint64 frameCount = 0;
        pSampleData = drmp3_open_file_and_read_pcm_frames_f32(filePath.c_str(), &config, &frameCount, NULL);
        if (pSampleData) {
            channels = config.channels;
            sampleRate = config.sampleRate;
            totalFrameCount = frameCount;
            format = AudioFormat::MP3;
        }
    } 
    else 
    {
        throw std::runtime_error("Unsupported file format: " + extension);
    }

    if (!pSampleData) {
        throw std::runtime_error("Failed to load or decode audio file: " + filePath);
    }

    audio.sampleRate = sampleRate;
    audio.channels = channels;
    audio.format = format;
    audio.samples.reserve(totalFrameCount);

    // Downmix Stereo/Multi-channel to Mono & copy to vector<double>
    for (uint64_t i = 0; i < totalFrameCount; ++i) 
    {
        if (channels == 1) 
        {
            audio.samples.push_back(pSampleData[i]);
        } 
        else 
        {
            // Average all channels to create a mono signal
            float left = pSampleData[i * channels];
            float right = pSampleData[i * channels + 1];
            audio.samples.push_back(static_cast<double>(left + right) / 2.0);
        }
    }

    // Free memory based on format
    if (format == AudioFormat::WAV) {
        drwav_free(pSampleData, NULL);
    } else if (format == AudioFormat::FLAC) {
        drflac_free(pSampleData, NULL);
    } else if (format == AudioFormat::MP3) {
        drmp3_free(pSampleData, NULL);
    }

    return audio;
}