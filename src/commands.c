#include "./headers/commands.h"
#include <stdlib.h>
#include <sys/stat.h>

int commands ( int argc, char* argv[], int bg_task_id ) {

	if ( argv[0] == NULL )
		return CONTINUE_NORMAL;

	if ( !strcmp( argv[0], "exit" ) ) {
		return EXIT_NORMAL;
	}

	else if ( !strcmp( argv[0], "cd" ) ) {
		pestatus = cd(argc, argv);
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "ls" ) ) {
		pestatus = ls(argc, argv);
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "history" ) ) {
		pestatus = printHistory(argc, argv);
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "pinfo" ) ) {
		pestatus = pinfo(argc, argv);
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "discover" ) ) {
		pestatus = discover(argc, argv);
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "pwd" ) ) {
		printf("%s\n", cwd);
		pestatus = 0;
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "echo" ) ) {
		for ( int i = 1; argv[i] != NULL; i++ )
			printf("%s ", argv[i]);
		printf("\n");
		pestatus = 0;
		return CONTINUE_NORMAL;
	}

	else {
		if ( !bg_task_id ) {
			pid_t child_pid = fork();
			if ( child_pid > 0 ) {
				int status;
				cpid = child_pid;
				waitpid( child_pid, &status, 0 );
				cpid = 0;
				pestatus = WIFEXITED(status) ? WEXITSTATUS(status) : -100;
				return CONTINUE_NORMAL;
			} else if ( child_pid == 0 ) {
				int err = execvp(argv[0], argv);
				perror(argv[0]);
				exit(err);
			} else {
				perror("execvp");
				pestatus = -1;
				return CONTINUE_AFTER_SHELL_ERROR;
			}
		} else {
			execvp(argv[0], argv);
			perror(argv[0]);
			pestatus = -1;
			return CONTINUE_AFTER_SHELL_ERROR;
		}
	}
}

int ls ( int argc, char* argv[] ) {
	void (*printfn) ( char*, struct stat* ) = printlsn;

	int FLAG_all = 0;

	{
		opterr = 0;
		optind = 0;
		loop:;
			 switch ( getopt(argc, argv, "al") ) {
				 case 'a' :
					 FLAG_all = 1;
					 break;
				 case 'l' :
					 printfn = printlsl;
					 break;
				 case -1 :
					 goto processed;
				 default :
					 fprintf(stderr, "%s: invalid option - '%c' (%#x)\n", argv[0], optopt, optopt);
					 return -1;
			 }
			 goto loop;
		processed:;
	}

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
			qsort(entries, num_entries, sizeof(struct dirent*), filecmp);

			if ( query_count > 1 )
				printf("%s:\n%d entries>\n", query_path, num_entries);

			for ( int i = 0; i < num_entries; i++ ) {
				char* entry_path = malloc( (strlen(query_path) + 1 + strlen(entries[i]->d_name) + 1) * sizeof(char) );
				sprintf(entry_path, "%s/%s", query_path, entries[i]->d_name);
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

int printHistory ( int argc, char* argv[] ) {

	int depth = 10;

	if ( argc > 2 ) {
		fprintf(stderr, "history: usage: history [<max depth>]\n");
		return -1;
	} else if ( argc == 2 ) {
		depth = atoi( argv[1] );
		if ( depth < 0 ) {
			fprintf(stderr, "history: invalid depth %d\n", depth);
			return -1;
		}
	}

	if ( depth > MAX_HISTORY )
		depth = MAX_HISTORY;

	int start = history_count - depth;
	if ( start < 0 )
		start = 0;

	for ( int i = start; i < history_count; i++ )
		printf("%s\n", history[ i % MAX_HISTORY ]);

	return 0;
}

int pinfo ( int argc, char* argv[] ) {

	pid_t pid = getpid();

	if ( argc > 2 ) {
		fprintf(stderr, "pinfo: usage: pinfo [<pid>]\n");
		return -1;
	} else if ( argc == 2 ) {
		pid = atoi( argv[1] );
	}

	char* ppath = malloc( (strlen("/proc") + MAX_PID_LEN + 1 + 1) * sizeof(char) );
	if ( !ppath ) {
		perror("malloc");
		return -1;
	}
	sprintf(ppath, "/proc/%d", pid);

	char* exe_path = malloc( (strlen(ppath) + strlen("/exe") + 1) * sizeof(char) );
	if ( !exe_path ) {
		perror("malloc");
		free(ppath);
		return -1;
	}
	sprintf( exe_path, "%s/exe", ppath );
	char* path_to_exe = malloc( (PATH_MAX+1) * sizeof(char) );
	if ( !path_to_exe ) {
		perror("malloc");
		free(ppath);
		free(exe_path);
		return -1;
	}
	size_t len_path_to_exe = readlink(exe_path, path_to_exe, PATH_MAX);
	if ( len_path_to_exe != -1 ) {
		path_to_exe[len_path_to_exe] = '\0';
	} else {
		// if process is zombie
		strcpy(path_to_exe, "-");
	}

	char* stat_path = malloc( (strlen(ppath) + strlen("/stat") + 1) * sizeof(char) );
	if ( !stat_path ) {
		perror("malloc");
		free(ppath);
		free(exe_path);
		free(path_to_exe);
		return -1;
	}
	sprintf( stat_path, "%s/stat", ppath );
	int stat_fd = open(stat_path, O_RDONLY);
	if ( stat_fd < 0 ) {
		fprintf(stderr, "pinfo: process %d does not exist\n", pid);
		free(ppath);
		free(exe_path);
		free(path_to_exe);
		free(stat_path);
		return -1;
	}

	char status = ' ';
	int isFG = 0;
	long unsigned vmemsize = -1;

	char line[1024];
	char* line_ite = line;
	size_t len_read = read(stat_fd, line, 1024);
	if ( len_read < 0 ) {
		perror("reading stat file");
		free(ppath);
		free(exe_path);
		free(path_to_exe);
		free(stat_path);
		close(stat_fd);
		return -1;
	}

	int i = 0;
	char* val;
	while ( (val = strtok_r(NULL, " ", &line_ite)) != NULL ) {
		switch ( ++i ) {
			case 3 :
				status = val[0];
				break;
			case 8 :
				isFG = atoi(val) == pid;
				break;
			case 23 :
				vmemsize = strtoul(val,NULL,10);
				break;
		}
	}

	printf("pid:\t%d\n", pid);
	printf("status:\t%c%c\n", status, isFG ? '+' : ' ');
	printf("memory:\t%lu\n", vmemsize);
	printf("exec:\t%s\n", path_to_exe);

	close(stat_fd);
	free(ppath);
	free(exe_path);
	free(path_to_exe);
	free(stat_path);

	return 0;
}

int discover ( int argc, char* argv[] ) {
	int FLAG_dir = 0;
	int FLAG_fil = 0;

	{
		opterr = 0;
		optind = 0;
		loop:;
			 switch ( getopt(argc, argv, "df") ) {
				 case 'd' :
					 FLAG_dir = 1;
					 break;
				 case 'f' :
					 FLAG_fil = 1;
					 break;
				 case -1 :
					 goto processed;
				 default :
					 fprintf(stderr, "%s: invalid option - '%c' (%#x)\n", argv[0], optopt, optopt);
					 return -1;
			 }
			 goto loop;
		processed:;
	}

	char* arguments[2] = {NULL, NULL};

	for ( int i = optind; i < argc; i++ ) {
		char* arg = parsePath( argv[i] );
		struct stat statbuf;
		if ( ( stat(arg, &statbuf) == 0 ) && ( S_ISDIR(statbuf.st_mode) ) ) {
			if ( arguments[0] ) {
				fprintf(stderr, "%s: %s: Multiple directories specified\n", argv[0], arg);
				for ( int i = 0; i < 2; i++ )
					if ( arguments[i] )
						free(arguments[i]);
				return -1;
			}
			arguments[0] = arg;
		} else {
			if ( arguments[1] ) {
				fprintf(stderr, "%s: %s: Multiple target files specified\n", argv[0], arg);
				for ( int i = 0; i < 2; i++ )
					if ( arguments[i] )
						free(arguments[i]);
				return -1;
			}
			arguments[1] = arg;
		}
	}

	if ( !arguments[0] ) {
		arguments[0] = strdup(".");
	}
	if ( !arguments[1] && !FLAG_dir && !FLAG_fil ) {
		FLAG_dir = FLAG_fil = 1;
	}

	int err = recursivelyDiscover(arguments[0], arguments[1], FLAG_dir, FLAG_fil);

	for ( int i = 0; i < 2; i++ )
		if ( arguments[i] )
			free(arguments[i]);

	return err;
}

int recursivelyDiscover ( char* dir, char* fil, int FLAG_dir, int FLAG_fil ) {

	DIR* dirp = opendir(dir);
	if ( !dirp ) {
		fprintf(stderr, "discover: %s: directory not found\n", dir);
		return -1;
	}

	struct dirent* entry;
	while ( (entry = readdir(dirp)) != NULL ) {

		char* entry_path = malloc( (strlen(dir) + 1 + strlen(entry->d_name) + 1) * sizeof(char) );
		sprintf( entry_path, "%s%s%s", dir, dir[strlen(dir)-1] == '/' ? "" : "/", entry->d_name);
		struct stat st;
		if ( stat(entry_path, &st) == -1 ) {
			perror(entry_path);
			free(entry_path);
			return -1;
		}

		if ( entry->d_type == DT_DIR ) {
			if ( !!strcmp(entry->d_name, ".") && !!strcmp(entry->d_name, "..") ) {
				if ( FLAG_dir ) {
					printlsn(entry_path, &st);
					printf("\n");
				}
				recursivelyDiscover(entry_path, fil, FLAG_dir, FLAG_fil);
			}
		}
		else {
			if ( FLAG_fil || ( !!fil && !strcmp( entry->d_name, fil ) ) ) {
				printlsn(entry_path, &st);
				printf("\n");
			}
		}

		free(entry_path);
	}

	return 0;
}
