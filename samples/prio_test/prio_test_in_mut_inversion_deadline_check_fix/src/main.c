

#include <zephyr/kernel.h>
// #include <zephyr/random/random.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#define STACK_SIZE 1024
#define LOOP 2

struct k_thread; 
void sys_port_track_k_thread_absolute_deadline_set(struct k_thread *thread, int deadline) {}
void sys_port_track_k_thread_deadline_set(struct k_thread *thread, int deadline) {}
K_THREAD_STACK_DEFINE(stack_l, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_m, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_h, STACK_SIZE);

struct k_thread thread_l_data, thread_m_data, thread_h_data;
struct k_mutex_ceiling_floor shared_mutex;

void thread_low_entry(void *p1, void *p2, void *p3) {
     printk("[L] Low Priority Task arrived\n");   
        k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(800));
         k_reschedule();
   
 int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(1000);


      
      printk(" [l] deadline %lld\n",_current->base.prio_deadline);
      printk("[L] Low Priority Task started. Attempting to lock mutex...\n");
   if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
      printk("[L] Late Task (500ms) has the mutex.\n");
      printk(" [l] deadline %lld\n",_current->base.prio_deadline);
    for(int i=0;i<5;i++){
    
    k_busy_wait(k_ms_to_ticks_ceil64(100000)); // 
      
    }
     printk("[L] Late Task releasing mutex.\n");
      k_mutex_ceiling_floor_unlock(&shared_mutex);
   }


   printk("[L] deadline of L before finish  %lld\n",_current->base.prio_deadline);
   printk("[L] Finished work.\n");
  k_sleep(K_TIMEOUT_ABS_MS(next_run));
count++;;

      k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(800));
             k_reschedule();

 }

}

void thread_med_entry(void *p1, void *p2, void *p3) {
   printk("[M] Middle Priority Task arrived\n");   
    k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(50));
           k_reschedule();

 int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(200);

    
   printk(" [m] deadline %lld\n",_current->base.prio_deadline);
    
    
    k_busy_wait(k_ms_to_ticks_ceil64(40)); 
    
      printk("[M] deadline of M before finish  %lld\n",_current->base.prio_deadline);
       printk("[M] Middle Task finished.\n");
      
  k_sleep(K_TIMEOUT_ABS_MS(next_run));

count++;
  k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(50));
        k_reschedule();
 }
}

void thread_high_entry(void *p1, void *p2, void *p3) {
   printk("[H] High Priority Task arrived\n");   
      k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(10));
             k_reschedule();
   int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(50);

  
     
      printk("[H] deadline of H  %lld\n",_current->base.prio_deadline);

       printk("[H] Critical Task (100ms) waiting for mutex...\n");
    
   if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
           printk("[H] Critical Task finally got mutex executed!\n");
           printk(" [h] deadline %lld\n",_current->base.prio_deadline);
         k_mutex_ceiling_floor_unlock(&shared_mutex);
    }

      printk("[H] deadline of H before finish  %lld\n",_current->base.prio_deadline);    
      printk("[H] Finished work.\n");
       k_sleep(K_TIMEOUT_ABS_MS(next_run));
       count++;
             k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(10));
                    k_reschedule();
  }
}

int main(void) {

 k_tid_t thread1_pointer, thread2_pointer, thread3_pointer, thread4_pointer,thread5_pointer;
    
  	k_mutex_ceiling_floor_init(&shared_mutex, 90,k_ms_to_cyc_floor64(10));

    // 1. Create threads in suspended state (K_FOREVER)
thread1_pointer= k_thread_create(&thread_l_data, stack_l, STACK_SIZE, thread_low_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread2_pointer= k_thread_create(&thread_m_data, stack_m, STACK_SIZE, thread_med_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread3_pointer =k_thread_create(&thread_h_data, stack_h, STACK_SIZE, thread_high_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);

k_thread_absolute_deadline_set(thread1_pointer,0);
k_thread_absolute_deadline_set(thread2_pointer,0);
k_thread_absolute_deadline_set(thread3_pointer,0);
k_thread_name_set(thread1_pointer,"therad_1");  
k_thread_name_set(thread2_pointer,"therad_2");  
k_thread_name_set(thread3_pointer,"therad_3");  


    k_thread_start(&thread_l_data);

k_sleep(K_TICKS(1));
    k_thread_start(&thread_m_data);
k_sleep(K_TICKS(1));
     k_thread_start(&thread_h_data);  

 k_thread_join(thread1_pointer, K_FOREVER);
  k_thread_join(thread2_pointer, K_FOREVER);
 k_thread_join(thread3_pointer, K_FOREVER);

    return 0;
}



