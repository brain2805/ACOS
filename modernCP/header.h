// программа modernCP при копировании файлов удаляет дырки, в отличие от cat file > file.copy, которая просто записывает
// дырки как последовательность '\0'
// размер блока stat.blksize = 4096
//
//
//
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <locale.h>
#include <time.h>
#include <langinfo.h>
#include <fcntl.h>


void print_error(const char * str)
{
    int len = 0;
    while (str[len++]);

    write(2, str, len);
    _exit(1);
}

int getlastpos(char* str, char c)
{
    int i = strlen(str);
    while( i > 0 && str[--i] != c)
        ;
    return i;
}

using namespace std;


