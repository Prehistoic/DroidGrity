#include "mylibc.h"

int my_open(const char * path, int flags) {
    return (int) syscall(__NR_open, path, flags);
}

int my_openat(int dirfd, const char* path, int flags) {
    return (int) syscall(__NR_openat, dirfd, path, flags);
}

ssize_t my_read(int fd, void* buf, size_t count) {
    return (ssize_t) syscall(__NR_read, fd, buf, count);
}

int my_close(int fd) {
    return (int) syscall(__NR_close, fd);
}

off_t my_lseek(int fd, off_t offset, int whence) {
    return (off_t) syscall(__NR_lseek, fd, offset, whence);
}

__attribute__((always_inline))
size_t my_strlcpy(char *dst, const char *src, size_t size)
{
    char *d = dst;
    const char *s = src;
    size_t n = size;
    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }
    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (size != 0)
            *d = '\0';		/* NUL-terminate dst */
        while (*s++)
            ;
    }
    return(s - src - 1);	/* count does not include NUL */
}

__attribute__((always_inline))
size_t my_strlen(const char *s)
{
    size_t len = 0;
    while(*s++) len++;
    return len;
}

__attribute__((always_inline))
int my_strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return (0);
    do {
        if (*s1 != *s2++)
            return (*(unsigned char *)s1 - *(unsigned char *)--s2);
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return (0);
}

__attribute__((always_inline))
char* my_strstr(const char *s, const char *find)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = my_strlen(find);
        do {
            do {
                if ((sc = *s++) == '\0')
                    return NULL;
            } while (sc != c);
        } while (my_strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

__attribute__((always_inline))
int my_memcmp(const void *s1, const void *s2, size_t n)
{
    if (n != 0) {
        const auto *p1 = (const unsigned char *)s1;
        const auto *p2 = (const unsigned char *)s2;

        do {
            if (*p1++ != *p2++)
                return (*--p1 - *--p2);
        } while (--n != 0);
    }
    return (0);
}

__attribute__((always_inline))
void * my_memcpy(void *dest, const void *src, size_t len) {
    // Typecast src and dest addresses to (char *)
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    // Copy contents of src[] to dest[]
    for (int i=0; i<len; i++) {
        cdest[i] = csrc[i];
    }

    return dest;
}

__attribute__((always_inline))
char * my_strdup(const char *s)
{
    size_t len = my_strlen (s) + 1;
    void * newstr = malloc(len);

    if (newstr == NULL)
        return NULL;

    return (char *) my_memcpy(newstr, s, len);
}

__attribute__((always_inline))
char * my_strchr(const char *s, int c_in)
{
    const unsigned char *char_ptr;
    const unsigned long int *longword_ptr;
    unsigned long int longword, magic_bits, charmask;
    unsigned char c;

    c = (unsigned char) c_in;

    for (char_ptr = (const unsigned char *) s;
        ((unsigned long int) char_ptr & (sizeof (longword) - 1)) != 0;
        ++char_ptr)
        if (*char_ptr == c)
        return (char *) char_ptr;
        else if (*char_ptr == '\0')
        return NULL;

    longword_ptr = (unsigned long int *) char_ptr;

    magic_bits = -1;
    magic_bits = magic_bits / 0xff * 0xfe << 1 >> 1 | 1;

    charmask = c | (c << 8);
    charmask |= charmask << 16;
    if (sizeof (longword) > 4)
        charmask |= (charmask << 16) << 16;
    if (sizeof (longword) > 8)
        abort ();

    
    for (;;)
        {

        longword = *longword_ptr++;

        if ((((longword + magic_bits)

            ^ ~longword)

        & ~magic_bits) != 0 ||

        ((((longword ^ charmask) + magic_bits) ^ ~(longword ^ charmask))
        & ~magic_bits) != 0)
        {

        const unsigned char *cp = (const unsigned char *) (longword_ptr - 1);

        if (*cp == c)
            return (char *) cp;
        else if (*cp == '\0')
            return NULL;
        if (*++cp == c)
            return (char *) cp;
        else if (*cp == '\0')
            return NULL;
        if (*++cp == c)
            return (char *) cp;
        else if (*cp == '\0')
            return NULL;
        if (*++cp == c)
            return (char *) cp;
        else if (*cp == '\0')
            return NULL;
        if (sizeof (longword) > 4)
            {
            if (*++cp == c)
            return (char *) cp;
            else if (*cp == '\0')
            return NULL;
            if (*++cp == c)
            return (char *) cp;
            else if (*cp == '\0')
            return NULL;
            if (*++cp == c)
            return (char *) cp;
            else if (*cp == '\0')
            return NULL;
            if (*++cp == c)
            return (char *) cp;
            else if (*cp == '\0')
            return NULL;
            }
        }
        }

    return NULL;
}

__attribute__((always_inline))
char * my_strrchr(const char *s, int c)
{
    const char *found, *p;

    c = (unsigned char) c;

    if (c == '\0')
        return my_strchr (s, '\0');

    found = NULL;
    while ((p = my_strchr (s, c)) != NULL)
        {
        found = p;
        s = p + 1;
        }

    return (char *) found;
}

__attribute__((always_inline))
char my_tolower(char c) {
    // Check if the character is uppercase
    if (c >= 'A' && c <= 'Z') {
        // Convert to lowercase by adding the difference between 'a' and 'A'
        return c + ('a' - 'A');
    }
    // If it's not uppercase, return the character as is
    return c;
}

__attribute__((always_inline))
int my_strcasecmp(const char *s1, const char *s2)
{
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;
    int result;

    if (p1 == p2)
        return 0;

    while ((result = my_tolower(*p1) - my_tolower(*p2++)) == 0)
        if (*p1++ == '\0')
        break;

    return result;
}

__attribute__((always_inline))
char * my_strcat(char *dest, const char *src)
{
  my_strlcpy(dest + my_strlen(dest), src, my_strlen(src));
  return dest;
}

__attribute__((always_inline))
char* my_strtok(char *str, const char *delim) {
    static char* last = NULL;  // Static variable to store the position between calls

    if (str) {
        last = str;  // On the first call, save the string
    }

    // If there's no string to process, return NULL
    if (!last) {
        return NULL;
    }

    // Skip over leading delimiters
    while (*last && my_strchr(delim, *last)) {
        last++;
    }

    // If we've reached the end of the string, return NULL
    if (*last == '\0') {
        last = NULL;
        return NULL;
    }

    // Mark the beginning of the token
    char* start = last;

    // Find the next delimiter or end of string
    while (*last && !my_strchr(delim, *last)) {
        last++;
    }

    // If a delimiter was found, terminate the token with a null character
    if (*last) {
        *last = '\0';
        last++;  // Move past the delimiter for the next call
    } else {
        last = NULL;  // End of string
    }

    return start;
}