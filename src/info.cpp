#include <iostream>
#include <string_view>
#include "version.h"

#include "headers/info.h"

void printVersion() {
    std::cout << PROJECT_NAME << " v" << PROJECT_VERSION << std::endl;
}

void printHelp() {
    std::cout << "Usage: " << PROJECT_NAME << " [options]\n\n"
              << "Options:\n"
              << "  -v, --version    Show version info\n"
              << "  -h, --help       Show help message\n"
              << "  --path=<file>    Specify the path to the audio file (-p=<file> for short)\n"
              << "  --fft-size=<int> Number of FFT Size (default: 8192)\n"
              << "  --use-chroma     Using chroma optimization (default: no)\n"
              << "  -swz=<int>       Smoothing window size (default: 25)\n"
              << "  --use-weighted   Using weighted 7th chord templates (default: no)\n"
              << "  --weight=<float> Weight for 7th interval in weighted templates (0.0 to 1.0, default: 0.6)\n"
              << "  -w=<float>       Short form of --weight\n";
}