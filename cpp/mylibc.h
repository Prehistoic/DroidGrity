#ifndef MYLIBC_H
#define MYLIBC_H

#include <asm/unistd.h> // For syscall ids
#include <unistd.h> // For syscall
#include <stdlib.h> // For malloc, free...
#include <fcntl.h> // For O_RDONLY, O_DIRECTORY, AT_FDCWD
#include <stddef.h> // For some types

int my_open(const char * path, int flags);

int my_openat(int dirfd, const char* path, int flags);

ssize_t my_read(int fd, void* buf, size_t count);

int my_close(int fd);

size_t my_strlcpy(char *dst, const char *src, size_t siz);

size_t my_strlen(const char *s);

int my_strncmp(const char *s1, const char *s2, size_t n);

char* my_strstr(const char *s, const char *find);

int my_memcmp(const void *s1, const void *s2, size_t n);

void * my_memcpy(void *dest, const void *src, size_t len);

char * my_strdup(const char *s);

char * my_strchr(const char *s, int c_in);

char * my_strrchr(const char *s, int c);

char my_tolower(char c);

int my_strcasecmp(const char *s1, const char *s2);

char * my_strcat(char *dest, const char *src);

char * my_strtok(char *str, const char *delim);

#endif //MYLIBC_H
