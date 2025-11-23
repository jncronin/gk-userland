#if __GAMEKID__ != 4

#define _POSIX_THREADS

#include "gk.h"
#include "syscalls.h"
#include "deferred.h"
#include <pthread.h>
#include "../../nemagfx/inc/nema_core.h"
#include <sys/mman.h>
#include <semaphore.h>

static pthread_mutex_t nema_mutexes[MUTEX_MAX + 1];
static sem_t nema_irq_sem;
#define GPU2D_BASE 0x52014000U
#define GPU2D_CLID                      (0x148U)   /*!< GPU2D Last Command List Identifier Register Offset */

extern "C" void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void clock_get_now_monotonic(struct timespec *tp);

int GK_NemaEnable(void **rb, pthread_mutex_t *eof_mutex, void **cl_a, void **cl_b,
    void **ones, void **zeros)
{
    __syscall_nemaenable_params p {
        .mutexes = nema_mutexes,
        .nmutexes = MUTEX_MAX + 1,
        .rb = rb,
        .irq_sem = &nema_irq_sem,
        .eof_mutex = eof_mutex,
        .cl_a = cl_a,
        .cl_b = cl_b,
        .ones = ones,
        .zeros = zeros
    };
    auto ret = deferred_call(__syscall_nemaenable, &p);
    return ret;
}

uint32_t nema_reg_read(uint32_t reg)
{
    auto ret = *(volatile uint32_t *)(GPU2D_BASE + reg);
    __asm__ volatile("dsb \n" ::: "memory");
    return ret;
}

void nema_reg_write(uint32_t reg, uint32_t value)
{
    __asm__ volatile("dmb \n" ::: "memory");
    *(volatile uint32_t *)(GPU2D_BASE + reg) = value;
}

nema_buffer_t nema_buffer_create_pool(int pool, int size)
{
    printf("nema_buffer_create_pool(%d, %d)\n", pool, size);
    return nema_buffer_create(size);
}

nema_buffer_t nema_buffer_create(int size)
{
    printf("nema_buffer_create(%d)\n", size);

    auto mret = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SYNC | MAP_ANON, 0, 0);
    
    nema_buffer_t ret;
    if(mret != MAP_FAILED)
    {
        ret.size = size;
        ret.base_phys = (uintptr_t)mret;
        ret.base_virt = mret;
        ret.fd = 0;
    }
    else
    {
        auto mallret = malloc(size);
        if(mallret)
        {
            ret.size = size;
            ret.base_virt = mallret;
            ret.base_phys = (uintptr_t)mallret;
            ret.fd = 1;
        }
        else
        {
            ret.size = 0;
            ret.base_phys = 0;
            ret.base_virt = 0;
            ret.fd = 0;
        }
    }

    return ret;
}

void nema_buffer_destroy(nema_buffer_t *bo)
{
    if(bo->base_virt)
    {
        if(bo->fd == 0)
            munmap(bo->base_virt, bo->size);
        else if(bo->fd == 1)
            free(bo->base_virt);
        // fd == 2 is pre-allocated by gk - don't free
    }
}

void *nema_buffer_map(nema_buffer_t *bo)
{
    return bo->base_virt;
}

void nema_buffer_unmap(nema_buffer_t *bo)
{
}

void *nema_host_malloc(size_t size)
{
    return malloc(size);
}

void nema_host_free(void *ptr)
{
    free(ptr);
}

void nema_buffer_flush(nema_buffer_t *bo)
{
    if(bo->base_virt && bo->fd == 1)
    {
        GK_CacheFlush(bo->base_virt, bo->size, false);
    }
}

int nema_mutex_lock(int mutex_id)
{
    if(mutex_id < 0 || mutex_id > MUTEX_MAX)
        return -1;
    return pthread_mutex_lock(&nema_mutexes[mutex_id]);
}

int nema_mutex_unlock(int mutex_id)
{
    if(mutex_id < 0 || mutex_id > MUTEX_MAX)
        return -1;
    return pthread_mutex_unlock(&nema_mutexes[mutex_id]);
}

int nema_wait_irq()
{
    /*timespec now;
    clock_get_now_monotonic(&now);
    now.tv_nsec += 16666667;
    while(now.tv_nsec >= 1000000000)
    {
        now.tv_nsec -= 1000000000;
        now.tv_sec++;
    }

    return sem_clockwait(&nema_irq_sem, CLOCK_MONOTONIC, &now);*/
    return sem_wait(&nema_irq_sem);
}

int nema_wait_irq_cl(int cl_id)
{
    static int last_cl_id = 0;
    last_cl_id = nema_reg_read(GPU2D_CLID);

    timespec until;
    clock_get_now_monotonic(&until);
    until.tv_nsec += 16666667;
    while(until.tv_nsec >= 1000000000)
    {
        until.tv_nsec -= 1000000000;
        until.tv_sec++;
    }

    do
    {
        nema_wait_irq();
        last_cl_id = nema_reg_read(GPU2D_CLID);       

        /*timespec now;
        clock_get_now_monotonic(&now);
        if(now.tv_sec > until.tv_sec)
            return 0;
        if(now.tv_sec == until.tv_sec)
        {
            if(now.tv_nsec >= until.tv_nsec)
                return 0;
        }*/
    } while(last_cl_id < cl_id);

    return 0;
}

int GK_ICACHEInvalidate()
{
    return deferred_call<void *>(__syscall_icacheinvalidate, nullptr);
}

#endif
