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


LOG_MODULE_REGISTER(foo, 4);



struct k_thread thread_l_data;

/*
This show the wrap around of the deadline value after 5 minutes 

*/



void thread_low_entry(void *p1, void *p2, void *p3) {

    k_sleep(K_SECONDS(358));

	   k_thread_deadline_set_64(&thread_l_data,1);
	    k_reschedule(); 
  
      LOG_DBG("Thread_Low: Started with deadline %" PRId64 " ticks\n", _current->base.prio_deadline);

	  
    
}



int main(void) {

LOG_DBG("Main: start\n");
    k_tid_t thread1_pointer;

   thread1_pointer= k_thread_create(&thread_l_data, stack_l, STACK_SIZE, thread_low_entry, 
                    NULL, NULL, NULL, 0, 0, K_FOREVER);

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