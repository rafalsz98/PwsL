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

int main(int argc, char* argv[]) {
    int option;
    int wasQ = 0, wasErr = 0;

    while ((option = getopt(argc, argv, "q")) != -1) {
        if (option == 'q' && !wasQ) {
            wasQ = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (argc - optind != 1) {
        wasErr = 1;
    }
    if (wasErr) {
        printf("Usage: %s \n\
            \t-q | [optional, add value to signal]\n\
            \t <process name>\n\
        ", argv[0]);
        return 1;
    }
    char arg[64] = {0};
    if (wasQ) {
        strcpy(arg, "-11");
    }
    else {
        strcpy(arg, "-10");
    }
    execlp("pkill", "pkill", arg, argv[optind], NULL);
}