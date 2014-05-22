#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

int numMax;
int* processId;

// параметр не используется, но для signal он нужен
void KillAllChildren (int m) {
    int i;
    for (i = 0; i < numMax; ++i)
        kill(processId[i], SIGKILL);
    exit(0);
}

// чтобы не нужно было жать ctrl+C для завершения отработавшей программы
void waiting() {
    int status;
    while (waitpid(0, &status, 0) != -1);
}

void process(char* buf, int descRead, int descWrite) {
    if (descWrite != 1)
        dup2(descWrite, 1);
    if (descRead != 0)
        dup2(descRead, 0);
    char** argv = NULL;
    int len = strlen(buf);
    int i;
    int args = 1;
    int argLen = 0;
    argv = (char**)malloc(2 * sizeof(char*));
    argv[0] = NULL;
    for (i = 0; i < len && isspace(buf[i]); ++i);
    for (; i < len; ++i) {
        if (!isspace(buf[i])) {
            argv[args - 1] = (char*)realloc(argv[args - 1], (argLen + 2) * sizeof(char));
            if (argv[args - 1] == NULL) {
                perror("Memory leaking\n");
                return;
            }
            argv[args - 1][argLen] = buf[i];
            ++argLen;
        }
        else {
            while (i + 1 < len && isspace(buf[i + 1]))
                ++i;
            if (i + 1 == len)
                break;
            argv[args - 1][argLen] = 0;
            argLen = 0;
            ++args;
            argv = (char**)realloc(argv, (args + 1) * sizeof(char*));
            if (argv == NULL) {
                perror("Memory leaking\n");
                return;
            }
            argv[args - 1] = (char*)malloc(sizeof(char));
            argv[args - 1][0] = 0;
        }
    }
    if (argLen == 0)
        --args;
    else
        argv[args - 1][argLen] = 0;
    argv[args] = NULL;
    int a = execvp(argv[0], argv);
    if (a == -1)
        printf("Error in execvp\n");
}

int divideComp(char* buf, int len, int descRead, int descWrite) {
    char** list = (char**)malloc(2 * sizeof(char*));
    if (list == NULL) {
        printf("Memory leaking\n");
        return 0;
    }
    int i;
    int curIndex = 0;
    int curPos = 0;
    list[curIndex] = NULL;
    for (i = 0; i < len; ++i) {
        if (buf[i] == '|') {
            list[curIndex][curPos] = 0;
            ++curIndex;
            list = (char**)realloc(list, (curIndex + 1) * sizeof(char*));
            if (list == NULL) {
                perror("Memory leaking\n");
                return 0;
            }
            list[curIndex] = (char*)malloc(2 * sizeof(char));
            curPos = 0;
            continue;
        }
        list[curIndex] = (char*)realloc(list[curIndex], (curPos + 2) * sizeof(char));
        if (list[curIndex] == NULL) {
            perror("Memory leaking\n");
            return 0;
        }
        list[curIndex][curPos] = buf[i];
        ++curPos;
    }
    if (curPos == 0)
        --curIndex;
    else
        list[curIndex][curPos] = 0;
    ++curIndex;
    numMax = curIndex;
    int** p = (int**)malloc((curIndex + 1) * sizeof(int*));
    processId = (int*)malloc((curIndex + 1) * sizeof(int*));
    if (p == NULL || processId == NULL) {
        perror("Memory leaking\n");
        return 0;
    }
    for (i = 0; i < curIndex; ++i)
        p[i] = (int*)malloc(2 * sizeof(int));
    for (i = 0; i < curIndex; ++i)
        pipe(p[i]);
    for (i = 0; i < curIndex; ++i) {
        processId[i] = fork();
        int dr = descRead, dw = descWrite;
        if (i > 0)
            dr = p[i - 1][0];
        if (i + 1 < curIndex)
            dw = p[i][1];
        if (processId[i] == 0) {
            int j;
            for (j = 0; j < curIndex; ++j) {
                if (j != i)
                    close(p[j][1]);
                if (j != i - 1)
                    close(p[j][0]);
            }
            if (i == 0)
                close(p[0][0]);
            if (i + 1 == curIndex)
                close(p[i][1]);
            process(list[i], dr, dw);
        }
        if (dr != 0)
            close(dr);
        if (dw != 1)
            close(dw);
    }
    waiting();
}

char* getFileName (char* buf, int len) {
    int i;
    char* res = NULL;
    int resLen = 0;
    for (i = 0; i < len; ++i) {
        if (!isspace(buf[i])) {
            while (i < len && !isspace(buf[i])) {
                res = (char*)realloc(res, (resLen + 2) * sizeof(char));
                if (res == NULL) {
                    perror("Memory leaking\n");
                    return NULL;
                }
                res[resLen] = buf[i];
                ++resLen;
                ++i;
            }
            res[resLen] = 0;
            while (i < len && isspace(buf[i]))
                ++i;
            if (i != len) {
                perror("Error in filename\n");
                return NULL;
            }
            return res;
        }
    }
    perror("Error in filename\n");
    return NULL;
}

int divideReadWrite(char* buf, int len) {
    int i;
    for (i = 0; i < len; ++i) {
        if (buf[i] == '<') {
            char* fileName = getFileName(buf + i + 1, len - i - 1);
            if (fileName == NULL)
                return 0;
            int desc = open(fileName, O_RDONLY);
            free(fileName);
            if (desc == -1) {
                perror("Can't open the file\n");
                return 0;
            }
            int f = divideComp(buf, i, desc, 1);
            return f;
        }
        if (buf[i] == '>') {
            char* fileName = getFileName(buf + i + 1, len - i - 1);
            if (fileName == NULL)
                return 0;
            int desc = open(fileName, O_WRONLY);
            free(fileName);
            if (desc == -1) {
                perror("Can't open the file\n");
                return 0;
            }
            int f = divideComp(buf, i, 0, desc);
            return f;
        }
    }
    int f = divideComp(buf, len, 0, 1);
    return f;
}

int divideAmp (char* buf, int len) {
    int i;
    int num = 1;
    int* borders = (int*)malloc((num + 1) * sizeof(int));

    if (borders == NULL) {
        perror("Memory leaking\n");
        return 0;
    }

    borders[0] = 0;
    for (i = 0; i < len; ++i) {
        if (buf[i] == '&') {
            ++num;

            // выделение памяти сделать лучше
            borders = (int*)realloc(borders, (num + 1) * sizeof(int));
            if (borders == NULL) {
                perror("Memory leaking\n");
                return 0;
            }

            borders[num - 1] = i + 1;
        }
    }
    borders[num] = len + 1;
    for (i = 0; i < num; ++i) {
        int k = fork();
        if (k == 0) {
            divideReadWrite(buf + borders[i], borders[i + 1] - borders[i] - 1);
            exit(0);
        }
    }
    waiting();
    free(borders);
    return 1;
}

int divideAndOr(char* buf, int len) {
    if (len <= 0) {
        perror("Error occurred\n");
        return 0;
    }
    int i;
    for (i = 0; i + 1 < len; ++i) {
        // &&
        if (buf[i] == '&' && buf[i + 1] == '&') {
            int f = divideAmp(buf, i - 1);
            f |= divideAndOr(buf + i + 2, len - i - 2);
            return f;
        }
        // ||
        if (buf[i] == '|' && buf[i + 1] == '|') {
            int f = divideAmp(buf, i - 1);
            if (f)
                return f;
            f |= divideAndOr(buf + i + 2, len - i - 2);
            return f;
        }
    }
    return divideAmp(buf, len);
}

int main() {
    // переписать с sigaction
    signal(SIGKILL, KillAllChildren);
    int need = 1;
    while (need) {
        printf("shell:~$ ");
        char* buf = (char*)malloc(sizeof(char));
        int len = 0;
        int c;
        // получение строки
        while ((c = getchar()) != '\n') {
            if (c == EOF) {
                need = 0;
                break;
            }
            ++len;
            //не лучшее управление памятью, быстрее блоками увеличивать длину строки
            buf = (char*)realloc(buf, (len + 1) * sizeof(char));
            if (buf == NULL) {
                perror("Memory leaking\n");
                return 0;
            }
            buf[len - 1] = c;
        }
        buf[len] = 0;
        int i, last = 0;
        for (i = 0; i < len; ++i) {
            if (buf[i] == ';') {
                if (!divideAndOr(buf + last, i - last))
                    perror("Error occurred\n");
                while (i + 1 < len && buf[i + 1] == ';')
                    ++i;
                last = i + 1;
            }
        }
        if (last < len)
            if (!(buf + last, len - last))
                perror("Error occurred\n");
        free(buf);
    }
    return 0;
}
