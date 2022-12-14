#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <unordered_map>
#include <vector>

#define PORT 8888
#define DBG 1
#define DBG_COMMIT 0

// split_strings(str, delim)
//     Splits `str` into substrings separated by `delim`.
//     Returns a new vector of substrings.
std::vector<std::string> split_strings(std::string str, const char delim);

// dbg_printf(debug, format, ...)
//     Prints the formatted string if `debug` is set to true.
void dbg_printf(const bool debug, const char* format, ...);

struct Message {
    size_t length;
    char* payload; // TODO: depending on how this is used, may not be needed
};

#endif
