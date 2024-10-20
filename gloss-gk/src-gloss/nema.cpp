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
static nema_ringbuffer_t nema_rb;
#define GPU2D_BASE 0x52014000U
#define GPU2D_CLID                      (0x148U)   /*!< GPU2D Last Command List Identifier Register Offset */

extern "C" void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void clock_get_now_monotonic(struct timespec *tp);

int GK_NemaEnable(void **rb)
{
    __syscall_nemaenable_params p {
        .mutexes = nema_mutexes,
        .nmutexes = MUTEX_MAX + 1,
        .rb = &nema_rb,
        .irq_sem = &nema_irq_sem
    };
    auto ret = deferred_call(__syscall_nemaenable, &p);
    if(ret == 0)
    {
        if(rb)
            *rb = &nema_rb;
    }
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
    return nema_buffer_create(size);
}

nema_buffer_t nema_buffer_create(int size)
{
    auto mret = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SYNC | MAP_ANON, 0, 0);
    
    nema_buffer_t ret;
    if(mret != MAP_FAILED)
    {
        ret.size = size;
        ret.base_phys = (uintptr_t)mret;
        ret.base_virt = mret;
    }
    else
    {
        ret.size = 0;
        ret.base_phys = 0;
        ret.base_virt = 0;
    }
    ret.fd = 0;

    return ret;
}

void nema_buffer_destroy(nema_buffer_t *bo)
{
    munmap(bo->base_virt, bo->size);
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
    timespec now;
    clock_get_now_monotonic(&now);
    now.tv_nsec += 16666667;
    while(now.tv_nsec >= 1000000000)
    {
        now.tv_nsec -= 1000000000;
        now.tv_sec++;
    }

    return sem_clockwait(&nema_irq_sem, CLOCK_MONOTONIC, &now);
}

int nema_wait_irq_cl(int cl_id)
{
    static int last_cl_id = -1;
    last_cl_id = nema_reg_read(GPU2D_CLID);

    while(last_cl_id < cl_id)
    {
        nema_wait_irq();
        last_cl_id = nema_reg_read(GPU2D_CLID);        
    }

    return 0;
}
