#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <locale.h>
#include <time.h>
#include <langinfo.h>
#include <fcntl.h>


//using namespace std;

void limiter(int period, char* progname, char* const * args)
{
    int pid;
    bool work = 1;
    if ( (pid = fork()) == 0)
    {// ребенок
        execvp(progname, args); // передаем дважды имя программы, в progname и в args[0]
    } else {
        while ( waitpid(pid, NULL, WNOHANG) == 0)
        {// состояние ребенка не изменилось
            if (work)
            {// если процесс выполняется
                kill(pid, SIGSTOP);
                usleep( (100-period)*1000); // usleep в мс
            } else {
                kill(pid, SIGCONT);
                usleep( (period)*1000);
            }
            work = !work;
        }
    }


}




int main(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Wrong number of arguments");
    }
    int time = atoi(argv[1]);
    if (time <= 0 || time >= 100)
    {
        fprintf(stderr, "Wrong time");
    }
    char** args = (char**)malloc( (argc - 2) * sizeof(char*));
    for (int i = 0; i < argc - 2; ++i)
    {
        args[i] = argv[i + 2];
    }
    args[argc - 2] = NULL;
    limiter(time, argv[2], (char* const *)args);

    return 0;
}

