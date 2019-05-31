
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
#define STACK_PAGE_SIZE    (4096)
#define STACK_MAPPED_PAGES (32)
#define STACK_END_ADDR     (0x800000000000 - STACK_PAGE_SIZE)
#define STACK_START_ADDR   (STACK_END_ADDR - STACK_SIZE)

/* stack relocation arch dep macros */

#define arch_stack_switch(stack_top, stack_offset) \
	({ __asm__ volatile("sub %1, %0, %1 \n\t" \
			"mov sp, %1 \n\t" \
			: :"r" (stack_top), "r" (stack_offset) \
			: "memory"); })

/* TODO maybe move the following */

//applies to linux only

#define arch_vvar_get_pagesz() (STACK_PAGE_SIZE *1)

// per arch/platform (wasn't able to find this anywhere else in the code)

#define arch_vaddr_max() (0x1000000000000)

