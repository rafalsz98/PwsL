#define _GNU_SOURCE
#include <unistd.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int StringToInt(char *string, int* err)
{
    if (!string) {
        *err = 1;
        return -1;
    }
    char *endptr = NULL;
    int val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX) 
        || val == LONG_MIN 
        || (errno != 0 && val == 0)
        || endptr == string 
        || *endptr != 0
        )
    {
        *err = 1;
        return -1;
    }
    return val;
}

double StringToDouble(char *string, int* err)
{
    if (!string) {
        *err = 1;
        return -1;
    }
    char *endptr = NULL;
    double val = strtod(string, &endptr);
    if ((errno == ERANGE && val == FLT_MAX) 
        || val == FLT_MIN 
        || (errno != 0 && val == 0)
        || endptr == string
        || *endptr != 0
        )
    {
        *err = 1;
        return -1;
    }
    return val;
}

int CheckDescriptors(int* fds, int fdSize, int* openedDescriptors, int initDescriptors) {
    for (int i = 0; i < fdSize; i++) {
        if (fds[i] == -1) {
            continue;
        }

        struct stat st = {0};
        if (fstat(fds[i], &st) == -1) {
            perror("fstat");
            exit(EXIT_FAILURE);
        }
        int flags = fcntl(fds[i], F_GETFL);
        if (flags == -1) {
            perror("fcntl");
            exit(EXIT_FAILURE);
        }
        int accessMode = flags & O_ACCMODE;
        if (
            !S_ISFIFO(st.st_mode) ||
            (i == 0 && accessMode != O_WRONLY) ||
            (i != 0 && accessMode != O_RDONLY)
        ) {
            close(fds[i]);
            (*openedDescriptors)--;
            fds[i] = -1;
        }
        else if (initDescriptors) {
            flags |= O_NONBLOCK;
            if (fcntl(fds[i], F_SETFL, flags) == -1) {
                perror("fcntl");
                exit(EXIT_FAILURE);
            }
        }
    }
    return (*openedDescriptors > 0 && fds[0] != -1) ? 1 : 0;
}

int main(int argc, char* argv[]) {
    int option = 0;
    int wasC = 0, wasErr = 0;
    int controlFd = -1;
    while ((option = getopt(argc, argv, "c:")) != -1) {
        if (option == 'c' && !wasC) {
            controlFd = StringToInt(optarg, &wasErr);
            wasC = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (argc == optind || !wasC) {
        wasErr = 1;
    }
    // Arguments init
    int fdSize = argc - optind + 1;
    int* fds = (int*)calloc(fdSize, sizeof(int));
    fds[0] = controlFd;
    for (int i = 1; i < fdSize && !wasErr; i++) {
        fds[i] = StringToInt(argv[optind], &wasErr);
        optind++;
    }

    if (wasErr) {
        printf("Usage: %s \n\
            \t-c <int of control stream descriptor>\n\
            \tlist of ints (stream descriptors)\n\
        ", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Env variables init
    int envErr = 0;
    double d_wait = StringToDouble(getenv("D_WAIT"), &envErr);
    if (envErr) {
        d_wait = 0.5;
        envErr = 0;
    }
    int t_read = StringToInt(getenv("T_READ"), &envErr);
    if (envErr) {
        t_read = 3;
        envErr = 0;
    }
    int c_read = StringToInt(getenv("C_READ"), &envErr);
    if (envErr) {
        c_read = 16;
        envErr = 0;
    }

    int openedDescriptors = fdSize;
    int init = 1;
    while (CheckDescriptors(fds, fdSize, &openedDescriptors, init)) {
        init = 0;
        // check if any buffer is full, then clear it
        for (int i = 1; i < fdSize; i++) {
            if (fds[i] == -1) {
                continue;
            }
            int size;
            ioctl(fds[i], FIONREAD, &size);
            int buffer_size = fcntl(fds[i], F_GETPIPE_SZ);
            printf("%d: %d   %d\n", fds[i], size, buffer_size);
            if (size == buffer_size) {
                printf("fd: %d full\n", fds[i]);
                return 0;
            }
        }
        sleep(d_wait);
    }

    printf("No more descriptors\n");
    
    printf("end\n");
    exit(EXIT_SUCCESS);
}