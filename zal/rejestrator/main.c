#include "utils.h"

#define ERROR_CHECK(func, string) if((func)==-1){perror(string);exit(EXIT_FAILURE);}

int textFd = -1;
int binFd = -1;
volatile Data lastData = {0};
volatile sig_atomic_t signalFlag = 0;
volatile int commandFlags = 0;
volatile pid_t infoPid = 0;
int currentStatus = NOT_WORKING;

void setupApp(char* pathToBinary, char* pathToText, int dataSignal, int commandSignal, sigset_t* emptySet, sigset_t* maskSet);
void dataHandler(int sig, siginfo_t *siginfo, void *ucontext);
void commandHandler(int sig, siginfo_t *siginfo, void *ucontext);



int main(int argc, char* argv[]) {
    int dataSignal, commandSignal;
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
    // debug only
    printf("%d\n", getpid());

    sigset_t emptySet, maskSet;
    setupApp(pathToBinary, pathToText, dataSignal, commandSignal, &emptySet, &maskSet);

    for (;;) {
        pause();
        sigprocmask(SIG_SETMASK, &maskSet, NULL), "sigprocmask";
        if (signalFlag == 0 && currentStatus & WORKING) {
            ERROR_CHECK(saveData(lastData, currentStatus, currTs), "saveData");
        }
        else if (signalFlag == 1) {
            ERROR_CHECK(parseCommand(&currentStatus, commandFlags, &prevTs, &currTs, commandSignal), "parseCommand")
        }
        sigprocmask(SIG_SETMASK, &emptySet, NULL), "sigprocmask";
    }

    exit(EXIT_FAILURE);
}

void dataHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    memcpy((void*)&(lastData.data), &(siginfo->si_value), sizeof(float)); //memcpy is async-safe
    lastData.source = siginfo->si_pid;
    signalFlag = 0;
}

void commandHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    commandFlags = siginfo->si_value.sival_int;
    infoPid = siginfo->si_pid;
    signalFlag = 1;
}

void setupApp(char* pathToBinary, char* pathToText, int dataSignal, int commandSignal, sigset_t* emptySet, sigset_t* maskSet) {
    // Setup descriptors
    if (pathToBinary){
        ERROR_CHECK(binFd = open(pathToBinary, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666), "open");
    }
    if (pathToText) {
        ERROR_CHECK(textFd = open(pathToText, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666), "open");
        dup2(textFd, STDOUT_FILENO);
    }
    else textFd = STDOUT_FILENO;

    // Setup signals
    struct sigaction sa = {.sa_flags = SA_SIGINFO, .sa_sigaction = dataHandler};
    ERROR_CHECK(sigemptyset(&(sa.sa_mask)), "sigemptyset");
    ERROR_CHECK(sigaddset(&(sa.sa_mask), commandSignal), "sigaddset");
    ERROR_CHECK(sigaddset(&(sa.sa_mask), dataSignal), "sigaddset");
    ERROR_CHECK(sigaction(dataSignal, &sa, NULL), "sigaction");

    sa.sa_sigaction = commandHandler;
    ERROR_CHECK(sigaction(commandSignal, &sa, NULL), "sigaction");

    ERROR_CHECK(sigemptyset(maskSet), "sigemptyset");
    ERROR_CHECK(sigemptyset(emptySet), "sigemptyset");
    ERROR_CHECK(sigaddset(maskSet, dataSignal), "sigaddset");
    ERROR_CHECK(sigaddset(maskSet, commandSignal), "sigaddset");
}