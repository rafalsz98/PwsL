#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "func.h"

void copyConstant(char* arguments, int outputFd)
{
    const int no_data = 2;
    unsigned long long data[no_data];
    char* args = strtok(arguments, ", ");

    if (args == NULL)
    {
        printf("Wrong argument!\n");
        exit(EXIT_FAILURE);
    }

    char byte = *args;
    int i = 0;
    args = strtok(NULL, ", ");
    while (args != NULL)
    {
        if (i < no_data)
            data[i] = GetBytes(args);
        i++;
        args = strtok(NULL, ", ");
    }

    if (i != no_data)
    {
        printf("Wrong argument!\n");
        exit(EXIT_FAILURE);
    }

    lseek(outputFd, data[1], SEEK_SET);
    for (unsigned long long i = 0; i < data[0]; i++)
    {
        int n = write(outputFd, &byte, sizeof(char));
        if (n == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
}

void copyFrom(char* arguments, int inputFd, int outputFd)
{
    const int no_data = 3;
    unsigned long long data[no_data];

    char* args = strtok(arguments, ", ");
    int i = 0;
    while (args != NULL)
    {
        if (i < no_data)
            data[i] = GetBytes(args);
        args = strtok(NULL, ", ");
        i++;
    }
    if (i != no_data)
    {
        printf("Wrong arguments!\n");
        exit(EXIT_FAILURE);
    }

    lseek(inputFd, data[0], SEEK_SET);
    lseek(outputFd, data[2], SEEK_SET);
    
    char* readResult = (char*)calloc(data[1] + 1, sizeof(char));
    int n = read(inputFd, readResult, data[1]);
    if (n <= 0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    n = write(outputFd, readResult, data[1]);
    if (n == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }    

    free(readResult);
}

// Program is copying asked number of bytes from one file to another, or copies asked byte to output file
//  Parameters:
//      -I <InputPath> (must exist)
//      -O <OutputPath>
//      -c <Input offset from beginning>,<Number of bytes to copy>,<Output offset from beginning>
//      -d <byte>,<Number of bytes to copy>,<Output offset from beginning>
//  Note:
//      Offset is positive integer, can be given with suffix M, Mi, K, Ki, k, B
//      -c and -d can be given many times
int main(int argc, char* argv[])
{
    int inputFd = 0;
    int outputFd = 0;

    int wasI = 0;
    int wasO = 0;

    int option = 0;
    while ((option = getopt(argc, argv, "I:O:c:d:")) != -1)
    {
        if (option == 'I' && !wasI)
        {
            inputFd = open(optarg, O_RDONLY);
            if (inputFd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            wasI = 1;
        }
        else if (option == 'O' && !wasO)
        {
            outputFd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (outputFd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            wasO = 1;
        }
        else if (option == 'c' && wasI && wasO)
        {
            copyFrom(optarg, inputFd, outputFd);
        }
        else if (option == 'd' && wasI && wasO)
        {
            copyConstant(optarg, outputFd);
        }
        else
        {
            printf("Wrong parameters!\n");
            break;
        }
    }
    if (!wasI || !wasO)
    {
        printf("Usage: %s -I <path> -O <path> -c <D>,<C>,<W> -d <bajt>,<C>,<W>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return 0;
}