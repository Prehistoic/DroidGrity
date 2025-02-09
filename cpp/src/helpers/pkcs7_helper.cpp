#include "pkcs7_helper.h"

/*PKCS7 structure
*pkcs7Data : SEQUENCE
*	contentType : ObjectIdentifier  {signedData}
*	content[optional] : SEQUENCE 							#CERT.RSA Belongs to signedData type
*		version : INTEGER
*		digestAlgorithms : SET : DigestAlgorithmIdentifier  #Message digest algorithm
*		contentInfo : SEQUENCE   							#
*		certificates[optional] : SEQUENCE 					#Certificate information
*			tbsCertificate : SEQUENCE #
*				version : INTEGER
*				serialNumber : INTEGER  					#The serial number of the certificate, uniquely determined by the certificate issuer and serial number
*				signature ： SEQUENCE : AlgorithmIdentifier
*				issuer : SET 								#Certificate issuer
*				validity : SEQUENCE    						#The validity of the certificate
*				subject : SET                               #Subject of certificate
*				subjectPublicKeyInfo : SEQUENCE 			#Public key related information, including encryption algorithm and public key
*				issuerUniqueID[optional] : BITSTRING
*				subjectUniqueID[optional] : BITSTRING
*				extensions[optional] : SEQUENCE  			#certificate extended information
*			signatureAlgorithm : AlgorithmIdentifier 		#Signature algorithms
*			signatureValue : BITSTRING 						#This is the digital signature of the tbsCertificate section to prevent the tbsCertificate content from being modified
*		crls[optional] : SET 								#Certificate revocation list
*		signerInfos : SET
*			signerInfo : SEQUENCE							#Signer information
*				version : INTEGER
*				issuerAndSerialNumber : SEQUENCE 			#The issuer and serial number of the certificate
*				digestAlgorithmId : SEQUENCE : DigestAlgorithmIdentifier #Message digest algorithm
*				authenticatedAttributes[optional]
*				digestEncryptionAlgorithmId : SEQUENCE 		#Signature algorithm
*				encryptedDigest : OCTETSTRING   			#Private key encrypted data
*				unauthenticatedAttributes[optional]
*
*Each item is saved in the form of{tag，length，content}
*/

static uint32_t m_pos = 0;
static size_t m_length = 0;
static struct element *head = NULL;
static struct element *tail = NULL;

/**
 * Calculate the number of bytes occupied by length according to lenbyte.
 * 1) The most significant bit of the byte is 1, then the length of the low 7-bit number of bytes;
 * 2) The highest bit is 0, then lenbyte represents the length
 */
static uint32_t getLengthNum(unsigned char lenbyte) {
    uint32_t num = 1;
    if (lenbyte & 0x80) {
        num += lenbyte & 0x7f;
    }
    return num;
}

/**
 * Calculate length information based on lenbyte，
 * The algorithm is lenbyte the highest bit is 1，
 * Then lenbyte & 0x7F represent the length of the byte length,
 * Subsequent bytes are stored in big-endian mode. The highest bit is 0， Lenbyte directly represents the length
 *
 * 1) If 0x82 0x34 0x45 0x22 .... 0x82 is lenbyte,
 * The high level is 1, 0x82 & 0x7F == 2,
 * Then the following two bytes are the length information stored at the high end. The length information is 0x3445
   2)If lenbyte == 0x34, the highest bit is 0, then the length information is 0x34
*/
static uint32_t getLength(unsigned char *certrsa, unsigned char lenbyte, int offset) {
    int32_t len = 0, num;
    unsigned char tmp;
    if (lenbyte & 0x80) {
        num = lenbyte & 0x7f;
        if (num < 0 || num > 4) {
            LOGE("ASN.1 element length is too long ! => %d", num);
            return 0;
        }
        while (num) {
            len <<= 8;
            tmp = certrsa[offset++];
            len += (tmp & 0xff);
            num--;
        }
    } else {
        len = lenbyte & 0xff;
    }
    assert(len >= 0);
    return (uint32_t) len;
}

/**
 * Each element has a corresponding element.
 */
int32_t createElement(unsigned char *certrsa, unsigned char tag, char *name, int level) {
    LOGD("Creating element \"%s\"", name);

    unsigned char get_tag = certrsa[m_pos++];
    if (get_tag != tag) {
        m_pos--;
        LOGE("Failed to create element \"%s\"", name);
        return -1;
    }

    unsigned char lenbyte = certrsa[m_pos];
    int len = getLength(certrsa, lenbyte, m_pos + 1);
    m_pos += getLengthNum(lenbyte);

    element *node = (element *) calloc(1, sizeof(element));
    node->tag = get_tag;
    my_strlcpy(node->name, name, my_strlen(name) + 1);
    node->begin = m_pos;
    node->len = len;
    node->level = level;
    node->next = NULL;

    if (head == NULL) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
    return len;
}

/**
 * Parse certificate information
 */
int parseCertificate(unsigned char *certrsa, int level) {
    char *names[] = {
            "tbsCertificate",
            "version",
            "serialNumber",
            "signature",
            "issuer",
            "validity",
            "subject",
            "subjectPublicKeyInfo",
            "issuerUniqueID-[optional]",
            "subjectUniqueID-[optional]",
            "extensions-[optional]",
            "signatureAlgorithm",
            "signatureValue"};
    int len = 0;
    unsigned char tag;

    len = createElement(certrsa, TAG_SEQUENCE, names[0], level);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }

    //version
    tag = certrsa[m_pos];
    if (((tag & 0xc0) == 0x80) && ((tag & 0x1f) == 0)) {
        m_pos += 1;
        m_pos += getLengthNum(certrsa[m_pos]);
        len = createElement(certrsa, TAG_INTEGER, names[1], level + 1);
        if (len == -1 || m_pos + len > m_length) {
            return -1;
        }
        m_pos += len;
    }

    for (int i = 2; i < 11; i++) {
        switch (i) {
            case 2:
                tag = TAG_INTEGER;
                break;
            case 8:
                tag = 0xA1;
                break;
            case 9:
                tag = 0xA2;
                break;
            case 10:
                tag = 0xA3;
                break;
            default:
                tag = TAG_SEQUENCE;
        }
        len = createElement(certrsa, tag, names[i], level + 1);
        if (i < 8 && len == -1) {
            return -1;
        }
        if (len != -1)
            m_pos += len;
    }
    //signatureAlgorithm
    len = createElement(certrsa, TAG_SEQUENCE, names[11], level);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }
    m_pos += len;
    //signatureValue
    len = createElement(certrsa, TAG_BITSTRING, names[12], level);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }
    m_pos += len;
    return 0;
}

/**
 * Resolve signer information
 */
int parseSignerInfo(unsigned char *certrsa, int level) {
    char *names[] = {
            "version",
            "issuerAndSerialNumber",
            "digestAlgorithmId",
            "authenticatedAttributes-[optional]",
            "digestEncryptionAlgorithmId",
            "encryptedDigest",
            "unauthenticatedAttributes-[optional]"};
    int len;
    unsigned char tag;
    for (int i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        switch (i) {
            case 0:
                tag = TAG_INTEGER;
                break;
            case 3:
                tag = 0xA0;
                break;
            case 5:
                tag = TAG_OCTETSTRING;
                break;
            case 6:
                tag = 0xA1;
                break;
            default:
                tag = TAG_SEQUENCE;

        }
        len = createElement(certrsa, tag, names[i], level);
        if (len == -1 || m_pos + len > m_length) {
            if (i == 3 || i == 6)
                continue;
            return -1;
        }
        m_pos += len;
    }
    return m_pos == m_length ? 0 : -1;
}

int parseContent(unsigned char *certrsa, int level) {
    char *names[] = {"version",
                     "DigestAlgorithms",
                     "contentInfo",
                     "certificates-[optional]",
                     "crls-[optional]",
                     "signerInfos",
                     "signerInfo"};

    unsigned char tag;
    int len = 0;

    //version
    len = createElement(certrsa, TAG_INTEGER, names[0], level);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }
    m_pos += len;

    //DigestAlgorithms
    len = createElement(certrsa, TAG_SET, names[1], level);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }
    m_pos += len;

    //contentInfo
    len = createElement(certrsa, TAG_SEQUENCE, names[2], level);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }
    m_pos += len;

    //certificates-[optional]
    tag = certrsa[m_pos];
    if (tag == TAG_OPTIONAL) {
        m_pos++;
        m_pos += getLengthNum(certrsa[m_pos]);
        len = createElement(certrsa, TAG_SEQUENCE, names[3], level);
        if (len == -1 || m_pos + len > m_length) {
            return -1;
        }
        int ret = parseCertificate(certrsa, level + 1);
        if (ret == -1) {
            return ret;
        }
    }

    //crls-[optional]
    tag = certrsa[m_pos];
    if (tag == 0xA1) {
        m_pos++;
        m_pos += getLengthNum(certrsa[m_pos]);
        len = createElement(certrsa, TAG_SEQUENCE, names[4], level);
        if (len == -1 || m_pos + len > m_length) {
            return -1;
        }
        m_pos += len;
    }

    //signerInfos
    tag = certrsa[m_pos];
    if (tag != TAG_SET) {
        return -1;
    }
    len = createElement(certrsa, TAG_SET, names[5], level);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }

    //signerInfo
    len = createElement(certrsa, TAG_SEQUENCE, names[6], level + 1);
    if (len == -1 || m_pos + len > m_length) {
        return -1;
    }
    return parseSignerInfo(certrsa, level + 2);
}

/**
 *Finds the element in pkcs7 by name, returns NULL if it is not found.
 *name: name of the element we look for
 *begin: beginning of the search
 */
static element *getElement(const char *name, element *begin) {
    if (begin == NULL)
        begin = head;

    element *p = begin;
    while (p != NULL) {
        if (my_strncmp(p->name, name, my_strlen(name)) == 0) {
            return p;
        }

        p = p->next;
    }

    LOGE("Failed to find the element \"%s\"", name);
    return p;
}

static int parse(unsigned char *certrsa, size_t length) {
    unsigned char tag, lenbyte;
    int len = 0;
    int level = 0;
    m_pos = 0;
    m_length = length;

    tag = certrsa[m_pos++];
    if (tag != TAG_SEQUENCE) {
        LOGE("Tag does not match ASN.1");
        return -1;
    }

    lenbyte = certrsa[m_pos];
    len = getLength(certrsa, lenbyte, m_pos + 1);
    m_pos += getLengthNum(lenbyte);
    if (m_pos + len > m_length)
        return -1;

    //contentType
    len = createElement(certrsa, TAG_OBJECTID, "contentType", level);
    if (len == -1) {
        LOGE("Failed to find contentType");
        return -1;
    }
    m_pos += len;

    //optional
    tag = certrsa[m_pos++];
    lenbyte = certrsa[m_pos];
    m_pos += getLengthNum(lenbyte);

    //content-[optional]
    len = createElement(certrsa, TAG_SEQUENCE, "content-[optional]", level);
    if (len == -1) {
        LOGE("Failed to find content-[optional]");
        return -1;
    }

    return parseContent(certrsa, level + 1);
}

/**
 * Convert length information to ASN.1 length format
 * len <= 0x7f       1
 * len >= 0x80       1 + Non-zero bytes
 */
static size_t getNumFromLen(size_t len) {
    size_t num = 0;
    size_t tmp = len;
    while (tmp) {
        num++;
        tmp >>= 8;
    }
    if ((num == 1 && len >= 0x80) || (num > 1))
        num += 1;
    return num;
}

/**
 *Each element element is a {tag, length, data} triple, tag and length are saved by tag and len, and data is saved by [begin, begin+len].
 *
 *This function calculates the offset from the data position to the tag position
 */
size_t getTagOffset(element *p, unsigned char *certrsa) {
    if (p == NULL)
        return 0;
    size_t offset = getNumFromLen(p->len);
    if (certrsa[p->begin - offset - 1] == p->tag)
        return offset + 1;
    else
        return 0;
}

// Extracts the first X.509 certificate from PKCS7 DER
int extract_cert_from_pkcs7(unsigned char * pkcs7_cert, size_t len_in, size_t *len_out, unsigned char * data) {
    // We make sure to reset every static data
    m_pos = 0;
    m_length = 0;
    element *head = NULL;
    element *tail = NULL;

    if (parse(pkcs7_cert, len_in) == -1) {
        LOGE("Failed to parse PKCS7 data...");
        return -1;
    }

    // We get the 1st certificate
    element *p_cert = getElement("certificates-[optional]", head);
    if (!p_cert) {
        return -1;
    }

    size_t offset = getTagOffset(p_cert, pkcs7_cert);
    if (offset == 0) {
        LOGD("Failed to find offset !");
        return -1;
    }

    *len_out = p_cert->len + offset;
    my_memcpy(data, pkcs7_cert + p_cert->begin - offset, *len_out);

    return 0;
}

