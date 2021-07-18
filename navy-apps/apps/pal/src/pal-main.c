#include <common.h>

void main_loop(bool skip);
void hal_init();

int
// main(int argc, char *argv[], char *envp[]) {
main() {

  // 解析命令行参数
  bool skip = 1;
  // for (int i = 1; i < argc; ++i) {
  //   if (strcmp(argv[i], "--skip")==0) {
  //      skip = 1;
  //   }
  // }
  hal_init();
	main_loop(skip);

	return 0;
}
