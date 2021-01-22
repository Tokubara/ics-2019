#define macro(x,y) x##y

int main() {
	int x = 2;
	int y = 3;
	printf("%d\n", macro(x,y));
}