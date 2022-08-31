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

	char* input = malloc( (MAX_INP_LEN + 1) * sizeof(char));
	if ( !input ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return -1;
	}
	if ( ! fgets(input, MAX_INP_LEN, stdin) ) {
		fprintf(stderr, "Input: Error reading input\n");
		return -1;
	}

	for ( char* input_str = input, * input_ptr = NULL; ; input_str = NULL ) {

		char* command = strtok_r(input_str, ";\n", &input_ptr);
		if (command == NULL)
			break;

		int num_children = 0;

		for ( char* command_str = command, * command_ptr = NULL; ; command_str = NULL ) {

			char* subcommand = strtok_r(command_str, "&", &command_ptr);
			if (subcommand == NULL)
				break;
			// TODO: make this prettier
			int isBG = strrchr(subcommand, '\0') < input_ptr-1;

			char* args[MAX_ARGS_LEN+1];

			size_t num_args = 0;
			for ( char* subcommand_str = subcommand, * subcommand_ptr = NULL ; ; subcommand_str = NULL ) {

				if ( num_args >= MAX_ARGS_LEN ) {
					fprintf(stderr, "Arguments: Number of arguments exceeded\n");
					return -1;
				}

				char* arg = strtok_r(subcommand_str, " \t", &subcommand_ptr);
				args[num_args++] = arg;
				if (arg == NULL)
					break;
			}

			if ( args[0] == NULL )
				continue;

			if ( !strcmp( args[0], "exit" ) ) {
				return 1;
			}
			else if ( !strcmp( args[0], "cd" ) ) {

				char* dir = NULL;

				// `.` and `..` are handled by chdir itself
				if ( args[1] == NULL ) {
					dir = malloc( ( strlen(homedir) + 1 ) * sizeof(char) );
					strcpy(dir, homedir);
				}
				else if ( args[1][0] == '~' ) {
					dir = malloc( ( strlen(homedir) + strlen(args[1])-1 + 1 ) * sizeof(char) );
					strcpy(dir, homedir);
					strcat(dir, args[1]+1);
				}
				else if ( !strcmp(args[1], "-") ) {
					dir = malloc( ( strlen(owd) + 1 ) * sizeof(char) );
					strcpy(dir, owd);
				}
				else {
					dir = malloc( ( strlen(args[1]) + 1 ) * sizeof(char) );
					strcpy(dir, args[1]);
				}

				if ( chdir(dir) == -1 ) {
					perror(dir);
					free(dir);
					return -1;
				}

				free(owd);
				owd = cwd;
				cwd = getcwd(NULL, 0);

				free(dir);
			}
			else if ( !strcmp( args[0], "pwd" ) ) {
				printf("%s\n", cwd);
			}
			else if ( !strcmp( args[0], "echo" ) ) {
				for ( int i = 1; args[i] != NULL; i++ )
					printf("%s ", args[i]);
				printf("\n");
			}
			else {
				pid_t child_pid = fork();
				if ( !child_pid ) {
					execvp(args[0], args);
					perror(args[0]);
				} else {
					if ( isBG )
						printf("[%d] %d\n", ++num_children, child_pid);
					else
						wait(NULL);
					fflush(stdout);
				}
			}
		}
	}

	free(input);
	return 0;
}
