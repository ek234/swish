#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

extern char* homedir;
extern char* cwd;
extern char* owd;
extern int pestatus;

int commands ( char* [], int );

#endif
