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
    
    off_t centralDirectoryOffset = (off_t) readLE32(eocdBuffer + 16); // Central Directory offset
    LOGD("Central Directory Offset : %ld", centralDirectoryOffset);

    return centralDirectoryOffset;
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

        // LOGD("Central Directory entry found : %s", fileName);

        if (my_strstr(fileName, "META-INF/") && (my_strstr(fileName, ".RSA") || my_strstr(fileName, ".DSA"))) {
            LOGD("Central Directory - Found certificate file : %s", fileName);

            my_strlcpy(certFileName, fileName, my_strlen(fileName));
            fileOffset = (off_t) readLE32(buffer + 42); // Local header offset
            fileSize = readLE32(buffer + 24); // decompressed size

            LOGD("File Offset: %ld", fileOffset);
            LOGD("File Size: %zu", fileSize);

            return 0; // Found
        }

        // Skip extra fields and comments
        my_lseek(fd, extraFieldLength + commentLength, SEEK_CUR);
    }

    return -1; // Not found
}

// Extract the certificate file data
int extractCertFile(int fd, off_t fileOffset, size_t& certSize, unsigned char* certData) {
    LOGD("Trying to extract certificate file data...");

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
    LOGD("Local File Header - Compression Method: %u => %s", compressionMethod, compressionMethodStr);

    size_t compressedSize = (size_t) readLE32(header + 18);
    size_t decompressedSize = (size_t) readLE32(header + 22);
    uint16_t fileNameLength = readLE16(header + 26);
    uint16_t extraLength = readLE16(header + 28);

    LOGD("Local File Header - Compressed Data Size: %zu", compressedSize);
    LOGD("Local File Header - Decompressed Data Size: %zu", decompressedSize);
    LOGD("Local File Header - File Name Length: %u", fileNameLength);
    LOGD("Local File Header - File Extra Length: %u", extraLength);

    // Confirming file name
    char fileName[256];
    my_read(fd, fileName, fileNameLength);
    fileName[fileNameLength] = '\0';

    LOGD("Filename: %s", fileName);

    my_lseek(fd, extraLength, SEEK_CUR); // Skip to file data

    unsigned char * compressedPkcs7RawData = (unsigned char *) malloc(compressedSize);
    ssize_t compressedPkcs7RawDataSize = my_read(fd, compressedPkcs7RawData, (size_t) compressedSize);

    if (compressedPkcs7RawDataSize != compressedSize) {
        LOGE("Failed to read certificate file, expected: %zu, got: %zd", compressedSize, compressedPkcs7RawDataSize);
        delete[] compressedPkcs7RawData;
        return -1;
    }

    LOGD("Inflating the compressed DER encoded PKCS#7 raw data");
    unsigned char* pkcs7RawData = new unsigned char[decompressedSize];
    size_t pkcs7RawDataSize = decompressedSize;
    int ret = inflate(pkcs7RawData, &pkcs7RawDataSize, compressedPkcs7RawData, compressedPkcs7RawDataSize);

    if (ret < 0) {
        LOGE("Inflating data failed with error %d", ret);
        return -1;
    }

    if (pkcs7RawDataSize != decompressedSize) {
        LOGE("Inflated file size (%zu) doesn't match expected size (%zu)", pkcs7RawDataSize, decompressedSize);
        return -1;
    }

    LOGD("Extracting certificate from DER encoded PKCS#7 raw data");

    extract_cert_from_pkcs7(pkcs7RawData, pkcs7RawDataSize, &certSize, certData);

    if (certData == NULL) {
        LOGE("Could not find cert data in DER encoded PKCS#7 raw data");
        return -1;
    }
    
    LOGD("Found cert data in DER encoded PKCS#7 raw data");
    return 0;
}