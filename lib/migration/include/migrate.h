#ifndef _MIGRATE_H
#define _MIGRATE_H

#if !defined(__aarch64__) && !defined(__powerpc64__) && !defined(__x86_64__)
# error Unknown/unsupported architecture!
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int my_flag;

/**
 * Check if thread should migrate, and if so, invoke migration.  The optional
 * callback function will be invoked before execution resumes on destination
 * architecture.
 *
 * @param callback a callback function to be invoked before execution resumes
 *                 on destination architecture
 * @param callback_data data to be passed to the callback function
 */
void check_migrate(void (*callback)(void*), void *callback_data);

#include <stdio.h>
#include <sys/time.h>

#define _UNIFICO_TIMER_NUM 38
//#define UNIFICO_TIMERS_DECLARE
extern double unifico_timers[_UNIFICO_TIMER_NUM];
extern int unifico_current_timer;
extern struct timeval tv;

#define UNIFICO_TIMERS_INIT                                                      \
    double unifico_timers[_UNIFICO_TIMER_NUM] = {0};                               \
    int unifico_current_timer = -1;

#define unifico_timers_snapshot()                                                \
    do {                                                                       \
		printf("snapshot\n"); \
        gettimeofday(&tv, (void *)1);                                          \
        unifico_timers[++unifico_current_timer] = tv.tv_sec + 1.0e-6 * tv.tv_usec; \
    } while (0)

#define unifico_timers_print()                                                   \
    do {                                                                       \
        for (int i = 1; i < _UNIFICO_TIMER_NUM; ++i) {                           \
            printf("unifico time (in secs) for part %d %9.3f\n", i - 1,          \
                   unifico_timers[i] - unifico_timers[i - 1]);                     \
        }                                                                      \
    } while (0)

//#else
//
//#define UNIFICO_TIMERS_DECLARE ;
//#define UNIFICO_TIMERS_INIT ;
//
//#define unifico_timers_snapshot()                                                \
//    do {                                                                       \
//    } while (0);
//
//#define unifico_timers_print()                                                   \
//    do {                                                                       \
//    } while (0);

#ifdef __cplusplus
}
#endif

#endif /* _MIGRATE_H */

