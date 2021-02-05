#include "utils.h"

#define ERROR_CHECK(func) if((func)==-1){return -1;}

int probingStopped = 1;
int lastSendSuccessful = 0;

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


double StringToDouble(char *string, int *wasErr) {
    char *endptr = NULL;
    errno = 0;
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

int parseParameters(int argc, char* argv[], in_port_t* port) {
    if (argc != 2) return -1;
    int wasErr = 0;
    *port = StringToInt(argv[1], &wasErr);

    if (*port < 0 || *port > 65535) wasErr = 1;

    return wasErr;
}

int setupUDP(in_port_t port) {
    int udpFd;
    ERROR_CHECK(udpFd = socket(AF_INET, SOCK_DGRAM, 0));

    // Setup sockaddr struct
    struct sockaddr_in sockaddrIn = {0};
    sockaddrIn.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &(sockaddrIn.sin_addr)) <= 0) return -1;
    sockaddrIn.sin_port = htons(port);

    // bind
    ERROR_CHECK(bind(udpFd, (const struct sockaddr*)&sockaddrIn, sizeof(struct sockaddr_in)));

    // Listen
    listen(udpFd, MAX_UDP_CONNECTIONS);

    return udpFd;
}

int setupTimer(int signal, int sigValue, timer_t* timerId) {
    struct sigevent sigevent = {
            .sigev_notify = SIGEV_SIGNAL,
            .sigev_signo = signal,
            .sigev_value = {.sival_int = sigValue}
            };
    ERROR_CHECK(timer_create(CLOCK_MONOTONIC, &sigevent, timerId));
    return 0;
}

int launchTimer(timer_t timerId, long seconds, long nanoseconds, int isOneTimer) {
    long intervalSeconds = isOneTimer ? 0 : seconds;
    long intervalNanoseconds = isOneTimer ? 0 : nanoseconds;

    struct itimerspec ts = {
            .it_interval = {.tv_sec = intervalSeconds, .tv_nsec = intervalNanoseconds},
            .it_value = {.tv_sec = seconds, .tv_nsec = nanoseconds}
    };

    ERROR_CHECK(timer_settime(timerId, 0, &ts, NULL));
    return 0;
}

int stopTimer(timer_t timerId) {
    ERROR_CHECK(launchTimer(timerId, 0, 0, 0));
    return 0;
}

int isActive(timer_t timerId) {
    struct itimerspec tsLeft = {0};
    timer_gettime(timerId, &tsLeft);
    return
        (tsLeft.it_value.tv_sec > 0 || tsLeft.it_value.tv_nsec > 0) ||
        (tsLeft.it_interval.tv_sec != 0 || tsLeft.it_interval.tv_nsec != 0);
}

int setupSigset(sigset_t* emptySet, sigset_t* maskSet, int maskedSignal) {
    ERROR_CHECK(sigemptyset(emptySet));
    ERROR_CHECK(sigemptyset(maskSet));
    ERROR_CHECK(sigaddset(maskSet, maskedSignal));
    return 0;
}

int sendResultSignal(Commands* commands, struct timespec* startTs, timer_t probeTimer, timer_t periodTimer) {
    if (kill(commands->pid, 0) == -1 || (commands->rt < SIGRTMIN || commands->rt > SIGRTMAX)) {
        // no permissions, stop timers
        lastSendSuccessful = 0;
        printf("stopped sim\n");
        stopTimer(probeTimer);
        stopTimer(periodTimer);
        probingStopped = 1;
        return 0;
    }
    struct timespec tsNow = {0};
    clock_gettime(CLOCK_MONOTONIC, &tsNow);
    float x = tsNow.tv_sec - startTs->tv_sec + (tsNow.tv_nsec - startTs->tv_nsec) / 1e9;
    float y = commands->amp * sin(2 * M_PI * commands->freq * x);
    union sigval sigval = {0};
    printf("val: %f\n", y);
    memcpy(&sigval, &y, sizeof(float));
    ERROR_CHECK(sigqueue(commands->pid, commands->rt, sigval));
    lastSendSuccessful = 1;
    return 0;
}

// ------ private functions ------------
int skipSpaces(char* string) {
    int i = 0;
    while (string[i] == ' ' || string[i] == '\t') i++;
    return i;
}

int skipSeparation(char* string) {
    int i = skipSpaces(string);
    if (string[i] == ':') i++;
    i = i + skipSpaces(string + i);
    return i;
}

int getLastStringCharacter(char* string) {
    int i = 0;
    while (isalpha(string[i]) || isdigit(string[i]) || string[i] == '-' || string[i] == '.') i++;
    return i;
}

void resetTimer(timer_t timer, float time, int isOneTime) {
    long seconds = time;
    long nanoseconds = (time - seconds) * 1e9;
    launchTimer(timer, seconds, nanoseconds, isOneTime);
}

// --------------------------------------

int parseCommand(int udpFd, Commands* commands, timer_t probeTimer, timer_t periodTimer, struct timespec* ts)
{
    struct sockaddr_in sockaddrIn = {0};
    socklen_t addrlen;
    char buffer[UDP_BUFFER_SIZE] = {0};
    int size = 0;
    ERROR_CHECK(size = recvfrom(udpFd, buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr*)&sockaddrIn, &addrlen));
    if (size >= UDP_BUFFER_SIZE) {
        const char message[] = "Your command might have exceeded maximum length, try again with less characters!\n";
        sendto(udpFd, message, sizeof(message), 0, (const struct sockaddr*)&sockaddrIn, addrlen);
    }

    int writeRaport = 0;
    char* source = buffer;
    char* recordRow;
    while ((recordRow = strtok(source, "\n"))) {
        source = NULL;

        int i = skipSpaces(recordRow);
        int end = i + getLastStringCharacter(recordRow + i);
        if (end - i >= 64 || end - i == 0) continue;
        char command[64] = {0};
        strncpy(command, recordRow + i, end - i);

        if(recordRow[end] == '\0') {
            if (strcmp("raport", command)) continue;
            writeRaport = 1;
            continue;
        }

        i = end + skipSeparation(recordRow + end);
        end = i + getLastStringCharacter(recordRow + i);
        if (end - i >= 64 || end - i == 0) continue;
        char stringValue[64] = {0};
        strncpy(stringValue, recordRow + i, end - i);

        if (!strcmp(command, "amp")) {
            int err = 0;
            float value = (float)StringToDouble(stringValue, &err);
            if (err) continue;
            commands->amp = value;
            clock_gettime(CLOCK_MONOTONIC, ts);
        }
        else if (!strcmp(command, "freq")) {
            int err = 0;
            float value = (float)StringToDouble(stringValue, &err);
            if (err) continue;
            commands->freq = value;
            clock_gettime(CLOCK_MONOTONIC, ts);
        }
        else if (!strcmp(command, "probe")) {
            int err = 0;
            float value = (float)StringToDouble(stringValue, &err);
            if (err || value < 0) continue;
            commands->probe = value;

            if (
                (commands->period > 0 && isActive(periodTimer)) ||
                commands->period == 0
            ) {
                long seconds = value;
                long nanoseconds = (value - seconds) * 1e9;
                launchTimer(probeTimer, seconds, nanoseconds, 0);
            }
        }
        else if (!strcmp(command, "period")) {
            int err = 0;
            float value = (float)StringToDouble(stringValue, &err);

            if (err) continue;
            commands->period = value;
            if (value < 0) {
                stopTimer(periodTimer);
                stopTimer(probeTimer);
            }
            else if (value == 0) {
                stopTimer(periodTimer);
                resetTimer(probeTimer, commands->probe, 0);
                probingStopped = 0;
            }
            else {
                resetTimer(periodTimer, value, 1);
                resetTimer(probeTimer, commands->probe, 0);
                probingStopped = 0;
            }
        }
        else if (!strcmp(command, "pid")) {
            int err = 0;
            short value = (short)StringToInt(stringValue, &err);
            if (err || value <= 0) continue;
            commands->pid = value;
            if (!lastSendSuccessful) {
                if (commands->period >= 0) {
                    if (commands->period != 0) {
                        resetTimer(periodTimer, commands->period, 1);
                    }
                    resetTimer(probeTimer, commands->probe, 0);
                }
            }
        }
        else if (!strcmp(command, "rt")) {
            int err = 0;
            short value = (short)StringToInt(stringValue, &err);
            if (err || value < 0 || value > (SIGRTMAX - SIGRTMIN)) continue;
            commands->rt = value;
        }
    }
    if (writeRaport) {
        char message[256] = {0};
        char periodAdd[32] = {0};
        if (commands->period < 0) strcpy(periodAdd, "stopped");
        else if (probingStopped) strcpy(periodAdd, "suspended");
        else if (commands->period == 0) strcpy(periodAdd, "non-stop");

        char pidAdd[32] = {0};
        if (lastSendSuccessful) strcpy(pidAdd, "receiver exists");
        else strcpy(pidAdd, "receiver doesn't exist");
        snprintf(message, sizeof(message) - 1, "\
amp\t%f\n\
freq\t%f\n\
probe\t%f\n\
period\t%f\t%s\n\
pid\t%d\t%s\n\
rt\t%d\n", commands->amp, commands->freq, commands->probe, commands->period, periodAdd, commands->pid, pidAdd, commands->rt);
        sendto(udpFd, message, sizeof(message), 0, (struct sockaddr*)&sockaddrIn, addrlen);
    }
    return 0;
}

