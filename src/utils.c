#include "./headers/utils.h"

char* tokenizeInput ( char* *inpIte, int *isBG ) {
	char* str = *inpIte;
	if ( str == NULL ) {
		return NULL;
	}

	for ( int i = 0; ; i++ ) {
		switch ( str[i] ) {
			case '\0':
				str[i] = '\0';
				*inpIte = NULL;
				*isBG = 0;
				return str;
			case ';':
				str[i] = '\0';
				*inpIte = str + i + 1;
				*isBG = 0;
				return str;
			case '&':
				str[i] = '\0';
				*inpIte = str + i + 1;
				*isBG = 1;
				return str;
		}
	}
}
