#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "./colors.h"
#include "./def.h"

extern char* homedir;
extern char* cwd;
extern char* owd;
extern int pestatus;

extern int filecmp( const void* a, const void* b );
extern char* parsePath ( char* );
extern void printlsl ( char*, struct stat* );
extern void printlsn ( char*, struct stat* );

int commands ( int, char* [], int );
int cd ( int, char* [] );
int ls ( int, char* [] );

#endif
