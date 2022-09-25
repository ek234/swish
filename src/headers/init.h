#ifndef init_h
#define init_h

#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <termio.h>
#include "./utils.h"
#include "./def.h"

int init ();
void deinit ();
void handle_signal ( int );
int getnextbgid ();
void settermmode ( enum termmode );

#endif
