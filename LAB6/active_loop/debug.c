#include <unistd.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    printf("t");

    mkfifo("fifo1", 0666);
    mkfifo("fifo2", 0666);
    mkfifo("fifo3", 0666);
    mkfifo("fifocontrol", 0666);
    int pid = fork();
    if (pid == 0) {
        open("fifocontrol", O_WRONLY);
        open("fifo1", O_RDONLY);
        open("fifo2", O_RDONLY);
        open("fifo3", O_RDONLY);
        execl(
            "main.out", 
            "main.out", 
            "-c", "3", 
            "4", "5", "6",
            NULL);
    }
    int fds[4];
    sleep(0.1);
    fds[0] = open("fifocontrol", O_RDONLY);
    fds[1] = open("fifo1", O_WRONLY | O_NONBLOCK);
    fds[2] = open("fifo2", O_WRONLY | O_NONBLOCK);
    fds[3] = open("fifo3", O_WRONLY | O_NONBLOCK);
    printf("%d\n", fds[1]);
    char towrite[] = "aaaaaaaaaaa";
    while (1) {
        write(fds[1], towrite, sizeof(towrite));
        sleep(0.01);
    }

    return 0;
}