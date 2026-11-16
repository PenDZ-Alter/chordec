#include <iostream>
#include <string_view>
#include "version.h"

#include "../includes/info.h"

void printVersion() {
    std::cout << PROJECT_NAME << " v" << PROJECT_FULL_VERSION << std::endl;
    std::cout << "Author by PenDZ (2026)\n";
}

void printHelp() {
    std::cout << "Usage: " << PROJECT_NAME << " [options]\n\n"
              << "Options:\n"
              << "  -v, --version        Show version info\n"
              << "  -h, --help           Show help message\n"
              << "  --path=<file>        Specify the path to the audio file (alt. -p=<file>)\n"
              << "  --fft-size=<int>     Number of FFT Size (default: 8192)\n"
              << "  --hop-size=<int>     Step for FFT Size (default: FFT_SIZE/4)"
              << "  --use-chroma         Using chroma optimization (default: no)\n"
              << "  -swz=<int>           Smoothing window size (default: 20)\n"
              << "  --use-weighted       Using weighted 7th chord templates (default: no)\n"
              << "  --weight=<float>     Weight for 7th interval in weighted templates (0.0 to 1.0, default: 0.6) (alt. -w=<float>)\n"
              << "  --tuning=, -t=       Change tuning from A4 Frequency Reference (default: standard tuning (440 Hz))\n"
              << "  --signature=<string> Change Signature notes for chord names (default: 'default', options: 'sharp', 'flat') (alt. --sign=<string>)\n"
              << "  --disable-7th-chords Disable 7th chords in chord generator (alt. --disable-7th)\n"
              << "  --use-log            Enable Log Compression only (alt. --use-log-compression)\n"
              << "  --use-l2             Enable L2 Normalization only (alt. --use-l2-normalization)\n";
}