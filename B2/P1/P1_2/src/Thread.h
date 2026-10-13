#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "cmsis_os2.h"

#define THREAD_RIGHT_FLAG (1U << 0)
#define THREAD_JOYSTICK_FLAG (1U << 1)

extern osThreadId_t g_tid_manager;
extern osThreadId_t g_tid_joy;

void Init_Threads(void);
uint8_t System_IsLocked(void);
void App_Joystick_Thread(void *argument);

#endif