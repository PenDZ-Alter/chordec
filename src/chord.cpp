#include <cmath>
#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <map>

#include "chord.h"
#include "params.h"

int freqToPitchClass(double freq) 
{
    if (freq < 20.0 || freq > 5000.0) return -1; // Filter low & high freq (noise)
    
    // Formula MIDI Note
    double pitch = 69.0 + 12.0 * std::log2(freq / 440.0); // Use standard tuning A4 = 440Hz
    int note = static_cast<int>(std::round(pitch)) % 12;
    return (note < 0) ? note + 12 : note;
}

ChordTemplate createTemplate(int root, const std::string& suffix, const std::vector<int>& intervals) {
    ChordTemplate tmpl;
    tmpl.name = std::string(NOTE_NAMES[root]) + suffix;
    tmpl.profile = std::vector<double>(12, 0.0);
    
    for (int interval : intervals) {
        tmpl.profile[(root + interval) % 12] = 1.0;
    }
    return tmpl;
}

std::vector<ChordTemplate> generateChordTemplates() 
{
    std::vector<ChordTemplate> templates;

    for (int root = 0; root < 12; ++root) {
        /// --- TRIADS ---
        templates.push_back(createTemplate(root, "",        {0, 4, 7}));
        templates.push_back(createTemplate(root, "m",       {0, 3, 7})); // Minor
        templates.push_back(createTemplate(root, "sus4",    {0, 5, 7})); // Suspended 4th
        templates.push_back(createTemplate(root, "sus2",    {0, 2, 7})); // Suspended 2nd
        templates.push_back(createTemplate(root, "dim",     {0, 3, 6})); // Diminished
        templates.push_back(createTemplate(root, "aug",     {0, 4, 8})); // Augmented

        // --- 7TH CHORDS ---
        templates.push_back(createTemplate(root, "7",       {0, 4, 7, 10})); // Dominant 7th
        templates.push_back(createTemplate(root, "maj7",    {0, 4, 7, 11})); // Major 7th
        templates.push_back(createTemplate(root, "m7",      {0, 3, 7, 10})); // Minor 7th
    }

    return templates;
}

std::string getMajorityChord(const std::vector<std::string>& history) 
{
    std::map<std::string, int> counts;
    for (const auto& chord : history) {
        counts[chord]++;
    }
    
    std::string mostFrequent = history.back();
    int maxCount = 0;
    for (const auto& pair : counts) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            mostFrequent = pair.first;
        }
    }
    return mostFrequent;
}