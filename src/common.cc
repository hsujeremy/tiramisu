#include <cstdarg>
#include <cstdio>
#include <sstream>
#include "common.h"

std::vector<std::string> split_strings(std::string str, const char delim) {
    std::vector<std::string> substrings;
    std::stringstream sstream(str);
    while (sstream.good()) {
        std::string substring;
        std::getline(sstream, substring, ',');
        substrings.push_back(substring);
    }
    return substrings;
}

void dbg_printf(const bool debug, const char* format, ...) {
    if (debug) {
        va_list args1;
        va_start(args1, format);
        vfprintf(stdout, format, args1);
        va_end(args1);
    }
}
