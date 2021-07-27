#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#include <stdlib.h>

#define NR_WP 32

// static WP wp_pool[NR_WP] = {};
// static WP *head = NULL, *free_ = NULL;
WP wp_pool[NR_WP] = {};
WP *head = NULL, *free_ = NULL;

extern void *malloc(size_t size);

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
  head = (WP*)malloc(sizeof(WP)); // 对这里进行修改, head需要是个虚拟的头部节点, 使得每一个真实的节点都有父节点, 便于删除
  head->next=NULL;
  head->expr=NULL;
  head->NO=-1;
  free_ = wp_pool; // 其实想表达的意思是, free_=&wp_pool[0]
}

/**
 * 打印所有wp的信息, 注意保证指针未被修改
*/
void print_wps() {
  Assert(head!=NULL, "head is null");
  WP* start = head->next;
  if(start == NULL) {
    Log("No watchpoints");
    return;
  }
  Log("NO\tWhat\tValue");
  while(start) {
    Log("%d\t%s\t%u", start->NO, start->expr, start->val);
    start=start->next;
  }
}

/**
 * 搜索head中是否有相同的no, 返回的是父节点(因为父节点可以方便找到子节点), 没有返回NULL
*/
WP* find_NO_prev(int tar_NO) {
  Assert(head != NULL, "head is null");
  WP *start = head;
  while(start->next) {
    if(start->next->NO==tar_NO) {
      return start;
    }
    start=start->next;
  }
  return NULL;
}

/**
 * 遍历计算监视点, 会打印改变的监视点信息, 返回是否有变化
 */
void check_wps() {
  Assert(head != NULL, "head is null");
  WP *start = head->next;
  uint32_t new_val;
  i32 ret;
  while(start) {
    ret = expr(start->expr, &new_val);
    Assert(ret==0, "expr fails"); // 能存进去的不可能是错误的表达式
    if(new_val!=start->val) {
      printf("watchpoint %d: %s\nOld value = %u\nNew value = %u\n", start->NO, start->expr, start->val, new_val);
      start->val = new_val;
    }
    start = start->next;
  }
}

/**
 * 删除编号为NO的监视点
 * 如果不存在, 会打印错误信息
 */
void del_wp_NO(int NO) {
  WP* wp = find_NO_prev(NO);
  if(wp==NULL) {
    Log("%d is not in use", NO);
    return;
  }
  free_wp(wp);
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
  assert(wp_free->expr!=NULL);
  free(wp_free->expr);
  wp_free->expr = NULL; // 不是空串, 而是空指针, 这一点对expr调用可能有隐患
  wp_free->next = free_;
  // 维护free_
  free_=wp_free;
}

/**
 * 如果没有可用空间, 返回NULL
 * 如果expr发现表达式不合法(错误信息是eval给出的), 也返回NULL
 * 根据expr_str, 计算好, 存入新的节点, 返回新节点
 * 对于不能正常的情况, 都会打印错误信息
 * 改变了head与free_, free_指向它的子节点, head是新创建的节点
 */
WP* new_wp(char* expr_str) {
  // 如果没有可用空间, 返回NULL
  if(free_==NULL) {
    Log("no more available watchpoints");
    return NULL;
  }
  // 如果expr发现表达式不合法(错误信息是eval给出的), 也返回NULL
  i32 ret;
  uint32_t expr_val;
  ret = expr(expr_str, &expr_val); // 也会算好值
  if (ret<0) {
    Log("since the expression is not legal, no watchpoint is created");
    return NULL;
  }
  // 维护free_
  WP* old_free_ = free_;
  free_=free_->next; // 如果本来就为空那就为空了
  // 维护这个新节点
  old_free_->expr = (char*)malloc(strlen(expr_str)+1);
  strcpy(old_free_->expr, expr_str); // expr_str应有\0
  old_free_->val = expr_val;
  old_free_->next = head->next;
  // 维护head
  head->next = old_free_;
  // 返回新节点
  return old_free_;
}
