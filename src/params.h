#ifndef PARAMS_H_
#define PARAMS_H_

#include <complex>
#include <array>
#include <string_view>

#ifndef SAMPLE_DIR
#define SAMPLE_DIR "."
#endif

// Gunakan constexpr agar hemat memori & aman di-include di mana saja
constexpr double PI = 3.14159265358979323846;

using Complex = std::complex<double>;

// Gunakan inline constexpr untuk array global di C++17
inline constexpr std::array<std::string_view, 12> NOTE_NAMES = {
    "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"
};

#endif