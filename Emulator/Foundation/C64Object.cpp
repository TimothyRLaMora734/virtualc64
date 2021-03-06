// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "C64Object.h"

#define VC64OBJ_PARSE \
char buf[256]; \
va_list ap; \
va_start(ap, fmt); \
vsnprintf(buf, sizeof(buf), fmt, ap); \
va_end(ap);

#define VC64PRINTPLAIN(trailer) \
fprintf(stderr, "%s%s", trailer, buf);

#define VC64PRINT(trailer) \
prefix(); \
fprintf(stderr, "%s: %s%s", getDescription(), trailer, buf);

void
C64Object::prefix()
{
}

void
C64Object::msg(const char *fmt, ...)
{
    VC64OBJ_PARSE
    VC64PRINTPLAIN("")
}

void
C64Object::warn(const char *fmt, ...)
{
    VC64OBJ_PARSE;
    VC64PRINT("WARNING: ")
}

void
C64Object::panic(const char *fmt, ...)
{
    VC64OBJ_PARSE;
    VC64PRINT("PANIC: ")
    std::abort();
}

void
C64Object::debug(const char *fmt, ...)
{
#ifndef NDEBUG
    VC64OBJ_PARSE
    VC64PRINT("")
#endif
}

void
C64Object::debug(int verbose, const char *fmt, ...)
{
#ifndef NDEBUG
    if (verbose) {
        VC64OBJ_PARSE
        VC64PRINT("")
    }
#endif
}

void
C64Object::plaindebug(const char *fmt, ...)
{
#ifndef NDEBUG
    VC64OBJ_PARSE
    VC64PRINTPLAIN("")
#endif
}

void
C64Object::plaindebug(int verbose, const char *fmt, ...)
{
#ifndef NDEBUG
    if (verbose) {
        VC64OBJ_PARSE
        VC64PRINTPLAIN("")
    }
#endif
}

bool
C64Object::tracingEnabled()
{
    if (traceCounter == 0)
        return false;
    
    if (traceCounter > 0)
        traceCounter--;
    
    return true;
}
