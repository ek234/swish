#include "./headers/main.h"

int main () {
	init();

	int ret = 0;
	while (1) {
		ret = prompt();
	}

	deinit();
	return 0;
}
