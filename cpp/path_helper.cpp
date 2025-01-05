/*

The MIT License (MIT)

Copyright (c) 2018  Dmitrii Kozhevin <kozhevin.dima@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the “Software”), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#include "path_helper.h"

static const char * getFilenameExt(const char *filename) {
    const char *dot = my_strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

static char * getPathFromEntry(const char * line, char * packageName) {
    char path[PATH_SIZE];

    char * copied_line = my_strdup(line);
    const char * delim = " ";

    int field = 0;
    char * token = my_strtok(copied_line, delim);
    while (token != NULL) {
        if (field == 5) { // The 6th field is the path
            my_strlcpy(path, token, PATH_SIZE - 1);
            path[PATH_SIZE - 1] = '\0';

            if (my_strstr(path, packageName)) {
                char * last_slash = my_strrchr(path, '/');
                char * bname = last_slash ? last_slash + 1 : path;
                
                if (my_strcasecmp(getFilenameExt(bname), "apk") == 0) {
                    return my_strdup(path);
                }
            }
            break;
        }

        token = my_strtok(NULL, delim); // We have to pass NULL due to our custom strtok implementation !
        field++;
    }

    return NULL;
}

char * getApkPath(char * packageName) {
    // Open the /proc/self directory
    int dir_fd = my_open("/proc/self", O_RDONLY | O_DIRECTORY);
    if (dir_fd == -1) {
        return NULL;
    }

    // Open the maps file using openat
    int fd = my_openat(dir_fd, "maps", O_RDONLY);
    my_close(dir_fd); // Close directory file descriptor
    if (fd == -1) {
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;
    std::string current_line = "";
    char * path;

    // Read from the file descriptor directly
    while ((bytes_read = my_read(fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the buffer

        // Process the content of the buffer line by line
        for (int i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n' || buffer[i] == '\r') {
                path = getPathFromEntry(current_line.c_str(), packageName);
                current_line.clear();  // Reset the line for the next one

                if (path) {
                    break;
                }
            } else {
                current_line.push_back(buffer[i]);  // Append non-line-break characters to the current line
            }
        }

        // If there's any data left in `line`, it means the last part wasn't terminated by a newline
        if (!current_line.empty()) {
            path = getPathFromEntry(current_line.c_str(), packageName);
        }

        if (path) {
            break;
        }
    }

    my_close(fd); // Close file descriptor

    if (path) {
        return my_strdup(path);
    }

    return NULL;
}