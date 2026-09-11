/**

 * SPDX-License-Identifier: Apache-2.0
 * Demonstrates an EDF scheduling breakdown caused by a deadline overflow.
 * Chronologically, Thread 1's deadline should be later (greater) than Thread 2's. 
 * However, due to an integer overflow, Thread 1's absolute deadline incorrectly 
 * appears smaller, granting it a higher priority and causing it to execute 
 * before Thread 2.
 */
 /*
 *  Copyright (c) 2026 Instituto Superior de Engenharia do Porto (ISEP)
 *  SPDX-License-Identifier: Apache-2.0
 * These copyright is only for new changes made to the file for implementing the priority ceiling/floor mutex with deadline floor.
 */
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>  /* <-- Contains line_ctrl functions & macros */
#include <zephyr/logging/log.h>

#define STACK_SIZE 2048

/* Define stacks and thread data */
K_THREAD_STACK_DEFINE(stack_1, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_2, STACK_SIZE);



LOG_MODULE_REGISTER(foo, 4);


struct k_thread thread_1_data, thread_2_data;

void thread_1_entry(void *p1, void *p2, void *p3) {
    LOG_DBG("thread_1: Started\n");
   
	   k_thread_deadline_set(&thread_1_data,UINT32_MAX);
	   k_reschedule(); 
  
      LOG_DBG("thread_1: Started with deadline %" PRId64 " ticks\n", _current->base.prio_deadline);


volatile uint32_t count = UINT32_MAX; // Large value to simulate work
    while (count > 0) {
        count--; 
    }


        printk("64 bit timer t1: %lld\n", k_cycle_get_64());

 LOG_DBG("thread_1: Finished\n");
  

    
}

void thread_2_entry(void *p1, void *p2, void *p3) {
    LOG_DBG("thread_2: Started\n");
   
	   k_thread_deadline_set(&thread_2_data,UINT32_MAX);
	   k_reschedule(); 
  
      LOG_DBG("thread_2: Started with deadline %" PRId64 " ticks\n", _current->base.prio_deadline);
         for(int i=0;i<5;i++)
      {
        //simulate work
        k_busy_wait(10000);

        //  printk("arch timing %lld",arch_timing_counter_get());
        printk("64 bit timer t2: %lld\n", k_cycle_get_64());
      }

      LOG_DBG("thread_2: Finished\n");
  
    
}



int main(void) {

  
LOG_DBG("Main: start\n");

// while (1) {
//         k_msleep(1000); // Keep main thread alive
//     }

    k_tid_t thread1_pointer, thread2_pointer;

   thread1_pointer= k_thread_create(&thread_1_data, stack_1, STACK_SIZE, thread_1_entry, 
                    NULL, NULL, NULL, 10, 0, K_FOREVER);

      k_thread_name_set(thread1_pointer, "Thread_1_started\n");



   thread2_pointer= k_thread_create(&thread_2_data, stack_2, STACK_SIZE, thread_2_entry, 
                    NULL, NULL, NULL, 10, 0, K_FOREVER);

      k_thread_name_set(thread2_pointer, "Thread_2_started\n");
    k_thread_start(thread2_pointer);





   LOG_DBG("Main: starting thread1\n"); 
    k_thread_start(thread1_pointer);

   
    k_thread_join(thread1_pointer, K_FOREVER);
    k_thread_join(thread2_pointer, K_FOREVER);

    while (1) {
        k_msleep(1000); // Keep main thread alive
    }


    return 0;
}
