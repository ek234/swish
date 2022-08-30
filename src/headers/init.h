#ifndef init_h
#define init_h

#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_USERNAME_LEN (size_t)256+1
#define MAX_HOSTNAME_LEN _POSIX_HOST_NAME_MAX

int init ();
int deinit ();

#endif
