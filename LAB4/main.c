#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX 1024

char* GetReadWriteFlag(int flags)
{
    char* result = NULL;
    switch (flags) {
    case O_RDONLY:
        result = "O_RDONLY";
        break;
    case O_WRONLY:
        result = "O_WRONLY";
        break;
    case O_RDWR:
        result = "O_RDWR";
        break;
    default:
        result = "wtf";
    }
}

int main()
{
    for (int i = 0; i < MAX; i++)
    {
        int flags = fcntl(i, F_GETFL);
        if (flags == -1)
            continue;
        char* flag = GetReadWriteFlag(flags & O_ACCMODE);
        printf("Deskryptor %d: %s", i, flag);
        
        if (flags & O_APPEND) {
            printf(" | O_APPEND");
        }
        if (flags & O_ASYNC) {
            printf(" | O_ASYNC");
        }
        if (flags & O_SYNC) {
            printf(" | O_SYNC");
        }
        if (flags & O_NONBLOCK) {
            printf(" | O_NONBLOCK");
        }
        printf("\n");
    }
    return 0;
}