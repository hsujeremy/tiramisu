#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <unordered_map>
#include <vector>

#define PORT 8888
#define DBG 1

std::vector<std::string> split_strings(std::string str, const char delim);

void dbg_printf(const bool debug, const char* format, ...);

#endif
