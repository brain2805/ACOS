#include "header.h"


bool findinstr(const char* str, const char* pattern)
{
    const string s(str);
    const string pat(pattern);
    return (s.find(pat) == string::npos)? 0:1;
}

void findinfile(const char* path, const char* pattern)
{
    FILE* file = fopen(path, "r");
    char* str = (char*)malloc(BUFSIZE);
    while (fgets(str, BUFSIZE, file) != NULL)
    {
        if (findinstr(str, pattern))// если поиск удачный
        {
             printf("find in: %s: %s\n", path, str);
        }
    }
}

void find(const char* path, const char* pattern)
{
    struct stat info;
    stat(path, &info);
    if (S_ISDIR(info.st_mode))
    {// это папка
        DIR* curdir = opendir(path);
        struct dirent* item;
        while( (item = readdir(curdir)) != NULL)
        {
            if ((strcmp(item->d_name, ".") == 0) || (strcmp(item->d_name, "..") == 0))
                continue;
            char* buf1 = (char*)malloc(strlen(path) + strlen(item->d_name) + 2);
            strcpy(buf1, path);
            strcat(buf1, "/");
            strcat(buf1, item->d_name);
            find(buf1, pattern);
        }

    } else {
        // файл
        findinfile(path, pattern);
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Wrong number of arguments");
        exit(18);
    }
    find(argv[1], argv[2]);
    return 0;
}

