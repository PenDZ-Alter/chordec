#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
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

std::vector<ChordTemplate> generateChordTemplates() 
{
    std::vector<ChordTemplate> templates;

    for (int root = 0; root < 12; ++root) {
        // Major Template (Root, +4, +7)
        ChordTemplate maj;
        maj.name = std::string(NOTE_NAMES[root]) + " Major";
        maj.profile = std::vector<double>(12, 0.0);
        maj.profile[root] = 1.0;
        maj.profile[(root + 4) % 12] = 1.0;
        maj.profile[(root + 7) % 12] = 1.0;
        templates.push_back(maj);

        // Minor Template (Root, +3, +7)
        ChordTemplate min;
        min.name = std::string(NOTE_NAMES[root]) + " Minor";
        min.profile = std::vector<double>(12, 0.0);
        min.profile[root] = 1.0;
        min.profile[(root + 3) % 12] = 1.0;
        min.profile[(root + 7) % 12] = 1.0;
        templates.push_back(min);
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