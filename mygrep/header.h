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
#include <string>

using namespace std;

const int BUFSIZE = 1024;

