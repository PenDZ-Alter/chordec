#ifndef CHORD_H_
#define CHORD_H_

#include <string>
#include <vector>

/**
 * Struct to represent a chord template
 * Contains the chord name and its chroma profile (12 elements)
 * @var name Name of the chord (e.g., "C Major")
 * @var profile Chroma profile vector (size 12) representing the chord
 */
struct ChordTemplate {
    std::string name;
    std::vector<double> profile; // Vector 12 elemen
};

/**
 * Generate a list of chord templates
 * @return Vector of ChordTemplate structs
 */
std::vector<ChordTemplate> generateChordTemplates();

/**
 * Get the most frequently occurring chord in a history vector
 * @param history Vector of detected chords
 * @return The chord that appears most frequently
 */
std::string getMajorityChord(const std::vector<std::string>& history);

/**
 * Mapping Frequency (Hz) to Pitch Class (0 = C, 1 = C#, ..., 11 = B)
 * @param freq Frequency in Hz
 * @return Pitch Class (0-11) or -1 if frequency is out of range
 */
int freqToPitchClass(double freq);

#endif