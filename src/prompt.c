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

	char* input = malloc(MAX_INP_LEN * sizeof(char));
	if ( !input ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return -1;
	}
	char* inp_str = fgets(input, MAX_INP_LEN, stdin);
	if ( !inp_str ) {
		fprintf(stderr, "Input: Error reading input\n");
		return -1;
	}

	char* inp_ptr = NULL;
	while (1) {

		char* command = strtok_r(inp_str, ";\n", &inp_ptr);
		inp_str = NULL;
		if (command == NULL) {
			break;
		}

		int num_children = 0;

		char* command_ptr = NULL;
		while (1) {

			char* subcommand = strtok_r(command, "&", &command_ptr);
			command = NULL;
			if (subcommand == NULL) {
				break;
			}

			char* args[MAX_ARGS_LEN];

			char* args_ptr = NULL;
			size_t num_args;
			for ( num_args = 0; ; num_args++ ) {

				if ( num_args >= MAX_ARGS_LEN-1 ) {
					fprintf(stderr, "Arguments: Number of arguments exceeded\n");
					return -1;
				}

				char* arg = strtok_r(subcommand, " \t", &args_ptr);
				subcommand = NULL;
				args[num_args] = arg;
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
					dir = malloc( (strlen(homedir) + 1) * sizeof(char) );
					strcpy(dir, homedir);
				}
				else if ( args[1][0] == '~' ) {
					dir = malloc( (strlen(homedir) + strlen(args[1])-1 + 1) * sizeof(char) );
					strcpy(dir, homedir);
					strcat(dir, args[1]+1);
				//} else if ( args[1][0] == '.' ) {
				//	int num_dots = 0;
				//	while ( args[1][num_dots] == '.' )
				//		num_dots++;
				//	num_dots--;

				//	int endofpath = strlen(args[1])-1;
				//	while ( args[1][endofpath] == '/' )
				//		endofpath--;

				//	for ( ; endofpath >= 0; endofpath-- ) {
				//		if ( num_dots == 0 )
				//			goto found;
				//		if ( args[1][endofpath] == '/' )
				//			num_dots--;
				//	}
				//	dir = malloc( (strlen("/") + 1) * sizeof(char) );
				//	strcpy(dir, "/");
				//	found:;
				//	dir = malloc( (strlen(homedir) + strlen(args[1])-1 + 1) * sizeof(char) );
				//	strcpy(dir, homedir);
				//	strcat(dir, args[1]+1);
				}
				else if ( !strcmp(args[1], "-") ) {
					dir = malloc( (strlen(owd) + 1) * sizeof(char) );
					strcpy(dir, owd);
				}
				else {
					dir = malloc( (strlen(args[1]) + 1) * sizeof(char) );
					strcpy(dir, args[1]);
				}

				if ( chdir(dir) == -1 ) {
					perror(args[1]);
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
					if ( command_ptr+1 == inp_ptr )
						wait(NULL);
					else
						printf("[%d] started in bg\n", ++num_children);
				}
			}
		}
	}

	return 0;
}
