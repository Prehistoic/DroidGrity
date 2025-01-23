#ifndef APKSIGNINGBLOCK_HELPER_H
#define APKSIGNINGBLOCK_HELPER_H

#include <stdio.h> // For SEEK_END, SEEK_CUR...
#include <sys/types.h> // For some types...

#include "utils/logging.h"
#include "utils/common.h"
#include "mylibc.h"

#include "helpers/unzip_helper.h"

#define APK_SIG_BLOCK_MAGIC "APK Sig Block 42"
#define APK_SIG_V2_SCHEME_BLOCK_ID 0x7109871a
#define APK_SIG_BLOCK_MAGIC_LEN 16

#define BUFFER_SIZE 8192

off_t locateAPKSigningBlock(int fd, off_t eocdOffset);

int extractCertificateFromSignatureV2SchemeBlock(const unsigned char* signatureV2SchemeBlock, size_t& certSize, unsigned char* certData);

int parseAPKSigningBlock(int fd, off_t blockOffset, size_t& certSize, unsigned char* certData);

#endif // APKSIGNINGBLOCK_HELPER_H