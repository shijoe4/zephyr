

#include <zephyr/kernel.h>
// #include <zephyr/random/random.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#define STACK_SIZE 1024
#define LOOP 1
int counter1 = 0;

struct k_thread; 

LOG_MODULE_REGISTER(priotest, 4);
K_THREAD_STACK_DEFINE(stack_1, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_2, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_3, STACK_SIZE);


struct k_thread thread_1_data, thread_2_data, thread_3_data;
struct k_mutex_ceiling_floor shared_mutex1;

void thread_1_entry(void *p1, void *p2, void *p3) {

 int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

   next_run += k_ms_to_ticks_ceil64(1000);

   if(k_mutex_ceiling_lock(&shared_mutex1, K_FOREVER)==0){

     counter1++;

    for(int i=0;i<5;i++){
    
    k_busy_wait(k_ms_to_ticks_ceil64(100000)); // 
      
    }

      k_mutex_ceiling_unlock(&shared_mutex1);
   }



  k_sleep(K_TIMEOUT_ABS_MS(next_run));
count++;;



 }

}

void thread_2_entry(void *p1, void *p2, void *p3) {
LOG_DBG("thread 2 started \n");

 int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(200);
   
    k_busy_wait(k_ms_to_ticks_ceil64(40)); 
    
        k_sleep(K_TIMEOUT_ABS_MS(next_run));


count++;

 }
 LOG_DBG("thread 2 finished \n");
}

void thread_3_entry(void *p1, void *p2, void *p3) {

   int64_t next_run = k_uptime_get(); 
 int count = 0;
 while(count<LOOP)
 {

  next_run += k_ms_to_ticks_ceil64(50);

   if(k_mutex_ceiling_lock(&shared_mutex1, K_FOREVER)==0){

           counter1++;

         k_mutex_ceiling_unlock(&shared_mutex1);
    }


       k_sleep(K_TIMEOUT_ABS_MS(next_run));
       count++;

  }
}







int main(void) {

 k_tid_t thread1_pointer, thread2_pointer, thread3_pointer;
    
  	k_mutex_ceiling_init(&shared_mutex1, 50);



thread1_pointer= k_thread_create(&thread_1_data, stack_1, STACK_SIZE, thread_1_entry, 
                    NULL, NULL, NULL, 50, 0, K_FOREVER);
 thread2_pointer= k_thread_create(&thread_2_data, stack_2, STACK_SIZE, thread_2_entry, 
                    NULL, NULL, NULL, 70, 0, K_FOREVER);
 thread3_pointer =k_thread_create(&thread_3_data, stack_3, STACK_SIZE, thread_3_entry, 
                    NULL, NULL, NULL, 60, 0, K_FOREVER);

                    

k_thread_name_set(thread1_pointer,"therad_1");  
k_thread_name_set(thread2_pointer,"therad_2");  
k_thread_name_set(thread3_pointer,"therad_3");  





k_thread_start(&thread_1_data);
k_sleep(K_TICKS(1));
    k_thread_start(&thread_2_data);
k_sleep(K_TICKS(10));
     k_thread_start(&thread_3_data);  
 

 k_thread_join(thread1_pointer, K_FOREVER);
  k_thread_join(thread2_pointer, K_FOREVER);
 k_thread_join(thread3_pointer, K_FOREVER);



    return 0;
}



