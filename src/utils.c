#include "./headers/utils.h"

int filecmp ( const void* a, const void* b ) {
	char* a_name = (*(struct dirent**)a)->d_name;
	char* b_name = (*(struct dirent**)b)->d_name;
	while(a_name[0] == '.') a_name++;
	while(b_name[0] == '.') b_name++;

	return strcmp ( a_name, b_name );
}

char* parsePath ( char* path ) {
	char* dir = NULL;

	// path relative to cwd, `.` and `..` are handled by the resp functions
	if ( path[0] == '~' ) {
		dir = malloc( ( strlen(homedir) + strlen(path)-1 + 1 ) * sizeof(char) );
		if ( !dir ) {
			perror("malloc");
			return NULL;
		}
		sprintf(dir, "%s%s", homedir, &path[1]);
	}
	else {
		dir = strdup(path);
		if ( !dir ) {
			perror("Copying path");
			return NULL;
		}
	}

	return dir;
}

void printlsl ( char* name, struct stat* st ) {
	char permissions[10] = {0};
	permissions[0] = (S_ISDIR(st->st_mode))  ? 'd' : '-';
	permissions[1] = (st->st_mode & S_IRUSR) ? 'r' : '-';
	permissions[2] = (st->st_mode & S_IWUSR) ? 'w' : '-';
	permissions[3] = (st->st_mode & S_IXUSR) ? 'x' : '-';
	permissions[4] = (st->st_mode & S_IRGRP) ? 'r' : '-';
	permissions[5] = (st->st_mode & S_IWGRP) ? 'w' : '-';
	permissions[6] = (st->st_mode & S_IXGRP) ? 'x' : '-';
	permissions[7] = (st->st_mode & S_IROTH) ? 'r' : '-';
	permissions[8] = (st->st_mode & S_IWOTH) ? 'w' : '-';
	permissions[9] = (st->st_mode & S_IXOTH) ? 'x' : '-';

	char* time_mod = strdup(ctime(&st->st_mtime));
	time_mod = strtok(time_mod, "\n");
	// TODO: correct the time zone

	printf("%s %lu\t%s\t%s\t%ld\t%s\t", permissions, st->st_nlink, getpwuid(st->st_uid)->pw_name, getgrgid(st->st_gid)->gr_name, st->st_size, time_mod);
	printlsn(name, st);
	printf("\n");
}

void printlsn ( char* name, struct stat* st ) {
	printf(
			"%s%s%s  ",
			(
				S_ISDIR(st->st_mode)
					? BLU
					: S_IEXEC & st->st_mode
						? GRN
						: WHT
			),
			name,
			RESET
		  );
}
