#define _GNU_SOURCE /* For pthread_setname_mp() */

#include "osal.h"
/* #include "options.h" */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <limits.h>

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "cmsis_os2.h"
#include "../../Core/Src/System/system.h"

/* Priority of timer callback thread (if USE_SCHED_FIFO is set) */
#define TIMER_PRIO 30

#define USECS_PER_SEC (1 * 1000 * 1000)
#define NSECS_PER_SEC (1 * 1000 * 1000 * 1000)

static osMemoryPoolId_t timer_pool;

 os_thread_t * os_thread_create (
   const char * name,
   uint32_t priority,
   size_t stacksize,
   void (*entry) (void * arg),
   void * arg)
{
    osThreadId_t thread;
    osThreadAttr_t attr;
    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = name;
    attr.priority = (osPriority_t)priority;
    attr.stack_size  = stacksize;

    thread = osThreadNew(entry, arg, &attr);

    return (os_thread_t*)thread;
} 

void * os_malloc (size_t size)
{
    return malloc(size);
}

void os_free (void * ptr)
{
    free(ptr);
}

os_mutex_t * os_mutex_create (void)
{
    osMutexAttr_t attr;
    memset(&attr, 0, sizeof(osMutexAttr_t));
    osMutexId_t mutex = osMutexNew(NULL);

    return (os_mutex_t*)mutex;
}

void os_mutex_lock (os_mutex_t * _mutex)
{
    osMutexId_t mutex = (osMutexId_t)_mutex;
    if (mutex) {
       osMutexAcquire(mutex, osWaitForever);
   }
}

void os_mutex_unlock (os_mutex_t * _mutex)
{
    osMutexId_t mutex = (osMutexId_t)_mutex;
    if (mutex) {
       osMutexRelease(mutex);
   }
}

void os_mutex_destroy (os_mutex_t * _mutex)
{
    osMutexId_t mutex = (osMutexId_t)_mutex;
    if(mutex) {
        osMutexDelete(mutex);
    }
}

os_sem_t * os_sem_create (size_t count)
{
    osSemaphoreAttr_t attr;
    memset(&attr, 0, sizeof(osSemaphoreAttr_t));
    
    osSemaphoreId_t sem = osSemaphoreNew(0xFFFF, (uint32_t)count, &attr);
    return (os_sem_t*)sem;
}

bool os_sem_wait (os_sem_t * sem, uint32_t time)
{
    if(!sem) {
        return true;
    }
    uint32_t wait_ticks = (time == OS_WAIT_FOREVER) ? osWaitForever : time;
    osStatus_t status = osSemaphoreAcquire((osSemaphoreId_t)sem, wait_ticks);

    return (status != osOK);
}

void os_sem_signal (os_sem_t * sem)
{
    osSemaphoreId_t sem_id = (osSemaphoreId_t)sem;
    if(sem_id) {
        osSemaphoreRelease(sem_id);
    }
}

void os_sem_destroy (os_sem_t * sem)
{
    osSemaphoreId_t sem_id = (osSemaphoreId_t)sem;
    if(sem_id) {
        osSemaphoreDelete(sem_id);
    }
}

void os_usleep (uint32_t usec)
{
   if(usec > 1000) {
        osDelay(usec / 1000);
        usec = usec % 1000;
   }
   if(usec > 0) {
        DWT_Delay_us(usec);
   }
}

uint32_t os_get_current_time_us (void)
{
   return DWT_GetTick_us();
}

os_event_t * os_event_create (void)
{
    osEventFlagsId_t evt_id;
    osEventFlagsAttr_t attr;

    memset(&attr, 0, sizeof(attr));

    evt_id = osEventFlagsNew(&attr);

    return (os_event_t *)evt_id;
}


bool os_event_wait (os_event_t * event, uint32_t mask, uint32_t * value, uint32_t time)
{
    if (event == NULL) {
        return true; 
    }

    uint32_t wait_ticks = (time == OS_WAIT_FOREVER) ? osWaitForever : time;
    uint32_t result = osEventFlagsWait((osEventFlagsId_t)event, mask, osFlagsWaitAny | osFlagsNoClear, wait_ticks);

    bool is_error = ((result & osFlagsError) != 0);

    if (value != NULL) {
        if (is_error) {
            // On timeout, result is an error code, not the flag state.
            // We must manually read the current flags to mimic the original behavior.
            *value = osEventFlagsGet((osEventFlagsId_t)event) & mask;
        }
        else {
            *value = result & mask;
        }
    }
    return is_error; 
}

void os_event_set (os_event_t * event, uint32_t value)
{
    if(event) {
        osEventFlagsSet((osEventFlagsId_t)event, value);
    }
}

void os_event_clr (os_event_t * event, uint32_t value)
{
    if(event) {
        osEventFlagsClear((osEventFlagsId_t)event, value);
    }
}

void os_event_destroy (os_event_t * event)
{
    if(event) {
        osEventFlagsDelete((osEventFlagsId_t)event);
    }
}

os_mbox_t * os_mbox_create (size_t size) 
{
    osMessageQueueId_t mbox_id = osMessageQueueNew((uint32_t)size, sizeof(void *), NULL);
    return (os_mbox_t*)mbox_id;
}

bool os_mbox_fetch (os_mbox_t * mbox, void ** msg, uint32_t time)
{
    if(!mbox || !msg) return true;

    uint32_t wait_ticks = (time == OS_WAIT_FOREVER) ? osWaitForever : time;

    osStatus_t status = osMessageQueueGet((osMessageQueueId_t)mbox, msg, NULL, wait_ticks);
    return (status != osOK);
}

bool os_mbox_post (os_mbox_t * mbox, void * msg, uint32_t time)
{
   if(!mbox || !msg) return true;

    uint32_t wait_ticks = (time == OS_WAIT_FOREVER) ? osWaitForever : time;

    osStatus_t status = osMessageQueuePut((osMessageQueueId_t)mbox, msg, 0, wait_ticks);
    return (status != osOK);
}

void os_mbox_destroy (os_mbox_t * mbox)
{
    if(mbox) {
        osMessageQueueDelete((osMessageQueueId_t)mbox);
    }
}

static void os_timer_callback(void *arg)
{
    os_timer_t* timer = (os_timer_t*)arg;
    if (timer && timer->fn) {
        timer->fn(timer, timer->arg);
    }
}

os_timer_t * os_timer_create (
   uint32_t us,
   void (*fn) (os_timer_t *, void * arg),
   void * arg,
   bool oneshot)
{
    static bool initialized = false;

    if(!initialized) {
        timer_pool = osMemoryPoolNew(10, sizeof(os_timer_t), NULL);
    }
    os_timer_t* timer = (os_timer_t *)osMemoryPoolAlloc(timer_pool, 0U);

    if(timer = NULL) return NULL;

    timer->fn   = fn;
    timer->arg  = arg;
    timer->us   = us;

    osTimerType_t type = oneshot ? osTimerOnce : osTimerPeriodic;
    timer->id = osTimerNew(os_timer_callback, type, timer, NULL);

    if(timer->id == NULL) {
        osMemoryPoolFree(timer_pool, timer);
        return NULL;
    }

    return timer;
}

void os_timer_set (os_timer_t * timer, uint32_t us)
{
   if(timer) timer->us = us;
}

void os_timer_start (os_timer_t * timer)
{
    if(timer) {
        uint32_t ticks = timer->us / 1000;
        if(ticks == 0 && timer->us > 0) {
            ticks = 1;  // Minimum 1 tick resolution
        }

        osTimerStart(timer->id, ticks);
    }
}

void os_timer_stop (os_timer_t * timer)
{
    if(timer) {
        osTimerStop(timer->id);
    }
}

void os_timer_destroy (os_timer_t * timer)
{
    if (timer != NULL) {
        osTimerDelete(timer->id);
        osMemoryPoolFree(timer_pool, timer);
    }
}