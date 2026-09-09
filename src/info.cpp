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
              << "  -swz=<int>       Smoothing window size (default: 25)\n";
}