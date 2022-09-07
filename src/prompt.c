#include "./headers/prompt.h"

int printprompt () {
	if (!strncmp(cwd, homedir, strlen(homedir))) {
		char* base = "~";
		char* dir = malloc(( strlen(base) + strlen(cwd) - strlen(homedir) + 1 ) * sizeof(char));
		strcpy(dir, base);
		strcat(dir, cwd + strlen(homedir));
		printf("%s@%s:%s> ", username, hostname, dir);
		free(dir);
	} else {
		printf("%s@%s:%s> ", username, hostname, cwd);
	}
	return 0;
}

int prompt () {

	printprompt();

	char* input = NULL;
	size_t input_len = 0;
	int len_read = getline(&input, &input_len, stdin);
	if ( !input ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return -1;
	}
	if ( len_read < 0 ) {
		// error reading input: include EOF
		// TODO: find better way to handle EOF
		printf("\n");
		free(input);
		exit(0);
	} else if ( len_read == 0 ) {
		free(input);
		return 0;
	}

	// replace new line char with `;`
	while ( strchr(input, '\n') )
		*(strchr(input, '\n')) = ';';

	char* input_cp = strdup(input);
	if ( !input_cp ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return -1;
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
				return -1;
			}
		}
		else if ( input[input_ite-1-input_cp] == ';' )
			// FG task
			bg_task_id = 0;
		else
			// this should never reach
			exit(0);

		char* args[MAX_ARGS_LEN];
		size_t num_args = 0;
		char* command_ite = command;
		while (1) {

			if ( num_args >= MAX_ARGS_LEN ) {
				fprintf(stderr, "Arguments: Number of arguments exceeded\n");
				return -1;
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
				exit(commands(args, bg_task_id));
			} else {
				perror("subshell");
				return -1;
			}
		} else {
			pestatus = commands(args, bg_task_id);
		}

		for ( int i = 0; i < MAX_BG_TASKS; i++ )
			if ( !!bg_tasks[i] ) {
				int status;
				pid_t pid = waitpid(bg_tasks[i], &status, WNOHANG | WUNTRACED);
				if ( pid < 0) {
					perror("waitpid");
					return -1;
				} else if ( pid == bg_tasks[i] ) {
					if ( WIFEXITED(status) ) {
						printf("[%d] %d exited with status %d\n", i+1, bg_tasks[i], WEXITSTATUS(status));
						bg_tasks[i] = 0;
					} else if ( WIFSIGNALED(status) ) {
						printf("[%d] %d killed by signal %d\n", i+1, bg_tasks[i], WTERMSIG(status));
						bg_tasks[i] = 0;
					} else if ( WIFSTOPPED(status) ) {
						printf("[%d] %d stopped by signal %d\n", i+1, bg_tasks[i], WSTOPSIG(status));
					}
				}
			}
	}

	fflush(stdout);
	free(input);
	free(input_cp);
	return 0;
}
