
/* updated for Popcorn by Antonio Barbalace, Stevens 2019 */

#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include "syscall.h"
#include <string.h>

int epoll_create(int size)
{
	return epoll_create1(0);
}

int epoll_create1(int flags)
{
	int r = __syscall(SYS_epoll_create1, flags);
#ifdef SYS_epoll_create
	if (r==-ENOSYS && !flags) r = __syscall(SYS_epoll_create, 1);
#endif
	return __syscall_ret(r);
}

#ifdef __AARCH64EL__
struct __epoll_event {
    unsigned long events;
    unsigned long data;
};
#endif

int epoll_ctl(int fd, int op, int fd2, struct epoll_event *ev)
{
#ifdef __AARCH64EL__

    int retval; struct __epoll_event __ev; __ev.events=0; __ev.data=0;
    __ev.events = ((unsigned int*)ev)[0];
    __ev.data = (unsigned long*)((((unsigned int*)ev) +1))[0];

    retval = syscall(SYS_epoll_ctl, fd, op, fd2, &__ev);
    return retval;


#else
	return syscall(SYS_epoll_ctl, fd, op, fd2, ev);
#endif
}

int epoll_pwait(int fd, struct epoll_event *ev, int cnt, int to, const sigset_t *sigs)
{
#ifdef __AARCH64EL__
//	struct __epoll_event __ev; __ev.events=0; __ev.data=0;
	int i;
	struct __epoll_event __ev[32];
	memset(&__ev, 0, sizeof(struct __epoll_event)*32);

	int r = __syscall(SYS_epoll_pwait, fd, &__ev, (cnt > 32) ? 32 : cnt, 
		to, sigs, _NSIG/8);
 //       ev->events = __ev.events;
//	((unsigned long*)(((unsigned int*)ev)+1))[0] = __ev.data; //force cast

	for (i=0; (i< 32) && (i< cnt); i++) {
		ev[i].events = __ev[i].events;
		((unsigned long*)(((unsigned int*)&ev[i])+1))[0] = __ev[i].data;
	}
#else
    int r = __syscall(SYS_epoll_pwait, fd, ev, cnt, to, sigs, _NSIG/8);
#endif
    
#ifdef SYS_epoll_wait
	if (r==-ENOSYS && !sigs) {
 #ifdef __AARCH64EL__
		__ev.events=0; __ev.data=0;
        r = __syscall(SYS_epoll_wait, fd, &__ev, cnt, to);        
        ev->events = __ev.events;
        ev->data.u64 = __ev.data;
 #else       
        r = __syscall(SYS_epoll_wait, fd, ev, cnt, to);
 #endif
	}
#endif
	return __syscall_ret(r);
}

int epoll_wait(int fd, struct epoll_event *ev, int cnt, int to)
{
	return epoll_pwait(fd, ev, cnt, to, 0);
}
