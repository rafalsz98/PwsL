#include "utils.h"

#define ERROR_CHECK(func, string) if(func==-1){return -1;}

int StringToInt(char *string, int *wasErr) {
    char *endptr = NULL;
    int val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX)
        || val == LONG_MIN
        || (errno != 0 && val == 0)
        || endptr == string
        || *endptr != 0
            )
    {
        *wasErr = 1;
    }
    return val;
}

double StringToDouble(char *string, int *wasErr) {
    char *endptr = NULL;
    double val = strtod(string, &endptr);
    if ((errno == ERANGE && val == FLT_MAX)
        || val == FLT_MIN
        || (errno != 0 && val == 0)
        || endptr == string
        || *endptr != 0
            )
    {
        *wasErr = 1;
    }
    return val;
}

int parseParameters(int argc, char* argv[], double* productionPace, struct sockaddr_in* sockaddrIn)
{
    int option = 0;
    int wasP = 0, wasErr = 0;
    while ((option = getopt(argc, argv, "p:")) != -1) {
        if (option == 'p' && !wasP) {
            *productionPace = StringToDouble(optarg, &wasErr)
            wasP = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (argc - optind != 1 || !wasP) {
        wasErr = 1;
    }

    if (wasErr) {
        printf("Usage: %s \n\
        \t-p <float> | pace of material production (in 2662 B/s)\n\
        \t[<addr>:]port\n\
        ", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse IP
    sockaddrIn->sin_addr = AF_INET;
    char* port = 0;
    char* token = strtok(argv[optind], ":");
    if (!token) { // default addr
        if (inet_pton(AF_INET, "127.0.0.1", &(sockaddrIn->sin_addr)) != 1) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }
        port = strtok(NULL, ":");
    }
    else { // custom addr
        if (inet_pton(AF_INET, token, &(sockaddrIn->sin_addr)) != 1) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }
        port = argv[optind];
    }

    // Parse port



}

