#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "cmsis_os2.h"

extern osThreadId_t g_tid_led1;

void Init_Threads(void);
void App_LED1_Thread(void *argument);

#endif
