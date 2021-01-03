#define _POSIX_C_SOURCE 199309
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/time.h>
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

static void inline sendSignal(int pid, int signal) {
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    char* secondBytes = (char*)&(ts.tv_sec);
    char* nanosecondBytes = (char*)&(ts.tv_nsec);

    int coded;
    char *bytes = (char*)&coded;
    bytes[0] = secondBytes[sizeof(ts.tv_sec) - 1];
    bytes[1] = nanosecondBytes[0];
    bytes[2] = nanosecondBytes[1];
    bytes[3] = nanosecondBytes[2];

    union sigval val = {.sival_int = coded};
    sigqueue(pid, signal, val);
}

void usrHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    int pid = siginfo->si_value.sival_int;
    sendSignal(pid, SIGUSR1);
}

void minHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    int pid = siginfo->si_value.sival_int;
    sendSignal(pid, SIGRTMIN + 3);
}

int main(int argc, char* argv[]) {
    int option;
    int was1 = 0, was3 = 0, wasErr = 0;
    double usrInterval, minInterval;
    int pid;

    while ((option = getopt(argc, argv, "1:3:")) != -1) {
        if (option == '1' && !was1) {
            usrInterval = StringToDouble(optarg, &wasErr);
            was1 = 1;
        }
        else if (option == '3' && !was3) {
            minInterval = StringToDouble(optarg, &wasErr);
            was3 = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (!was1 || !was3 || argc - optind != 1) {
        wasErr = 1;
    }
    if (!wasErr) {
        pid = StringToInt(argv[optind], &wasErr);
    }
    if (wasErr) {
        printf("Usage: %s \n\
            \t-1 <float | usrInterval between signals>\n\
            \t-3 <float | minInterval between signals>\n\
            \t <int | process pid>\n\
        ", argv[0]);
        return 1;
    }

    // signal usr setup
    struct sigaction usrSa;
    sigemptyset(&(usrSa.sa_mask));
    sigaddset(&(usrSa.sa_mask), SIGUSR2);
    usrSa.sa_flags = SA_SIGINFO;
    usrSa.sa_sigaction = usrHandler;
    sigaction(SIGUSR1, &usrSa, NULL);

    // signal min setup
    struct sigaction minSa;
    sigemptyset(&(minSa.sa_mask));
    sigaddset(&(minSa.sa_mask), SIGUSR1);
    minSa.sa_flags = SA_SIGINFO;
    minSa.sa_sigaction = minHandler;
    sigaction(SIGUSR2, &minSa, NULL);


    // timer setup
    timer_t usrTimer, minTimer;
    struct sigevent usrSig = {.sigev_notify = SIGEV_SIGNAL, .sigev_signo = SIGUSR1, .sigev_value.sival_int = pid};
    struct sigevent minSig = {.sigev_notify = SIGEV_SIGNAL, .sigev_signo = SIGUSR2, .sigev_value.sival_int = pid};
    if (timer_create(CLOCK_MONOTONIC, &usrSig, &usrTimer) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }
    if (timer_create(CLOCK_MONOTONIC, &minSig, &minTimer) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    int sec = usrInterval;
    int nsec = (usrInterval - sec) * 1e9;
    struct itimerspec usrTimerSpec = {{sec, nsec},
                                      {sec, nsec}};
    sec = minInterval;
    nsec = (minInterval - sec) * 1e9;
    struct itimerspec minTimerSpec = {{sec, nsec},
                                      {sec, nsec}};
    timer_settime(usrTimer, 0, &usrTimerSpec, NULL);
    timer_settime(minTimer, 0, &minTimerSpec, NULL);


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (;;) {
        if (pause() == -1 && errno == EINTR) {
        }
    }
#pragma clang diagnostic pop

}