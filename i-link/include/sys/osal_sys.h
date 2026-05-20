/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 *
 * This software is licensed under the terms of the BSD 3-clause
 * license. See the file LICENSE distributed with this software for
 * full license information.
 ********************************************************************/

#ifndef OSAL_SYS_H
#define OSAL_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "cmsis_os2.h"
#include <time.h>

// #define OS_THREAD
// #define OS_MUTEX
// #define OS_SEM
// #define OS_EVENT
// #define OS_MBOX
#define OS_TIMER

// typedef void os_thread_t;
// typedef void os_mutex_t;
// typedef void os_sem_t; 
// typedef void os_event_t;

// typedef struct os_mbox
// {
//    osEventFlagsId_t cond;
//    osMutexId_t mutex;
//    size_t r;
//    size_t w;
//    size_t count;
//    size_t size;
//    void * msg[];
// } os_mbox_t;

typedef struct os_timer
{
    osTimerId_t id;
    void (*fn)(struct os_timer *, void *arg);
    void *arg;
    uint32_t us;
} os_timer_t;

#ifdef __cplusplus
}
#endif

#endif /* OSAL_SYS_H */