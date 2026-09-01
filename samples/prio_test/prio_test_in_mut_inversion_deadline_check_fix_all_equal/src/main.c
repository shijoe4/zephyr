

#include <zephyr/kernel.h>
// #include <zephyr/random/random.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#define STACK_SIZE 1024
K_THREAD_STACK_DEFINE(stack_l, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_m, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_h, STACK_SIZE);

struct k_thread thread_l_data, thread_m_data, thread_h_data;
struct k_mutex_ceiling_floor shared_mutex;

void thread_low_entry(void *p1, void *p2, void *p3) {
    //  k_thread_deadline_set(k_current_get(), 500000); // Late Deadline
    // k_reschedule(); 
     printk(" [l] deadline %lld\n",_current->base.prio_deadline);
     printk("[L] Low Priority Task started. Attempting to lock mutex...\n");
   if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
     printk("[L] Late Task (500ms) has the mutex.\n");
     printk(" [l] deadline %lld\n",_current->base.prio_deadline);
    
    // k_msleep(3000); // Simulate work
    k_busy_wait(3000000); // 
      printk("[L] Late Task releasing mutex.\n");
    k_mutex_ceiling_floor_unlock(&shared_mutex);
   }

   printk("[L] deadline of L after releasing mutex %lld\n",_current->base.prio_deadline);
    //  k_reschedule();

  printk("[L] Finished work.\n");

}

void thread_med_entry(void *p1, void *p2, void *p3) {

    //  printk("m started \n");
    // k_thread_deadline_set(k_current_get(), 500000); // Middle Deadline
    // k_reschedule();
  printk(" [m] deadline %lld\n",_current->base.prio_deadline);
    
    
    k_busy_wait(10000000); // 1 second of CPU hogging
    
      printk("[M] Middle Task finished.\n");
}

void thread_high_entry(void *p1, void *p2, void *p3) {
    // k_thread_deadline_set(k_current_get(), 100); // Earliest Deadline
    // k_reschedule();
     printk("[H] deadline of H  %lld\n",_current->base.prio_deadline);

      printk("[H] Critical Task (100ms) waiting for mutex...\n");
    
   if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
          printk("[H] Critical Task finally got mutex executed!\n");
          printk(" [h] deadline %lld\n",_current->base.prio_deadline);
         k_mutex_ceiling_floor_unlock(&shared_mutex);
    }

   // 
     printk("[H] Finished work.\n");
}

int main(void) {

 k_tid_t thread1_pointer, thread2_pointer, thread3_pointer, thread4_pointer,thread5_pointer;
    
  	k_mutex_ceiling_floor_init(&shared_mutex, 90, 0);

    // 1. Create threads in suspended state (K_FOREVER)
thread1_pointer= k_thread_create(&thread_l_data, stack_l, STACK_SIZE, thread_low_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread2_pointer= k_thread_create(&thread_m_data, stack_m, STACK_SIZE, thread_med_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread3_pointer =k_thread_create(&thread_h_data, stack_h, STACK_SIZE, thread_high_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);

k_thread_name_set(thread1_pointer,"therad_1");  
k_thread_name_set(thread2_pointer,"therad_2");  
k_thread_name_set(thread3_pointer,"therad_3");  

      k_thread_absolute_deadline_set(thread1_pointer, 1500000);
    k_thread_start(&thread_l_data);
    // k_msleep(1); 

  k_thread_absolute_deadline_set(thread2_pointer, 1500000);
   k_thread_absolute_deadline_set(thread3_pointer, 1500000);
   printk(" main started medium thread\n");

    k_thread_start(&thread_m_data);

    // k_msleep(10000);
     printk(" main started high thread\n");
     k_thread_start(&thread_h_data);  

 k_thread_join(thread1_pointer, K_FOREVER);
  k_thread_join(thread2_pointer, K_FOREVER);
 k_thread_join(thread3_pointer, K_FOREVER);

    return 0;
}



