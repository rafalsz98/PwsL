#include "function.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void first(FILE* file)
{
    static double val = 0.625;
    const int sizeOfBuffer = 10;
    for (int i = 0; i < 5; i++)
    {
        char buffer[sizeOfBuffer];
        if (snprintf(buffer, sizeOfBuffer, "%.3f\n", val) < 0)
        {
            perror("snprintf");
            exit(EXIT_FAILURE);
        }
        if (fwrite(buffer, strlen(buffer), 1, file) <= 0)
        {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        val *= 0.625;
    }
}

void second(FILE* file)
{
    static short int val = 0;
    const int sizeOfBuffer = 1024;
    char buffer[sizeOfBuffer];
    for (int i = 0; i < 19; i++)
    {
        int realSize = 0;
        if (realSize = snprintf(buffer, sizeOfBuffer, "%d\n", val) < 0)
        {
            perror("snprintf");
            exit(EXIT_FAILURE);
        }
        if (fwrite(buffer, strlen(buffer), sizeof(char), file) <= 0)
        {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        val += 80;
    }
}