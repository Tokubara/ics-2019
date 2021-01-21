#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
  FILE* fp =  fopen("/Users/quebec/tmp.txt","a");
  printf("%lld\n", fp->_offset);
}