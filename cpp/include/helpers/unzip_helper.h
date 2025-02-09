#ifndef UNZIP_HELPER_H
#define UNZIP_HELPER_H

#include <stdio.h> // For SEEK_END, SEEK_CUR...
#include <sys/types.h> // For some types...

#include "utils/common.h"
#include "utils/logging.h"
#include "mylibc.h"

#include "inflate_helper.h"
#include "pkcs7_helper.h"

#define EOCD_SIGNATURE 0x06054b50
#define LOCAL_FILE_HEADER_SIGNATURE 0x04034b50
#define CENTRAL_DIRECTORY_SIGNATURE 0x02014b50

#define BUFFER_SIZE 8192
#define EOCD_MIN_SIZE 22

off_t findEOCDOffset(int fd);

off_t getCentralDirectoryOffset(int fd, off_t eocdOffset);

int findCertificateFile(int fd, off_t centralDirOffset, char* certFileName, off_t& fileOffset, size_t& fileSize);

int extractCertFile(int fd, off_t fileOffset, size_t& fileSize, unsigned char* data);

#endif // UNZIP_HELPER_H