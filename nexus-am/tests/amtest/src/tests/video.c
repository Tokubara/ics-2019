#include <amtest.h>

#define FPS 30
#define N   32

static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}
static inline uint8_t R(uint32_t p) { return p >> 16; }
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }

static uint32_t canvas[N][N];
static int used[N][N];

static uint32_t color_buf[32 * 32];

void redraw() {
  int w = screen_width() / N; // 猜测N是块单位, 也就是说, 我们不是以像素为单位控制颜色的, 而是32*32这样一个小方块, 一个小方块内的颜色相同
  int h = screen_height() / N;
  int block_size = w * h;
  assert((uint32_t)block_size <= LENGTH(color_buf)); //? color_buf为什么不是二维数组?

  int x, y, k;
  for (y = 0; y < N; y ++) {
    for (x = 0; x < N; x ++) {
      for (k = 0; k < block_size; k ++) {
        color_buf[k] = canvas[y][x];
      }
      draw_rect(color_buf, x * w, y * h, w, h); // 看到这里的感觉和前面正好相反, 感觉N*N是不同的颜色, 而w,h才是颜色相同的单位. 貌似x是宽那个方向, y是高那个方向. 与cuda是一致的
    }
  }

  draw_sync();
}

//? 
static uint32_t p(int tsc) {
  int b = tsc & 0xff;
  return pixel(b * 6, b * 7, b);
}

//? 
void update() {
  static int tsc = 0;
  static int dx[4] = {0, 1, 0, -1};
  static int dy[4] = {1, 0, -1, 0};

  tsc ++;

  for (int i = 0; i < N; i ++) // N 是32
    for (int j = 0; j < N; j ++) {
      used[i][j] = 0;
    }

  int init = tsc * 1;
  canvas[0][0] = p(init); used[0][0] = 1;
  int x = 0, y = 0, d = 0;
  for (int step = 1; step < N * N; step ++) {
    for (int t = 0; t < 4; t ++) {
      int x1 = x + dx[d], y1 = y + dy[d];
      if (x1 >= 0 && x1 < N && y1 >= 0 && y1 < N && !used[x1][y1]) {
        x = x1; y = y1;
        used[x][y] = 1;
        canvas[x][y] = p(init + step / 2);
        break;
      }
      d = (d + 1) % 4;
    }
  }
}

void video_test() {
  unsigned long last = 0;
  unsigned long fps_last = 0;
  int fps = 0;

  while (1) {
    unsigned long upt = uptime();
    if (upt - last > 1000 / FPS) { // FPS=30, 也就是>1/30s, 因此FPS的意思是帧数
      update(); // 虽然我看不懂它的逻辑, 但是我怀疑它只是更新canvas的
      redraw();
      last = upt;
      fps ++;
    }
    if (upt - fps_last > 1000) {
      // display fps every 1s
      printf("%d: FPS = %d\n", upt, fps);
      fps_last = upt;
      fps = 0;
    }
  }
}
