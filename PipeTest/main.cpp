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


long long mtime()
{
  struct timeval t;

  gettimeofday(&t, NULL);
  long long mt = (long long)t.tv_sec * 1000 + t.tv_usec / 1000;
  return mt;
}

// возвращает время начала записи
int writer(int pipedes, int bufsize, int partsize)
{
    int *buf = (int*)malloc(4 * (bufsize / 4)); // округляем bufsize
    for (int i = 0; i < bufsize / 4; ++i)
    {
        buf[i] = rand() % 1000;
    }
    int ostatok = bufsize / 4; // ужасное имя

    int writeres = write(pipedes, &ostatok, sizeof(int));
    long long starttime = mtime();
    if (writeres <= 0)
    {
        fprintf(stderr, "Write in the pipe problem \n");
        exit(1);
    }
    int ps;
    while (ostatok > 0)
    {
        ps = min(partsize, ostatok);

        writeres = write(pipedes, buf + ((bufsize / 4) - ostatok), ps);
        if (writeres < 0)
        {
            fprintf(stderr, "Write in the pipe problem %d\n", writeres);
            exit(1);
        }
        ostatok -= writeres;
    }
    write(pipedes, &starttime, sizeof(long long));
    write(pipedes, &partsize, sizeof(int));
    fprintf(stderr, "Writing has been complited\n");
    return starttime;
}

//возвращает время окончания приема данных
long long reader(int pipedes)
{
    int ostatok = 0;
    int readres;
    readres = read(pipedes, &ostatok, sizeof(int));
    if (ostatok != 0)
    {
        fprintf(stderr, "Connected! %d\n", ostatok);
    } else {
        fprintf(stderr, "Connection is failed %d!\n", readres);
        exit(1);
    }
    int *buf = (int*)malloc(4 * ostatok);
    int bufsize = ostatok * 4;
    while(ostatok > 0)
    {
        readres = read(pipedes, buf + bufsize / 4 - ostatok, ostatok);
        if (readres == 0)
        {
            fprintf(stderr, "%d bytes needed to read\n", ostatok);
            exit(1);
        }
        ostatok -= readres;
    }
    long long finishtime = mtime();
    long long starttime;
    int partsize;
    read(pipedes, &starttime, sizeof(long long));
    read(pipedes, &partsize, sizeof(int));
    fprintf(stderr, "All OK time is: %lld, Partsize is: %d\n", finishtime - starttime, partsize);
    return finishtime;
}

int main()
{
    srand(time(NULL));
    int pipedes[2]; // 0 на чтение, 1 на запись
    long long time;

    int pid;
    for(int i = 5; i <= 15; ++i)
    {
        pipe(pipedes); // для передачи данных от ребенка к родителю
        if ( (pid = fork()) == 0)
        {// ребенок
            close(pipedes[0]);
            writer(pipedes[1], 134217728 / 4, pow((double)2, (double)i));
            close(pipedes[1]);
            exit(0);
        } else {
         // родитель
            close(pipedes[1]);
            time = reader(pipedes[0]);
            close(pipedes[0]);
        }
    }
    return 0;
}

