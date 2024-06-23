#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

struct sem_t
{
    void *opaque;
};

int    sem_close(sem_t *);
int    sem_destroy(sem_t *);
int    sem_getvalue(sem_t *, int *);
int    sem_init(sem_t *, int, unsigned);
sem_t *sem_open(const char *, int, ...);
int    sem_post(sem_t *);
int    sem_timedwait(sem_t *, const struct timespec *);
int    sem_trywait(sem_t *);
int    sem_unlink(const char *);
int    sem_wait(sem_t *);

#define SEM_FAILED (-1)

#ifdef __cplusplus
}
#endif

#endif
