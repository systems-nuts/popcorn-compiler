
/* Antonio Barbalace, Stevens 2019 */

/* tested only on 64bit architectures */

#include <stdio.h>
#include <unistd.h>
#include <elf.h>

#include "auxv.h"

int main (int argc, char * argv[], char * envp[])
{
    
    printf("argc %d &argc 0x%lx argv 0x%lx\n",
        argc, (unsigned long)&argc, (unsigned long)argv);
    
    printf("\n");
    int i;
    for (i=0; i< argc; i++)
        printf("argv %d at 0x%lx =%s\n",
            i, (unsigned long)argv[i], argv[i]);
    
    printf("\n");    
    i=0;
    while (envp[i++] != 0)
        printf("envp %d at 0x%lx =%s\n",
               i-1, (unsigned long)envp[i-1], envp[i-1]);
    
    printf("\n");
    Elf64_Phdr* phdr=0; long phent=0; long phnum=0;
    Elf64_auxv_t *auxv;
    printf("sizeof(Elf64_auxv_t) %d\n",
           (int)sizeof(Elf64_auxv_t));
    
    for (auxv = (Elf64_auxv_t *)&envp[i]; auxv->a_type != AT_NULL; auxv++)
        switch (auxv->a_type) {
            case AT_PLATFORM:
            case AT_BASE_PLATFORM:
            case AT_EXECFN:
                printf("%s (%d) value %s (0x%lx)\n",
                       at_desc[(int)auxv->a_type], (int)auxv->a_type, (char*)auxv->a_un.a_val, auxv->a_un.a_val);
                break;
            case AT_PHDR:
                phdr = (void*)auxv->a_un.a_val;
                break;
            case AT_PHENT:
                phent = auxv->a_un.a_val;
                break;
            case AT_PHNUM:
                phnum = auxv->a_un.a_val;
                break;
            default:
                printf("%s (%d) value 0x%lx\n",
                       at_desc[(int)auxv->a_type], (int)auxv->a_type, auxv->a_un.a_val);
        };

    printf("\n");
    printf("phdr 0x%lx phent %d (%d) phnum %d\n",
        (unsigned long)phdr, (int)phent, (int)sizeof(Elf64_Phdr), (int)phnum);
    for (i=0; i< phnum; i++) {
        printf("i: %d type: %d flags: %d off: 0x%lx vaddr: 0x%lx paddr: 0x%lx filesz: 0x%lx memsz: 0x%lx align: 0x%lx\n",
                   i, phdr[i].p_type, phdr[i].p_flags, phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_paddr,
                   phdr[i].p_filesz, phdr[i].p_memsz, phdr[i].p_align);
    }
        
    sleep(30);
        
    return 0;
}
