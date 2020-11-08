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

typedef struct ListStruct {
    struct ListStruct* next;
    struct ListStruct* prev;
    off_t offsetLeft;
    off_t offsetRight;
    unsigned char c;
} List;

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

    // Find all data
    List head;
    head.prev = NULL;
    head.next = NULL;
    head.offsetLeft = -1;
    head.offsetRight = -1;
    head.c = -1;

    List* it = &head;

    unsigned char* buffer = (unsigned char*)calloc(sizeof(unsigned char), blockSize);
    off_t offset = 0;
    while ((offset = lseek(fd, offset, SEEK_DATA)) != -1)
    {
        read(fd, buffer, blockSize);
        int i = 0;
        while (buffer[i] == 0)
            i++;
        if (i == blockSize)
            exit(EXIT_FAILURE);
        it->next = (List*)calloc(sizeof(List), 1);
        it->next->prev = it;
        it = it->next;
        it->next = NULL;
        it->c = buffer[i];
        it->offsetLeft = offset + i;

        while (buffer[i] == it->c)
            i++;
        it->offsetRight = offset + i;

        offset = lseek(fd, offset, SEEK_HOLE);
    }
    free(buffer);
    if (direction == ambo)
    {
        secondArgument = 1;
        secondSize = size;
        direction = ante;
        secondDirection = post;
    }

    // Rozrost
    it = &head;
    while (it->next)
    {
        it = it->next;
        off_t startOffset = 0;
        if (!secondArgument && direction != ambo)
        {
            if (direction == ante)
            {
                if ((startOffset = lseek(fd, it->offsetLeft - size, SEEK_SET)) == -1)
                    startOffset = lseek(fd, 0, SEEK_SET);
            }
            else if (direction == post)
            {
                startOffset = lseek(fd, it->offsetRight, SEEK_SET);
            }
            for (blksize_t i = 0; i < size; i++)
            {
                int n = write(fd, &(it->c), 1);
                if (n == -1)
                    exit(EXIT_FAILURE);
            }
        }
        else
        {
            blksize_t anteSize = 0, postSize = 0;
            if (direction == ante)
            {
                anteSize = size;
                postSize = secondSize;
            }
            else
            {
                anteSize = secondSize;
                postSize = size;
            }
            off_t localOffsetLeft = it->offsetLeft - anteSize;
            if (it->prev != &head)
            {
                if (it->prev->offsetRight + postSize > it->offsetLeft - anteSize)
                {
                    localOffsetLeft = it->offsetLeft - (anteSize + postSize) / 2;
                }
            }
            if (lseek(fd, localOffsetLeft, SEEK_SET) == -1)    
                lseek(fd, 0, SEEK_SET);
            for (blksize_t i = 0; i < it->offsetLeft - localOffsetLeft; i++)
            {
                int n = write(fd, &(it->c), 1);
                if (n == -1)
                    exit(EXIT_FAILURE);
            }

            lseek(fd, it->offsetRight, SEEK_SET);
            if (it->next)
            {
                if (it->next->offsetLeft - anteSize < it->offsetRight + postSize)
                {
                    postSize = (anteSize + postSize) / 2;
                }
            }
            for (blksize_t i = 0; i < postSize; i++)
            {
                int n = write(fd, &(it->c), 1);
                if (n == -1)
                    exit(EXIT_FAILURE);
            }
        }
    }

    close(fd);
}