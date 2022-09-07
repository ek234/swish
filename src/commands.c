#include "./headers/commands.h"

int commands ( char* args[], int bg_task_id ) {
	if ( args[0] == NULL )
		return 0;

	if ( !strcmp( args[0], "exit" ) ) {
		exit(EXIT_SUCCESS);
	}

	else if ( !strcmp( args[0], "cd" ) ) {

		char* dir = NULL;

		// `.` and `..` are handled by chdir itself
		if ( args[1] == NULL ) {
			dir = malloc( ( strlen(homedir) + 1 ) * sizeof(char) );
			if ( !dir ) {
				perror("malloc");
				return -1;
			}
			strcpy(dir, homedir);
		}
		else if ( args[2] != NULL ) {
			fprintf(stderr, "cd: Number of arguments exceeded\n");
			return -1;
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
		if ( !bg_task_id ) {
			pid_t child_pid = fork();
			if ( child_pid > 0 ) {
				int status;
				waitpid( child_pid, &status, 0 );
				return WEXITSTATUS(status);
			} else if ( child_pid == 0 ) {
				execvp(args[0], args);
				perror(args[0]);
				return -1;
			} else {
				perror("execvp");
				return -1;
			}
		} else {
			execvp(args[0], args);
			perror(args[0]);
			return -1;
		}
	}

	return 0;
}
