#include "./headers/main.h"

int main () {
	init();

	for ( int ret = 0; ret != 1; ) {
		ret = prompt();
	}

	deinit();
	return 0;
}
