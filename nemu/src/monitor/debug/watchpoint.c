#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

/**
 * 这个函数已经把wp_pool链接在一起了, 并且也初始化了free_和head, free_赋值为了wp_pool, head为NULL
 */
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/**
 * wp不是要释放的节点, 而是要释放的节点的父节点
 * wp_free会作为free_的头部, 调用这个函数的时候, 必须保证不为空, wp不为空, wp->child也不为空
 * wp会作为free_, head链表会去掉这个节点
 * 期望的调用是遍历head后调用, 保证节点的确在head中
 */
void free_wp(WP *wp_prev) {
  Assert(wp_prev!=NULL, "wp equals NULL"); // 显然这不会是用户的问题, 这是自己的问题
  WP* wp_free = wp_prev->next;
  Assert(wp_free!=NULL, "wp's next equals NULL"); // 显然这不会是用户的问题, 这是自己的问题
  // 维护head
  wp_prev->next = wp_free->next;
  // wp为新的free_头部, 维护wp
  wp_free->val = 0;
  wp_free->expr = NULL; // 不是空串, 而是空指针, 这一点对expr调用可能有隐患
  wp_free->next = free_;
  // 维护free_
  free_=wp_free;
}

/**
 * 如果没有可用空间, 返回NULL
 * 如果expr发现表达式不合法(错误信息是eval给出的), 也返回NULL
 * 根据expr_str, 计算好, 存入新的节点, 返回新节点
 * 改变了head与free_, free_指向它的子节点, head是新创建的节点
 */
WP* new_wp(char* expr_str) {
  // 如果没有可用空间, 返回NULL
  if(free_==NULL) {
    printf("no more available watchpoints\n");
    return NULL;
  }
  // 如果expr发现表达式不合法(错误信息是eval给出的), 也返回NULL
  bool success;
  uint32_t expr_val = expr(expr_str, &success); // 也会算好值
  if (!success) {
    printf("since the expression is not legal, no watchpoint is created\n");
    return NULL;
  }
  // 没有两种错误情况
  // 维护free_
  WP* old_free_ = free_;
  free_=free_->next; // 如果本来就为空那就为空了
  // 维护这个新节点
  old_free_->expr = expr_str;
  old_free_->val = expr_val;
  old_free_->next = head;
  // 维护head
  head = old_free_;
  // 返回新节点
  return old_free_;
}
