#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

double func(double x)
{
    double res = x * x;
    return res;
}

double calculate(double left, double right, double delta, double (*func)(double))
{
    int parts = (int)((right - left) / delta);
    double sum = 0;
    for (int i = 0; i < parts; ++i)
    {
        sum += func(left + i*delta + delta/2) * delta;
    }
    return sum;
}

double integrator(double left, double right, double delta, int processes, double (*func)(double))
{
    double partlefts[processes + 1];
    int fd = open("temp.txt", O_CREAT | O_TRUNC | O_WRONLY, 0777);
       // разбиение отрезка на равномерные части для процессов
    for (int i = 0; i <= processes; ++i)
    {
        partlefts[i] = left + (1.0*i/processes)*(right - left);
    }
    for (int i = 0; i < processes; ++i)
    {
        if (fork() != 0)
        {

        } else {
            double res = calculate(partlefts[i], partlefts[i+1], delta, func);
            fprintf(stderr, "%lf %lf %lf %lf\n", partlefts[i], partlefts[i+1], delta, res);
            write(fd, &res, sizeof(double));
            close(fd);
            exit(0);
        }
    }
    for (int i = 0; i < processes; ++i)
    {
        wait(NULL);
    }
    close(fd);

    fd = open("temp.txt", O_RDONLY);
    double result = 0;
    double temp;
    for (int i = 0; i < processes; ++i)
    {
        read(fd, &temp, sizeof(double));
        fprintf(stderr, "%lf\n", temp);
        result += temp;
    }
    fprintf(stderr, "%lf result \n", result);
    return result;
}

int main(int argc, char** argv)
{
 /*   double scan1;
    sscanf(argv[1], "%lf", &scan1);
    double scan2;
    sscanf(argv[2], "%lf", &scan2);
    double scan3;
    sscanf(argv[3], "%lf", &scan3);
    double scan4;
    sscanf(argv[4], "%lf", &scan4); */
   // printf("%lf\n", integrator(0.0, 100.0, 0.002, 3, func));
    integrator(0.0, 10.0, 0.0002, 10, func);
   // printf("%lf\n", calculate(0.0, 10.0, 0.02, func));
    return 17;
}

