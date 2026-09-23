#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

#include "headers/export.h"

// Helper for formatting seconds to SRT format (HH:MM:SS,mmm)
std::string formatSRTTime(double seconds) {
    int totalSec = static_cast<int>(seconds);
    int hours = totalSec / 3600;
    int minutes = (totalSec % 3600) / 60;
    int secs = totalSec % 60;
    int millis = static_cast<int>((seconds - totalSec) * 1000.0);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << secs << ","
        << std::setw(3) << millis;
    return oss.str();
}

// Export to SRT
bool exportToSRT(const std::vector<ChordEvent>& events, const std::string& outputPath) {
    std::ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        std::cerr << "Error: Failed to create SRT file at " << outputPath << "\n";
        return false;
    }

    int index = 1;
    for (const auto& event : events) {
        // ignore "N/C" events (No Chord / Silence)
        if (event.chordName == "N/C") continue;

        outFile << index++ << "\n";
        outFile << formatSRTTime(event.startTime) << " --> " << formatSRTTime(event.endTime) << "\n";
        outFile << event.chordName << "\n\n";
    }

    outFile.close();
    return true;
}