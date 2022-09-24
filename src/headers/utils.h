#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include "./colors.h"
#include "./def.h"

extern char* homedir;
extern char* cwd;
extern char* owd;
extern int pestatus;
extern int bg_tasks[];

int filecmp ( const void* a, const void* b );
char* parsePath ( char* path );
void printlsl ( char*, struct stat* );
void printlsn ( char*, struct stat* );
int getnextbgid ();

#endif
