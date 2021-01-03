#define _POSIX_C_SOURCE 199309
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <getopt.h>

int StringToInt(char *string, int *wasErr)
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

int main(int argc, char* argv[]) {
    int option;
    int wasI = 0, wasErr = 0;
    double interval;
    int pid;

    while ((option = getopt(argc, argv, "i:")) != -1) {
        if (option == 'i' && !wasI) {
            interval = StringToDouble(optarg, &wasErr);
            wasI = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (!wasI || argc - optind != 1) {
        wasErr = 1;
    }
    if (!wasErr) {
        pid = StringToInt(argv[optind], &wasErr);
    }
    if (wasErr) {
        printf("Usage: %s \n\
            \t-i <float | interval between signals>\n\
            \t <int | process pid>\n\
        ", argv[0]);
        return 1;
    }
    
    // sleep time
    int sec = interval;
    int nsec = (interval - sec) * 1e9;
    struct timespec ts = {sec, nsec};


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (;;) {
        if (kill(pid, SIGUSR1) == -1) {
            printf("Process with PID: %d doesnt exist or no permission", pid);
            exit(EXIT_FAILURE);
        }
        nanosleep(&ts, NULL);
    }
#pragma clang diagnostic pop

}