#include "utils.h"

int StringToInt(char *string, int *wasErr) {
    char *endptr = NULL;
    errno = 0;
    int val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX)
        || val == LONG_MIN
        || (errno != 0 && val == 0)
        || endptr == string
        || *endptr != 0
            )
    {
        *wasErr = -1;
    }
    return val;
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
    *currentStatus = NOT_WORKING;
    if (binFd != -1) *currentStatus |= USING_BINARY;
}

int saveData(Data data, int currentStatus, struct timespec refTs) {
    if (currentStatus & USING_REF_POINT) {
        data.ts = refTs;
    }
    else {
        clock_gettime(CLOCK_REALTIME, &(data.ts));
    }

    printf("[%ld %ld]: %f  [%d]\n", data.ts.tv_sec, data.ts.tv_nsec, data.data, data.source);

    return 0;
}

int parseCommand(int *currentStatus, int commandFlags, struct timespec *prevTs, struct timespec *currTs, int commandSignal) {
    if (commandFlags == 0) {
        printf("reset\n");
        resetStatusFlags(currentStatus);
        *prevTs = *currTs;
    }
    else if (commandFlags == 255) {
        printf("info\n");
        union sigval sigval = {.sival_int = *currentStatus};
        sigqueue(infoPid, commandSignal, sigval);
    }
    else {
        if (*currentStatus != NOT_WORKING) {
            *prevTs = *currTs;
        }
        commandFlags -= 1;
        resetStatusFlags(currentStatus);
        *currentStatus |= WORKING;
        int createdRefPoint = 0;

        if (commandFlags & TRUNCATE) {
            printf("truncated\n");
        }
        if (commandFlags & USE_SOURCE_ID) {
            *currentStatus |= USING_SOURCE_ID;
            printf("use source id\n");
        }
        if (commandFlags & PREV_REF_POINT) {
            printf("use prev\n");
            *currentStatus |= USING_REF_POINT;
            createdRefPoint = 1;
            if (prevTs->tv_sec != 0 || prevTs->tv_nsec != 0) {
                *currTs = *prevTs;
            }
            else {
                clock_gettime(CLOCK_MONOTONIC, currTs);
            }
        }
        if (commandFlags & NEW_REF_POINT && !createdRefPoint) {
            printf("use new\n");
            *currentStatus |= USING_REF_POINT;
            clock_gettime(CLOCK_MONOTONIC, currTs);
        }
    }
    return 0;
}
