#ifndef prompt_h
#define prompt_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include "./def.h"
#include "./colors.h"

extern struct passwd* user_details;
extern char* username;
extern char* homedir;
extern char* hostname;
extern char* cwd;
extern char* owd;
extern int pestatus;
extern pid_t bg_tasks[];
extern int ptime;
extern int history_count;
extern char* history[];
extern int BASE_STDIN_FD;
extern int BASE_STDOUT_FD;

extern int commands ( int, char* [], int );

int prompt ();
void printprompt ();

#endif
