#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("pid sig val\n");
    }
    int pid = atoi(argv[1]);
    int sig = atoi(argv[2]);
    int val = atoi(argv[3]);
    union sigval sigval = {.sival_int = val};
    sigqueue(pid, sig, sigval);
    return 0;
}