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

	char input[MAX_COMMAND_LEN];
	size_t input_len = 0;

	while (1) {
		int c = fgetc(stdin);
		switch (c) {
			case '\n' :
				input[input_len] = '\0';
				goto input_read;
			case '\t' :
				// TODO
				break;
			case EOF :
				return EXIT_NORMAL;
			default :
				if ( input_len > MAX_COMMAND_LEN-1 ) {
					// TODO
				}
				input[input_len++] = c;
		}
	}
	input_read:;

	char* input_ref = strndup(input, input_len);
	if ( !input_ref ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return CONTINUE_AFTER_SHELL_ERROR;
	}

	char* input_ite = input;
	while (1) {

		char* command = strtok_r(NULL, ";&", &input_ite);
		if (command == NULL)
			break;

		int bg_task_id = 0;
		if ( input_ref[input_ite-1-input] == '&' ) {
			// BG task;;; other wise it is FG task
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

		int infd = STDIN_FILENO;
		int outfd = STDOUT_FILENO;

		char* args[MAX_NUM_ARGS];
		size_t num_args = 0;
		char* command_ite = command;
		while (1) {
			char* arg = strtok_r(NULL, " \t", &command_ite);

			if ( !!arg && !strncmp( arg, ">>", 2 ) ) {
				if ( outfd != STDOUT_FILENO ) {
					fprintf(stderr, "Redirection: Multiple output redirections not allowed\n");
					return CONTINUE_AFTER_SHELL_ERROR;
				}
				char* file = arg+2;
				if ( !*file ) {
					file = strtok_r(NULL, " \t", &command_ite);
					if ( !file ) {
						fprintf(stderr, "Redirection: No file specified for output redirection\n");
						return CONTINUE_AFTER_SHELL_ERROR;
					}
				}
				outfd = open( file, O_WRONLY | O_CREAT | O_APPEND, 0644 );
				if ( outfd == -1 ) {
					perror("output redirection");
					return CONTINUE_AFTER_SHELL_ERROR;
				}
			}
			else if ( !!arg && !strncmp( arg, ">", 1 ) ) {
				if ( outfd != STDOUT_FILENO ) {
					fprintf(stderr, "Redirection: Multiple output redirections not allowed\n");
					return CONTINUE_AFTER_SHELL_ERROR;
				}
				char* file = arg+1;
				if ( !*file ) {
					file = strtok_r(NULL, " \t", &command_ite);
					if ( !file ) {
						fprintf(stderr, "Redirection: No file specified for output redirection\n");
						return CONTINUE_AFTER_SHELL_ERROR;
					}
				}
				outfd = open( file, O_WRONLY | O_CREAT | O_TRUNC, 0644 );
				if ( outfd == -1 ) {
					perror("output redirection");
					return CONTINUE_AFTER_SHELL_ERROR;
				}
			}
			else if ( !!arg && !strncmp( arg, "<", 1 ) ) {
				if ( infd != STDIN_FILENO ) {
					fprintf(stderr, "Redirection: Multiple input redirections not allowed\n");
					return CONTINUE_AFTER_SHELL_ERROR;
				}
				char* file = arg+1;
				if ( !*file ) {
					file = strtok_r(NULL, " \t", &command_ite);
					if ( !file ) {
						fprintf(stderr, "Redirection: No file specified for input redirection\n");
						return CONTINUE_AFTER_SHELL_ERROR;
					}
				}
				infd = open( file, O_RDONLY );
				if ( infd == -1 ) {
					perror("input redirection");
					return CONTINUE_AFTER_SHELL_ERROR;
				}
			}
			else {
				if ( num_args >= MAX_NUM_ARGS ) {
					fprintf(stderr, "Arguments: Number of arguments exceeded\n");
					return CONTINUE_AFTER_SHELL_ERROR;
				}

				args[num_args++] = arg;
			}

			if (arg == NULL)
				break;
		}

		if ( !!bg_task_id ) {
			pid_t child_pid = fork();
			if (child_pid > 0) {
				if ( infd != STDIN_FILENO )
					close(infd);
				if ( outfd != STDOUT_FILENO )
					close(outfd);
				printf("[%d] %d started %s\n", bg_task_id, child_pid, args[0]);
				bg_tasks[bg_task_id-1] = child_pid;
			} else if ( child_pid == 0 ) {
				// do not store return in pestatus
				dup2(  infd,  STDIN_FILENO );
				dup2( outfd, STDOUT_FILENO );
				exit(commands(num_args-1, args, bg_task_id));
			} else {
				perror("subshell");
				return CONTINUE_AFTER_SHELL_ERROR;
			}
		} else {

			if ( dup2( infd, STDIN_FILENO ) == -1 ) {
				perror("setting stdin");
				return CONTINUE_AFTER_SHELL_ERROR;
			}
			if ( infd != STDIN_FILENO )
				close(infd);

			if ( dup2( outfd, STDOUT_FILENO ) == -1 ) {
				perror("setting stdout");
				return CONTINUE_AFTER_SHELL_ERROR;
			}
			if ( outfd != STDOUT_FILENO )
				close(outfd);

			time_t start = time(NULL);
			int ret_val = commands(num_args-1, args, bg_task_id);
			if ( ret_val > final_ret_val )
				final_ret_val = ret_val;
			ptime = time(NULL) - start;

			if ( (dup2(  BASE_STDIN_FD,  STDIN_FILENO )) == -1 ) {
				perror("resetting stdin");
				return CONTINUE_AFTER_SHELL_ERROR;
			}
			if ( (dup2( BASE_STDOUT_FD, STDOUT_FILENO )) == -1 ) {
				perror("resetting stdout");
				return CONTINUE_AFTER_SHELL_ERROR;
			}
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

	// if history count is between 0 and MAX_HISTORY, then the other elements are empty.
	// if history count is greater than MAX_HISTORY, then the other elements are filled.
	// the true start of the history is at index history_count % MAX_HISTORY up till (history_count-1) % MAX_HISTORY
	if ( ( history_count != 0 && !strcmp(input_ref, history[ (history_count-1) % MAX_HISTORY ]) ) || !strcmp( input_ref, "" ) ) {
		free(input_ref);
	} else {
		if ( history_count < MAX_HISTORY ) {
			history[history_count++] = input_ref;
		} else {
			// free the earliest element and put the new input there
			free(history[history_count%MAX_HISTORY]);
			history[history_count%MAX_HISTORY] = input_ref;
			// increment history_count such that the modulo works while keeping history_count >= MAX_HISTORY
			history_count = MAX_HISTORY + (history_count+1) % MAX_HISTORY;
		}
	}

	return final_ret_val;
}
