

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
struct k_mutex_ceiling_floor shared_mutex1;


void thread_med_entry(void *p1, void *p2, void *p3) {
 k_thread_deadline_set(_current, 500000);

  printk(" [m] deadline %lld\n",_current->base.prio_deadline);
    
   if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
           printk("[M] medium Task finally got mutex executed!\n");
               k_busy_wait(10000000); // 1 second of CPU hogging

            if(k_mutex_ceiling_floor_lock(&shared_mutex1, K_FOREVER)==0){
                printk("[M] medium Task finally got mutex1 executed!\n");
                 k_busy_wait(10000000); // 1 second of CPU hogging
                 printk(" [m] deadline %lld\n",_current->base.prio_deadline);
                 k_mutex_ceiling_floor_unlock(&shared_mutex1);
            }


           printk(" [m] deadline %lld\n",_current->base.prio_deadline);
         k_mutex_ceiling_floor_unlock(&shared_mutex);
    }

    
      printk("[M] Middle Task finished.\n");
}

void thread_high_entry(void *p1, void *p2, void *p3) {

   k_thread_deadline_set(_current, 100);
  printk(" [h] deadline %lld\n",_current->base.prio_deadline);
    
      
   if(k_mutex_ceiling_floor_lock(&shared_mutex1, K_FOREVER)==0){
           printk("[H] High Task finally got mutex executed!\n");
               k_busy_wait(10000000); // 1 second of CPU hogging

            if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
                printk("[H] High Task finally got mutex1 executed!\n");
                 k_busy_wait(10000000); // 1 second of CPU hogging
                 printk(" [h] deadline %lld\n",_current->base.prio_deadline);
                 k_mutex_ceiling_floor_unlock(&shared_mutex);
            }


           printk(" [h] deadline %lld\n",_current->base.prio_deadline);
         k_mutex_ceiling_floor_unlock(&shared_mutex1);
    }

    
      printk("[H] High Task finished.\n");
}
int main(void) {

 k_tid_t thread1_pointer, thread2_pointer, thread3_pointer, thread4_pointer,thread5_pointer;
    
  	k_mutex_ceiling_floor_init(&shared_mutex, 90, 100);
    k_mutex_ceiling_floor_init(&shared_mutex1, 90, 100);

 thread2_pointer= k_thread_create(&thread_m_data, stack_m, STACK_SIZE, thread_med_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread3_pointer =k_thread_create(&thread_h_data, stack_h, STACK_SIZE, thread_high_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);

k_thread_name_set(thread2_pointer,"therad_2");  
k_thread_name_set(thread3_pointer,"therad_3");  

 
    // k_thread_deadline_set(thread3_pointer, 100);
    k_thread_start(&thread_m_data);

    // k_msleep(10000);
     printk(" main started high thread\n");
     k_thread_start(&thread_h_data);  


  k_thread_join(thread2_pointer, K_FOREVER);
 k_thread_join(thread3_pointer, K_FOREVER);

    return 0;
}



