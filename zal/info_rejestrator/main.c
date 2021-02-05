#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

#define ERROR_CHECK(func, string) if((func)==-1){perror(string);exit(EXIT_FAILURE);}

enum Status {NOT_WORKING, WORKING, USING_REF_POINT, USING_SOURCE_ID = 4, USING_BINARY = 8};


int parseParameters(int argc, char* argv[], int* signalNo, int* pid);
int StringToInt(char *string, int *wasErr);
void signalHandler(int sig, siginfo_t *siginfo, void *ucontext);

volatile sig_atomic_t returnStatus = 0;

int main(int argc, char* argv[]) {
    int signalNo, pid;
    if (parseParameters(argc, argv, &signalNo, &pid) == -1) {
        printf("Usage: %s \n\
\t-c <int, no. of RealTime signal>\n\
\t<int, pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    struct timespec ts = {.tv_sec = 2, .tv_nsec = 0};

    struct sigaction sa = {.sa_flags = SA_SIGINFO, .sa_sigaction = signalHandler};
    sigset_t maskSet;
    ERROR_CHECK(sigemptyset(&(sa.sa_mask)), "sigemptyset");
    ERROR_CHECK(sigemptyset(&maskSet), "sigemptyset");
    ERROR_CHECK(sigaddset(&maskSet, signalNo), "sigaddset");
    ERROR_CHECK(sigaction(signalNo, &sa, NULL), "sigaction");

    union sigval sigval = {.sival_int = 255};
    ERROR_CHECK(sigqueue(pid, signalNo, sigval), "sigqueue");
    errno = 0;
    if (nanosleep(&ts, NULL) == -1 && errno == EINTR) {
        sigprocmask(SIG_SETMASK, &maskSet, NULL);
        printf("Received reply\n");
        if (returnStatus & WORKING) printf("IS WORKING\n");
        if (returnStatus & USING_REF_POINT) printf("IS USING REFERENCE POINT\n");
        if (returnStatus & USING_SOURCE_ID) printf("IS USING SOURCE IDENTIFICATION\n");
        if (returnStatus & USING_BINARY) printf("IS USING BINARY FORMAT\n");
        if (returnStatus == NOT_WORKING) printf("IS NOT WORKING\n");
        exit(EXIT_SUCCESS);
    }
    printf("Didn't receive any reply!\n");
    exit(EXIT_FAILURE);
}


void signalHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    returnStatus = siginfo->si_value.sival_int;
}

int parseParameters(int argc, char* argv[], int* signalNo, int* pid) {
    int option = 0;
    int wasC = 0, wasErr = 0;

    while ((option = getopt(argc, argv, "c:")) != -1) {
        if (wasErr) break;
        if (option == 'c' && !wasC) {
            *signalNo = StringToInt(optarg, &wasErr);
            if (*signalNo < SIGRTMIN || *signalNo > SIGRTMAX) wasErr = 1;
            wasC = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (argc - optind != 1) return -1;
    *pid = StringToInt(argv[optind], &wasErr);
    if (wasErr || !wasC) return -1;
    return 0;
}

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