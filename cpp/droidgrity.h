#ifndef DROIDGRITY_H
#define DROIDGRITY_H

#include <jni.h>

#include "sys/types.h"

#include "mylibc.h"
#include "utils/logging.h"
#include "utils/common.h"

#include "helpers/path_helper.h"
#include "helpers/sha256_helper.h"
#include "helpers/unzip_helper.h"
#include "helpers/apksigningblock_helper.h"

int getCertDataFromJarSignature(int fd, off_t eocdOffset, size_t& certSize, unsigned char* certData);

int getCertDataFromAPKSigningBlock(int fd, off_t eocdOffset, size_t& certSize, unsigned char* certData);

int verifyCertificateFromAPK(const char* apkPath, unsigned char* knownCertHash, size_t hashLen);

#endif // DROIDGRITY_H