#ifndef init_h
#define init_h

#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "./utils.h"
#include "./def.h"

int init ();
int deinit ();
void handle_signal ( int );

#endif
