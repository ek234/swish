#ifndef DEF_H
#define DEF_H

#define MAX_COMMAND_LEN (size_t)8192+1
#define MAX_NUM_ARGS (size_t)200
#define MAX_USERNAME_LEN (size_t)256+1
#define MAX_BG_TASKS (size_t)100
#define MAX_NUM_CONTENTS (size_t)1024
#define MAX_HISTORY (size_t)20
#define MAX_PID_LEN (size_t)10

#define HISTORYFILE "~/.history.txt"

#define MAX_HOSTNAME_LEN _POSIX_HOST_NAME_MAX

// in order of priority
// ie, if one part of the command returns `CONTINUE_AFTER_SHELL_ERROR` and another
// part returns `EXIT_AFTER_SHELL_ERROR`` then the later will be returned
enum main_ite_returns {
	CONTINUE_NORMAL,
	CONTINUE_AFTER_SHELL_ERROR,
	EXIT_NORMAL,
	EXIT_AFTER_SHELL_ERROR
};

#endif
