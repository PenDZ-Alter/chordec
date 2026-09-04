#include <iostream>
#include <string_view>
#include "version.h"

#include "info.h"

void printVersion() {
    std::cout << PROJECT_NAME << " v" << PROJECT_VERSION << std::endl;
}

void printHelp() {
    std::cout << "Usage: " << PROJECT_NAME << " [options]\n\n"
              << "Options:\n"
              << "  -v, --version    Show version info\n"
              << "  -h, --help       Show help message\n";
}