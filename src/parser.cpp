#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

#include "parser.h"

AudioBuffer loadAudioFile(const std::string& filePath) 
{
    AudioBuffer audio;
    float* pSampleData = nullptr;
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    uint64_t totalFrameCount = 0;
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    AudioFormat format = AudioFormat::UNKNOWN;

    // Try decode to WAV first
    if (extension == "wav" || extension == "WAV") 
    {
        drwav_uint64 frameCount = 0;
        pSampleData = drwav_open_file_and_read_pcm_frames_f32(filePath.c_str(), &channels, &sampleRate, &frameCount, NULL);
        totalFrameCount = frameCount;
        format = AudioFormat::WAV;
    } 
    else if (extension == "flac" || extension == "FLAC") 
    {
        drflac_uint64 frameCount = 0;
        pSampleData = drflac_open_file_and_read_pcm_frames_f32(filePath.c_str(), &channels, &sampleRate, &frameCount, NULL);
        totalFrameCount = frameCount;
        format = AudioFormat::FLAC;
    } 
    else if (extension == "mp3" || extension == "MP3") 
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

    audio.sampleRate = sampleRate;
    audio.channels = channels;
    audio.samples.reserve(totalFrameCount);

    // Downmix Stereo to Mono & copy to vector<double>
    for (uint64_t i = 0; i < totalFrameCount; ++i) 
    {
        if (channels == 1) 
        {
            audio.samples.push_back(pSampleData[i]);
        } 
        else 
        {
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