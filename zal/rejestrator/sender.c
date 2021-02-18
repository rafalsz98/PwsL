#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

int StringToInt(char *string, int *wasErr) {
    char *endptr = NULL;
    errno = 0;
    long val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)
        || endptr == string
        || *endptr != 0
    ) {
        *wasErr = -1;
    }
    return (int)val;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s PID rt_signal value\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int wasErr = 0;
    int pid = StringToInt(argv[1], &wasErr);
    int sig = StringToInt(argv[2], &wasErr);
    int val = StringToInt(argv[3], &wasErr);
    if (wasErr || (sig < SIGRTMIN || sig > SIGRTMAX)) {
        printf("Wrong parameters\n");
        exit(EXIT_FAILURE);
    }

    union sigval sigval = {.sival_int = val};
    if (sigqueue(pid, sig, sigval) == -1) {
        perror("sigqueue");
        exit(EXIT_FAILURE);
    }
    return 0;
}