#include "./headers/commands.h"

int commands ( int argc, char* argv[], int bg_task_id ) {

	if ( argv[0] == NULL )
		return 0;

	if ( !strcmp( argv[0], "exit" ) ) {
		exit(EXIT_SUCCESS);
	}

	else if ( !strcmp( argv[0], "cd" ) ) {
		return cd(argc, argv);
	}

	else if ( !strcmp( argv[0], "ls" ) ) {
		return ls(argc, argv);
	}

	else if ( !strcmp( argv[0], "pwd" ) ) {
		printf("%s\n", cwd);
		return 0;
	}

	else if ( !strcmp( argv[0], "echo" ) ) {
		for ( int i = 1; argv[i] != NULL; i++ )
			printf("%s ", argv[i]);
		printf("\n");
		return 0;
	}

	else {
		if ( !bg_task_id ) {
			pid_t child_pid = fork();
			if ( child_pid > 0 ) {
				int status;
				waitpid( child_pid, &status, 0 );
				return WEXITSTATUS(status);
			} else if ( child_pid == 0 ) {
				int err = execvp(argv[0], argv);
				perror(argv[0]);
				exit(err);
			} else {
				perror("execvp");
				return -1;
			}
		} else {
			execvp(argv[0], argv);
			perror(argv[0]);
			return -1;
		}
	}
}

int ls ( int argc, char* argv[] ) {
	int FLAG_all = 0;
	int FLAG_long = 0;

	// dont print error
	opterr = 0;
	// reset getopt to start
	optind = 0;
	{
		loop:;
			 switch ( getopt(argc, argv, "al") ) {
				 case 'a' :
					 FLAG_all = 1;
					 break;
				 case 'l' :
					 FLAG_long = 1;
					 break;
				 case -1 :
					 goto processed;
				 case '?' :
					 fprintf(stderr, "ls: Invalid option %d\n", optopt);
					 return 0;
			 }
			 goto loop;
		processed:;
	}

	void (*printfn) ( char*, struct stat* ) = FLAG_long ? printlsl : printlsn;

	char** queries = &argv[optind];
	int query_count = argc - optind;
	if ( query_count == 0 ) {
		queries = (char* []) { ".", NULL };
		query_count = 1;
	}

	for ( int i = 0; i < query_count; i++ ) {
		char* query_path = parsePath( queries[i] );
		if ( !query_path ) {
			perror(queries[i]);
			return -1;
		}

		struct stat qst;
		if ( stat(query_path, &qst) == -1 ) {
			perror(query_path);
			return -1;
		}

		if ( S_ISDIR(qst.st_mode) ) {

			DIR* dir = opendir(query_path);
			if ( !dir ) {
				perror(query_path);
				return -1;
			}

			int num_entries = 0;
			struct dirent* entries[MAX_NUM_CONTENTS];

			struct dirent* entry;
			while ( (entry = readdir(dir)) != NULL ) {
				if ( FLAG_all || entry->d_name[0] != '.' ) {
					entries[num_entries++] = entry;
					if ( num_entries == MAX_NUM_CONTENTS )
						break;
				}
			}
			// TODO: sort

			if ( query_count > 1 )
				printf("%s:\n%d entries>\n", query_path, num_entries);

			for ( int i = 0; i < num_entries; i++ ) {
				char* entry_path = malloc( (strlen(query_path) + 1 + strlen(entries[i]->d_name) + 1) * sizeof(char) );
				strcpy(entry_path, query_path);
				strcat(entry_path, "/");
				strcat(entry_path, entries[i]->d_name);
				struct stat st;
				if ( stat(entry_path, &st) == -1 ) {
					perror(entry_path);
					free(entry_path);
					return -1;
				}

				printfn(entries[i]->d_name, &st);

				free(entry_path);
			}
			printf("\n");
			closedir(dir);
		} else {
			printfn(queries[i], &qst);
		}

		free(query_path);
	}
	return 0;
}

int cd ( int argc, char* argv[] ) {
	char* dir = NULL;

	// `.` and `..` are handled by chdir itself
	if ( argc > 2 ) {
		fprintf(stderr, "cd: Number of arguments exceeded\n");
		return -1;
	}

	if ( argv[1] == NULL ) {
		dir = strdup(homedir);
		if ( !dir ) {
			perror("malloc");
			return -1;
		}
	} else if ( !strcmp(argv[1], "-") )
		dir = strdup(owd);
	else
		dir = parsePath(argv[1]);

	if ( chdir(dir) == -1 ) {
		perror(dir);
		free(dir);
		return -1;
	}

	free(owd);
	owd = cwd;
	cwd = getcwd(NULL, 0);

	free(dir);
	return 0;
}
