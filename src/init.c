#include "./headers/init.h"
#include <unistd.h>

struct passwd* user_details;
char* username;
char* homedir;
char* hostname;

int init () {

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

	hostname = malloc(MAX_HOSTNAME_LEN * sizeof(char));
	{
		int hostname_status = gethostname(hostname, MAX_HOSTNAME_LEN);
		if ( !!hostname_status ) {
			perror("hostname");
			return -1;
		}
	}

	return 0;
}

int deinit () {

	free(homedir);
	free(hostname);

	return 0;
}
