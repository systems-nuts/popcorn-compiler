#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "migrate.h"
#include <sys/time.h>

volatile int __indicator = -1;
int my_flag = 0;
struct timeval tv;


/* Check if we should migrate, trap if migration needed. */
void check_migrate(void (*callback)(void *), void *callback_data)
{
    if(__indicator < 0 || !my_flag)
        return;
#if defined(__x86_64__)
    __asm__ volatile("int $0x03");
#elif defined(__aarch64__)
    __asm__ volatile(".inst 0xd4200000");
#endif
	my_flag = 0;
	unifico_timers_snapshot();
}

