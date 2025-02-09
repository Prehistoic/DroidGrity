// Credit goes to https://github.com/DimaKoz/stunning-signature/blob/master/app/src/main/c/pkcs7_helper.c

#ifndef PKCS7_HELPER_H
#define PKCS7_HELPER_H

#include <sys/types.h> // For some types...
#include "malloc.h"
#include "assert.h"

#include "mylibc.h"
#include "utils/logging.h"

// Tags:
// https://en.wikipedia.org/wiki/X.690
#define TAG_INTEGER         0x02
#define TAG_BITSTRING       0x03
#define TAG_OCTETSTRING     0x04
#define TAG_NULL            0x05
#define TAG_OBJECTID        0x06
#define TAG_UTCTIME         0x17
#define TAG_GENERALIZEDTIME 0x18
#define TAG_SEQUENCE        0x30
#define TAG_SET             0x31

#define TAG_OPTIONAL    0xA0

#define NAME_LEN    63

typedef struct element {
    unsigned char tag;
    char name[NAME_LEN];
    int begin;
    size_t len;
    int level;
    struct element *next;
} element;

int extract_cert_from_pkcs7(unsigned char * pkcs7_cert, size_t len_in, size_t *len_out, unsigned char * data);

#endif // PKCS7_HELPER_H