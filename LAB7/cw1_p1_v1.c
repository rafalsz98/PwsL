#define _POSIX_C_SOURCE 199309
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

typedef struct {
    pid_t pid;
    uid_t uid;
    int code;
    int codedTime;
} SignalInfo;

volatile struct timespec startTime;
volatile SignalInfo signalInfo;

void handler1(int sig, siginfo_t *siginfo, void *ucontext) {
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    signalInfo.pid = siginfo->si_pid;
    signalInfo.uid = siginfo->si_uid;
    signalInfo.code = siginfo->si_code;
    signalInfo.codedTime = siginfo->si_value.sival_int;
}

int main() {
    // sigaction init
    struct sigaction sa1;
    sigemptyset(&(sa1.sa_mask));
    sa1.sa_flags = SA_SIGINFO;
    sa1.sa_sigaction = handler1;
    sigaction(SIGUSR1, &sa1, NULL);
    sigaction(SIGRTMIN + 3, &sa1, NULL);

    sigset_t mask, emptySet;
    sigemptyset(&mask);
    sigemptyset(&emptySet);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGRTMIN + 3);

    // sleep time init
    struct timespec ts = {0, 5 * 1e8};
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (;;) {
        if (pause() == -1 && errno == EINTR) {
            sigprocmask(SIG_SETMASK, &mask, NULL);
            errno = 0;
            time_t sec = 0, nsec = 0;
            char* codedBytes = (char*)&(signalInfo.codedTime);
            char* secBytes = (char*)&sec;
            char* nsecBytes = (char*)&nsec;
            secBytes[sizeof(sec) - 1] = codedBytes[0];
            nsecBytes[0] = codedBytes[1];
            nsecBytes[1] = codedBytes[2];
            nsecBytes[2] = codedBytes[3];
            double delta = sec + (startTime.tv_nsec - nsec) / 1e9;


            printf("Start: %ld[s] %ld[ns]\t", startTime.tv_sec, startTime.tv_nsec);
            printf("PID: %d\tUID: %d\tCode: %d\tDelta time: %.9f\t", signalInfo.pid, signalInfo.uid, signalInfo.code, delta);
            fflush(stdout);
            sigprocmask(SIG_SETMASK, &emptySet, NULL);
            if (nanosleep(&ts, 0) == -1 && errno == EINTR) {
                printf("\tEINTR\t");
            }
            printf("\tend of sleep\n");
        }
    }
#pragma clang diagnostic pop
}
