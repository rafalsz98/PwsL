#include "utils.h"

#define ERROR_CHECK(func, string) if(func==-1){perror(string);exit(EXIT_FAILURE);}


int main(int argc, char* argv[]) {
    double interval;
    int intervalTimes, w, k;
    if (argc != 2 || parseParameters(argv[1], &interval, &intervalTimes, &w, &k) == -1) {
        printf("Usage: %s <float>:<int>:<int>,<int>\n \
                 - interval counted in decsecs(?)\n \
                 - interval count before change of programs behaviour\n \
                 - coords of print location\n \
        ", argv[0]);
        exit(EXIT_FAILURE);
    }
    interval *= 10;

    timer_t alarmTimer;
    // signal for alarm
    ERROR_CHECK(setupSignalHandler(SIGUSR1, alarmHandler), "setupSignalHandler");
    // cyclic alarm
    ERROR_CHECK(setupTimer(SIGUSR1, &alarmTimer), "setupTimer");
    ERROR_CHECK(launchTimer(alarmTimer, interval, intervalTimes, 0), "launchTimer");

    ERROR_CHECK(setupSignalHandler(SIGCHLD, childHandler), "setupSignalHandler");

    // one time alarm and signal setup
    timer_t signalContTimer;
    ERROR_CHECK(setupTimer(SIGUSR2, &signalContTimer), "setupTimer");
    ERROR_CHECK(setupSignalHandler(SIGUSR2, sendSigContHandler), "setupSignalHandler");

    sigset_t blockSet;
    sigset_t unblockSet;
    sigemptyset(&blockSet);
    sigemptyset(&unblockSet);
    sigaddset(&blockSet, SIGUSR1);

    clearScreen();
    for (;;) {
        pause();
        // phase 1
        if (flag == 0) {
            int _intervalCount;
            sigprocmask(SIG_SETMASK, &blockSet, NULL);
            _intervalCount = intervalCount;
            sigprocmask(SIG_SETMASK, &unblockSet, NULL);

            char buffer[64] = {0};
            sprintf(buffer, "odliczanie #%d (/%d)", _intervalCount, intervalTimes);
            writeAt(w, k, 31, 49, buffer);
        }
        // phase 2 init
        else if (flag == 1) {
            flag = 2;
            timer_delete(alarmTimer);
            clearScreen();
            ERROR_CHECK((childPid = fork()), "fork");
            if (childPid == 0) {
                childProcess();
            }
        }
        // phase 2
        else {
            // child stopped
            if (flag == 3) {
                writeAt(w, k, 31, 49, "Child stopped");
                ERROR_CHECK(launchTimer(signalContTimer, 2.0, 0, 1), "launchTimer");
            }
            // child continued
            else if (flag == 4) {
                writeAt(w, k, 31, 49, "Child continued");
                clock_gettime(CLOCK_MONOTONIC, &contTime);
            }
            // child exited
            else if (flag == 5) {
                clock_gettime(CLOCK_MONOTONIC, &endTime);
                double delta = endTime.tv_sec - contTime.tv_sec + (endTime.tv_nsec - contTime.tv_nsec) / 1e9;
                char buffer[64] = {0};
                sprintf(buffer, "Delta between exit signal and cont signal is: %.10f", delta);
                writeAt(w, k, 31, 49, buffer);
                writeAt(w + 2, k, 31, 49, "Child ended work, exiting\n");
                exit(EXIT_SUCCESS);
            }
            // errors
            else {
                writeAt(w, k, 31, 49, "Child's undefined behaviour, exiting");
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}
