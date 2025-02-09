#include "apksigningblock_helper.h"

// Locate APK Signing Block
off_t locateAPKSigningBlock(int fd, off_t eocdOffset) {
    // Central Directory offset
    off_t centralDirOffset = getCentralDirectoryOffset(fd, eocdOffset);

    // Check for APK Signing Block magic
    unsigned char buffer[BUFFER_SIZE];
    off_t searchOffset = centralDirOffset - BUFFER_SIZE;
    if (searchOffset < 0) searchOffset = 0;

    my_lseek(fd, searchOffset, SEEK_SET);
    ssize_t bytesRead = my_read(fd, buffer, BUFFER_SIZE);

    for (ssize_t i = bytesRead - APK_SIG_BLOCK_MAGIC_LEN; i >= 0; --i) {
        if (my_memcmp(buffer + i, APK_SIG_BLOCK_MAGIC, APK_SIG_BLOCK_MAGIC_LEN) == 0) {
            off_t blockOffset = searchOffset + i;

            LOGD("Found APK Signing Block at offset = %ld", blockOffset);
            return blockOffset;
        }
    }

    return -1; // APK Signing Block not found
}

// Helper to extract the 1st certificate from the APK Signature Scheme v2 block
int extractCertificateFromSignatureV2SchemeBlock(const unsigned char* signatureV2SchemeBlock, size_t& certSize, unsigned char* certData) {
    const unsigned char* ptr = signatureV2SchemeBlock;

    // Signing V2 scheme block format : https://source.android.com/docs/security/features/apksigning/v2#apk-signature-scheme-v2-block-format
    // Signer sequence length (uint32)
    //  - Signed data length (uint32)
    //     - Digests length (uint32)
    //       - Signature algorithm ID (uint32)
    //       - Digest length (uint32)
    //          - Digest
    //     - Certificates length (uint32)
    //       - Certificate length (uint32)
    //          - Certificate
    //     - Additional attributes length (uint32)
    //       - ID (uint32)
    //       - value (4 bytes)
    // - Signatures length (uint32)
    //     - Signature Algorithm ID (uint32)
    //     - Signature length (uint32)
    //       - Signature over signed data
    // - Public key length (uint32)
    //     - Public key

    // Note that Signing V3 Scheme starts in the same way so this method will work as well
    // Signing V4 Scheme exists as well and is very different but it requires having a V2 or V3 signature as well

    uint32_t signerSequenceSize = readLE32(ptr);
    ptr += 4;

    LOGD("Signer Sequence size: %u bytes", signerSequenceSize);

    uint32_t signedDataSize = readLE32(ptr);
    ptr += 4;

    LOGD("Signed data size: %u bytes", signedDataSize);

    uint32_t digestsSize = readLE32(ptr);
    ptr += 4;

    LOGD("Digests size: %u bytes", digestsSize);

    // Skipping digests
    ptr += digestsSize;

    uint32_t certificatesSize = readLE32(ptr);
    ptr += 4;

    LOGD("Certificates size: %u bytes", certificatesSize);

    // We will only retrieve the first certificate data
    uint32_t certificateSize = readLE32(ptr);
    ptr += 4;

    certSize = certificateSize;
    my_memcpy(certData, ptr, certificateSize);

    return 0;
}

// Parse APK Signing Block
int parseAPKSigningBlock(int fd, off_t blockOffset, size_t& certSize, unsigned char* certData) {
    unsigned char sizeBuffer[8];
    my_lseek(fd, blockOffset - 8, SEEK_SET); // Read the size field
    my_read(fd, sizeBuffer, sizeof(sizeBuffer));
    size_t blockSize = (size_t) readLE64(sizeBuffer);

    LOGD("APK Signing Block Size = %zu bytes", blockSize);

    unsigned char* blockData = (unsigned char*) malloc(blockSize);
    if (!blockData) {
        LOGE("Memory allocation for blockData failed");
        return -1;
    }

    my_lseek(fd, blockOffset - blockSize, SEEK_SET);
    my_read(fd, blockData, blockSize);

    // Iterate over key-value pairs (simplified)
    unsigned char* ptr = blockData;
    const unsigned char* end = blockData + blockSize;

    int success = -1;
    while (ptr + 8 <= end) {
        uint32_t id = readLE32(ptr);         // Read the ID
        uint64_t size = readLE64(ptr + 4);  // Read the size

        LOGD("Found block: ID=0x%x Size=%lld bytes", id, (long long)size);

        ptr += 8;

        if (id == APK_SIG_V2_SCHEME_BLOCK_ID) {
            LOGD("Found APK v2+ Signature Scheme block");
            success = extractCertificateFromSignatureV2SchemeBlock(ptr, certSize, certData);
            break;
        }

        ptr += size;

        // Ensure no overflow
        if (ptr > end) {
            LOGW("Block size exceeds payload boundary");
            break;
        }
    }

    free(blockData);

    return success;
}