#include <cstdarg>
#include <cstdio>
#include "common.h"

void dbg_printf(const bool debug, const char* format, ...) {
    if (debug) {
        va_list args1;
        va_start(args1, format);
        vfprintf(stdout, format, args1);
        va_end(args1);
    }
}
