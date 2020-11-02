#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>

int StringToInt(char *string)
{
    char *endptr = NULL;
    int val = strtol(string, &endptr, 0);
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

off_t GetRandomOffset(size_t max, off_t* arr, int sizeOfArr, int i)
{
    while (1)
    {
        int boolean = 0;
        off_t val = rand() % max;
        for (int j = 0; j < sizeOfArr; j++)
        {
            if (arr[j] == val)
                boolean = 1;
        }
        if (!boolean)
        {
            arr[i] = val;
            return val;
        }
    }
}

int main(int argc, char* argv[])
{
    char c;
    char* path;
    srand(time(NULL));
    int wasS = 0, wasF = 0, argErr = 0;
    int format = 0; // 0 - liczba, 1 - tekst
    while ((c = getopt(argc, argv, "s:f:")) != -1)
    {
        if (c == 's' && !wasS)
        {
            path = optarg;
            wasS = 1;
        }
        else if (c == 'f' && !wasF)
        {
            if (strcmp(optarg, "tekst") == 0)
                format = 1;
            else if (strcmp(optarg, "liczba") != 0)
                break;
            wasF = 1;
        }
        else
        {
            argErr = 1;
        }
    }
    if (!wasF || !wasS || argErr)
    {
        printf("Usage: %s -s <path> -f <format(tekst | liczba)> val1...valn\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(path, O_RDWR);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat st = {0};
    if (stat(path, &st) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    if (!format && (argc - optind) > st.st_size)
    {
        printf("Not enough space in file\n");
        exit(EXIT_FAILURE);
    }
    else if (format)
    {
        int size = 0;
        for (int i = optind; i < argc; i++)
        {
            size += strlen(argv[i]);
        }       
        if (size > st.st_size)
        {
            printf("Not enough space in file\n");
            exit(EXIT_FAILURE);
        }
    }

    int sizeOfArr = argc - optind;
    off_t* arr = (off_t*)calloc(sizeof(off_t), sizeOfArr);

    for (int i = optind; i < argc; i++)
    {
        lseek(fd, GetRandomOffset(st.st_size, arr, sizeOfArr, i), SEEK_SET);
        if (!format)
        {
            unsigned char val = StringToInt(argv[i]);
            if (write(fd, &val, sizeof(val)) <= 0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            for (int j = 0; j < strlen(argv[i]); j++)
            {
                unsigned char val = argv[i][j];
                if (write(fd, &val, sizeof(val)) <= 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                lseek(fd, GetRandomOffset(st.st_size, arr, sizeOfArr, i), SEEK_SET);
            }
        }
    }
    for (int i = 0; i < sizeOfArr; i++)
    {
        printf("%ld  ", arr[i]);
    }
    printf("\n");

    free(arr);
    return 0;
}