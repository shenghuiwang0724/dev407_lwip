/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/lwip_sys.h"
#include "lwip/mem.h"
#include "timer.h"

#include <includes.h>
#include "cc.h"
#include "ucos_arch/sys_arch.h"

#include "lwip/sys.h"
#include "sys_task.h"
#include "debug_printf.h"

u16 lwip_task_stk_offset;/*point to the next available task stack beginning*/
u16 lwip_task_tcb_offset;/*point to the next available lwip task tcb*/



void sys_init(void)
{
	lwip_task_stk_offset = 0;
	lwip_task_tcb_offset = 0;
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	OS_ERR err;
	

	OSSemCreate(sem, "lwip" ,count,&err);
	
	if(err != OS_ERR_NONE ){
	  DEBUG_ERROR("sys_sem_new");
	  return ERR_MEM;	
	}
	
	return ERR_OK;
}
void sys_sem_free(sys_sem_t *sem)
{
    OS_ERR     err;
    OSSemDel(sem, OS_OPT_DEL_ALWAYS, &err );
    DEBUG_ASSERT( err == OS_ERR_NONE );
}

void sys_sem_signal(sys_sem_t *sem)
{
	OS_ERR	  err;  
	OSSemPost(sem,OS_OPT_POST_ALL,&err);
	DEBUG_ASSERT(err == OS_ERR_NONE );  
}


/*
Blocks the thread while waiting for the semaphore to be
signaled. If the "timeout" argument is non-zero, the thread should
only be blocked for the specified time (measured in
milliseconds). If the "timeout" argument is zero, the thread should be
blocked until the semaphore is signalled.

If the timeout argument is non-zero, the return value is the number of
milliseconds spent waiting for the semaphore to be signaled. If the
semaphore wasn't signaled within the specified time, the return value is
SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
(i.e., it was already signaled), the function may return zero.

Notice that lwIP implements a function with a similar name,
sys_sem_wait(), that uses the sys_arch_sem_wait() function.

*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	OS_ERR	  err;
	OS_TICK  tick_timeout;
	OS_TICK  tick_start;
	OS_TICK  tick_stop;
	

	tick_start = OSTimeGet(&err);
	
	tick_timeout = (timeout * OS_CFG_TICK_RATE_HZ) / 1000;

	if((tick_timeout==0)&&(timeout!=0)){
		tick_timeout = 1;
	}
	
	OSSemPend (sem,tick_timeout,OS_OPT_PEND_BLOCKING,NULL,&err);


	if (OS_ERR_TIMEOUT==err){
		return SYS_ARCH_TIMEOUT;
	}
	
	
	if(OS_ERR_NONE == err){
		
		if(timeout==0){
			return 0;
		}
		
		tick_stop = OSTimeGet(&err);
		return ((tick_stop - tick_start)*1000) / OS_CFG_TICK_RATE_HZ;
	}

	DEBUG_ERROR("never here");

	return 0;

}


/*
Returns 1 if the semaphore is valid, 0 if it is not valid.
When using pointers, a simple way is to check the pointer for != NULL.
When directly using OS structures, implementing this may be more complex.
This may also be a define, in which case the function is not prototyped.

*/
int sys_sem_valid(sys_sem_t *sem)
{
	if(sem->Type != OS_OBJ_TYPE_SEM){
		return 0;
	}
	return 1;
}


/*
Invalidate a semaphore so that sys_sem_valid() returns 0.
ATTENTION: This does NOT mean that the semaphore shall be deallocated:
sys_sem_free() is always called before calling this function!
This may also be a define, in which case the function is not prototyped.

*/
void sys_sem_set_invalid(sys_sem_t *sem)
{
	sem->Type = OS_OBJ_TYPE_NONE;
}


/*
Creates an empty mailbox for maximum "size" elements. Elements stored
in mailboxes are pointers. You have to define macros "_MBOX_SIZE"
in your lwipopts.h, or ignore this parameter in your implementation
and use a default size.
If the mailbox has been created, ERR_OK should be returned. Returning any
other error will provide a hint what went wrong, but except for assertions,
no real error handling is implemented.

*/
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	OS_ERR		 err;
		
	OSQCreate(mbox,"lwip q", size, &err); 
	
	if( err == OS_ERR_NONE){ 
	  return ERR_OK; 
	}

	DEBUG_ERROR("sys_mbox_new %d",err);
	
	return ERR_MEM;

}

/*
Deallocates a mailbox. If there are messages still present in the
mailbox when the mailbox is deallocated, it is an indication of a
programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
	OS_ERR     err;
	    

	OSQDel(mbox, OS_OPT_DEL_ALWAYS, &err);
	DEBUG_ASSERT( err == OS_ERR_NONE );

}

/*
Posts the "msg" to the mailbox. This function have to block until
the "msg" is really posted.
*/
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	OS_ERR	   err;
	CPU_INT08U	i=0; 

	/* try 10 times */
	while(i<10){
	  OSQPost(mbox, msg,0,OS_OPT_POST_ALL,&err);
	  if(err == OS_ERR_NONE)
		break;
	  i++;
	  OSTimeDly(5,OS_OPT_TIME_DLY,&err);
	}
	
	DEBUG_ASSERT( i !=10 );  

}


/*
Try to post the "msg" to the mailbox. Returns ERR_MEM if this one
is full, else, ERR_OK if the "msg" is posted.

*/
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	OS_ERR	   err;


	OSQPost(mbox, msg,0,OS_OPT_POST_ALL,&err);	
	
	if(err != OS_ERR_NONE){
		return ERR_MEM;
	}
	
	return ERR_OK;

}


/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). If "timeout" is 0, the thread should
  be blocked until a message arrives. The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP. 

 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	OS_ERR	  err;
	OS_MSG_SIZE   msg_size;
	OS_TICK  tick_timeout;  
	OS_TICK  tick_start;
	OS_TICK  tick_stop;



	tick_start = OSTimeGet(&err);
	
	tick_timeout = (timeout * OS_CFG_TICK_RATE_HZ) / 1000;

	if((tick_timeout==0)&&(timeout!=0)){
		tick_timeout = 1;
	}
	
	if(msg==NULL){
		OSQPend (mbox,tick_timeout,OS_OPT_PEND_BLOCKING,&msg_size, NULL,&err);
	}
	else {
		*msg  = OSQPend (mbox,tick_timeout,OS_OPT_PEND_BLOCKING,&msg_size, NULL,&err);
	}


	if (OS_ERR_TIMEOUT==err){
		return SYS_ARCH_TIMEOUT;
	}
	
	
	if(OS_ERR_NONE == err){
		
		if(timeout==0){
			return 0;
		}
		
		tick_stop = OSTimeGet(&err);
		return ((tick_stop - tick_start)*1000) / OS_CFG_TICK_RATE_HZ;
	}

	DEBUG_ERROR("never here");

	return 0;

}


/*
This is similar to sys_arch_mbox_fetch, however if a message is not
present in the mailbox, it immediately returns with the code
SYS_MBOX_EMPTY. On success 0 is returned.

To allow for efficient implementations, this can be defined as a
function-like macro in sys_arch.h instead of a normal function. For
example, a naive implementation could be:
  #define sys_arch_mbox_tryfetch(mbox,msg) \
	sys_arch_mbox_fetch(mbox,msg,1)
although this would introduce unnecessary delays.

*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	OS_ERR	  err;
	OS_MSG_SIZE   msg_size;

	if(msg==NULL){
		OSQPend (mbox,0,OS_OPT_PEND_NON_BLOCKING,&msg_size, NULL,&err);
	}
	else {
		*msg  = OSQPend (mbox,0,OS_OPT_PEND_NON_BLOCKING,&msg_size, NULL,&err);
	}

	
	if(err==OS_ERR_NONE){
		return ERR_OK;
	}
	
	if(err==OS_ERR_PEND_WOULD_BLOCK){
		return SYS_MBOX_EMPTY;
	}
	
	DEBUG_ERROR("never here");
	return SYS_MBOX_EMPTY;

}

/*
Returns 1 if the mailbox is valid, 0 if it is not valid.
When using pointers, a simple way is to check the pointer for != NULL.
When directly using OS structures, implementing this may be more complex.
This may also be a define, in which case the function is not prototyped.

*/
int sys_mbox_valid(sys_mbox_t *mbox)
{
	if(mbox->Type != OS_OBJ_TYPE_Q){
		return 0;
	}
	return 1;

}

/*
Invalidate a mailbox so that sys_mbox_valid() returns 0.
ATTENTION: This does NOT mean that the mailbox shall be deallocated:
sys_mbox_free() is always called before calling this function!
This may also be a define, in which case the function is not prototyped.

*/
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	mbox->Type = OS_OBJ_TYPE_NONE;
}


/*
Starts a new thread named "name" with priority "prio" that will begin its
execution in the function "thread()". The "arg" argument will be passed as an
argument to the thread() function. The stack size to used for this thread is
the "stacksize" parameter. The id of the new thread is returned. Both the id
and the priority are system dependent.

*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	OS_ERR err;

	if((lwip_task_stk_offset + stacksize) > LWIP_TASK_STK_POOL_SIZE){
		DEBUG_ERROR("lwip task stack not enough");
		return NULL;
	}

	if(lwip_task_tcb_offset==LWIP_TASK_MAX){
		DEBUG_ERROR("lwip task tcb not enough");
		return NULL;
	}
	
    OSTaskCreate(&lwip_task_tcb[lwip_task_tcb_offset],
                 (CPU_CHAR  *)name,
                 (OS_TASK_PTR)thread, 
                 (void      *)arg,
                 (OS_PRIO    )prio,
                 (CPU_STK   *)&lwip_task_stk[lwip_task_stk_offset],
                 (CPU_STK_SIZE)(stacksize/10),
                 (CPU_STK_SIZE)stacksize,
                 (OS_MSG_QTY )0,
                 (OS_TICK    )0,
                 (void      *)0,
                 (OS_OPT     )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR    *)&err);

	lwip_task_stk_offset +=stacksize;
	lwip_task_tcb_offset++;
	
	DEBUG_ASSERT(err == OS_ERR_NONE);

	return prio;

}

/*
This optional function returns the current time in milliseconds (don't care
for wraparound, this is only used for time diffs).
Not implementing this function means you cannot use some modules (e.g. TCP
timestamps, internal timeouts for NO_SYS==1).

*/
u32_t sys_now(void)
{
	OS_ERR err;

	return (u32_t)((OSTimeGet(&err)*1000)/OS_CFG_TICK_RATE_HZ);
}


void sys_msleep_real(u32_t ms)
{
	OS_ERR err;
	OSTimeDly((ms*OS_CFG_TICK_RATE_HZ)/1000,OS_OPT_TIME_DLY,&err);
	DEBUG_ASSERT(err==OS_ERR_NONE);
}



