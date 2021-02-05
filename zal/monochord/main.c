#include "utils.h"

#define ERROR_CHECK(func, string) if((func)==-1){perror(string);exit(EXIT_FAILURE);}

volatile sig_atomic_t signalFlag = 0;
volatile sig_atomic_t periodExecution = 0;
void dataHandler(int sig, siginfo_t *siginfo, void *ucontext);
void setupApp(int port, int *udpFd, timer_t *probeTimer, timer_t *periodTimer, sigset_t *emptySet, sigset_t *maskSet,
              Commands *commands);

int main(int argc, char* argv[]) {
    in_port_t port;
    if (parseParameters(argc, argv, &port) == -1) {
        printf("Usage: %s <short int | port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Setup App
    int udpFd;
    timer_t probeTimer, periodTimer;
    sigset_t emptySet, maskSet;
    Commands commands;
    setupApp(port, &udpFd, &probeTimer, &periodTimer, &emptySet, &maskSet, &commands);

    // Main program loop
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    char buffer[64] = {0};
    for (;;) {
        ssize_t rec;
        if ((rec = recvfrom(udpFd, buffer, sizeof(buffer), MSG_PEEK, NULL, NULL) == -1) && errno == EINTR) {
            // Interruped by signal, check type
            sigprocmask(SIG_SETMASK, &maskSet, NULL);
            if (signalFlag == 0) { // probe timer
                ERROR_CHECK(sendResultSignal(&commands, &ts, probeTimer, periodTimer), "sendResultSignal");
            }
            else if (signalFlag == 1) { // period timer
                printf("end probing!\n");
                stopTimer(probeTimer);
                probingStopped = 1;
                periodExecution = 0;
            }
            sigprocmask(SIG_SETMASK, &emptySet, NULL);
        }
        else if (rec == -1) {
            perror("recvfrom");
            break;
        }
        else {
            // UDP socket has some command
            sigprocmask(SIG_SETMASK, &maskSet, NULL);
            parseCommand(udpFd, &commands, probeTimer, periodTimer, &ts);
            sigprocmask(SIG_SETMASK, &emptySet, NULL);
        }

    }

    exit(EXIT_FAILURE);
}

void dataHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    int val = siginfo->si_value.sival_int;
    if (!periodExecution) signalFlag = val;
    if (signalFlag == 1) {
        periodExecution = 1;
    }
}

void setupApp(int port, int *udpFd, timer_t *probeTimer, timer_t *periodTimer, sigset_t *emptySet, sigset_t *maskSet,
              Commands *commands) {
    // Setup UDP server
    ERROR_CHECK(*udpFd = setupUDP(port), "setupUDP");

    // Setup timers
    ERROR_CHECK(setupTimer(SIGUSR1, 0, probeTimer), "setupTimer");
    ERROR_CHECK(setupTimer(SIGUSR2, 1, periodTimer), "setupTimer");

    // Setup sigset
    ERROR_CHECK(setupSigset(emptySet, maskSet, SIGUSR1), "setupSigset");;
    ERROR_CHECK(sigaddset(maskSet, SIGUSR2), "sigaddset");

    // Setup sigaction
    struct sigaction sa1 = {.sa_sigaction = dataHandler, .sa_flags = SA_SIGINFO};
    ERROR_CHECK(sigemptyset(&(sa1.sa_mask)), "sigemptyset");
    ERROR_CHECK(sigaction(SIGUSR1, &sa1, NULL), "sigaction");

    struct sigaction sa2 = {.sa_sigaction = dataHandler, .sa_flags = SA_SIGINFO};
    ERROR_CHECK(sigemptyset(&(sa2.sa_mask)), "sigemptyset");
    ERROR_CHECK(sigaddset(&(sa2.sa_mask), SIGUSR1), "sigaddset");
    ERROR_CHECK(sigaction(SIGUSR2, &sa2, NULL), "sigaction");

    commands->amp = 1.0f;
    commands->freq = 0.25f;
    commands->probe = 1.0f;
    commands->period = -1.0f;
    commands->pid = 1;
    commands->rt = 0;
}