#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <readline/readline.h>

int main() {
	while(1) {
		char* text = readline(">");
		int a = strtol(text, NULL, 16);
		printf("%d\n", a);
		free(text);
	}
}

