#ifndef prompt_h
#define prompt_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_INP_LEN (size_t)5000+1
#define MAX_ARGS_LEN (size_t)200+1

extern struct passwd* user_details;
extern char* username;
extern char* homedir;
extern char* hostname;

extern char* cwd;
extern char* owd;

int prompt ();
int get_builtin_id ();
int printprompt ();

#endif
