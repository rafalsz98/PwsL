#ifndef FUNCTION_BASE
#define FUNCTION_BASE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <float.h>


double StringToDoubleWithErrorControl(char *string, int *errors)
{
    char *endptr = NULL;
    double val = strtod(string, &endptr);
    if ((errno == ERANGE && val == FLT_MAX) || val == FLT_MIN || (errno != 0 && val == 0))
    {
        (*errors)++;
        return 0;
    }
    if (endptr == string || *endptr != 0)
    {
        (*errors)++;
        return 0;
    }
    return val;
}

int StringToIntWithErrorControlAndDoubleDetection(char *string, int *isDouble, int *errors)
{
    char *endptr = NULL;
    int val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX) || val == LONG_MIN || (errno != 0 && val == 0))
    {
        (*errors)++;
        return 0;
    }
    if (endptr == string || (*endptr != 0 && *endptr != '.'))
    {
        (*errors)++;
        return 0;
    }
    else if (*endptr == '.' && isDouble)
    {
        *isDouble = 1;
    }
    return val;
}

int StringToInt(char *string)
{
    char *endptr = NULL;
    int val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX) 
        || val == LONG_MIN 
        || (errno != 0 && val == 0)
        || endptr == string 
        || *endptr != 0
        )
    {
        printf("Wrong parameter\n");
        exit(EXIT_FAILURE);
    }
    return val;
}

unsigned long long StringToLongLongWithEndPointer(char *string, char** endptr)
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
    unsigned long long res = StringToLongLongWithEndPointer(string, &endptr);
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

off_t GetRandomOffset(size_t max, off_t *arr, int sizeOfArr, int i)
{
    while (1)
    {
        int boolean = 0;
        off_t val = rand() % max;
        for (int j = 0; j < sizeOfArr; j++)
        {
            if (arr[j] == val)
                boolean = 1;
        }
        if (!boolean)
        {
            arr[i] = val;
            return val;
        }
    }
}

#endif