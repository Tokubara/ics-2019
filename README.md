x86版本. 实现了PA1到PA4所有必做+选做功能(比如am实现在navy之上). 用户栈实现在虚拟地址空间, 栈顶为0x48000000, 大小固定为8个PG_SIZE, 机制是简化版的TSS.

可以设置nanos-lite下的common.h中的HAS_VME, 决定是否打开虚拟内存, 这样的话就不能同时运行多个进程(因为都会被加载在同一个位置0x3000000), 可以运行开机程序.

要运行什么app可以在`init_proc`中修改. 当前是3个开机程序, 用F1, F2, F3切换, 实现中每3s才会检测是不是按了功能键(检测放在时钟中断中).

在nanos-lite, nexus-am, nemu中的debug.h有彩色打印宏, `Log_trace`可以关闭, 当前是nanos-lite和nexus-am是打开状态, 如要关闭, 注释debug.h中的`#define LOG_TRACE`.

时钟中断可以关闭, 在nemu的common.h中的`TIMER_ITERRUPT_ENABLE`. 但是这样就不能切换进程了, 原因是nanos-lite的`events_read`等device函数注释了`_yield`(原因是`hello_fun`输出得太多), 时钟中断是切换进程的唯一途径. 可以修改这个逻辑, 不注释nanos-lite/src/device.c中的`_yield`调用, 也可以正常运行.

 为了不重复链接加快build, 魔改了部分Makefile(比如设置了navy-apps/Makefile中的APPS和TESTS), 会导致只有某些app会被build, 因此如果需要build, Makefile还是得用原来的. 还修改了某些测例, 比如dummy.c.
