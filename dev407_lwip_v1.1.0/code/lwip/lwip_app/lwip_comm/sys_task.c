#include <includes.h>
#include "app_cfg.h"
#include "lwip_cfg.h"

//OS_TCB  start_task_TCB;
OS_TCB  lwip_task_tcb[LWIP_TASK_MAX];
//OS_TCB  wifi_test_task_tcb;

//CPU_STK  start_task_stk[START_TASK_STK_SIZE];
CPU_STK  lwip_task_stk[LWIP_TASK_STK_POOL_SIZE];
//CPU_STK  wifi_test_task_stk[WIFI_TEST_TASK_STK_SIZE];