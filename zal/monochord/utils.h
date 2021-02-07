#ifndef UTILS_H
#define UTILS_H

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 199309
#define _GNU_SOURCE
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
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <ctype.h>

#define MAX_UDP_CONNECTIONS 10
#define UDP_BUFFER_SIZE 256

typedef struct CommandsStruct {
    float amp;
    float freq;
    float probe;
    float period;
    short pid;
    short rt;

} Commands;

extern int probingStopped;
extern int lastSendSuccessfulPID;
extern int lastSendSuccessfulRT;

int StringToInt(char *string, int *wasErr);
double StringToDouble(char *string, int *wasErr);
int parseParameters(int argc, char* argv[], in_port_t* port);
int setupUDP(in_port_t port);
int setupTimer(int signal, int sigValue, timer_t* timerId);
int launchTimer(timer_t timerId, long seconds, long nanoseconds, int isOneTimer);
int stopTimer(timer_t timerId);
int isActive(timer_t timerId);
int setupSigset(sigset_t* emptySet, sigset_t* maskSet, int maskedSignal);
int sendResultSignal(Commands* commands, struct timespec*, timer_t probeTimer, timer_t periodTimer);
int parseCommand(int udpFd, Commands* commands, timer_t probeTimer, timer_t periodTimer, struct timespec* ts);

#endif
