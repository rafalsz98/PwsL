#include "utils.h"

#define ERROR_CHECK(func) if(func==-1){return -1;}

volatile int intervalCount = 0;
volatile sig_atomic_t flag = 0;
struct timespec contTime = {0};
struct timespec endTime = {0};
pid_t childPid = 0;

int _intervalTimes;
double _interval;
int _w, _k;

int StringToInt(char *string, int *wasErr) {
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

// <float>:<int>:<int>,<int>,
int parseParameters(char* string, double* intervalPtr, int* intervalTimesPtr, int* wPtr, int* kPtr) {
    // interval
    char* fragment = strtok(string, ":");
    if (!fragment) return -1;
    int err = 0;
    *intervalPtr = StringToDouble(fragment, &err);
    if (err) return -1;

    // interval times
    fragment = strtok(NULL, ":");
    if (!fragment) return -1;
    *intervalTimesPtr = StringToInt(fragment, &err);
    if (err) return -1;

    // coords
    fragment = strtok(NULL, ":");
    if (!fragment) return -1;
    // w
    char* coords = strtok(fragment, ",");
    if (!coords) return -1;
    *wPtr = StringToInt(coords, &err);
    if (err) return -1;
    // k
    coords = strtok(NULL, ",");
    if (!coords) return -1;
    *kPtr = StringToInt(coords, &err);
    if (err) return -1;

    _w = *wPtr;
    _k = *kPtr;
    return 0;
}

int setupTimer(int signal, timer_t *timerid) {
    // setup timer
    struct sigevent sigevent = {0};
    sigevent.sigev_notify = SIGEV_SIGNAL;
    sigevent.sigev_signo = signal;
    ERROR_CHECK(timer_create(CLOCK_MONOTONIC, &sigevent, timerid));
}

int launchTimer(timer_t timerid, double interval, int intervalTimes, int oneTimer) {
    if (!oneTimer) {
        _intervalTimes = intervalTimes;
        _interval = interval;
    }
    int sec = interval;
    int nsec = (interval - sec) * 1e9;
    struct itimerspec its = {{sec, nsec}, {sec, nsec}};
    if (oneTimer) {
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
    }
    //printf("%d  %d\n", its.it_interval.tv_sec, its.it_interval.tv_nsec);
    ERROR_CHECK(timer_settime(timerid, 0, &its, NULL));
}

int setupSignalHandler(int signal, void (*handler)(int, siginfo_t*, void *)) {
    struct sigaction sa;
    ERROR_CHECK(sigemptyset(&(sa.sa_mask)));
    sa.sa_flags = SA_NOCLDWAIT | SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = handler;

    ERROR_CHECK(sigaction(signal, &sa, NULL));
}

void alarmHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    if (intervalCount >= _intervalTimes) {
        flag = 1;
        return;
    }
    intervalCount++;
}

void childHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    if (siginfo->si_code == CLD_STOPPED) {
        flag = 3;
    }
    else if (siginfo->si_code == CLD_CONTINUED) {
        flag = 4;
    }
    else if (siginfo->si_code == CLD_EXITED) {
        flag = 5;
    }
    else {
        flag = 6; // undefined behaviour
    }
}

void sendSigContHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    union sigval sigval = {0};
    sigqueue(childPid, SIGCONT, sigval);
}

timer_t stopSignalTimer;
void childProcess() {
    // reset signal handlers
    struct sigaction sa;
    sigemptyset(&(sa.sa_mask));
    sa.sa_handler = SIG_DFL;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    struct timespec ts = {0};
    struct tm *tm;
    struct timespec sleepTime = {0, 0.3 * 1e9};

    // setup timer
    if (setupTimer(SIGSTOP, &stopSignalTimer) == -1) {
        perror("setupTimer");
        exit(EXIT_FAILURE);
    }

    setupSignalHandler(SIGCONT, childProcessSignalHandler);

    // launch timer
    if (launchTimer(stopSignalTimer, _interval, 0, 1) == -1) {
        perror("launchTimer");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        clock_gettime(CLOCK_REALTIME, &ts);
        tm = localtime(&ts.tv_sec);
        char buffer[64] = {0};
        sprintf(buffer, "%d/%d/%d %d:%d:%d",
            tm->tm_mday,
            tm->tm_mon,
            tm->tm_year,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec
        );
        writeAt(_w + 1, _k + 4, 32, 49, buffer);
        nanosleep(&sleepTime, NULL);
    }
}

volatile int sigCount = 0;
void childProcessSignalHandler(int sig, siginfo_t *siginfo, void *ucontext) {
    sigCount++;
    if (sigCount >= _intervalTimes) {
        _exit(EXIT_SUCCESS);
    }
    else {
        if (launchTimer(stopSignalTimer, _interval, 0, 1) == -1) {
            perror("launchTimer");
            _exit(EXIT_FAILURE);
        }
    }
}

void clearScreen() {
    printf("\033[2J");
    fflush(stdout);
}

void writeAt(int w, int k, int colorFg, int colorBg, const char* what) {
    printf("\033[%d;%dH\033[K\033[%d;%dm%s\033[0m", w, k, colorFg, colorBg, what);
    fflush(stdout);
}