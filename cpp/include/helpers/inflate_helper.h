#ifndef INFLATE_HELPER_H
#define INFLATE_HELPER_H

#include <sys/types.h> // For some types...
#include <assert.h>

#include "mylibc.h"

typedef enum {
    INFLATE_OK = 1,      // Success
    INFLATE_ERROR = -1,   // Input error
    INFLATE_OVERFLOW = -2 // Not enough room for output
} InflateResult;

InflateResult inflate(void*, size_t*, const void*, size_t);

#endif // INFLATE_HELPER_H