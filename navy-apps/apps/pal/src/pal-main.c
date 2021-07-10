#include <common.h>

void main_loop();
void hal_init();

int
main(int argc, char *argv[], char *envp[]) {
	Log("game start! argv[0]:%s, argv[1]:%s", argv[0], argv[1]);

  hal_init();
	main_loop();

	return 0;
}
