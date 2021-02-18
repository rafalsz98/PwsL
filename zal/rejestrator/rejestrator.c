#include "utils.h"

int textFd = -1;
int binFd = -1;
volatile Data lastData = {0};
volatile sig_atomic_t signalFlag = 0;
volatile sig_atomic_t wasSignal = 0;
volatile int commandFlags = 0;
volatile pid_t infoPid = 0;

void setupApp(char *pathToBinary, char *pathToText, int dataSignal, int commandSignal, sigset_t *emptySet, sigset_t *maskSet, struct sigaction *dataSa, struct sigaction *commandSa);
void dataHandler(int sig, siginfo_t *siginfo, void *ucontext);
void commandHandler(int sig, siginfo_t *siginfo, void *ucontext);


int main(int argc, char* argv[]) {
    int currentStatus = NOT_WORKING;
    int dataSignal, commandSignal;
    struct sigaction dataSa     = {.sa_flags = SA_SIGINFO, .sa_sigaction = dataHandler},
                     commandSa  = {.sa_flags = SA_SIGINFO, .sa_sigaction = commandHandler};
    struct timespec currTs = {0}, prevTs = {0};
    char* pathToBinary = NULL;
    char* pathToText = NULL;
    if (parseParameters(argc, argv, &pathToBinary, &pathToText, &dataSignal, &commandSignal) == -1) {
        printf("Usage: %s \n\
\t-b <path to binary file | optional>\n\
\t-t <path to text file | optional>\n\
\t-d <int, no. of RealTime signal for data>\n\
\t-c <int, no. of RealTime signal for commands\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("Process PID is: %d\n", getpid());

    sigset_t emptySet, maskSet;
    setupApp(pathToBinary, pathToText, dataSignal, commandSignal, &emptySet, &maskSet, &dataSa, &commandSa);
    if (binFd != -1) currentStatus |= USING_BINARY;

    for (;;) {
        if (!wasSignal) pause();
        // Blokuję sygnały na cały okres, ponieważ gdyby w tym czasie przyszedł, nie zostałby poprawnie przetworzony. Dzięki temu zostaną one skolejkowane (RT)
        ERROR_CHECK(sigprocmask(SIG_SETMASK, &maskSet, NULL), "sigprocmask");
        if (signalFlag == 0 && currentStatus & WORKING) {
            ERROR_CHECK(saveData(lastData, currentStatus, currTs), "saveData");
        }
        else if (signalFlag == 1) {
            ERROR_CHECK(parseCommand(&currentStatus, commandFlags, &prevTs, &currTs, commandSignal), "parseCommand")
        }
        wasSignal = 0;
        ERROR_CHECK(sigprocmask(SIG_SETMASK, &emptySet, NULL), "sigprocmask");
    }
    exit(EXIT_FAILURE);
}

void dataHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    lastData.data = *(float*)&siginfo->si_value;
    lastData.source = siginfo->si_pid;
    signalFlag = 0;
    wasSignal = 1;
}

void commandHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    commandFlags = siginfo->si_value.sival_int;
    infoPid = siginfo->si_pid;
    signalFlag = 1;
    wasSignal = 1;
}

void setupApp(char *pathToBinary, char *pathToText, int dataSignal, int commandSignal, sigset_t *emptySet, sigset_t *maskSet, struct sigaction *dataSa, struct sigaction *commandSa) {
    // Setup descriptors
    if (pathToBinary){
        ERROR_CHECK(binFd = open(pathToBinary, O_WRONLY | O_CREAT | O_TRUNC, 0644), "open"); // O_TRUNC will be ignored if file is not regular
    }
    if (pathToText) {
        ERROR_CHECK(textFd = open(pathToText, O_WRONLY | O_CREAT | O_TRUNC, 0644), "open");
    }
    else textFd = STDOUT_FILENO;

    // Setup signals
    ERROR_CHECK(sigemptyset(&(dataSa->sa_mask)), "sigemptyset");
    ERROR_CHECK(sigemptyset(&(commandSa->sa_mask)), "sigemptyset");

    ERROR_CHECK(sigaddset(&(dataSa->sa_mask), commandSignal), "sigaddset");
    ERROR_CHECK(sigaddset(&(commandSa->sa_mask), dataSignal), "sigaddset");

    ERROR_CHECK(sigaction(dataSignal, dataSa, NULL), "sigaction");
    ERROR_CHECK(sigaction(commandSignal, commandSa, NULL), "sigaction");

    ERROR_CHECK(sigemptyset(maskSet), "sigemptyset");
    ERROR_CHECK(sigemptyset(emptySet), "sigemptyset");
    ERROR_CHECK(sigaddset(maskSet, dataSignal), "sigaddset");
    ERROR_CHECK(sigaddset(maskSet, commandSignal), "sigaddset");
}