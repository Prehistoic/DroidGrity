#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef MYLIBC_H
#define MYLIBC_H

int my_open(const char* path, int flags) {
    return (int) syscall(AT_FDCWD, SYS_openat, path, flags);
}

ssize_t my_read(int fd, void* buf, size_t count) {
    return (ssize_t) syscall(SYS_read, fd, buf, count);
}

int my_close(int fd) {
    return (int) syscall(SYS_close, fd);
}

__attribute__((always_inline))
static inline size_t my_strlcpy(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;
    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }
    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';		/* NUL-terminate dst */
        while (*s++)
            ;
    }
    return(s - src - 1);	/* count does not include NUL */
}

__attribute__((always_inline))
static inline size_t my_strlen(const char *s)
{
    size_t len = 0;
    while(*s++) len++;
    return len;
}

__attribute__((always_inline))
static inline int my_strncmp(const char *s1, const char *s2, size_t n)
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
static inline char* my_strstr(const char *s, const char *find)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = my_strlen(find);
        do {
            do {
                if ((sc = *s++) == '\0')
                    return (nullptr);
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
void my_memcpy(void *dest, const void *src, size_t len) {
    // Typecast src and dest addresses to (char *)
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    // Copy contents of src[] to dest[]
    for (int i=0; i<len; i++) {
        cdest[i] = csrc[i];
    }
}

#endif //MYLIBC_H
