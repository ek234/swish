#include "./headers/main.h"

int main () {
	init();

	while (1) {
		switch ( prompt() ) {
			case CONTINUE_AFTER_SHELL_ERROR :
			case CONTINUE_NORMAL:
				break;

			case EXIT_AFTER_SHELL_ERROR :
			case EXIT_NORMAL :
				return 0;

			default :
				return 1;
		}
	}
}
