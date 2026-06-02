/*
 * Copyright (c) 2026 ISEP.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#define STACK_SIZE 1024

/* Define stacks and thread data */
K_THREAD_STACK_DEFINE(stack_l, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_m, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_h, STACK_SIZE);

LOG_MODULE_REGISTER(foo, 4);

/*
This will cause  the deadline to overflow 
deadline to change to 15800 ticks which will change the order of execution 


*/

struct k_thread thread_l_data;

void thread_low_entry(void *p1, void *p2, void *p3) {
   
	   k_thread_deadline_set(&thread_l_data,21474836478);
	   k_reschedule(); 
  
      LOG_DBG("32 bit deadline: Started with deadline %" PRId64 " ticks\n", _current->base.prio_deadline);

       k_thread_deadline_set_64(&thread_l_data,21474836478);
	   k_reschedule(); 
  
      LOG_DBG("64 bit deadline: Started with deadline %" PRId64 " ticks\n", _current->base.prio_deadline);



         
	  
    
}



int main(void) {

LOG_DBG("Main: start\n");
    k_tid_t thread1_pointer;

   thread1_pointer= k_thread_create(&thread_l_data, stack_l, STACK_SIZE, thread_low_entry, 
                    NULL, NULL, NULL, 10, 0, K_FOREVER);

      k_thread_name_set(thread1_pointer, "Thread_Low");



   LOG_DBG("Main: starting thread1\n"); 
    k_thread_start(thread1_pointer);
    k_msleep(1);
   
    k_thread_join(thread1_pointer, K_FOREVER);

    while (1) {
        k_msleep(1000); // Keep main thread alive
    }


    return 0;
}