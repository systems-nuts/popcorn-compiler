
/* Original version by the musl authors */
/* Current version by Antonio Barbalace, Stevens 2019 */

__asm__(
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
"	mov x0, sp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp x1, _DYNAMIC\n"
"	add x1, x1, #:lo12:_DYNAMIC\n"
"	and sp, x0, #-16\n"
"	b " START "_c\n"
);

/* TODO copy more than a byte at the time */
#define __memcpy_nostack(dest, src, n) \
	({ unsigned long retval =-1; \
	__asm__ volatile(".weak __memcpy_nostack \n" \
		".weak __memcpy_nostack_exit \n" \
		".weak __memcpy_nostack_copy \n" \
		"__memcpy_nostack:" \
		"mov x4, %1 \n\t" \
		"cmp %2, x4 \n\t" \
		"b.le __memcpy_nostack_exit \n" \
		"__memcpy_nostack_copy:" \
		"ldrb w5, [%4, x4] \n\t" \
		"strb w5, [%3, x4] \n\t" \
		"add x4, x4, #0x1 \n\t" \
		"cmp %2, x4 \n\t" \
		"b.gt __memcpy_nostack_copy \n" \
		"__memcpy_nostack_exit:" \
		"mov %0, x4 \n\t" \
		: "=r" (retval) \
		: "I" (0), "r" (n), \
		  "r" (dest), "r" (src) \
		: "x5", "x4", "memory"); \
	retval; })

/* comment the following to disable relocation before libc start */
#define STACK_RELOC




