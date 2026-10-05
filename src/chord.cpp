#include <cmath>
#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <map>

#include "headers/chord.h"
#include "headers/params.h"

int freqToPitchClass(double freq) 
{
    if (freq < 20.0 || freq > 5000.0) return -1; // Filter low & high freq (noise)
    
    // Formula MIDI Note
    double pitch = 69.0 + 12.0 * std::log2(freq / 440.0); // Use standard tuning A4 = 440Hz
    int note = static_cast<int>(std::round(pitch)) % 12;
    return (note < 0) ? note + 12 : note;
}

ChordTemplate createTemplate(int root, const std::string& suffix, const std::vector<int>& intervals, const std::string& sign) {
    ChordTemplate tmpl;
    if (sign == "default") {
        tmpl.name = std::string(NOTE_NAMES[root]) + suffix;
        tmpl.profile = std::vector<double>(12, 0.0);
    }
    else if (sign == "sharp") {
        tmpl.name = std::string(NOTE_NAMES_SHARP[root]) + suffix;
        tmpl.profile = std::vector<double>(12, 0.0);
    }
    else if (sign == "flat") {
        tmpl.name = std::string(NOTE_NAMES_FLAT[root]) + suffix;
        tmpl.profile = std::vector<double>(12, 0.0);
    }
    else {
        throw std::invalid_argument("Invalid signature: must be 'default', 'sharp', or 'flat'");
    }
    
    for (int interval : intervals) {
        tmpl.profile[(root + interval) % 12] = 1.0;
    }
    return tmpl;
}

ChordTemplate createWeightedTemplate(int root, const std::string& suffix, const std::vector<std::pair<int, double>>& intervalWeights, const std::string& sign) {
    ChordTemplate tmpl;
    if (sign == "default") {
        tmpl.name = std::string(NOTE_NAMES[root]) + suffix;
        tmpl.profile = std::vector<double>(12, 0.0);
    }
    else if (sign == "sharp") {
        tmpl.name = std::string(NOTE_NAMES_SHARP[root]) + suffix;
        tmpl.profile = std::vector<double>(12, 0.0);
    }
    else if (sign == "flat") {
        tmpl.name = std::string(NOTE_NAMES_FLAT[root]) + suffix;
        tmpl.profile = std::vector<double>(12, 0.0);
    }
    else {
        throw std::invalid_argument("Invalid signature: must be 'default', 'sharp', or 'flat'");
    }

    double sumSq = 0.0;
    for (const auto& iw : intervalWeights) {
        int interval = iw.first;
        double weight = iw.second;
        int noteIdx = (root + interval) % 12;

        tmpl.profile[noteIdx] = weight;
        sumSq += weight * weight;
    }

    // Pre-normalize template profile (L2 norm)
    double norm = std::sqrt(sumSq);
    if (norm > 0.0) {
        for (int i = 0; i < 12; ++i) {
            tmpl.profile[i] /= norm;
        }
    }

    return tmpl;
}

std::vector<ChordTemplate> generateChordTemplates(std::string& sign) 
{
    std::vector<ChordTemplate> templates;

    for (int root = 0; root < 12; ++root) {
        /// --- TRIADS ---
        templates.push_back(createTemplate(root, "",        {0, 4, 7}, sign));
        templates.push_back(createTemplate(root, "m",       {0, 3, 7}, sign)); // Minor
        templates.push_back(createTemplate(root, "sus4",    {0, 5, 7}, sign)); // Suspended 4th
        templates.push_back(createTemplate(root, "sus2",    {0, 2, 7}, sign)); // Suspended 2nd
        templates.push_back(createTemplate(root, "dim",     {0, 3, 6}, sign)); // Diminished
        templates.push_back(createTemplate(root, "aug",     {0, 4, 8}, sign)); // Augmented

        // --- 7TH CHORDS ---
        templates.push_back(createTemplate(root, "7",       {0, 4, 7, 10}, sign)); // Dominant 7th
        templates.push_back(createTemplate(root, "maj7",    {0, 4, 7, 11}, sign)); // Major 7th
        templates.push_back(createTemplate(root, "m7",      {0, 3, 7, 10}, sign)); // Minor 7th
    }

    return templates;
}

std::vector<ChordTemplate> generateWeightedChordTemplates(double weighted7thSize, std::string& sign) {
    std::vector<ChordTemplate> templates;

    for (int root = 0; root < 12; ++root) {
        // --- TRIADS (Full weighted 1.0) ---
        // Major: Root, Maj 3rd, 5th
        templates.push_back(createWeightedTemplate(root, "", {{0, 1.0}, {4, 1.0}, {7, 1.0}}, sign));
        
        // Minor: Root, Min 3rd, 5th
        templates.push_back(createWeightedTemplate(root, "m",       {{0, 1.0}, {3, 1.0}, {7, 1.0}}, sign));
        
        // Suspended & Dim/Aug
        templates.push_back(createWeightedTemplate(root, "sus4",    {{0, 1.0}, {5, 1.0}, {7, 1.0}}, sign));
        templates.push_back(createWeightedTemplate(root, "sus2",    {{0, 1.0}, {2, 1.0}, {7, 1.0}}, sign));
        templates.push_back(createWeightedTemplate(root, "dim",     {{0, 1.0}, {3, 1.0}, {6, 1.0}}, sign));
        templates.push_back(createWeightedTemplate(root, "aug",     {{0, 1.0}, {4, 1.0}, {8, 1.0}}, sign));

        // --- 7TH CHORDS (7th chord assign by weighted7thSize) ---
        // Dominant 7th (Root, Maj 3rd, 5th, Min 7th [weighted7thSize])
        templates.push_back(createWeightedTemplate(root, "7",       {{0, 1.0}, {4, 1.0}, {7, 1.0}, {10, weighted7thSize}}, sign));
        
        // Major 7th (Root, Maj 3rd, 5th, Maj 7th [weighted7thSize])
        templates.push_back(createWeightedTemplate(root, "maj7",    {{0, 1.0}, {4, 1.0}, {7, 1.0}, {11, weighted7thSize}}, sign));
        
        // Minor 7th (Root, Min 3rd, 5th, Min 7th [weighted7thSize])
        templates.push_back(createWeightedTemplate(root, "m7",      {{0, 1.0}, {3, 1.0}, {7, 1.0}, {10, weighted7thSize}}, sign));
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