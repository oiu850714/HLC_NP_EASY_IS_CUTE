#include <cstdio>
#include <sys/stat.h>

#include "UDP_socketutil.h"


#ifndef FILE_UTIL
#define FILE_UTIL
FILE* Fopen(const char *file_name, const char* mode);
off_t get_file_size(FILE *fp);


#endif