#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

const double PI = 3.14159265358979323846;
using Complex = std::complex<double>;

// 1. Cooley-Tukey Radix-2 FFT (Ukuran N harus perpangkatan 2)
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
    if (freq < 20.0) return -1; // Ignore low noise
    // MIDI Note number formula
    double pitch = 69.0 + 12.0 * std::log2(freq / 440.0);
    int note = static_cast<int>(std::round(pitch)) % 12;
    return (note < 0) ? note + 12 : note;
}

int main() {
    // Contoh sederhana: 4096 sample buffer pada Sample Rate 44100 Hz
    size_t N = 4096;
    double sampleRate = 44100.0;
    std::vector<Complex> buffer(N, 0.0);

    // TODO: Isi 'buffer' dari pembacaan file WAV kamu di sini

    // Jalankan FFT
    fft(buffer);

    // Hitung Chromagram 12 Nada
    std::vector<double> chroma(12, 0.0);
    for (size_t i = 0; i < N / 2; ++i) {
        double freq = i * sampleRate / N;
        double magnitude = std::abs(buffer[i]);
        
        int pitchClass = freqToPitchClass(freq);
        if (pitchClass >= 0) {
            chroma[pitchClass] += magnitude;
        }
    }

    // Tampilkan energi per pitch class
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::cout << "--- Pitch Class Energy ---\n";
    for (int i = 0; i < 12; ++i) {
        std::cout << noteNames[i] << ": " << chroma[i] << "\n";
    }

    return 0;
}