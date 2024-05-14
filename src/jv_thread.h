#ifndef JV_THREAD_H
#define JV_THREAD_H

#ifdef WIN32
#ifndef __MINGW32__
#include <windows.h>
#include <winnt.h>
#include <errno.h>

/* Copied from Heimdal: pthread-like mutexes for WIN32 -- see lib/base/heimbase.h in Heimdal */
typedef struct pthread_mutex {
    HANDLE      h;
} pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER { INVALID_HANDLE_VALUE }

static inline int
pthread_mutex_init(pthread_mutex_t *m)
{
    m->h = CreateSemaphore(NULL, 1, 1, NULL);
    if (m->h == INVALID_HANDLE_VALUE)
        return EAGAIN;
    return 0;
}

static inline int
pthread_mutex_lock(pthread_mutex_t *m)
{
    HANDLE h, new_h;
    int created = 0;

    h = InterlockedCompareExchangePointer(&m->h, m->h, m->h);
    if (h == INVALID_HANDLE_VALUE || h == NULL) {
        created = 1;
        new_h = CreateSemaphore(NULL, 0, 1, NULL);
        if (new_h == INVALID_HANDLE_VALUE)
            return EAGAIN;
        if (InterlockedCompareExchangePointer(&m->h, new_h, h) != h) {
            created = 0;
            CloseHandle(new_h);
        }
    }
    if (!created)
        WaitForSingleObject(m->h, INFINITE);
    return 0;
}

static inline int
pthread_mutex_unlock(pthread_mutex_t *m)
{
    if (ReleaseSemaphore(m->h, 1, NULL) == FALSE)
        return EPERM;
    return 0;
}
static inline int
pthread_mutex_destroy(pthread_mutex_t *m)
{
    HANDLE h;

    h = InterlockedCompareExchangePointer(&m->h, INVALID_HANDLE_VALUE, m->h);
    if (h != INVALID_HANDLE_VALUE)
        CloseHandle(h);
    return 0;
}

typedef unsigned long pthread_key_t;
int pthread_key_create(pthread_key_t *, void (*)(void *));
int pthread_setspecific(pthread_key_t, void *);
void *pthread_getspecific(pthread_key_t);
#else
#include <pthread.h>
#endif
#else
#include <pthread.h>
#endif
#endif /* JV_THREAD_H */
