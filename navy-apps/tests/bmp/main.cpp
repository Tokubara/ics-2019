#include <assert.h>
#include <stdlib.h>
#include <ndl.h>
#include <stdio.h>

int main() {
  NDL_Bitmap *bmp = (NDL_Bitmap*)malloc(sizeof(NDL_Bitmap));
  NDL_LoadBitmap(bmp, "/share/pictures/projectn.bmp");
  printf("load finish\n");
  assert(bmp->pixels);
  NDL_OpenDisplay(bmp->w, bmp->h);
  printf("opendisplay finish\n");
  NDL_DrawRect(bmp->pixels, 0, 0, bmp->w, bmp->h);
  printf("drawrect finish\n");
  NDL_Render();
  printf("render finish\n");
  NDL_CloseDisplay();
  while (1);
  return 0;
}
