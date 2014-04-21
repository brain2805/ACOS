#include "header.h"

const int BUFSIZE = 16;




void filecp(char* frompath, char* topath)
{
    printf("\n filecp args: %s, %s ", frompath, topath);
    int fd1, fd2;
    char buf[BUFSIZE];
    if ( (fd1 = open(frompath, O_RDONLY)) < 0)
    {
        printf("modernCP: open problem\n");
    }
    if ( (fd2 = open(topath, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
    {
        printf("modernCP: open problem\n");
    }
    int readcnt;
    int writecnt;
    do
       {
           readcnt = read(fd1, buf, BUFSIZE);
           if (readcnt > 0)
           {
               writecnt = write(fd2, buf, readcnt);
               if (readcnt != writecnt)
               {
                   close(fd1);
                   close(fd2);
                   printf("%d %d read-write problem\n", readcnt, writecnt);
                   readcnt = 0;
               }
           }
           else if (readcnt < 0)
           {
               close(fd1);
               close(fd2);
               printf("%d read-write problem\n", readcnt);
               readcnt = 0;
           }
       } while (readcnt);
    close(fd1);
    close(fd2);
}

void cp (char* frompath, char* topath)
{
    //папка или файл?
    struct stat info;
    stat(frompath, &info);
    if (S_ISDIR(info.st_mode) == 1)
    {//если это папка -> создаем такую же папку
        rmdir(topath);
        mkdir(topath, info.st_mode & 0777);
        int i = getlastpos(frompath,'/'); // получаем позицию последнего вхождения '/' либо 0
        char* buf;
        if (i == 0)
        {//не нужно отрезать путь к папке
            buf = (char*)malloc(strlen(topath) + strlen(frompath) + 2);
            strcpy(buf, topath);
            strcat(buf, "/");
            strcat(buf, frompath);
        } else {
            buf = (char*)malloc(strlen(topath) + strlen(frompath) - i - 1 + 2);
            strcpy(buf, topath);
            strcat(buf, "/");
            strcat(buf, frompath + i + 1);
        }

        // теперь buf содержит имя новой папки
        printf("!! %s !!", buf);
        //создаем новую папку с именем, лежащим в buf

        mkdir(buf, info.st_mode & 0777); // права такие тк, они лежать самыми правыми битами в st_mode
        DIR* curdir = opendir(frompath);
        struct dirent* item;
        // ходим по директории и рекурсивно запускаемся
        while ((item = readdir(curdir)) != NULL)
        {
          //  printf("!! %s !!", item->d_name);
            if ((strcmp(item->d_name, ".") == 0) || (strcmp(item->d_name, "..") == 0))
                continue;
            char* buf1 = (char*)malloc(strlen(frompath) + strlen(item->d_name) + 2);
            strcpy(buf1, frompath);
            strcat(buf1, "/");
            strcat(buf1, item->d_name);
            if (S_ISDIR(item->d_type))
            {
                cp(buf1, buf);
            } else {
                char * buf2 = (char*)malloc(strlen(buf) + strlen(item->d_name) + 2);
                strcpy(buf2, buf);
                strcat(buf2, "/");
                strcat(buf2, item->d_name);
                cp(buf1, buf2);
                free(buf2);
            }
        }
    } else {
     // это файл -> простое копирование
        filecp(frompath, topath);
    }

}


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("wrong number of arguments\n");
    }
    //printf("%s, %s", argv[1], argv[2]);
    cp(argv[1], argv[2]);



    /*struct stat info;
    stat("file.txt", &info);
    printf("%d %d \n", info.st_blksize, info.st_blocks);

    printf("%d, %d \n", (int)(char)'0', (int)(char)'\0');


    char buf1[] = "qwerrtyyuuiop";
    if ( (fd = open("file.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0)
        printf("create_problem\n");
    printf("%d %d\n", fd, strlen(buf1));
    if (write(fd, "qwerrtyyuuiop", 12) != 12)
       printf("write_problem\n");
    lseek(fd, 16512384, SEEK_SET);
    if (write(fd, buf1, 12) != 12)
        printf("write_problem\n");
     close(fd);
*/
    exit(0);

    return 0;
}

