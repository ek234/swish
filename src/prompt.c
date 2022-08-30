#include "./headers/prompt.h"

int printprompt () {
	char* cwd = getcwd(NULL, 0);
	char* dir = cwd;
	if (!strncmp(cwd, homedir, strlen(homedir))) {
		char* base = "~";
		dir = malloc(( strlen(base) + strlen(dir) - strlen(homedir) + 1 ) * sizeof(char));
		strcpy(dir, base);
		strcat(dir, cwd + strlen(homedir));
		free(cwd);
	}

	printf("%s@%s:%s> ", username, hostname, dir);

	free(dir);
	return 0;
}

int prompt () {

	printprompt();

	char* input = malloc(MAX_INP_LEN * sizeof(char));
	if ( !input ) {
		fprintf(stderr, "Input: Memory exceeded\n");
		return -1;
	}
	char* inp_str = fgets(input, MAX_ARGS_LEN, stdin);
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
				if (arg == NULL)
					break;
				args[num_args] = arg;
			}
			args[num_args] = NULL;

			pid_t child_pid;

			switch ( get_builtin_id( args[0] ) ) {

				case 0:
					return 1;
					break;

				case 1:
					if ( chdir(args[1]) == -1 ) {
						perror("chdir");
						return -1;
					}
					break;

				case 2:
					printf("%s\n", getcwd(NULL, 0));
					break;

				default:
					child_pid = fork();
					if ( !child_pid ) {
						execvp(args[0], args);
						perror(args[0]);
					} else {
						printf("\n%p\n%p\n", command_ptr+1, inp_ptr);
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

int get_builtin_id ( char* command ) {

	char* builtin_table[] = {
		"exit",
		"cd",
		"pwd",
	};

	for ( int i = 0; i < sizeof(builtin_table)/sizeof(builtin_table[0]) ; i++ ) {
		if ( ! strcmp(command, builtin_table[i]) )
			return i;
	}
	return -1;
}
