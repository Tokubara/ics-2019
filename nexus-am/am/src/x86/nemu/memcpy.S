#define PARMS	12 // 因为有2个callee-saved寄存器
#define RTN	PARMS
#define DEST	RTN
#define SRC	DEST+4
#define LEN	SRC+4

.section        .text
.global memcpy
memcpy:
  pushl %edi
  pushl %esi
	movl	DEST(%esp), %edi
	movl	SRC(%esp), %esi
	movl	LEN(%esp), %ecx

	/* We need this in any case.  */
	cld
  rep; movsb

  popl %esi
  popl %edi

	ret
