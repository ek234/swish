#include "./headers/prompt.h"

void printprompt () {
	printf( CYN "%s" RESET "@" MAG "%s" RESET ":" YEL , username, hostname);
	if (!strncmp(cwd, homedir, strlen(homedir)))
		printf("~%s" , &cwd[ strlen(homedir) ]);
	else
		printf("%s", cwd);
	printf( RESET "|" RED );
	if ( ptime >= 1 )
		printf("%d", ptime);
	printf( RESET ">> " );
}

int prompt () {

	printprompt();

	int final_ret_val = CONTINUE_NORMAL;

	char* input = NULL;
	size_t input_len = 0;
	int len_read = getline(&input, &input_len, stdin);
	if ( !input ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return CONTINUE_AFTER_SHELL_ERROR;
	}
	if ( len_read < 0 ) {
		// error reading input: include EOF
		// TODO: find better way to handle EOF
		free(input);
		return EXIT_NORMAL;
	} else if ( len_read == 0 ) {
		// TODO: decide whether to run this scenerio anyway
		free(input);
		return CONTINUE_NORMAL;
	}
	// remove trailing newline
	for ( int i = len_read - 1; i >= 0 && input[i] == '\n'; i-- )
		input[i] = '\0';

	char* input_cp = strdup(input);
	if ( !input_cp ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return CONTINUE_AFTER_SHELL_ERROR;
	}

	char* input_ite = input_cp;
	while (1) {

		char* command = strtok_r(NULL, ";&", &input_ite);
		if (command == NULL)
			break;

		int bg_task_id = 0;
		if ( input[input_ite-1-input_cp] == '&' ) {
			// BG task
			for ( int i = 0; i < MAX_BG_TASKS; i++ ) {
				if (bg_tasks[i] == 0) {
					bg_task_id = i+1;
					break;
				}
			}
			if ( bg_task_id == 0 ) {
				fprintf(stderr, "Background tasks: Number of background tasks exceeded\n");
				return CONTINUE_AFTER_SHELL_ERROR;
			}
		}
		else
			// FG task
			bg_task_id = 0;

		char* args[MAX_ARGS_LEN];
		size_t num_args = 0;
		char* command_ite = command;
		while (1) {

			if ( num_args >= MAX_ARGS_LEN ) {
				fprintf(stderr, "Arguments: Number of arguments exceeded\n");
				return CONTINUE_AFTER_SHELL_ERROR;
			}

			char* arg = strtok_r(NULL, " \t", &command_ite);
			args[num_args++] = arg;
			if (arg == NULL)
				break;
		}

		if ( !!bg_task_id ) {
			pid_t child_pid = fork();
			if (child_pid > 0) {
				printf("[%d] %d started %s\n", bg_task_id, child_pid, args[0]);
				bg_tasks[bg_task_id-1] = child_pid;
			} else if ( child_pid == 0 ) {
				// do not store return in pestatus
				exit(commands(num_args-1, args, bg_task_id));
			} else {
				perror("subshell");
				return CONTINUE_AFTER_SHELL_ERROR;
			}
		} else {
			time_t start = time(NULL);
			int ret_val = commands(num_args-1, args, bg_task_id);
			if ( ret_val > final_ret_val )
				final_ret_val = ret_val;
			ptime = time(NULL) - start;
		}
	}

	for ( int i = 0; i < MAX_BG_TASKS; i++ )
		if ( !!bg_tasks[i] ) {
			int status;
			pid_t pid = waitpid(bg_tasks[i], &status, WNOHANG | WUNTRACED);
			if ( pid < 0) {
				printf("pid %d by task %d id %d ", pid, i+1, bg_tasks[i]);
				perror("bg task");
				return CONTINUE_AFTER_SHELL_ERROR;
			} else if ( pid == bg_tasks[i] ) {
				if ( WIFEXITED(status) ) {
					fprintf(stderr, "[%d] %d exited with status %d\n", i+1, bg_tasks[i], WEXITSTATUS(status));
					bg_tasks[i] = 0;
				} else if ( WIFSIGNALED(status) ) {
					fprintf(stderr, "[%d] %d killed by signal %d\n", i+1, bg_tasks[i], WTERMSIG(status));
					bg_tasks[i] = 0;
				} else if ( WIFSTOPPED(status) ) {
					fprintf(stderr, "[%d] %d stopped by signal %d\n", i+1, bg_tasks[i], WSTOPSIG(status));
				}
			}
		}

	fflush(stdout);
	free(input_cp);

	// if history count is between 0 and MAX_HISTORY, then the other elements are empty.
	// if history count is greater than MAX_HISTORY, then the other elements are filled.
	// the true start of the history is at index history_count % MAX_HISTORY up till (history_count-1) % MAX_HISTORY
	if ( ( history_count != 0 && !strcmp(input, history[ (history_count-1) % MAX_HISTORY ]) ) || !strcmp( input, "" ) ) {
		free(input);
	} else {
		if ( history_count < MAX_HISTORY ) {
			history[history_count++] = input;
		} else {
			// free the earliest element and put the new input there
			free(history[history_count%MAX_HISTORY]);
			history[history_count%MAX_HISTORY] = input;
			// increment history_count such that the modulo works while keeping history_count >= MAX_HISTORY
			history_count = MAX_HISTORY + (history_count+1) % MAX_HISTORY;
		}
	}

	return final_ret_val;
}
