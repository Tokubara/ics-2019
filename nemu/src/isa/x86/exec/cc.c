#include "rtl/rtl.h"

/* Condition Code */
/** 猜测是set指令
 * 
*/
void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1; // 原因是setcc的规律, 偶数(最后一位是0), 是OF=1为1(以OF举例), 而它对应的奇数, 就是OF=0为1
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  }; // 与setcc文档是对应的

  // dest <- ( cc is satisfied ? 1 : 0)
  switch (subcode & 0xe) { // 不看最后一位, 那是invert
    case CC_O: {rtl_get_OF(dest);break;} // Set byte if overflow (OF=1)
    case CC_B: {rtl_get_CF(dest);break;} // 2 Set byte if below (CF=1)
    case CC_E: {rtl_get_ZF(dest);break;} // 4 Set byte if equal (ZF=1)
    case CC_BE: {rtl_get_CF(dest);rtl_get_ZF(&t0);rtl_or(dest,dest,&t0);break;} // Set byte if below or equal (CF=1 or ZF=1)
    case CC_S: {rtl_get_SF(dest);break;} // 8 Set byte if sign (SF=1)
    case CC_L: {rtl_get_SF(dest);rtl_get_OF(&t0);rtl_xor(dest,dest,&t0);break;} // c Set if not greater or equal (SF<>OF)
    case CC_LE: {rtl_get_SF(dest);rtl_get_OF(&t0);rtl_xor(dest,dest,&t0);rtl_get_ZF(&t0);rtl_xor(dest,dest,&t0);break;} // Set byte if not greater (ZF=1 or SF<>OF)
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF"); // 对应的是A, 是PA
  }

  if (invert) {
    rtl_xori(dest, dest, 0x1);
  }
  assert(*dest == 0 || *dest == 1);
}
