#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

#include "src/parser.cpp"

#ifndef SAMPLE_DIR
#define SAMPLE_DIR "."
#endif

const double PI = 3.14159265358979323846;
using Complex = std::complex<double>;

// 1. Cooley-Tukey Radix-2 FFT
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

// 2. Memetakan Frekuensi (Hz) ke Pitch Class (0 = C, 1 = C#, ..., 11 = B)
int freqToPitchClass(double freq) {
    if (freq < 20.0 || freq > 5000.0) return -1; // Filter frekuensi rendah & tinggi (noise)
    
    // Formula MIDI Note
    double pitch = 69.0 + 12.0 * std::log2(freq / 440.0);
    int note = static_cast<int>(std::round(pitch)) % 12;
    return (note < 0) ? note + 12 : note;
}

int main(int argc, char* argv[]) {
    std::string filePath = (argc > 1) ? argv[1] : std::string(SAMPLE_DIR) + "/sample.mp3";
    
    std::cout << "Memuat file: " << filePath << "...\n";

    try {
        // A. Panggil parser untuk ambil data audio
        AudioBuffer audio = loadAudioFile(filePath);
        std::cout << "Sample Rate : " << audio.sampleRate << " Hz\n";
        std::cout << "Total Sample: " << audio.samples.size() << "\n\n";

        // B. Tentukan Ukuran Window FFT (Must be Power of 2)
        const size_t FFT_SIZE = 4096; 
        
        // Hop size (jarak pergeseran window), misal 50% overlap biar mulus
        const size_t HOP_SIZE = 2048; 

        // Vector untuk menampung total energi Chromagram dari seluruh lagu
        std::vector<double> globalChroma(12, 0.0);

        // C. Process audio frame by frame (Sliding Window)
        size_t totalSamples = audio.samples.size();
        size_t processedFrames = 0;

        for (size_t startIdx = 0; startIdx + FFT_SIZE <= totalSamples; startIdx += HOP_SIZE) {
            std::vector<Complex> buffer(FFT_SIZE);

            // 1. Copy sample dari AudioBuffer ke input FFT & pakai Hanning Window
            for (size_t i = 0; i < FFT_SIZE; ++i) {
                double sample = audio.samples[startIdx + i];
                
                // Hanning Window untuk mengurangi spectral leakage
                double windowMultiplier = 0.5 * (1.0 - std::cos(2.0 * PI * i / (FFT_SIZE - 1)));
                
                buffer[i] = Complex(sample * windowMultiplier, 0.0);
            }

            // 2. Eksekusi FFT pada frame ini
            fft(buffer);

            // 3. Akumulasi energi frekuensi ke Chromagram (0 sampai FFT_SIZE/2)
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

        // D. Tampilkan Hasil Chromagram Total Lagu
        const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        std::cout << "--- Accumulated Pitch Class Energy (" << processedFrames << " frames) ---\n";
        
        // Cari nada dengan energi paling dominan
        auto maxIt = std::max_element(globalChroma.begin(), globalChroma.end());
        double maxEnergy = *maxIt;

        for (int i = 0; i < 12; ++i) {
            // Normalisasi bar visual sederhana (0 - 30 karakter '#')
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