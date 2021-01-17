#ifndef LAB8_UTILS_H
#define LAB8_UTILS_H

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 199309
#define _GNU_SOURCE
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

extern volatile int intervalCount;
extern volatile sig_atomic_t flag;
extern struct timespec contTime;
extern struct timespec endTime;
extern pid_t childPid;

int StringToInt(char *string, int *wasErr);
double StringToDouble(char *string, int *wasErr);
int parseParameters(char* string, double* intervalPtr, int* intervalTimesPtr, int* wPtr, int* kPtr);

int setupTimer(int signal, timer_t *timerid);
int launchTimer(timer_t timerid, double interval, int intervalTimes, int oneTimer);
int setupSignalHandler(int signal, void (*handler)(int, siginfo_t*, void *));

void alarmHandler(int sig, siginfo_t *siginfo, void *ucontext);
void childHandler(int sig, siginfo_t *siginfo, void *ucontext);
void sendSigContHandler(int sig, siginfo_t *siginfo, void *ucontext);

void childProcess();
void childProcessSignalHandler(int sig, siginfo_t *siginfo, void *ucontext);

void clearScreen();
void writeAt(int w, int k, int colorFg, int colorBg, const char* what);
#endif //LAB8_UTILS_H
