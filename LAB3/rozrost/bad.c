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

enum Direction {ante, post, ambo};

unsigned long long StringToLongLongWithEndPointer(char *string, char** endptr)
{
    unsigned long long val = strtoull(string, endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX) 
        || val == LONG_MIN 
        || (errno != 0 && val == 0)
        || (endptr != NULL && *endptr == string) 
        )
    {
        return -1;
    }
    return val;
}

unsigned long long GetBytes(char* string)
{
    char* endptr = NULL;
    unsigned long long res = StringToLongLongWithEndPointer(string, &endptr);
    switch (*endptr)
    {
        case 0:
        case 'B':
            return res;
        case 'K':
            return res * 1024;
    }
    if (*endptr == 'b' && *(endptr + 1) == 'b')
    {
        return res * 512;
    }
    
    return -1;
}

int ParseExpansionArg(char* arg, unsigned long long* size)
{
    char* expansion = strtok(arg, ":");
    int direction = -1;
    if (!expansion)
        return -1;
    if (strcmp(expansion, "ante") == 0)
        direction = ante;
    else if (strcmp(expansion, "post") == 0)
        direction = post;
    else if (strcmp(expansion, "ambo") == 0)
        direction = ambo;
    else
        return -1;
    
    expansion = strtok(NULL, ":");

    *size = GetBytes(expansion);
    return direction;
}

int main(int argc, char* argv[])
{
    char c;
    char *path;
    int wasS = 0, argErr = 0;
    while ((c = getopt(argc, argv, "s:")) != -1)
    {
        if (c == 's' && !wasS)
        {
            path = optarg;
            wasS = 1;
        }
        else
        {
            argErr = 1;
        }
    }

    unsigned long long size = -1;
    int direction = -1;
    if (optind != argc)
        direction = ParseExpansionArg(argv[optind++], &size);
    if (direction == -1 || size == -1)
    {
        argErr = 1;
    }

    int secondArgument = 0;
    int secondDirection = -1;
    unsigned long long secondSize = -1;
    if (optind != argc)
    {
        secondArgument = 1;
        secondDirection = ParseExpansionArg(argv[optind], &secondSize);
        if (secondDirection == -1 || secondSize == -1 || secondDirection == direction || secondDirection == ambo)
        {
            argErr = 1;
        }
    }

    if (!wasS || argErr)
    {
        printf("Usage: %s -s <path> <expansion> [<expansion>]\n", argv[0]);
        printf("expansion is in format: <ante|post|ambo>:<size>\n");
        printf("size can have additional suffix: B(byte, default)|bb(block)|K(kilobyte)\n");
        exit(EXIT_FAILURE);
    }

    int fd = open(path, O_RDWR);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat st = {0};
    stat(path, &st);
    blksize_t blockSize = st.st_blksize;
    unsigned char* buffer = (unsigned char*)calloc(sizeof(unsigned char), blockSize);
    off_t offset = lseek(fd, 0, SEEK_DATA);
    while (offset != -1)
    {
        off_t currentOffset = offset;
        read(fd, buffer, blockSize);
        blksize_t i = 0;
        for (i; i < blockSize; i++)
        {
            if (buffer[i] != 0)
                break;
        }
        if (i == blockSize)
            exit(EXIT_FAILURE);
        offset = lseek(fd, offset, SEEK_HOLE);
        offset = lseek(fd, offset, SEEK_DATA);

        if (direction == post)
            lseek(fd, currentOffset + i, SEEK_SET);
        else if (direction == ante || direction == ambo)
        {
            if (lseek(fd, currentOffset + i - size, SEEK_SET) == -1)    
                lseek(fd, 0, SEEK_SET);
        }
        for (blksize_t j = 0; j < size; j++)
        {
            int n = write(fd, &(buffer[i]), 1);
            if (n == -1)
                break;
        }
        if (direction == ambo || secondArgument)
        {
            if (direction == ambo || secondDirection == post)
                lseek(fd, currentOffset + i, SEEK_SET);
            else if (secondDirection == ante)
            {
                if (lseek(fd, currentOffset + i - size, SEEK_SET) == -1)    
                    lseek(fd, 0, SEEK_SET);
            }
            for (blksize_t j = 0; j < size; j++)
            {
                int n = write(fd, &(buffer[i]), 1);
                if (n == -1)
                    break;
            }
        }
        lseek(fd, offset, SEEK_SET);
    }



    free(buffer);
}