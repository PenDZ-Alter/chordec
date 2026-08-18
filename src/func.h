#ifndef FUNC_H
#define FUNC_H

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

/**
 * Mapping Frequency (Hz) to Pitch Class (0 = C, 1 = C#, ..., 11 = B)
 * @param freq Frequency in Hz
 * @return Pitch Class (0-11) or -1 if frequency is out of range
 */
int freqToPitchClass(double freq) 
{
    if (freq < 20.0 || freq > 5000.0) return -1; // Filter low & high freq (noise)
    
    // Formula MIDI Note
    double pitch = 69.0 + 12.0 * std::log2(freq / 440.0);
    int note = static_cast<int>(std::round(pitch)) % 12;
    return (note < 0) ? note + 12 : note;
}

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

#endif