#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

struct sem_t
{
    void *opaque;
};

int    sem_close(struct sem_t *);
int    sem_destroy(struct sem_t *);
int    sem_getvalue(struct sem_t *, int *);
int    sem_init(struct sem_t *, int, unsigned);
struct sem_t *sem_open(const char *, int, ...);
int    sem_post(struct sem_t *);
int    sem_timedwait(struct sem_t *, const struct timespec *);
int    sem_trywait(struct sem_t *);
int    sem_unlink(const char *);
int    sem_wait(struct sem_t *);

#define SEM_FAILED (-1)

#ifdef __cplusplus
}
#endif

#endif
