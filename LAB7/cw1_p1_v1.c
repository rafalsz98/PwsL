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
} SignalInfo;

volatile struct timespec startTime;
volatile SignalInfo signalInfo;

void handler1(int sig, siginfo_t *siginfo, void *ucontext) {
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    signalInfo.pid = siginfo->si_pid;
    signalInfo.uid = siginfo->si_uid;
    signalInfo.code = siginfo->si_code;
}

void handler2(int sig, siginfo_t *siginfo, void *ucontext) {
    int si_val = siginfo->si_value.sival_int;
    printf("%d\n", si_val); //unsafe
}

int main() {
    // sigaction init
    struct sigaction sa1, sa2;
    sigemptyset(&(sa1.sa_mask));
    sigemptyset(&(sa2.sa_mask));
    sa1.sa_flags = SA_SIGINFO;
    sa1.sa_sigaction = handler1;
    sigaction(SIGUSR1, &sa1, NULL);

    sa2.sa_flags = SA_SIGINFO;
    sa2.sa_sigaction = handler2;
    sigaction(SIGRTMIN + 3, &sa2, NULL);

    sigset_t usrSet, emptySet;
    sigemptyset(&usrSet);
    sigemptyset(&emptySet);
    sigaddset(&usrSet, SIGUSR1);

    // sleep time init
    struct timespec ts = {0, 5 * 1e8};
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (;;) {
        if (pause() == -1 && errno == EINTR) {
            sigprocmask(SIG_SETMASK, &usrSet, NULL);
            errno = 0;
            printf("Start: %ld[s] %ld[ns]\t", startTime.tv_sec, startTime.tv_nsec);
            printf("PID: %d\tUID: %d\tCode: %d\t", signalInfo.pid, signalInfo.uid, signalInfo.code);
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
