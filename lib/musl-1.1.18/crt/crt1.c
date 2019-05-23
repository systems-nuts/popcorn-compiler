
/* Original version by the musl authors */
/* Current version by Antonio Barbalace, Stevens 2019 */

/* The current version supports aarch64 stack relocation, but can compile
 * on any architecture. Support for stack relocation on other architecture is
 * future work. A couple of heurisitcs are used in order to keep code compact.
 * Future versions may try to use mremap instead of creating a new area,
 * copying and unmapping, but also moving [vdso] and [vvar]. Finally, canaries
 * for the stack must be added.
 */

#include <features.h>

#define START "_start"

#include "crt_arch.h"


#ifdef STACK_RELOC
#include "stack_arch.h"
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "syscall.h"
#include <elf.h>

#if ULONG_MAX == 0xffffffff
typedef Elf32_auxv_t Auxv;
#else
typedef Elf64_auxv_t Auxv;
#endif
#endif /* STACK_RELOC */


int main();
void _init() __attribute__((weak));
void _fini() __attribute__((weak));
_Noreturn int __libc_start_main(int (*)(), int, char **,
	void (*)(), void(*)(), void(*)());


void _start_c(long *p)
{
	register int argc = p[0];
	register char **argv = (void *)(p+1);

#ifdef STACK_RELOC
    /* stack relocation code */
    char **envp = argv+argc+1;
    Auxv *auxv; 
    int i, copied =-1, size =-1, total_size =-1;
    long stack_ptr =-1, max =0;
    void* stack_addr;
	
    /* ARCH getting the the current stack pointer */
    stack_ptr = arch_stack_get();
    
    /* check if relocation is not needed. This may happen when the current
     * stack is below the requested stack address.
     */
    if ( STACK_START_ADDR > (unsigned long) stack_ptr)
        goto _abort_relocation;
    
    /* getting the current dimension of the stack, using heuristics */
    for (i=0; i<argc; i++) {
        if (max < (long)argv[i])
            max = (long)argv[i];
    }
    for (i=0; envp[i]; i++) {
        if (max < (long)envp[i])
            max = (long)envp[i];
    }
    auxv = (Auxv *)(&envp[i+1]);
    for (i=0; (auxv[i].a_type != AT_NULL); i++) {
        if (max < (long)auxv[i].a_un.a_val)
            max = (long) auxv[i].a_un.a_val;

	/* check if we need to abort relocation, for example in case of dynamic 
	 * linking. The key heuristic is to check if the text section is above
	 * the new stack address -- as we don't relocate the text section, we 
	 * need to abort.
	 */
        if ( (auxv[i].a_type == AT_ENTRY) &&
	    (auxv[i].a_un.a_val >= STACK_END_ADDR) )
		goto _abort_relocation;
    }
    /* align max address */
    max = (max & ~(STACK_PAGE_SIZE -1)) + STACK_PAGE_SIZE;
    size = ((max - (unsigned long)stack_ptr));
	
    /* update expected total mapped size in [stack] */
    total_size = STACK_PAGE_SIZE * (STACK_MAPPED_PAGES + 1 + size/STACK_PAGE_SIZE);

    /* get the memory for the stack */
#ifdef SYS_mmap2
    stack_addr = (void*) __syscall(SYS_mmap2, STACK_START_ADDR, STACK_SIZE, PROT_READ|PROT_WRITE, (MAP_PRIVATE|MAP_ANON|MAP_FIXED), -1, 0);
#else
    stack_addr = (void*) __syscall(SYS_mmap, STACK_START_ADDR, STACK_SIZE, PROT_READ|PROT_WRITE, (MAP_PRIVATE|MAP_ANON|MAP_FIXED), -1, 0);
#endif
    if ( stack_addr == ((void*)-1) )
        goto _error;
    memset(stack_addr, STACK_SIZE, 0);     
    
    /* rewrite pointers for the new stack */
    for (i=0; i<argc; i++)
        argv[i] = (void*) (STACK_END_ADDR - (max - (unsigned long) argv[i])); 
    for (i=0; envp[i]; i++)
        envp[i] = (void*) (STACK_END_ADDR - (max - (unsigned long) envp[i]));
    for (i=0; (auxv[i].a_type != AT_NULL); i++)
        switch (auxv[i].a_type) {
        case AT_PHDR: case AT_BASE: case AT_ENTRY:
        case AT_PLATFORM: case AT_BASE_PLATFORM:
        case AT_EXECFN: case AT_RANDOM: 
            /* check if it is != 0 and greater than the new stack end addr */
            if (auxv[i].a_un.a_val > STACK_END_ADDR)
                auxv[i].a_un.a_val = STACK_END_ADDR - (max - auxv[i].a_un.a_val);

	    /* we don't do VDSO relocation for now (TODO fix when we do VDSO relocation) */
	    case AT_SYSINFO: case AT_SYSINFO_EHDR: 
	    /* all others handled by the kernel */
	    case AT_HWCAP: case AT_PAGESZ: case AT_CLKTCK: case AT_PHENT:
	    case AT_PHNUM: case AT_FLAGS: case AT_UID: case AT_EUID:
	    case AT_GID: case AT_EGID: case AT_SECURE: case AT_EXECFD:
	    case AT_HWCAP2:
            break;
        }
    /* update argv pointer with the new address */
    argv = (void*) (STACK_END_ADDR - (max - (unsigned long) argv));

    /* ARCH copy of the stack */ //TODO can we use SYS_mremap instead?
    copied = __memcpy_nostack((STACK_END_ADDR -size), stack_ptr, size);
    if (copied != size)
        goto _error;
  
    /* ARCH stack switch */
    arch_stack_switch(STACK_END_ADDR, size);

    /* unmap previous stack */
    __syscall(SYS_munmap, (max - total_size), total_size);

_abort_relocation:
#endif /* STACK_RELOC */

    /* now continue to normal startup */
    __libc_start_main(main, argc, argv, _init, _fini, 0);

#ifdef STACK_RELOC
    /* we should reach here only in case of errors */
_error:
    /* from src/exit/_Exit.c */
    //int ec =1;
    __syscall(SYS_exit_group, 1); //ec);
    for (;;) __syscall(SYS_exit, 1); //ec);
#endif /* STACK_RELOC */
}


