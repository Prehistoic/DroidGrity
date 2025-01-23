#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdlib.h> // malloc

// Helper to read a little-endian 16-bit value from the buffer
inline uint16_t readLE16(const void* data) {
    const unsigned char *buffer = (const unsigned char *)data;
    return buffer[0] | (buffer[1] << 8);
}

// Helper to read a little-endian 32-bit value from the buffer
inline uint32_t readLE32(const void* data) {
    const unsigned char *buffer = (const unsigned char *)data;
    return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}

// Helper to read a little-endian 64-bit value from the buffer
inline uint64_t readLE64(const void* data) {
    const unsigned char *buffer = (const unsigned char *)data;
    return ((uint64_t)buffer[0]) |
           ((uint64_t)buffer[1] << 8) |
           ((uint64_t)buffer[2] << 16) |
           ((uint64_t)buffer[3] << 24) |
           ((uint64_t)buffer[4] << 32) |
           ((uint64_t)buffer[5] << 40) |
           ((uint64_t)buffer[6] << 48) |
           ((uint64_t)buffer[7] << 56);
}

inline char* convertToHex(const unsigned char* hash, size_t length) {
    // Each byte takes 2 hex digits + optional separators (e.g., ":" or space) + null terminator
    size_t bufferSize = (length * 2) + 1; // +1 for null terminator
    char* hexString = (char*) malloc(bufferSize);
    if (!hexString) {
        return nullptr;
    }

    // Fill the buffer with hex representation. This is a bit dirty but allows us to avoid using any glibc function like sprintf
    char* ptr = hexString;
    for (size_t i = 0; i < length; i++) {
        unsigned char byte = hash[i];
        *ptr++ = "0123456789abcdef"[byte >> 4];  // High nibble
        *ptr++ = "0123456789abcdef"[byte & 0x0F]; // Low nibble
    }

    // Null-terminate the string
    *ptr = '\0';
    return hexString;
}

#endif // COMMON_H