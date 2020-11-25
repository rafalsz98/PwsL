#define _XOPEN_SOURCE 500
#define _GNU_SOURCE 
#include <ftw.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <getopt.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <alloca.h>
#include <sys/wait.h>

#define ERROR_CHECK(func, string) if(func==-1){perror(string);exit(EXIT_FAILURE);}

struct timespec raceEndTime = {0};

int StringToInt(char *string)
{
    char *endptr = NULL;
    int val = strtol(string, &endptr, 10);
    if ((errno == ERANGE && val == LONG_MAX) 
        || val == LONG_MIN 
        || (errno != 0 && val == 0)
        || endptr == string 
        || *endptr != 0
        )
    {
        printf("Wrong parameter\n");
        exit(EXIT_FAILURE);
    }
    return val;
}

double StringToDouble(char *string)
{
    char *endptr = NULL;
    double val = strtod(string, &endptr);
    if ((errno == ERANGE && val == FLT_MAX) 
        || val == FLT_MIN 
        || (errno != 0 && val == 0)
        || endptr == string
        || *endptr != 0
        )
    {
        printf("Wrong parameter\n");
        exit(EXIT_FAILURE);
    }
    return val;
}

int OnNFTW(const char* pathname, const struct stat* statbuf, int typeflag, struct FTW* ftwbuf) {
    if (typeflag == FTW_D && strcmp(pathname, ".") != 0) {
        return FTW_SKIP_SUBTREE;
    }
    if (strstr(pathname, "property") != NULL) {
        unlink(pathname);
    }
    else if (
        statbuf->st_ctim.tv_sec > raceEndTime.tv_sec ||
        (statbuf->st_ctim.tv_sec == raceEndTime.tv_sec && 
        statbuf->st_ctim.tv_nsec > raceEndTime.tv_nsec)   
    ) {
        unlink(pathname);
    }

    return FTW_CONTINUE;
}

int main(int argc, char* argv[]) {
    int option = 0;
    int wasT = 0, wasN = 0, wasZ = 0, wasP = 0, wasErr = 0;
    double time = 0.0f;
    int quantity = 16;
    char* resourceName = NULL;
    char* processNameBeginning = NULL;
    char* fifoPath;
    while ((option = getopt(argc, argv, "t:n:z:p:")) != -1) {
        if (option == 't' && !wasT) {
            time = StringToDouble(optarg);
            wasT = 1;
        }
        else if (option == 'n' && !wasN) {
            quantity = StringToInt(optarg);
            if (quantity <= 4) {
                printf("Wrong parameter\n");
                exit(EXIT_FAILURE);
            }
            wasN = 1;
        }
        else if (option == 'z' && !wasZ) {
            resourceName = optarg;
            wasZ = 1;
        }
        else if (option == 'p' && !wasP) {
            processNameBeginning = optarg;
            wasP = 1;
        }
        else {
            wasErr = 1;
        }
    }
    if (argc - optind > 1 || !wasT || !wasZ || !wasP) {
        wasErr = 1;
    }

    if (wasErr) {
        printf("Usage: %s \n\
            \t-t <time>\n\
            \t-n <quantity | min 5, default 16>\n\
            \t-z <name of resource>\n\
            \t-p <beginning of the child name>\n\
            \t<path | optional>\n\
        ", argv[0]);
        return 1;
    }
    srand(getpid());
    if (optind < argc) {
        fifoPath = argv[optind];
    }
    else {
        // generate name
        fifoPath = (char*)alloca(64);
        char randomChars[6] = {0};
        for (int i = 0; i < 5; i++) {
            randomChars[i] = rand() % 26 + 65;
        }
        sprintf(fifoPath, "/tmp/urzadFIFO_%s", randomChars);
    }

    int fd[2];
    ERROR_CHECK(pipe(fd), "pipe")
    ERROR_CHECK(mkfifo(fifoPath, 0666), "mkfifo")

    for (int i = 0; i < quantity; i++) {
        // Generate process name
        char processName[256] = {0};
        char randomChars[6] = {0};
        for (int i = 0; i < 5; i++) {
            randomChars[i] = rand() % 26 + 65;
        }
        sprintf(processName, "%s%s", processNameBeginning, randomChars);
        
        int pid = fork();
        ERROR_CHECK(pid, "fork")
        else if (pid == 0) {
            // child
            ERROR_CHECK(close(fd[1]), "close")
            dup2(fd[0], 4);

            execl("./poszukiwacz.out", processName, "-z", resourceName, "-s", fifoPath, (char*)NULL);
        }
    }
    ERROR_CHECK(close(fd[0]), "close pipe") // pipe read end

    int fifoFdRead;
    ERROR_CHECK((fifoFdRead = open(fifoPath, O_RDWR)), "open fifo")

    // start the race
    ERROR_CHECK(close(fd[1]), "close pipe")

    // nanosleep section
    int sec = time;
    int nsec = (time - sec) * 1e9;
    struct timespec ts = { .tv_sec = sec, .tv_nsec = nsec };
    nanosleep(&ts, NULL);

    // end the race
    ERROR_CHECK(close(fifoFdRead), "close fifo")
    clock_gettime(CLOCK_REALTIME, &raceEndTime);

    // wait for children
    for (int i = 0; i < quantity; i++) {
        int status;
        ERROR_CHECK(wait(&status), "wait")
        if (!WIFEXITED(status) || (WIFEXITED(status) && WEXITSTATUS(status) != 0)) {
            printf("Err: Child didnt return correctly\n");
        }
    }

    // delete files forgotten and created after the race
    nftw(".", OnNFTW, 10, FTW_ACTIONRETVAL);

    unlink(fifoPath);
    return 0;
}