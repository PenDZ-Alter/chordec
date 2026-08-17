#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

#include "src/parser.cpp"
#include "src/params.h"
#include "src/math.h"

void fft(std::vector<Complex>& x) {
    size_t N = x.size();
    if (N <= 1) return;

    std::vector<Complex> even(N / 2), odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = x[i * 2];
        odd[i]  = x[i * 2 + 1];
    }

    fft(even);
    fft(odd);

    for (size_t k = 0; k < N / 2; ++k) {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        x[k]       = even[k] + t;
        x[k + N/2] = even[k] - t;
    }
}

// Mapping Frequency (Hz) to Pitch Class (0 = C, 1 = C#, ..., 11 = B)
int freqToPitchClass(double freq) {
    if (freq < 20.0 || freq > 5000.0) return -1; // Filter low & high freq (noise)
    
    // Formula MIDI Note
    double pitch = 69.0 + 12.0 * std::log2(freq / 440.0);
    int note = static_cast<int>(std::round(pitch)) % 12;
    return (note < 0) ? note + 12 : note;
}

int main(int argc, char* argv[]) {
    std::string filePath = (argc > 1) ? argv[1] : std::string(SAMPLE_DIR) + "/sample.flac";
    
    std::cout << "Loading file: " << filePath << "...\n";

    try {
        // Call parser to add audio
        AudioBuffer audio = loadAudioFile(filePath);
        std::cout << "Sample Rate : " << audio.sampleRate << " Hz\n";
        std::cout << "Total Sample: " << audio.samples.size() << "\n\n";

        // Size of window FFT (Must be Power of 2)
        const size_t FFT_SIZE = 4096; 
        
        // Hop size, half of FFT Size
        const size_t HOP_SIZE = 2048; 

        // Vector to hold the total Chromagram energy of the entire song
        std::vector<double> globalChroma(12, 0.0);

        // Process audio frame by frame (Sliding Window)
        size_t totalSamples = audio.samples.size();
        size_t processedFrames = 0;

        for (size_t startIdx = 0; startIdx + FFT_SIZE <= totalSamples; startIdx += HOP_SIZE) {
            std::vector<Complex> buffer(FFT_SIZE);

            // Copy sample from AudioBuffer to input FFT & using Hanning Window
            for (size_t i = 0; i < FFT_SIZE; ++i) {
                double sample = audio.samples[startIdx + i];
                
                // Hanning Window to reduce spectral leakage
                double windowMultiplier = 0.5 * (1.0 - std::cos(2.0 * PI * i / (FFT_SIZE - 1)));
                
                buffer[i] = Complex(sample * windowMultiplier, 0.0);
            }

            // Execute FFT in this buffer
            fft(buffer);

            // Accumulation of frequency energy into Chromagram (0 to FFT_SIZE/2)
            for (size_t i = 0; i < FFT_SIZE / 2; ++i) {
                double freq = static_cast<double>(i) * audio.sampleRate / FFT_SIZE;
                double magnitude = std::abs(buffer[i]);

                int pitchClass = freqToPitchClass(freq);
                if (pitchClass >= 0) {
                    globalChroma[pitchClass] += magnitude;
                }
            }

            processedFrames++;
        }

        // Shown Chromagram result from extracted song
        const char* noteNames[] = {"C", "C#", "D", "D#/Eb", "E", "F", "F#", "G", "G#/Ab", "A", "A#/Bb", "B"};
        std::cout << "--- Accumulated Pitch Class Energy (" << processedFrames << " frames) ---\n";
        
        // Find the note with the most dominant
        auto maxIt = std::max_element(globalChroma.begin(), globalChroma.end());
        double maxEnergy = *maxIt;

        for (int i = 0; i < 12; ++i) {
            // Normalize bar (simple) (0 - 30 character '#')
            int barWidth = static_cast<int>((globalChroma[i] / maxEnergy) * 30);
            std::string bar(barWidth, '#');
            
            std::cout << noteNames[i] << (i < 3 || i == 4 || i == 6 || i == 8 || i == 10 ? "  " : " ") 
                      << "[" << bar << "]\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}