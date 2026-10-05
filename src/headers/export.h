#ifndef EXPORT_H
#define EXPORT_H

#include <iostream>
#include <string>

#include "libs/api.h"

/**
 * Struct to represent a chord event with start time, end time, and chord name
 */
struct CHORDEC_API ChordEvent {
    double startTime;
    double endTime;
    std::string chordName;
};

/**
 * Helper for formatting seconds to SRT format (HH:MM:SS,mmm)
 * @param seconds Time in seconds
 * @return Formatted time string in SRT format
 */
CHORDEC_API std::string formatSRTTime(double seconds);

/**
 * Export a list of chord events to an SRT file
 * @param events Vector of ChordEvent to export
 * @param outputPath Path to the output SRT file
 * @return True if export was successful, false otherwise
 */
CHORDEC_API bool exportToSRT(const std::vector<ChordEvent>& events, const std::string& outputPath);

#endif // EXPORT_H