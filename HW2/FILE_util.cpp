#include <cstdio>
#include <cstdlib>
#include "FILE_util.h"


FILE* Fopen(const char *file_name, const char* mode)
{
    FILE *fp = fopen(file_name, mode);
    if(!fp)
    {
        printf("open file failed.\n");
        exit(1);
    }
}

off_t get_file_size(FILE *fp)
{
    int fp_fd = fileno(fp);
    struct stat file_stat;
    if(fstat(fp_fd, &file_stat) != 0)
    {
        printf("get file stat failed.\n");
        exit(1);
    }

    return file_stat.st_size;
}