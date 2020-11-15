#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

void printCount(unsigned char startingChar, blksize_t count)
{
    if (!isprint(startingChar))
        printf("%d: %ld\n", startingChar, count);
    else
        printf("'%c': %ld\n", startingChar, count);
}

int main(int argc, char* argv[])
{
    char c;
    char *path;
    int wasArg = 0;
    int err = 0;
    while ((c = getopt(argc, argv, "!")) != -1)
    {
        if (c == '!' && !wasArg)
        {
            wasArg = 1;
        }
        else
        {
            err = 1;
        }
    }
    if (err || argc - optind != 1)
    {
        printf("Usage: %s [-!] path\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    path = argv[optind];

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat st = {0};
    stat(path, &st);
    blksize_t blockSize = st.st_blksize;
    unsigned char* buffer = (unsigned char*)calloc(sizeof(unsigned char), blockSize);

    off_t prevOffset = lseek(fd, 0, SEEK_SET);
    if (!wasArg)
    {
        while ((prevOffset = lseek(fd, prevOffset, SEEK_DATA)) != -1)
        {
            read(fd, buffer, blockSize);
            unsigned char startingChar = buffer[0];
            blksize_t count = 1;
            for (int i = 1; i < blockSize; i++)
            {
                if (buffer[i] == startingChar)
                    count++;
                else
                {
                    printCount(startingChar, count);
                    startingChar = buffer[i];
                    count = 1;
                }
            }
            printCount(startingChar, count);
            prevOffset = lseek(fd, prevOffset, SEEK_HOLE);
        }
    }
    else
    {
        // Associative array, initialized with 0's
        int array[256] = {0};
        while (read(fd, buffer, blockSize) > 0) 
        {
            for (int i = 0; i < blockSize; i++) 
            {
                array[buffer[i]]++;
            }
        }
        for (int i = 0; i < 256; i++)
        {
            if (array[i] != 0)
            {
                printCount(i, array[i]);
            }
        }
    }

    free(buffer);
}