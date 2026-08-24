#include <iostream>

#include "../src/parser.cpp"
#include "../src/chord.cpp"
#include "../src/math.cpp"
#include "../src/params.h"

int main(int argc, char* argv[]) {
    std::string filePath = (argc > 1) ? argv[1] : std::string(SAMPLE_DIR) + "/sample.flac";

    try {
        AudioBuffer audio = loadAudioFile(filePath);
        auto chordTemplates = generateChordTemplates();

        const size_t FFT_SIZE = 8192; // Larger window for sharper low frequency resolution
        const size_t HOP_SIZE = 4096; // Shift window

        size_t totalSamples = audio.samples.size();
        std::string lastDetectedChord = "";
        double lastTime = 0.0;

        std::cout << "--- CHORD TIMELINE DETECTION ---\n";
        std::cout << "Time (s)\tDetected Chord\n";
        std::cout << "---------------------------------\n";

        for (size_t startIdx = 0; startIdx + FFT_SIZE <= totalSamples; startIdx += HOP_SIZE) {
            std::vector<Complex> buffer(FFT_SIZE);

            // Copy sample & Hanning Window
            for (size_t i = 0; i < FFT_SIZE; ++i) {
                double sample = audio.samples[startIdx + i];
                double window = 0.5 * (1.0 - std::cos(2.0 * PI * i / (FFT_SIZE - 1)));
                buffer[i] = Complex(sample * window, 0.0);
            }

            fft(buffer);

            // Calculate Chromagram for "this" frame
            std::vector<double> frameChroma(12, 0.0);
            for (size_t i = 0; i < FFT_SIZE / 2; ++i) {
                double freq = static_cast<double>(i) * audio.sampleRate / FFT_SIZE;
                double magnitude = std::abs(buffer[i]);

                int pitchClass = freqToPitchClass(freq);
                if (pitchClass >= 0) {
                    frameChroma[pitchClass] += magnitude;
                }
            }

            // Matching with 24 Templates
            std::string bestChord = "Unknown";
            double maxScore = -1.0;

            for (const auto& tmpl : chordTemplates) {
                double score = calculateCosineSimilarity(frameChroma, tmpl.profile);
                if (score > maxScore) {
                    maxScore = score;
                    bestChord = tmpl.name;
                }
            }

            // Calculate timestamp (seconds)
            double currentTime = static_cast<double>(startIdx) / audio.sampleRate;

            // Print ONLY when chord changes (to avoid flooding the terminal with thousands of lines)
            if (bestChord != lastDetectedChord) {
                std::cout << "[" << currentTime << "s]\t\t" << bestChord << "\n";
                lastDetectedChord = bestChord;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}