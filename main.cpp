#include <iostream>

#include "src/parser.cpp"
#include "src/chord.cpp"
#include "src/math.cpp"
#include "src/params.h"

int main(int argc, char* argv[]) 
{
    std::string file;
    // std::string currentFilePath = (argc > 1) ? argv[1] : std::string(SAMPLE_DIR);
    std::string currentFilePath = (argc > 1) ? argv[1] : "";

    std::cout << "Current Directory: " << currentFilePath << std::endl;
    std::cout << "Enter File name (Based on current directory): ";
    getline(std::cin, file);
    
    std::string filePath = currentFilePath + file;

    AudioBuffer audio;
    size_t FFT_SIZE;
    size_t HOP_SIZE;

    try
    {
        audio = loadAudioFile(filePath);

        std::cout << "--- Audio Loaded Successfully ---\n";
        std::cout << "Sample Rate : " << audio.sampleRate << " Hz\n";
        std::cout << "Channels    : " << audio.channels << "\n";
        std::cout << "Total Sample: " << audio.samples.size() << " frame\n";
        std::cout << "Duration    : " << static_cast<double>(audio.samples.size()) / audio.sampleRate << " seconds\n";
        std::cout << "Format      : " << (audio.format == AudioFormat::WAV ? "WAV" : (audio.format == AudioFormat::FLAC ? "FLAC" : (audio.format == AudioFormat::MP3 ? "MP3" : "UNKNOWN"))) << "\n";
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Enter FFT Size (e.g., 8192): ";
    std::cin >> FFT_SIZE;
    
    HOP_SIZE = FFT_SIZE / 4; // Overlap (~0.04s per hop in 48kHz)

    size_t SMOOTHING_WINDOW_SIZE;

    std::cout << "Enter Smoothing Window Size (e.g., 150): ";
    std::cin >> SMOOTHING_WINDOW_SIZE;

    bool usingChromaOptimization = false;
    char optimizationChoice;

    std::cout << "Use Chroma Optimization? (y/n): ";
    std::cin >> optimizationChoice;
    if (optimizationChoice == 'y' || optimizationChoice == 'Y') {
        usingChromaOptimization = true;
        std::cout << "Chroma Optimization Enabled.\n";
    } else {
        std::cout << "Chroma Optimization Disabled.\n";
    }

    try 
    {
        auto chordTemplates = generateChordTemplates();

        // const size_t FFT_SIZE = 8192;
        // const size_t HOP_SIZE = 2048; // Overlap (~0.04s per hop in 48kHz)
        
        // Size of buffer smoothing: 25 frame (~1 second window duration)
        // const size_t SMOOTHING_WINDOW_SIZE = 150;
        std::vector<std::string> chordHistory;

        size_t totalSamples = audio.samples.size();
        std::string lastPrintedChord = "";

        std::cout << "--- CHORD TIMELINE DETECTOR (SMOOTHED) ---\n";
        std::cout << "Time (s)\tDetected Chord\n";
        std::cout << "------------------------------------------\n";

        for (size_t startIdx = 0; startIdx + FFT_SIZE <= totalSamples; startIdx += HOP_SIZE) {
            std::vector<Complex> buffer(FFT_SIZE);

            for (size_t i = 0; i < FFT_SIZE; ++i) {
                double sample = audio.samples[startIdx + i];
                double window = 0.5 * (1.0 - std::cos(2.0 * PI * i / (FFT_SIZE - 1)));
                buffer[i] = Complex(sample * window, 0.0);
            }

            fft(buffer);

            // Calculate Chromagram for the current frame
            std::vector<double> frameChroma(12, 0.0);
            double totalMagnitude = 0.0;

            for (size_t i = 0; i < FFT_SIZE / 2; ++i) {
                double freq = static_cast<double>(i) * audio.sampleRate / FFT_SIZE;
                double magnitude = std::abs(buffer[i]);

                int pitchClass = freqToPitchClass(freq);
                if (usingChromaOptimization) {
                    // Apply a simple threshold to filter out low-magnitude frequencies
                    // Peak Detection
                    if (pitchClass >= 0 && magnitude > std::abs(buffer[i-1]) && magnitude > std::abs(buffer[i+1])) 
                    {
                        frameChroma[pitchClass] += magnitude;
                        totalMagnitude += magnitude;
                    }
                }
                else {
                    // If not using optimization, consider all frequencies
                    if (pitchClass >= 0) {
                        frameChroma[pitchClass] += magnitude;
                        totalMagnitude += magnitude;
                    }
                }
            }

            if (usingChromaOptimization) {
                // Logarithmic Frequency & Energy Scaling
                for (int p = 0; p < frameChroma.size(); ++p)
                {
                    frameChroma[p] = std::log1p(10.0 * frameChroma[p]);
                }

                // L2 Normalization
                double norm = 0.0;
                for (int i = 0; i < frameChroma.size(); ++i) {
                    norm += frameChroma[i] * frameChroma[i];
                }
                norm = std::sqrt(norm);
                for (int i = 0; i < frameChroma.size(); ++i) {
                    frameChroma[i] /= norm;
                }
            }

            std::string rawChord = "N/C"; // No Chord / Silence

            // if signal too low, skip chord detection for this frame
            if (totalMagnitude > 1.0) { 
                double maxScore = -1.0;
                for (const auto& tmpl : chordTemplates) {
                    double score = calculateCosineSimilarity(frameChroma, tmpl.profile);
                    if (score > maxScore) {
                        maxScore = score;
                        rawChord = tmpl.name;
                    }
                }
            }

            // --- SMOOTHING PIPELINE ---
            chordHistory.push_back(rawChord);
            if (chordHistory.size() > SMOOTHING_WINDOW_SIZE) {
                chordHistory.erase(chordHistory.begin());
            }

            // Catch the majority chord in the current smoothing window
            std::string smoothedChord = getMajorityChord(chordHistory);
            double currentTime = static_cast<double>(startIdx) / audio.sampleRate;

            // print only when the smoothed chord changes to avoid flooding the terminal
            if (smoothedChord != lastPrintedChord) {
                std::cout << "[" << currentTime << "s]\t\t" << smoothedChord << "\n";
                lastPrintedChord = smoothedChord;
            }
        }

    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}