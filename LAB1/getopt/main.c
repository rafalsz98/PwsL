#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

#define BASE 10

double StringToDouble(char *string, int *errors)
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

int StringToInt(char *string, int *isDouble, int *errors)
{
    char *endptr = NULL;
    int val = strtol(string, &endptr, BASE);
    if ((errno == ERANGE && val == FLT_MAX) || val == FLT_MIN || (errno != 0 && val == 0))
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

// Program takes any number of parameters -d <int> and -f <float>, and arguments of type
// int and float. It counts every incorrect parameter/argument and gives as output
// mean of every correct element, grouping it.
int main(int argc, char *argv[])
{
    int option;
    int intSum = 0;
    double doubleSum = 0;
    int intArgumentCount = 0;
    int intParameterCount = 0;
    int doubleArgumentCount = 0;
    int doubleParameterCount = 0;

    // Error control
    int intParameterErrors = 0;
    int intArgumentErrors = 0;
    int doubleArgumentErrors = 0;
    int doubleParameterErrors = 0;
    // ---

    int intTempResult = 0;
    double doubleTempResult = 0;

    while ((option = getopt(argc, argv, "d:f:")) != -1)
    {
        if (option == 'd')
        {
            int errors = 0;
            intTempResult = StringToInt(optarg, NULL, &errors);
            if (errors == 0)
            {
                intSum += intTempResult;
                intParameterCount++;
            }
            else
                intParameterErrors++;
        }
        else if (option == 'f')
        {
            int errors = 0;
            doubleTempResult = StringToDouble(optarg, &errors);
            if (errors == 0)
            {
                doubleSum += doubleTempResult;
                doubleParameterCount++;
            }
            else
                doubleParameterErrors++;
        }
        else
        {
            perror("Wrong parameter");
            exit(EXIT_FAILURE);
        }
    }

    while (optind < argc)
    {
        int isDouble = 0;
        int resInt = 0;
        double resDouble = 0;
        int errors = 0;
        resInt = StringToInt(argv[optind], &isDouble, &errors);
        if (errors == 0)
        {
            if (isDouble)
            {
                errors = 0;
                resDouble = StringToDouble(argv[optind], &errors);
                if (errors == 0)
                {
                    doubleSum += resDouble;
                    doubleArgumentCount++;
                }
                else
                    doubleArgumentErrors++;
            }
            else
            {
                intSum += resInt;
                intArgumentCount++;
            }
        }
        else
            intArgumentErrors++;
        optind++;
    }
    printf("----Stats----\n");
    printf("Valid int parameters: %d\n", intParameterCount);
    printf("Valid int arguments: %d\n", intArgumentCount);
    printf("Valid double parameters: %d\n", doubleParameterCount);
    printf("Valid double arguments: %d\n", doubleArgumentCount);
    printf("Int mean: %f\n", ((double)intSum / (intArgumentCount + intParameterCount)));
    printf("Double mean: %f\n", (doubleSum / (doubleArgumentCount + doubleParameterCount)));
    printf("\n----Errors stats----\n");
    printf("Invalid int parameters: %d\n", intParameterErrors);
    printf("Invalid int arguments: %d\n", intArgumentErrors);
    printf("Invalid double parameters: %d\n", doubleParameterErrors);
    printf("Invalid double arguments: %d\n", doubleArgumentErrors);

    return 0;
}
