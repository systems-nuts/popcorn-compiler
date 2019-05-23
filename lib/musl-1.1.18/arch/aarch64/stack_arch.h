
/* Original version by the musl authors */
/* Current version by Antonio Barbalace, Stevens 2019 */

#define arch_stack_get() \
	({ unsigned long stack_ptr =-1; \
	__asm__ volatile ("mov %0, sp\n\t" \
		: "=r" (stack_ptr) \
		: : "memory"); \
	stack_ptr; })

/* stack relocation configuration parameters */

#define STACK_MB           (1024*1024)
#define STACK_SIZE         (16*STACK_MB)
#define STACK_END_ADDR     (0x800000000000)
#define STACK_START_ADDR   (STACK_END_ADDR - STACK_SIZE)
#define STACK_PAGE_SIZE    (4096)
#define STACK_MAPPED_PAGES (32)

/* stack relocation arch dep macros */

#define arch_stack_switch(stack_top, stack_offset) \
	({ __asm__ volatile("sub %1, %0, %1 \n\t" \
			"mov sp, %1 \n\t" \
			: :"r" (stack_top), "r" (stack_offset) \
			: "memory"); })


