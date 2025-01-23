#include "unzip_helper.h"

// Read the last N bytes of the file to locate EOCD
off_t findEOCDOffset(int fd) {
    char buffer[BUFFER_SIZE];

    off_t fileSize = my_lseek(fd, 0, SEEK_END);
    off_t offset = fileSize - BUFFER_SIZE;
    if (offset < 0) offset = 0;

    my_lseek(fd, offset, SEEK_SET);
    ssize_t bytesRead = my_read(fd, buffer, BUFFER_SIZE);

    for (int i = (int) bytesRead - EOCD_MIN_SIZE; i >= 0; --i) {
        if (*(uint32_t*)(buffer + i) == EOCD_SIGNATURE) {
            return offset + i;
        }
    }

    return -1; // EOCD not found
}

// Parse EOCD and get the central directory offset
off_t getCentralDirectoryOffset(int fd, off_t eocdOffset) {
    char eocdBuffer[EOCD_MIN_SIZE];
    
    my_lseek(fd, eocdOffset, SEEK_SET);
    my_read(fd, eocdBuffer, EOCD_MIN_SIZE);
    
    return (off_t) readLE32(eocdBuffer + 16); // Central Directory offset
}

static unsigned char* inflate_data(const unsigned char* compressedData, size_t compressedSize, size_t decompressedSize) {
    // Create a buffer to store the decompressed data.
    unsigned char* decompressedData = new unsigned char[decompressedSize];

    // Initialize the zlib inflate stream structure
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int ret = inflateInit(&strm);
    if (ret != Z_OK) {
        LOGE("Failure in zlib.inflateInit: %d", ret);
        delete[] decompressedData;
        return nullptr;
    }

    // Set up the zlib stream with the compressed data
    strm.avail_in = compressedSize;
    strm.next_in = const_cast<unsigned char*>(compressedData);
    strm.avail_out = decompressedSize;
    strm.next_out = decompressedData;

    // Debug: Log compressed data
    const size_t chunkSize = 64; // Adjust the chunk size as needed
    char buffer[compressedSize * 3 + 1];
    size_t bufferIndex = 0;
    for (size_t i = 0; i < compressedSize; ++i) {
        bufferIndex += std::snprintf(buffer + bufferIndex, sizeof(buffer) - bufferIndex, "%02x ", compressedData[i] & 0xFF);

        // If the buffer is full or we've reached the end of a chunk
        if ((i + 1) % chunkSize == 0 || i == compressedSize - 1) {
            buffer[bufferIndex] = '\0'; // Null-terminate the string
            LOGD("%s", buffer); // Log the current chunk
            bufferIndex = 0; // Reset the buffer index for the next chunk
        }
    }

    // Perform the decompression
    ret = inflate(&strm, Z_NO_FLUSH);
    if (ret != Z_STREAM_END) {
        LOGE("Failure in zlib.inflate: %d", ret);
        inflateEnd(&strm);
        delete[] decompressedData;
        return nullptr;
    }

    // Clean up the zlib stream
    inflateEnd(&strm);

    // Return the decompressed data
    return decompressedData;
}

// Extract Central Directory and find META-INF/*.RSA or META-INF/*.DSA
int findCertificateFile(int fd, off_t centralDirOffset, char* certFileName, off_t& fileOffset, size_t& fileSize) {
    LOGD("Trying to find certificate file from Central Directory...");

    my_lseek(fd, centralDirOffset, SEEK_SET);
    char buffer[46]; // Central Directory file header size

    while (my_read(fd, buffer, sizeof(buffer)) > 0) {
        uint32_t signature = readLE32(buffer);
        if (signature != CENTRAL_DIRECTORY_SIGNATURE) {
            LOGE("Central Directory signature mismatch");
            break;
        }

        uint16_t fileNameLength = readLE16(buffer + 28);
        uint16_t extraFieldLength = readLE16(buffer + 30);
        uint16_t commentLength = readLE16(buffer + 32);

        // Read file name
        char fileName[256];
        my_read(fd, fileName, fileNameLength);
        fileName[fileNameLength] = '\0';

        // LOGD("Filename found : %s", fileName);

        if (my_strstr(fileName, "META-INF/") && (my_strstr(fileName, ".RSA") || my_strstr(fileName, ".DSA"))) {
            LOGD("Found certificate file : %s", fileName);

            my_strlcpy(certFileName, fileName, my_strlen(fileName));
            fileOffset = (off_t) readLE32(buffer + 42); // Local header offset
            fileSize = readLE32(buffer + 24); // decompressed size
            return 0; // Found
        }

        // Skip extra fields and comments
        my_lseek(fd, extraFieldLength + commentLength, SEEK_CUR);
    }

    return -1; // Not found
}

// Extract the certificate file data
int extractCertFile(int fd, off_t fileOffset, size_t& fileSize, unsigned char* data) {
    LOGD("Trying to extract certificate file data...");
    LOGD("File Offset: %ld", fileOffset);
    LOGD("File Size: %zu", fileSize);

    my_lseek(fd, fileOffset, SEEK_SET);
    char header[30]; // Local file header size
    my_read(fd, header, sizeof(header));

    uint32_t signature = readLE32(header);
    if (signature != LOCAL_FILE_HEADER_SIGNATURE) {
        LOGE("Invalid Local File Header signature");
        return -1;
    }

    uint16_t compressionMethod = readLE16(header + 8);
    const char* compressionMethodStr = "UNKNOWN";
    switch (compressionMethod) {
        case 0:
            compressionMethodStr = "NONE";
            break;
        case 8:
            compressionMethodStr = "DEFLATE";
            break;
        default:
            break;
    }
    LOGD("Compression Method: %u => %s", compressionMethod, compressionMethodStr);

    size_t compressedSize = (size_t) readLE32(header + 18);
    size_t decompressedSize = (size_t) readLE32(header + 22);
    uint16_t fileNameLength = readLE16(header + 26);
    uint16_t extraLength = readLE16(header + 28);

    LOGD("Compressed Data Size: %zu", compressedSize);
    LOGD("Decompressed Data Size: %zu", decompressedSize);
    LOGD("Certificate File Name Length: %u", fileNameLength);
    LOGD("Certificate File Extra Length: %u", extraLength);

    my_lseek(fd, fileNameLength + extraLength, SEEK_CUR); // Skip to file data

    unsigned char * compressedPkcs7RawData = (unsigned char *) malloc(compressedSize);
    ssize_t compressedPkcs7RawDataSize = my_read(fd, compressedPkcs7RawData, (size_t) compressedSize);

    if (compressedPkcs7RawDataSize != compressedSize) {
        LOGE("Failed to read certificate file, expected: %zu, got: %zd", compressedSize, compressedPkcs7RawDataSize);
        delete[] compressedPkcs7RawData;
        return -1;
    }

    LOGD("Inflating the compressed DER encoded PKCS#7 raw data");
    size_t pkcs7RawDataSize = 0;
    unsigned char * pkcs7RawData = inflate_data(compressedPkcs7RawData, compressedPkcs7RawDataSize, pkcs7RawDataSize);

    if (pkcs7RawDataSize != decompressedSize) {
        LOGE("Inflated file size (%zu) doesn't match expected size (%zu)", pkcs7RawDataSize, decompressedSize);
        return -1;
    }

    LOGD("Extracting certificate from DER encoded PKCS#7 raw data");

    const uint8_t certTag[] = {0x30, 0x82}; // DER-encoded certificate tag    
    for (size_t i = 0; i < pkcs7RawDataSize - 2; i++) {
        if (pkcs7RawData[i] == certTag[0] && pkcs7RawData[i + 1] == certTag[1]) {
            
            // Length of the certificate is encoded in the next two bytes
            fileSize = (pkcs7RawData[i + 2] << 8) | pkcs7RawData[i + 3];
            
            if (i + 4 + fileSize <= pkcs7RawDataSize) {
                data = (uint8_t *)&pkcs7RawData[i];
                LOGD("Found cert data in DER encoded PKCS#7 raw data");
                return 0;
            }
        } else {
            LOGE("Raw data found is not DER encoded...");
            return -1;
        }
    }

    LOGE("Could not find cert data in DER encoded PKCS#7 raw data");
    return -1;
}