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
		for ( int i = 1; argv[i] != NULL; i++ ) {
			printf("%s", argv[i]);
			if ( argv[i+1] != NULL )
				printf(" ");
		}
		printf("\n");
		pestatus = 0;
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "jobs" ) ) {
		pestatus = jobs(argc, argv);
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "sig" ) ) {
		pestatus = sig(argc, argv);
		return CONTINUE_NORMAL;
	}

	else if ( !strcmp( argv[0], "fg" ) || !strcmp( argv[0], "bg" ) ) {
		pestatus = changeground(argc, argv);
		return CONTINUE_NORMAL;
	}

	else {
		pid_t child_pid = fork();
		if ( child_pid > 0 ) {
			if ( !bg_task_id ) {
				int waiter_exit_status = makefg(child_pid);
				if ( waiter_exit_status )
					return CONTINUE_AFTER_SHELL_ERROR;
			} else {
				int bg_id = getnextbgid();
				if ( bg_id < 0 )
					return -1;
				bg_tasks[bg_id-1] = child_pid;
			}
			return CONTINUE_NORMAL;
		} else if ( child_pid == 0 ) {
			setpgid(0, 0);
			signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);
			int err = execvp(argv[0], argv);
			perror(argv[0]);
			exit(-1);
		} else {
			perror("execvp");
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
			printf("\n");
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

int jobs ( int argc, char* argv[] ) {
	int FLAG_r = 0;
	int FLAG_s = 0;

	{
		opterr = 0;
		optind = 0;
		loop:;
			 switch ( getopt(argc, argv, "rs") ) {
				 case 'r' :
					 FLAG_r = 1;
					 break;
				 case 's' :
					 FLAG_s = 1;
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
	if ( !FLAG_r && !FLAG_s )
		FLAG_s = FLAG_r = 1;

	char* stat_path = malloc( (strlen("/proc/") + MAX_PID_LEN + strlen("/stat") + 1) * sizeof(char) );
	if ( !stat_path ) {
		perror("malloc");
		return -1;
	}
	char* cmd_path = malloc( (strlen("/proc/") + MAX_PID_LEN + strlen("/cmdline") + 1) * sizeof(char) );
	if ( !stat_path ) {
		perror("malloc");
		free(stat_path);
		return -1;
	}

	char line[1024];

	for ( int i = 0; i < MAX_BG_TASKS; i++ ) {
		if ( !!bg_tasks[i] ) {
			sprintf(stat_path, "/proc/%d/stat", bg_tasks[i]);
			int stat_fd = open(stat_path, O_RDONLY);
			if ( stat_fd < 0 ) {
				fprintf(stderr, "%s: process %d does not exist\n", argv[0], bg_tasks[i]);
				continue;
			}

			size_t len_read = read(stat_fd, line, 1024);
			close(stat_fd);

			if ( len_read < 0 ) {
				perror("reading /proc/[pid]/stat");
				free(stat_path);
				free(cmd_path);
				return -1;
			}
			char* line_ite = line;

			char* pid = strtok_r(NULL, " ", &line_ite);
			strtok_r(NULL, " ", &line_ite);
			char* status = strtok_r(NULL, " ", &line_ite);

			sprintf(cmd_path, "/proc/%d/cmdline", bg_tasks[i]);
			int cmd_fd = open(cmd_path, O_RDONLY);
			if ( cmd_fd < 0 ) {
				fprintf(stderr, "%s: process %d does not exist\n", argv[0], bg_tasks[i]);
				continue;
			}
			len_read = read( cmd_fd, line, 1024 );
			close(cmd_fd);
			if ( len_read < 0 ) {
				perror("reading /proc/[pid]/cmdline");
				free(stat_path);
				free(cmd_path);
				return -1;
			}
			// replace null bytes with spaces but dont change the last null byte
			for ( int i = 0; i < len_read-1; i++ )
				if ( line[i] == '\0' )
					line[i] = ' ';
			char* name = line;

			if ( !strncmp(status, "T", 1) )
				FLAG_s && printf("[%d]: stopped `%s` (%d)\n", i+1, name, bg_tasks[i]);
			else
				FLAG_r && printf("[%d]: running `%s` (%d)\n", i+1, name, bg_tasks[i]);
		}
	}

	free(stat_path);
	free(cmd_path);
	return 0;
}

int sig ( int argc, char* argv[] ) {
	if ( argc < 3 || argc > 3 ) {
		fprintf(stderr, "%s: usage: sig [job_id] [signal]\n", argv[0]);
		return -1;
	}

	int job_id = atoi(argv[1]) - 1;
	if ( job_id < 0 || job_id >= MAX_BG_TASKS || !bg_tasks[job_id] ) {
		fprintf(stderr, "%s: invalid job id %d\n", argv[0], job_id+1);
		return -1;
	}

	int sig = atoi(argv[2]);
	if ( sig < 0 || sig > 31 ) {
		fprintf(stderr, "%s: invalid signal %d\n", argv[0], sig);
		return -1;
	}

	if ( kill(bg_tasks[job_id], sig) == -1 ) {
		perror(argv[0]);
		return -1;
	}

	printf("signal %d sent to process %d\n", sig, bg_tasks[job_id]);
	return 0;
}

int makefg ( pid_t pid ) {
	cpid = pid;
	int status;

	pid_t pgrp = tcgetpgrp(STDIN_FILENO);

	tcsetpgrp(STDIN_FILENO, pid);
	waitpid( pid, &status, WUNTRACED );
	signal(SIGTTOU, SIG_IGN);
	tcsetpgrp(STDIN_FILENO, pgrp);
	signal(SIGTTOU, SIG_DFL);

	if ( WIFSTOPPED(status) ) {
		int bg_id = getnextbgid();
		if ( bg_id < 0 )
			return -1;
		bg_tasks[bg_id-1] = pid;
		fprintf(stderr, "\n[%d] %d stopped\n", bg_id, bg_tasks[bg_id-1]);
	}
	cpid = 0;
	pestatus = WIFEXITED(status) ? WEXITSTATUS(status) : -100;
	return 0;
}

int changeground ( int argc, char* argv[] ) {
	if ( argc < 2 || argc > 2 ) {
		fprintf(stderr, "%s: usage: %s [job_id]\n", argv[0], argv[0]);
		return -1;
	}

	int job_id = atoi(argv[1]) - 1;
	if ( job_id < 0 || job_id >= MAX_BG_TASKS || !bg_tasks[job_id] ) {
		fprintf(stderr, "%s: invalid job id %d\n", argv[0], job_id+1);
		return -1;
	}

	if ( kill(bg_tasks[job_id], SIGCONT) == -1 ) {
		perror(argv[0]);
		return -1;
	}

	if ( !strcmp(argv[0], "fg") ) {
		int pid = bg_tasks[job_id];
		bg_tasks[job_id] = 0;
		if ( makefg(pid) == -1 )
			return -1;
	}

	return 0;
}

char* tabcomplete ( char* query ) {

	char* dirpath = cwd;

	DIR* dir = opendir(dirpath);
	if ( !dir ) {
		perror("tabcomplete");
		return NULL;
	}

	int num_entries = 0;
	struct dirent* entries[MAX_NUM_CONTENTS];

	struct dirent* entry;
	while ( (entry = readdir(dir)) != NULL ) {
		if ( !strncmp( entry->d_name, query, strlen(query) ) ) {
			entries[num_entries++] = entry;
			if ( num_entries == MAX_NUM_CONTENTS )
				break;
		}
	}
	qsort(entries, num_entries, sizeof(struct dirent*), filecmp);

	if ( num_entries == 1 ) {
		char* fullpath = malloc( (strlen(dirpath) + strlen("/") + strlen(entries[0]->d_name) + 1) * sizeof(char) );
		sprintf(fullpath, "%s/%s", dirpath, entries[0]->d_name);
		struct stat st;
		if ( stat(fullpath, &st) == -1 ) {
			perror(fullpath);
			free(fullpath);
			return NULL;
		}
		free(fullpath);

		char* completed = malloc( (strlen(entries[0]->d_name) + strlen(" ") + 1) * sizeof(char) );
		if ( S_ISDIR(st.st_mode) )
			sprintf(completed, "%s/", entries[0]->d_name);
		else
			sprintf(completed, "%s ", entries[0]->d_name);

		return completed;
	}

	// else if multiple entries are there
	printf("\n");
	for ( int i = 0; i < num_entries; i++ ) {
		printf("%s\t", entries[i]->d_name);
	}
	printf("\n");
	closedir(dir);

	return NULL;
}
