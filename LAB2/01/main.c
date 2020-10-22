#include "function.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define BASE 10

int StringToInt(char *string)
{
    char *endptr = NULL;
    int val = strtol(string, &endptr, BASE);
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

void GatherArguments(int argc, char* argv[], int *dParam, int *kParam, char** filePath)
{
    int option = 0;
    int wasD = 0;
    int wasK = 0;

    while ((option = getopt(argc, argv, "d:k:")) != -1)
    {
        if (option == 'd' && !wasD)
        {
            *dParam = StringToInt(optarg);
            if (*dParam != 1 && *dParam != 2)
            {
                printf("g: Wrong parameter\n");
                exit(EXIT_FAILURE);
            }
            wasD = 1;
        }
        else if (option == 'k' && !wasK)
        {
            *kParam = StringToInt(optarg);
            if (*kParam <= 0)
            {
                printf("g: Wrong parameter\n");
                exit(EXIT_FAILURE);
            }
            wasK = 1;
        }
        else
        {
            printf("Wrong parameter\n");
            exit(EXIT_FAILURE);
        }
    }
    if (!wasD || !wasK || (argc - optind) > 1 || (argc - optind) == 0)
    {
        printf("Usage: %s -d <int> -k <int> <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *filePath = argv[optind];
}

int main(int argc, char* argv[])
{
    int option = 0;
    int dParam = 0;
    int kParam = 0;
    char* filePath = NULL;

    GatherArguments(argc, argv, &dParam, &kParam, &filePath);

    FILE** files = (FILE**)calloc(dParam, sizeof(FILE));
    for (int i = 0; i < dParam; i++)
    {
        files[i] = fopen(filePath, "w");
        if (!(files[i]))
        {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }    
    for (int i = 0; i < kParam; i++)
    {
        if (dParam == 1)
        {
            first(files[0]);
            second(files[0]);
        }
        else if (dParam == 2)
        {
            first(files[0]);
            second(files[1]);
        }
    }


    return 0;
}