#ifndef CHORD_H_
#define CHORD_H_

#include <string>
#include <vector>
#include "libs/api.h"

/**
 * Struct to represent a chord template
 * Contains the chord name and its chroma profile (12 elements)
 * @var name Name of the chord (e.g., "C Major")
 * @var profile Chroma profile vector (size 12) representing the chord
 */
struct CHORDEC_API ChordTemplate {
    std::string name;
    std::vector<double> profile; // Vector 12 elemen
};

/**
 * Create a chord template given a root note, suffix, and intervals
 * This is a utility function to generate chord templates based on musical theory.
 * @param root Root note (0 = C, 1 = C#, ..., 11 = B)
 * @param suffix Chord suffix (e.g., "Major", "Minor")
 * @param intervals Vector of intervals (in semitones) from the root note
 * @return ChordTemplate struct representing the chord
 */
ChordTemplate createTemplate(int root, const std::string& suffix, const std::vector<int>& intervals);

/**
 * Generate a list of chord templates
 * @return Vector of ChordTemplate structs
 */
CHORDEC_API std::vector<ChordTemplate> generateChordTemplates();

/**
 * Get the most frequently occurring chord in a history vector
 * @param history Vector of detected chords
 * @return The chord that appears most frequently
 */
CHORDEC_API std::string getMajorityChord(const std::vector<std::string>& history);

/**
 * Mapping Frequency (Hz) to Pitch Class (0 = C, 1 = C#, ..., 11 = B)
 * @param freq Frequency in Hz
 * @return Pitch Class (0-11) or -1 if frequency is out of range
 */
CHORDEC_API int freqToPitchClass(double freq);

/**
 * Create a weighted chord template given a root note, suffix, and interval weights
 * This function allows for more nuanced chord templates by assigning weights to each interval.
 * Also, this is normalized 7th and major 7th chord, so it's not always detect as 7th / maj 7th chord
 * @param root Root note (0 = C, 1 = C#, ..., 11 = B)
 * @param suffix Chord suffix (e.g., "Major", "Minor")
 * @param intervalWeights Vector of pairs (interval in semitones, weight)
 * @return ChordTemplate struct representing the weighted chord
 */
CHORDEC_API ChordTemplate createWeightedTemplate(int root, const std::string& suffix, const std::vector<std::pair<int, double>>& intervalWeights);

/**
 * Generate a list of weighted chord templates
 * @param weighted7thSize The weight to assign to the 7th interval
 * @return Vector of ChordTemplate structs with weighted profiles
 */
CHORDEC_API std::vector<ChordTemplate> generateWeightedChordTemplates(double weighted7thSize);

#endif