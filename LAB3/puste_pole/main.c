#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define BASE 10

int StringToInt(char *string)
{
    char *endptr = NULL;
    int val = strtol(string, &endptr, BASE);
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


int main(int argc, char* argv[])
{
    char c;
    char* path;
    int size = 0;
    int wasR = 0;
    while ((c = getopt(argc, argv, "r:")) != -1)
    {
        if (c == 'r' && !wasR)
        {
            size = StringToInt(optarg);
            wasR = 1;
        }
        else
        {
            printf("Usage: %s -r <size> <path>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    size *= 8000;
    path = argv[optind];
    
    int fd = open(
        path, 
        O_WRONLY | O_CREAT | O_EXCL, 
        S_IWGRP | 
        S_IWUSR |
        S_IWOTH |
        S_IRGRP |
        S_IRUSR | 
        S_IROTH
    );
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    lseek(fd, size, SEEK_SET);
    //write(fd, 0, 0);
    ftruncate(fd, size);
    close(fd);
    struct stat st = {0};
    
    stat(path, &st);
    printf("size/bytes: %ld\n", st.st_size); /* 'official' size in bytes */
    printf("block size/bytes: %ld\n", st.st_blksize);
    printf("blocks: %ld\n", st.st_blocks); /* number of blocks actually on disk */
    if (st.st_size > (st.st_blksize * st.st_blocks))  
       printf("file is (at least partially) a sparse file\n");

    return 0;
}