#ifndef UTILS_H
#define UTILS_H

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 199309
#define _GNU_SOURCE
#include <sys/prctl.h>
#include <ftw.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <getopt.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <alloca.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <ctype.h>

typedef struct DataStruct {
    struct timespec ts;
    float data;
    pid_t source;
} Data;


enum Status {NOT_WORKING, WORKING, USING_REF_POINT, USING_SOURCE_ID = 4, USING_BINARY = 8};
enum CommandFlags {NO_REF, NEW_REF_POINT, PREV_REF_POINT, USE_SOURCE_ID = 4, TRUNCATE = 8};


extern int textFd;
extern int binFd;
extern volatile pid_t infoPid;

int StringToInt(char *string, int *wasErr);
int parseParameters(int argc, char **argv, char **pathToBinary, char **pathToText, int *dataSignal, int *commandSignal);
void resetStatusFlags(int* currentStatus);
int saveData(Data data, int currentStatus, struct timespec refTs);
int parseCommand(int *currentStatus, int commandFlags, struct timespec *prevTs, struct timespec *currTs, int commandSignal);


#endif
