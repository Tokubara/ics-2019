#ifndef __X86_RTL_H__
#define __X86_RTL_H__

#include "rtl/rtl.h"
#include "reg.h"

/** RTL pseudo instructions
 * *rtlreg_t* dest, int r, int width
 * 接受寄存器索引, 宽度, 把相应的寄存器的内容存到某个地址中(虚拟上可能是改寄存器, 也可能是改内存).load register.
 */

static inline void rtl_lr(rtlreg_t* dest, int r, int width) {
  switch (width) {
    case 4: rtl_mv(dest, &reg_l(r)); return;
    case 1: rtl_host_lm(dest, &reg_b(r), 1); return;
    case 2: rtl_host_lm(dest, &reg_w(r), 2); return;
    default: assert(0);
  }
}
/**
 * 根据索引r, 写寄存器, 写的内容是*src1, 宽度是width
 */
static inline void rtl_sr(int r, const rtlreg_t* src1, int width) {
  switch (width) {
    case 4: rtl_mv(&reg_l(r), src1); return;
    case 1: rtl_host_sm(&reg_b(r), src1, 1); return;
    case 2: rtl_host_sm(&reg_w(r), src1, 2); return;
    default: assert(0);
  }
}

//TODO push esp这种特殊情况
static inline void rtl_push(const rtlreg_t* src1) {
  // esp <- esp - 4
  // M[esp] <- src1
	reg_l(4)-=4;
	rtl_sm(&reg_l(4), src1, 4);
}

static inline void rtl_pop(rtlreg_t* dest) {
  // dest <- M[esp]
  // esp <- esp + 4
	rtl_lm(dest, &reg_l(4), 4);
	reg_l(4)+=4;
}

#define switch_case_width(func) switch(width) {\
			case 1:{func(int8_t);break;}\
			case 2:{func(int16_t);break;}\
			case 4:{func(int32_t);break;}\
			default:assert(0);\
		}

#define rtl_is_sub_overflow_macro(type) type s1=(type)*src1 ; type s2=(type)*src2 ; type r=(type)*res ; int positive_of_flag=0;if(s1>0 && s2<0&& r<=0) {positive_of_flag=1;} int negative_of_flag=0;if(s1<0 && s2>0&& r>=0) {negative_of_flag=1;} *dest=!positive_of_flag&&!negative_of_flag
static inline void rtl_is_sub_overflow(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
		switch_case_width(rtl_is_sub_overflow_macro)
}

/**
 * 判断减法是否有借位
*/
static inline void rtl_is_sub_carry(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1) {
		*dest = res>src1; // 如果结果比被减数还大, 就是借位了
}

#define rtl_is_add_overflow_macro(type) rtl_add(&t0,src1,src2);*dest=(((type)*src1<0)==((type)*src2<0)) && (((type)*src1<0)!=((type)t0<0))
static inline void rtl_is_add_overflow(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) { //? res参数难道不是多余的? 要它有何用?
		switch_case_width(rtl_is_add_overflow_macro)
}

static inline void rtl_is_add_carry(rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1) { // 已是无符号数
		*dest = (res<src1);
}

#define getter(f) cpu.eflags.f

#define make_rtl_setget_eflags(f) \
  static inline void concat(rtl_set_, f) (const rtlreg_t* src) { \
    getter(f)=*src; \
  } \
  static inline void concat(rtl_get_, f) (rtlreg_t* dest) { \
    *dest =  getter(f); \
  }

make_rtl_setget_eflags(CF)
make_rtl_setget_eflags(OF)
make_rtl_setget_eflags(ZF)
make_rtl_setget_eflags(SF)
make_rtl_setget_eflags(PF)

#define rtl_update_ZF_macro(type) t0=((type)*result==0); rtl_set_ZF(&t0);
static inline void rtl_update_ZF(const rtlreg_t* result, int width) {
  switch_case_width(rtl_update_ZF_macro)
}

#define rtl_update_SF_macro(type) t0=((type)*result<0); rtl_set_ZF(&t0);
static inline void rtl_update_SF(const rtlreg_t* result, int width) {
	switch_case_width(rtl_update_SF_macro)
}

static inline void rtl_update_PF(const rtlreg_t* result) {
	rtlreg_t res=1;
	rtlreg_t tmp = *result;
	for(int i = 0; i < 8; i++) {
		res^=(tmp&1);
		tmp>>=1;
	}
	rtl_set_PF(&res);
}

static inline void rtl_update_ZFSF(const rtlreg_t* result, int width) {
  rtl_update_ZF(result, width);
  rtl_update_SF(result, width);
}

#endif
