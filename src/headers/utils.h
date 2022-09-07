#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

extern char* homedir;
extern char* cwd;
extern char* owd;
extern int pestatus;

int filecmp ( const void* a, const void* b );
char* parsePath ( char* path );
void printlsl ( char*, struct stat* );
void printlsn ( char*, struct stat* );

#endif
