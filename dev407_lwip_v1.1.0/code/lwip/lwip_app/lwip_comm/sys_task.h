#ifndef _SYS_TASK_H
#define _SYS_TASK_H
#include <includes.h>
#include "app_cfg.h"


extern OS_TCB   start_task_TCB;
extern CPU_STK  start_task_stk[];

extern OS_TCB lwip_task_tcb[];
extern CPU_STK lwip_task_stk[];

extern OS_TCB   wifi_test_task_tcb;
extern CPU_STK  wifi_test_task_stk[];

#endif
