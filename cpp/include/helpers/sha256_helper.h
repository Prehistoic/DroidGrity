// Self ported sha256 implementation. All credits go to @983 : https://github.com/983/SHA-256

#include <sys/types.h>
#include <stdint.h>

#ifndef SHA256_H
#define SHA256_H

typedef struct sha256 {
    uint32_t state[8];
    uint8_t buffer[64];
    uint64_t n_bits;
    uint8_t buffer_counter;
} sha256;

#define SHA256_HEX_SIZE (64 + 1)
#define SHA256_BYTES_SIZE 32

void sha256_init(struct sha256 *sha);

void sha256_append_byte(struct sha256 *sha, uint8_t byte);

void sha256_append(struct sha256 *sha, const void *src, size_t n_bytes);

void sha256_finalize(struct sha256 *sha);

void sha256_finalize_hex(struct sha256 *sha, char *dst_hex65);

void sha256_finalize_bytes(struct sha256 *sha, void *dst_bytes32);

void sha256_hex(const void *src, size_t n_bytes, char *dst_hex65);

void sha256_bytes(const void *src, size_t n_bytes, void *dst_bytes32);

#endif