#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX 1024

int main()
{
    for (int i = 0; i < MAX; i++)
    {
        char mode[64];
        int flags = fcntl(i, F_GETFL);
        if (flags == -1)
            continue;
        flags = flags & O_ACCMODE;
        switch (flags) {
        case O_RDONLY:
            strcpy(mode, "O_RDONLY");
            break;
        case O_WRONLY:
            strcpy(mode, "O_WRONLY");
            break;
        case O_RDWR:
            strcpy(mode, "O_RDWR");
            break;
        default:
            strcpy(mode, "wtf");
        }

        printf("Deskryptor %d: %s\n", i, mode);
    }
    return 0;
}