/**
 * @brief Demonstrates a failure in EDF scheduling order due to a deadline overflow.
 *
 * Scenario:
 * 1. Thread 1 starts at the 5-minute mark with a relative deadline of (INT32_MAX / 2)
 * and a long computation time.
 * 2. Thread 2 arrives later, after Thread 1
 * 3. Chronologically, Thread 2 should have a lower priority (later absolute deadline) 
 * than Thread 1.
 * 4. However, due to an integer overflow in k_cycle_get_32(), Thread 2's calculated 
 * absolute deadline wraps around and becomes numerically smaller than Thread 1's.
 *
 * Result:
 * Thread 2 is incorrectly assigned a higher priority and preempts Thread 1, 
 * breaking the expected EDF execution order.
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
   
	  k_thread_absolute_deadline_set(&thread_1_data,INT32_MAX/2);
	   k_reschedule(); 
  
      LOG_DBG("thread_1: Started with deadline %" PRId32 " ticks\n", _current->base.prio_deadline);

     for(int i=0;i<1000;i++)
      {
        //simulate work
        k_busy_wait(10000);
      }
  
      LOG_DBG("thread_1: Finished\n");
    
}

void thread_2_entry(void *p1, void *p2, void *p3) {
   
	   k_thread_deadline_set(&thread_2_data,1);
	   k_reschedule(); 
  
      LOG_DBG("thread_2: Started with deadline %" PRId32 " ticks\n", _current->base.prio_deadline);
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

   k_sleep(K_SECONDS(360));

  k_thread_start(thread1_pointer);  
    k_thread_start(thread2_pointer); 
    k_thread_join(thread1_pointer, K_FOREVER);
    k_thread_join(thread2_pointer, K_FOREVER);

    while (1) {
        k_msleep(1000); // Keep main thread alive
    }


    return 0;
}
