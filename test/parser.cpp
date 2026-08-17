#include <iostream>
#include <string>
#include "../src/parser.cpp"

#ifndef SAMPLE_DIR
#define SAMPLE_DIR "."
#endif

int main() {
    try {
        std::string sampleFilePath = std::string(SAMPLE_DIR) + "/sample.flac";
        AudioBuffer audio = loadAudioFile(sampleFilePath);
        
        std::cout << "--- Audio Loaded Successfully ---\n";
        std::cout << "[PASS] Sample Rate : " << audio.sampleRate << " Hz\n";
        std::cout << "[PASS] Channels    : " << audio.channels << "\n";
        std::cout << "[PASS] Total Sample: " << audio.samples.size() << " frame\n";
        std::cout << "[PASS] Duration    : " << static_cast<double>(audio.samples.size()) / audio.sampleRate << " seconds\n";

    } catch (const std::exception& e) {
        std::cerr << "[FAIL] Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}