#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

volatile static struct timespec ts;

static void handler(int sig, struct siginfo_t* siginfo, void* _) {
    sigset_t sa_mask;
    sigemptyset(&sa_mask);
    sigaddset(&sa_mask, SIGUSR1);
    // if (sigprocmask(SIG_BLOCK, &sa_mask, NULL) == -1) {
    //     perror("sigprocmask");
    //     exit(EXIT_FAILURE);
    // }
    printf("t\n");
    sleep(1);
    // clock_gettime(CLOCK_MONOTONIC, &ts);
    // if (sigprocmask(SIG_UNBLOCK, &sa_mask, NULL) == -1) {
    //     perror("sigprocmask");
    //     exit(EXIT_FAILURE);
    // }
}

int main() {
    struct sigaction sa;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    while (1) {
        pause();
        printf("%ld  -   %ld\n", ts.tv_sec, ts.tv_nsec);
    }

    return 0;
}