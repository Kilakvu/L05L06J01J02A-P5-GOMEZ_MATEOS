#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "cmsis_os2.h"

#define THREAD_RIGHT_FLAG (1U << 0)

extern osThreadId_t g_tid_manager;

void Init_Threads(void);

#endif