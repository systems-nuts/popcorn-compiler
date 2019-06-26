
/* updated for Popcorn by Antonio Barbalace, Stevens 2019 */

#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include "syscall.h"

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
    unsigned long event;
    unsigned long data;
};
#endif

int epoll_ctl(int fd, int op, int fd2, struct epoll_event *ev)
{
#ifdef __AARCH64EL__
    struct __epoll_event __ev;
    __ev.events = (long)ev->events;
    __ev.data = ev->data.u64;
    return syscall(SYS_epoll_ctl, fd, op, fd2, __ev);
#else
	return syscall(SYS_epoll_ctl, fd, op, fd2, ev);
#endif
}

int epoll_pwait(int fd, struct epoll_event *ev, int cnt, int to, const sigset_t *sigs)
{
#ifdef __AARCH64EL__
    struct __epoll_event __ev;
    int r = __syscall(SYS_epoll_pwait, fd, __ev, cnt, to, sigs, _NSIG/8);
    ev.events = __ev.events;
    ev.data.u64 = __ev.data;
#else
    int r = __syscall(SYS_epoll_pwait, fd, ev, cnt, to, sigs, _NSIG/8);
#endif
    
#ifdef SYS_epoll_wait
	if (r==-ENOSYS && !sigs) 
 #if #ifdef __AARCH64EL__
        r = __syscall(SYS_epoll_wait, fd, __ev, cnt, to);        
        ev.events = __ev.events;
        ev.data.u64 = __ev.data;
 #else       
        r = __syscall(SYS_epoll_wait, fd, ev, cnt, to);
 #endif
#endif
	return __syscall_ret(r);
}

int epoll_wait(int fd, struct epoll_event *ev, int cnt, int to)
{
	return epoll_pwait(fd, ev, cnt, to, 0);
}
