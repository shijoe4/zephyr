/**

 * SPDX-License-Identifier: Apache-2.0
 * Demonstrates an EDF scheduling breakdown caused by a deadline overflow.
 * Chronologically, Thread 1's deadline should be later (greater) than Thread 2's. 
 * However, due to an integer overflow, Thread 1's absolute deadline incorrectly 
 * appears smaller, granting it a higher priority and causing it to execute 
 * before Thread 2.
 */
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#define STACK_SIZE 1024

/* Define stacks and thread data */
K_THREAD_STACK_DEFINE(stack_1, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_2, STACK_SIZE);



LOG_MODULE_REGISTER(foo, 4);


struct k_thread thread_1_data, thread_2_data;

void thread_1_entry(void *p1, void *p2, void *p3) {
   
	   k_thread_deadline_set(&thread_1_data,INT32_MAX);
	   k_reschedule(); 
  
      LOG_DBG("thread_1: Started with deadline %" PRId64 " ticks\n", _current->base.prio_deadline);

     for(int i=0;i<5;i++)
      {
        //simulate work
        k_busy_wait(10000);
      }
  
      LOG_DBG("thread_1: Finished\n");
    
}

void thread_2_entry(void *p1, void *p2, void *p3) {
   
	   k_thread_deadline_set(&thread_2_data,INT32_MAX/2);
	   k_reschedule(); 
  
      LOG_DBG("thread_2: Started with deadline %" PRId64 " ticks\n", _current->base.prio_deadline);
         for(int i=0;i<5;i++)
      {
        //simulate work
        k_busy_wait(10000);
      }

      LOG_DBG("thread_2: Finished\n");
  
    
}



int main(void) {

LOG_DBG("Main: start\n");
    k_tid_t thread1_pointer, thread2_pointer;

   thread1_pointer= k_thread_create(&thread_1_data, stack_1, STACK_SIZE, thread_1_entry, 
                    NULL, NULL, NULL, 10, 0, K_FOREVER);

      k_thread_name_set(thread1_pointer, "Thread_1_started\n");



   thread2_pointer= k_thread_create(&thread_2_data, stack_2, STACK_SIZE, thread_2_entry, 
                    NULL, NULL, NULL, 10, 0, K_FOREVER);

      k_thread_name_set(thread2_pointer, "Thread_2_started\n");


   LOG_DBG("Main: starting thread1\n"); 
    k_thread_start(thread1_pointer);
    k_thread_start(thread2_pointer);
   
    k_thread_join(thread1_pointer, K_FOREVER);
    k_thread_join(thread2_pointer, K_FOREVER);

    while (1) {
        k_msleep(1000); // Keep main thread alive
    }


    return 0;
}