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

void ls (const char * path, int l, int r)
{
    DIR* curdir = opendir(path);
    struct dirent* item;
    while ((item = readdir(curdir)) != NULL)
    {
        if ((strcmp(item->d_name, ".") == 0) || (strcmp(item->d_name, "..") == 0))
            continue;
        char* buf = (char*)malloc(strlen(path) + strlen(item->d_name) + 2);
        strcpy(buf, path);
        strcat(buf, "/");
        strcat(buf, item->d_name);

        struct stat info;
        stat(buf, &info);
        char rights[] = "---------"; // права
        if (info.st_mode & S_IRUSR) rights[0] = 'r';
        if (info.st_mode & S_IWUSR) rights[1] = 'w';
        if (info.st_mode & S_IXUSR) rights[2] = 'x';
        if (info.st_mode & S_IRGRP) rights[3] = 'r';
        if (info.st_mode & S_IWGRP) rights[4] = 'w';
        if (info.st_mode & S_IXGRP) rights[5] = 'x';
        if (info.st_mode & S_IROTH) rights[6] = 'r';
        if (info.st_mode & S_IWOTH) rights[7] = 'w';
        if (info.st_mode & S_IXOTH) rights[8] = 'x';

        int linknum = info.st_nlink; // количество жестких ссылок
        char* username = (getpwuid(info.st_uid))->pw_name; // получение имени пользователя
        char* groupname = (getgrgid(info.st_gid))->gr_name; // пролучение имени группы
        char strdate[256];
        strftime(strdate, sizeof(strdate), nl_langinfo(D_T_FMT), localtime(&info.st_mtime));// получение даты создания
        if (l)
        {
            printf("%s %d %s %s %d %s %s\n", rights, (int)linknum, username, groupname, (int)info.st_size, strdate, item->d_name);
        } else {
            printf("%s\n", item->d_name);
        }

        if (r == 1 && (item->d_type & DT_DIR)) // побитовым умножением проверяем маску
        {
            ls(buf, l, r);
        }
        free(buf);
    }

}

int main(int argc, char** argv)
{
    char l = 0;
    char r = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'l')
            {
                l = 1;
            }
            if (argv[i][1] == 'r')
            {
                r = 1;
            }
        }
    }
    ls(".", l, r);
    return 0;
}

