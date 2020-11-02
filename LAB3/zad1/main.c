#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 100000000


void FillArrayWithRandomDoubles(FILE* urandom, double* arr, size_t size)
{
    fread(arr, sizeof(double), size, urandom);
    int i = 0;
    while (i < size)
    {
        if (fpclassify(arr[i]) != FP_NORMAL)
        {
            fread(&(arr[i]), sizeof(double), 1, urandom);
        }
        else
            i++;
    }
}


int main() {
    const size_t fileSize = 1000 * 1000 * 1000;
    FILE* urandom = fopen("/dev/urandom", "r");
    if (!urandom)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    int fd = open64("test", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IWGRP | S_IWGRP | S_IRGRP | S_IRUSR | S_IROTH);
    if (fd == -1)
    {
        perror("open64");
        exit(EXIT_FAILURE);
    }

    double* val = (double*)calloc(sizeof(double), BUFFER_SIZE);
    if (!val)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    size_t i = 0;
    while (i < fileSize)
    {
        FillArrayWithRandomDoubles(urandom, val, BUFFER_SIZE);
        if (write(fd, val, BUFFER_SIZE * sizeof(double)) <= 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        i += BUFFER_SIZE;
    }

    free(val);
    return 0;
}
