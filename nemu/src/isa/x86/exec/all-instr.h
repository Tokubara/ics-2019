#include "cpu/exec.h"

make_EHelper(mov);
make_EHelper(movzx);
make_EHelper(movsx);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(leave);
make_EHelper(cltd);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
make_EHelper(nop);
make_EHelper(lea);

make_EHelper(call);
make_EHelper(ret);
make_EHelper(jcc);
make_EHelper(jmp);

make_EHelper(add);
make_EHelper(sbb);
make_EHelper(adc);
make_EHelper(sub);
make_EHelper(inc);
make_EHelper(dec);
make_EHelper(cmp);

make_EHelper(imul2);
make_EHelper(div);
make_EHelper(idiv);


make_EHelper(and);
make_EHelper(or);
make_EHelper(xor);
make_EHelper(test);

make_EHelper(sar);
make_EHelper(shl);

make_EHelper(setcc);

