#define _BSD_SOURCE

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

#define SIZE_MULTIPLIER 384
#define MAX_STRING 256

typedef struct AtExitArgumentsStruct {
    char fileName[MAX_STRING];
    char processName[MAX_STRING];
    char resourceName[MAX_STRING];
    int nr;
} AtExitArguments;

void RenameFileAtExit(int exitStatus, void* args) {
    AtExitArguments* arg = (AtExitArguments*)args;
    char newFileName[3 * MAX_STRING] = {0};
    sprintf(newFileName, "%s_%smine.#%d", arg->processName, arg->resourceName, arg->nr);

    rename(arg->fileName, newFileName);
}

int main(int argc, char* argv[]) {
    int option = 0;
    int wasZ = 0, wasS = 0, wasErr = 0;
    char* fifoPath = NULL;
    char* resourceName = NULL;
    char* processName = NULL;

    while ((option = getopt(argc, argv, "z:s:")) != -1) {
        if (option == 'z' && !wasZ) {
            resourceName = optarg;
            wasZ = 1;
        }
        else if (option == 's' && !wasS) {
            fifoPath = optarg;
            wasS = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (argc != optind || !wasZ || !wasS) {
        wasErr = 1;
    }
    if (wasErr) {
        printf("Usage: %s \n\
            \t-z <name of resource>\n\
            \t-s <path to fifo>\n\
        ", argv[0]);
        return 1;
    }
    processName = argv[0];

    // Block until parent start the race
    read(4, NULL, 1);

    char fileName[MAX_STRING] = {0};
    int k = 1;
    int N = 0;
    int fifoFd;
    while ((fifoFd = open(fifoPath, O_WRONLY | O_NONBLOCK)) != -1) {
        close(fifoFd);
        if (N >= 100) {
            continue;
        }

        // File name generation
        char number[3] = {0};
        if (N < 10) {
            sprintf(number, "0%d", N);
        }
        else {
            sprintf(number, "%d", N);
        }
        sprintf(fileName, "property_%s.%smine", number, resourceName);
        N++;

        // Create file
        int resourceFd = open(fileName, O_CREAT | O_EXCL, 0666);
        if (resourceFd != -1) {
            // set file size
            lseek(resourceFd, k * SIZE_MULTIPLIER, SEEK_SET);
            write(resourceFd, '\0', 1);
            close(resourceFd);

            // register on_exit func
            AtExitArguments* arguments = (AtExitArguments*)calloc(1, sizeof(AtExitArguments));
            strcpy(arguments->processName, processName);
            strcpy(arguments->fileName, fileName);
            strcpy(arguments->resourceName, resourceName);
            arguments->nr = k;
            if (on_exit(RenameFileAtExit, (void*) arguments) != 0) {
                perror("on_exit");
                _exit(EXIT_FAILURE);
            }
            k++;
        }
    }

    return 0;
}