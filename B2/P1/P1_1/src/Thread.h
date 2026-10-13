#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "cmsis_os2.h"

#define THREAD_LED2_FLAG (1U << 1)
#define THREAD_LED3_FLAG (1U << 2)

extern osThreadId_t g_tid_led1;
extern osThreadId_t g_tid_led2;
extern osThreadId_t g_tid_led3;

void Init_Threads(void);
void App_LED1_Thread(void *argument);
void App_LED2_Thread(void *argument);
void App_LED3_Thread(void *argument);

#endif
