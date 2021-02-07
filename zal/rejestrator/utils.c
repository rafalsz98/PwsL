#include "utils.h"

#define ERROR_CHECK(func) if((func)==-1){return -1;}

int StringToInt(char *string, int *wasErr) {
    char *endptr = NULL;
    errno = 0;
    long val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)
        || endptr == string
        || *endptr != 0
    ) {
        *wasErr = -1;
    }
    return (int)val;
}

int parseParameters(int argc, char **argv, char **pathToBinary, char **pathToText, int *dataSignal, int *commandSignal) {
    int option = 0;
    int wasB = 0, wasT = 0, wasD = 0, wasC = 0, wasErr = 0;

    while ((option = getopt(argc, argv, "b:t:d:c:")) != -1) {
        if (wasErr) break;
        if (option == 'b' && !wasB) {
            *pathToBinary = optarg;
            wasB = 1;
        }
        else if (option == 't' && !wasT) {
            *pathToText = strcmp(optarg, "-") == 0 ? NULL : optarg;
            wasT = 1;
        }
        else if (option == 'd' && !wasD) {
            *dataSignal = StringToInt(optarg, &wasErr);
            if (*dataSignal < SIGRTMIN || *dataSignal > SIGRTMAX) wasErr = 1;
            wasD = 1;
        }
        else if (option == 'c' && !wasC) {
            *commandSignal = StringToInt(optarg, &wasErr);
            if (*commandSignal < SIGRTMIN || *commandSignal > SIGRTMAX) wasErr = 1;
            wasC = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (*dataSignal == *commandSignal || wasErr || !wasC || !wasD) return -1;
    return 0;
}

void resetStatusFlags(int* currentStatus) {
    *currentStatus = binFd == -1 ? NOT_WORKING : USING_BINARY;
}

int saveData(Data data, int currentStatus, struct timespec refTs) {
    char timestamp[256] = {0};
    if (currentStatus & USING_REF_POINT) {
        struct timespec currTs = {0};
        clock_gettime(CLOCK_MONOTONIC, &currTs);
        struct timespec delta = {
                .tv_sec = (currTs.tv_sec - refTs.tv_sec),
                .tv_nsec = (currTs.tv_nsec - refTs.tv_nsec)
        };
        if (delta.tv_nsec < 0) {
            delta.tv_sec -= 1;
            delta.tv_nsec += 1e9;
        }
        data.ts = delta;

        int hour = delta.tv_sec / 3600;
        delta.tv_sec -= (hour * 3600);
        int minutes = delta.tv_sec / 60;
        delta.tv_sec -= (minutes * 60);
        int seconds = delta.tv_sec;
        int miliseconds = delta.tv_nsec / 1e6;
        snprintf(timestamp, 256, "[%02d:%02d:%02d.%03d]", hour, minutes, seconds, miliseconds);
    }
    else {
        clock_gettime(CLOCK_REALTIME, &(data.ts));
        struct tm* tm = localtime(&(data.ts.tv_sec));
        int miliseconds = data.ts.tv_nsec / 1e6;
        snprintf(timestamp, 256, "[%02d/%02d/%d %02d:%02d:%02d.%03d]",
                 tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
                 tm->tm_hour, tm->tm_min, tm->tm_sec, miliseconds
        );
    }

    if (currentStatus & USING_SOURCE_ID) {
        // Can print to file thanks to dup2
        if (printf("%s: %f  [%d]\n", timestamp, data.data, data.source) < 0) return -1;
    }
    else {
        data.source = 0;
        if (printf("%s: %f\n", timestamp, data.data) < 0) return -1;
    }

    if (currentStatus & USING_BINARY) {
        ERROR_CHECK(write(binFd, (void*)&data, sizeof(data)));
    }

    return 0;
}

int parseCommand(int *currentStatus, int commandFlags, struct timespec *prevTs, struct timespec *currTs, int commandSignal) {
    if (commandFlags == 0) {
        resetStatusFlags(currentStatus);
    }
    else if (commandFlags == 255) {
        union sigval sigval = {.sival_int = *currentStatus};
        sigqueue(infoPid, commandSignal, sigval); // not checking if successed on purpose
    }
    else {
        commandFlags -= 1;
        resetStatusFlags(currentStatus);
        *currentStatus |= WORKING;
        int createdRefPoint = 0;

        if (commandFlags & TRUNCATE) {
            errno = 0;
            if (*currentStatus & USING_BINARY) {
                if (ftruncate(binFd, 0) == -1 && errno != EINVAL) return -1;
                errno = 0;
            }
            if (ftruncate(textFd, 0) == -1 && errno != EINVAL) return -1;
        }
        if (commandFlags & USE_SOURCE_ID) {
            *currentStatus |= USING_SOURCE_ID;
        }
        if (commandFlags & PREV_REF_POINT) {
            *currentStatus |= USING_REF_POINT;
            createdRefPoint = 1;
            struct timespec temp = *prevTs;
            *prevTs = *currTs;
            *currTs = temp;

            if (currTs->tv_sec == 0 && currTs->tv_nsec == 0) {
                ERROR_CHECK(clock_gettime(CLOCK_MONOTONIC, currTs));
            }
        }
        if (commandFlags & NEW_REF_POINT && !createdRefPoint) {
            *currentStatus |= USING_REF_POINT;
            *prevTs = *currTs;
            ERROR_CHECK(clock_gettime(CLOCK_MONOTONIC, currTs));
        }
    }
    return 0;
}
