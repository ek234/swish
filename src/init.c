#include "./headers/init.h"

struct passwd* user_details;
char* username;
char* homedir;
char* hostname;

char* cwd;
char* owd;

int pestatus;
int ptime;

int history_count;
char* history[MAX_HISTORY];

int BASE_STDIN_FD;
int BASE_STDOUT_FD;

pid_t cpid;

pid_t bg_tasks[MAX_BG_TASKS] = {0};

int init () {

	signal( SIGINT, handle_signal );
	signal( SIGTSTP, handle_signal );

	user_details = getpwuid(getuid());
	if ( !user_details ) {
		perror("getpwent");
		return -1;
	}

	username = user_details->pw_name;

	homedir = getcwd(NULL, 0);
	if ( !homedir ) {
		perror("getcwd");
		return -1;
	}

	cwd = getcwd(NULL, 0);
	if ( !cwd ) {
		perror("getcwd");
		return -1;
	}

	owd = getcwd(NULL, 0);
	if ( !owd ) {
		perror("getcwd");
		return -1;
	}

	hostname = malloc(MAX_HOSTNAME_LEN * sizeof(char));
	{
		int hostname_status = gethostname(hostname, MAX_HOSTNAME_LEN);
		if ( !!hostname_status ) {
			perror("hostname");
			return -1;
		}
	}

	pestatus = 0;
	ptime = 0;

	history_count = 0;
	{
		char* hfilepath = parsePath(HISTORYFILE);
		if ( hfilepath < 0 )
			return -1;

		FILE* historyfile = fopen(hfilepath, "r");
		if ( !!historyfile ) {
			// if history file exists
			size_t n = 0;
			char* line = NULL;
			int read;
			while ( (read = getline(&line, &n, historyfile)) != -1 ) {
				// remove trailing newline
				for ( int i = read - 1; i >= 0 && line[i] == '\n'; i-- )
					line[i] = '\0';

				history[history_count%MAX_HISTORY] = line;
				if ( history_count < MAX_HISTORY )
					history_count++;
				else
					// increment history_count such that modulus is the correct index while keeping history > MAX_HISTORY
					history_count = MAX_HISTORY + ( history_count+1 ) % MAX_HISTORY;
				// getline malloc's the string. history[] just stores the address of this string.
				// So, we don't need to free line. We just reset it to NULL for the next lines.
				line = NULL;
			}
			fclose(historyfile);
		}

		for ( int i = history_count; i < MAX_HISTORY; i++ )
			history[i] = NULL;

		free(hfilepath);
	}

	BASE_STDIN_FD  = dup(  STDIN_FILENO );
	BASE_STDOUT_FD = dup( STDOUT_FILENO );

	cpid = 0;

	printf("Welcome to the shell, %s!\n", username);

	return 0;
}

int deinit () {

	{
		char* hfilepath = parsePath(HISTORYFILE);
		if ( hfilepath < 0 )
			return -1;

		int start = history_count - MAX_HISTORY;
		if ( start < 0 )
			start = 0;

		FILE* historyfile = fopen(hfilepath, "w");
		if ( !historyfile ) {
			perror(hfilepath);
			return -1;
		}

		for ( int i = start; i < history_count; i++ ) {
			fprintf(historyfile, "%s\n", history[ i % MAX_HISTORY ]);
			free(history[ i % MAX_HISTORY ]);
		}

		fclose(historyfile);
		free(hfilepath);
	}

	free(homedir);
	free(hostname);
	free(cwd);
	free(owd);

	close(BASE_STDIN_FD);
	close(BASE_STDOUT_FD);

	return 0;
}

void handle_signal ( int sig ) {
	switch ( sig ) {
		case SIGINT :
		case SIGTSTP :
			if ( cpid != 0 )
				kill(cpid, sig);
			break;
	}
}
