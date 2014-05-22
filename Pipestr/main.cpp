#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <locale.h>
#include <math.h>
#include <time.h>
#include <langinfo.h>
#include <fcntl.h>
#include <string>
#include <time.h>
using namespace std;

int counter(char* progname, char* const * args)
{
    int fd[2];
    pipe(fd);
    int pid;
    int strcount = 0;
    if ((pid = fork()) == 0)
    {// ребенок пишет
        close(fd[0]);
        dup2(fd[1], 1);
        execvp(progname, args);
        exit(0);
    } else {
     // родитель читает и считает
        close(fd[1]);
        dup2(fd[0], 0);
        char str[1024];
        fprintf(stderr, "Вход");
        while (gets(str))
        {
            ++strcount;
            fprintf(stderr,"%s\n", str);
        }

        return strcount;
    }
}

int main(int argc, char** argv)
{
    char** args = (char**)malloc( (argc - 1) * sizeof(char*));
    for (int i = 0; i < argc - 1; ++i)
    {
        args[i] = argv[i + 1];
    }
    args[argc - 1] = NULL;
    int strcount = counter(argv[1], args);
    fprintf(stderr,"The amount of strings is: %d\n", strcount);
    return 0;
}

