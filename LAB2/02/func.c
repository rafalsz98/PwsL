
#include "func.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

unsigned long long StringToInt(char *string, char** endptr)
{
    unsigned long long val = strtoull(string, endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX) 
        || val == LONG_MIN 
        || (errno != 0 && val == 0)
        || (endptr != NULL && *endptr == string) 
        )
    {
        printf("Wrong parameter\n");
        exit(EXIT_FAILURE);
    }
    return val;
}

unsigned long long GetBytes(char* string)
{
    char* endptr = NULL;
    unsigned long long res = StringToInt(string, &endptr);
    switch (*endptr)
    {
        case 0:
        case ',':
        case ' ':
        case 'B':
            return res;
        case 'k':
            return res * 1e3;
        case 'K':
            return res * pow(2, 10);
    }
    if (*endptr == 'M' && *(endptr + 1) == 'i')
    {
        return res * pow(2, 20);
    }
    if (*endptr == 'M')
    {
        return res * 1e9;
    }
    printf("Wrong string!\n");
    exit(EXIT_FAILURE);
}

