

#include <zephyr/kernel.h>

#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

LOG_MODULE_REGISTER(priotest, 4);

#define STACK_SIZE 1024
#define LOOP 1

struct k_thread; 
void sys_port_track_k_thread_absolute_deadline_set(struct k_thread *thread, int deadline) {}
void sys_port_track_k_thread_deadline_set(struct k_thread *thread, int deadline) {}
K_THREAD_STACK_DEFINE(stack_1, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_2, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_3, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_4, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_5, STACK_SIZE);
struct k_thread thread_1_data, thread_2_data, thread_3_data, thread_4_data, thread_5_data;
struct k_mutex_ceiling_floor shared_mutex1;
struct k_mutex_ceiling_floor shared_mutex2;

void thread_1_entry(void *p1, void *p2, void *p3) {
     printk("[1] task arrived\n");   
        k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(8000));
         k_reschedule();
   
 int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {
  next_run += k_ms_to_ticks_ceil64(8000);
      
      LOG_DBG(" [1] deadline %lld\n",_current->base.prio_deadline);
      LOG_DBG(" [1] Priority Task started. Attempting to lock mutex...\n");
   if(k_mutex_ceiling_floor_lock(&shared_mutex2, K_FOREVER)==0){
      LOG_DBG("[1]  has the mutex.\n");
      LOG_DBG(" [1] deadline %lld\n",_current->base.prio_deadline);
    for(int i=0;i<5;i++){
    
    k_busy_wait(k_ms_to_ticks_ceil64(100000)); // 
      
    }
     LOG_DBG("[1] Task releasing mutex.\n");
      k_mutex_ceiling_floor_unlock(&shared_mutex2);
   }

   LOG_DBG("[1] deadline of 1 before finish  %lld\n",_current->base.prio_deadline);
   LOG_DBG("[1] Finished work.\n");
  k_sleep(K_TIMEOUT_ABS_MS(next_run));
count++;;

      k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(8000));
             k_reschedule();

 }

}



void thread_2_entry(void *p1, void *p2, void *p3) {
     printk("[2] Task arrived\n");   
        k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(7000));
         k_reschedule();
   
 int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(7000);


      LOG_DBG(" [2] deadline %lld\n",_current->base.prio_deadline);
      LOG_DBG(" [2]  Attempting to lock mutex...\n");
   if(k_mutex_ceiling_floor_lock(&shared_mutex2, K_FOREVER)==0){
      LOG_DBG(" [2]  has the mutex.\n");
      LOG_DBG(" [2] deadline %lld\n",_current->base.prio_deadline);
    for(int i=0;i<5;i++){
    
    k_busy_wait(k_ms_to_ticks_ceil64(100000)); // 
      
    }
     LOG_DBG(" [2]  releasing mutex.\n");
      k_mutex_ceiling_floor_unlock(&shared_mutex2);
   }


   LOG_DBG("[2] deadline of 2 before finish  %lld\n",_current->base.prio_deadline);
   LOG_DBG("[2] Finished work.\n");
  k_sleep(K_TIMEOUT_ABS_MS(next_run));
count++;;

      k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(7000));
             k_reschedule();

 }

}



void thread_3_entry(void *p1, void *p2, void *p3) {
   printk("[3] Task arrived\n");   
    k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(500));
           k_reschedule();

 int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(500);

    
   LOG_DBG(" [3] deadline %lld\n",_current->base.prio_deadline);
    
   if(k_mutex_ceiling_floor_lock(&shared_mutex1, K_FOREVER)==0){
      LOG_DBG("[3]  has the mutex.\n");
      LOG_DBG(" [3] deadline %lld\n",_current->base.prio_deadline);
    for(int i=0;i<5;i++){
    
    k_busy_wait(k_ms_to_ticks_ceil64(100000)); // 
      
    }
     LOG_DBG("[3]  Task releasing mutex.\n");
      k_mutex_ceiling_floor_unlock(&shared_mutex1);
   }
    
    LOG_DBG("[3] deadline of 3 before finish  %lld\n",_current->base.prio_deadline);
  k_sleep(K_TIMEOUT_ABS_MS(next_run));

count++;
  k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(500));
        k_reschedule();
 }
}

void thread_4_entry(void *p1, void *p2, void *p3) {
   printk("[4]  Task arrived\n");   
      k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(10));
             k_reschedule();
   int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(50);

  
     
      LOG_DBG("[4] deadline of 4  %lld\n",_current->base.prio_deadline);

       LOG_DBG("[4]  waiting for mutex...\n");
    
   if(k_mutex_ceiling_floor_lock(&shared_mutex1, K_FOREVER)==0){
           LOG_DBG("[4] Task finally got mutex executed!\n");
           LOG_DBG(" [4] deadline %lld\n",_current->base.prio_deadline);
         k_mutex_ceiling_floor_unlock(&shared_mutex1);
    }

      LOG_DBG("[4] deadline of 4 before finish  %lld\n",_current->base.prio_deadline);    
      LOG_DBG("[4] Finished work.\n");
       k_sleep(K_TIMEOUT_ABS_MS(next_run));
       count++;
             k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(10));
                    k_reschedule();
  }
}



void thread_5_entry(void *p1, void *p2, void *p3) {
   printk("[5]  Task arrived\n");   
      k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(10));
             k_reschedule();
   int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(50);

  
     
      LOG_DBG("[5] deadline of 5  %lld\n",_current->base.prio_deadline);

       LOG_DBG("[5]waiting for mutex...\n");
    
   if(k_mutex_ceiling_floor_lock(&shared_mutex1, K_FOREVER)==0){
           LOG_DBG("[5] got mutex executed!\n");
           LOG_DBG(" [5] deadline %lld\n",_current->base.prio_deadline);
         k_mutex_ceiling_floor_unlock(&shared_mutex1);
    }

      LOG_DBG("[5] deadline of 5 before finish  %lld\n",_current->base.prio_deadline);    
      LOG_DBG("[5] Finished work.\n");
       k_sleep(K_TIMEOUT_ABS_MS(next_run));
       count++;
             k_thread_deadline_set(k_current_get(), k_ms_to_cyc_floor64(10));
                    k_reschedule();
  }
}

int main(void) {
 
 k_tid_t thread1_pointer, thread2_pointer, thread3_pointer, thread4_pointer,thread5_pointer;
    
  	
    k_mutex_ceiling_floor_init(&shared_mutex1, 90,k_ms_to_cyc_floor64(20));
    k_mutex_ceiling_floor_init(&shared_mutex2, 90,k_ms_to_cyc_floor64(50));

    // 1. Create threads in suspended state (K_FOREVER)
thread1_pointer= k_thread_create(&thread_1_data, stack_1, STACK_SIZE, thread_1_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread2_pointer= k_thread_create(&thread_2_data, stack_2, STACK_SIZE, thread_2_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread3_pointer =k_thread_create(&thread_3_data, stack_3, STACK_SIZE, thread_3_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread4_pointer =k_thread_create(&thread_4_data, stack_4, STACK_SIZE, thread_4_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);
 thread5_pointer =k_thread_create(&thread_5_data, stack_5, STACK_SIZE, thread_5_entry, 
                    NULL, NULL, NULL, 90, 0, K_FOREVER);

k_thread_absolute_deadline_set(thread1_pointer,0);
k_thread_absolute_deadline_set(thread2_pointer,0);
k_thread_absolute_deadline_set(thread3_pointer,0);
k_thread_absolute_deadline_set(thread4_pointer,0);
k_thread_absolute_deadline_set(thread5_pointer,0);
k_thread_name_set(thread1_pointer,"therad_1");  
k_thread_name_set(thread2_pointer,"therad_2");  
k_thread_name_set(thread3_pointer,"therad_3");  
k_thread_name_set(thread4_pointer,"therad_4");  
k_thread_name_set(thread5_pointer,"therad_5");


    k_thread_start(&thread_1_data);

k_sleep(K_TICKS(1));
    k_thread_start(&thread_2_data);
k_sleep(K_TICKS(1));
     k_thread_start(&thread_3_data);  
k_thread_start(&thread_4_data);
k_thread_start(&thread_5_data);

 k_thread_join(thread1_pointer, K_FOREVER);
  k_thread_join(thread2_pointer, K_FOREVER);
 k_thread_join(thread3_pointer, K_FOREVER);
 k_thread_join(thread4_pointer, K_FOREVER);
 k_thread_join(thread5_pointer, K_FOREVER);

    return 0;
}



