



/*
 * Copyright (c) 2024 Instituto Superior de Engenharia do Porto (ISEP).
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
// #include <zephyr/random/random.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
//LOG_MODULE_REGISTER(priotest, 4);

  #define MAX_LOOP 10
/*
zephyr priority inversion test cases 


 */
  uint32_t mutex_valuee=0;
   uint32_t mutex_valuee2=0;
      uint32_t mutex_valuee3=0;


  uint32_t busy_wait_time=500;

struct k_mutex_ceiling_floor my_mutex;
struct k_mutex_ceiling_floor my_mutex2;
struct k_mutex_ceiling_floor my_mutex3;



K_THREAD_STACK_DEFINE(stack1, 2048);
K_THREAD_STACK_DEFINE(stack2, 2048);
K_THREAD_STACK_DEFINE(stack3, 2048);
K_THREAD_STACK_DEFINE(stack4, 2048);
K_THREAD_STACK_DEFINE(stack5, 2048);

static struct k_thread therad1_data;
static struct k_thread therad2_data;
static struct k_thread therad3_data;
static struct k_thread therad4_data;
static struct k_thread therad5_data;

void thread_function1(void *ignored, void *iter, void *interv);
void thread_function2(void *ignored, void *iter, void *interv);
void thread_function3(void *ignored, void *iter, void *interv);
void thread_function4(void *ignored, void *iter, void *interv);
void thread_function5(void *ignored, void *iter, void *interv);








void thread_function1(void *ignored, void *iter, void *interv)
{

  uint32_t loop=0;
while (loop<MAX_LOOP) {
  loop+=1;

   if(k_mutex_ceiling_floor_lock(&my_mutex3, K_FOREVER)==0){

    if(k_mutex_ceiling_floor_lock(&my_mutex2,K_FOREVER)==0) {
    mutex_valuee3++;
     mutex_valuee2++;
  // k_sleep(K_MSEC(20));
        k_busy_wait(100000);
   
     k_mutex_ceiling_floor_unlock(&my_mutex2);
 

   }
 k_mutex_ceiling_floor_unlock(&my_mutex3);
     
     k_sleep(K_MSEC(20));

    
}
else{
    printf("thread 1 failed to lock my_mutex3\n");
   }

}
}



void thread_function2(void *ignored, void *iter, void *interv)
{
  uint32_t loop=0;
while (loop<MAX_LOOP) {
loop+=1;

   if(k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER)==0){

   mutex_valuee++;
     k_busy_wait(100000);
//  k_busy_wait(sys_rand32_get() % 10);
// k_sleep(K_MSEC(20));
     k_mutex_ceiling_floor_unlock(&my_mutex);
      //  k_sleep(K_MSEC(15));
           
          
   }
   else{
    printf("thread 2 failed to lock my_mutex\n");
   }

 k_sleep(K_MSEC(15));
  
}
// printf("thread 2 is done\n");
}

void thread_function3(void *ignored, void *iter, void *interv)
{

  uint32_t loop=0;

while (loop<MAX_LOOP) {
    loop+=1;

    if(k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER)==0){
     mutex_valuee++;
        k_busy_wait(1000000);
        // k_sleep(K_MSEC(100));

        if(k_mutex_ceiling_floor_unlock(&my_mutex)==0);

        // k_sleep(K_MSEC(10));

    }
    else{
    printf("thread 3 failed to lock my_mutex\n");
    }

  




  k_sleep(K_MSEC(10));

}
//  printf("thread 3 is done\n");
}




void thread_function4(void *ignored, void *iter, void *interv)
{

  uint32_t loop=0;

while (loop<MAX_LOOP) {
    loop+=1;

   if(k_mutex_ceiling_floor_lock(&my_mutex2, K_FOREVER)==0){



   if(k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER)==0){
      mutex_valuee++;
// k_busy_wait(sys_rand32_get() % 10);
    mutex_valuee2++;

                k_busy_wait(100000);
         
// k_sleep(K_MSEC(20));
    k_mutex_ceiling_floor_unlock(&my_mutex);
     
  

   }
  
     k_mutex_ceiling_floor_unlock(&my_mutex2);
  
  }
   else{
    printf("thread 4 failed to lock my_mutex2\n");
   }


  

   k_sleep(K_MSEC(5));

}
// printf("thread 4 is done\n");
}



void thread_function5(void *ignored, void *iter, void *interv)
{

  uint32_t loop=0;
while (loop<MAX_LOOP) {
  loop+=1;
int ret=k_mutex_ceiling_floor_lock(&my_mutex2, K_FOREVER);
   if(ret==0){

   if(k_mutex_ceiling_floor_lock(&my_mutex,K_FOREVER)==0) {
    mutex_valuee++;
    mutex_valuee2++;
       k_busy_wait(100000);

    // k_sleep(K_MSEC(10)); 
     k_mutex_ceiling_floor_unlock(&my_mutex);


   }


     k_mutex_ceiling_floor_unlock(&my_mutex2);
     k_sleep(K_MSEC(10));  


}
else{
    printf("thread 5 failed to lock my_mutex %d\n", ret);

}
}
}







int main(void)
{

  	k_mutex_ceiling_floor_init(&my_mutex2, 0, 0);
    k_mutex_ceiling_floor_init(&my_mutex, 0, 0);
    k_mutex_ceiling_floor_init(&my_mutex3, 0, 0);

k_sleep(K_SECONDS(1));


  k_tid_t thread1_pointer, thread2_pointer, thread3_pointer, thread4_pointer,thread5_pointer;
thread1_pointer=k_thread_create(&therad1_data, stack1, 2048,
                thread_function1,
                NULL, NULL, NULL,
                99, 0, K_FOREVER);

        
k_thread_name_set	(thread1_pointer,"therad_1" );       
 


thread2_pointer=k_thread_create(&therad2_data, stack2, 2048,
                thread_function2,
                NULL, NULL, NULL,
                101, 0, K_FOREVER);
k_thread_name_set	(thread2_pointer,"therad_2");   



thread3_pointer=k_thread_create(&therad3_data, stack3, 2048,
                thread_function3,
                NULL, NULL, NULL,
                102, 0, K_FOREVER);
k_thread_name_set	(thread3_pointer,"therad_3");   
thread4_pointer=k_thread_create(&therad4_data, stack4, 2048,
                thread_function4,
                NULL, NULL, NULL,
                100, 0, K_FOREVER);
k_thread_name_set	(thread4_pointer,"therad_4");  



thread5_pointer=k_thread_create(&therad5_data, stack5, 2048,
                thread_function5,
                NULL, NULL, NULL,
                99, 0, K_FOREVER);
k_thread_name_set	(thread5_pointer,"therad_5");  

// printf("All threads have been created\n");

// k_thread_ever_needed_resource(thread5_pointer,1,99);
// k_thread_ever_needed_resource(thread4_pointer,1,99);
// k_thread_ever_needed_resource(thread3_pointer,1,99);
// k_thread_ever_needed_resource(thread2_pointer,1,99);
// k_thread_ever_needed_resource(thread1_pointer,1,99);



// k_thread_deadline_set	(	thread3_pointer,600);
// k_thread_deadline_set	(	thread1_pointer,200);
// k_thread_deadline_set	(	thread2_pointer,400);

k_thread_start(thread3_pointer);

//printf("Started thread 3\n");
k_usleep(100);
// k_sleep(K_MSEC(1));
 k_thread_start(thread5_pointer);
 k_thread_start(thread2_pointer);
 k_thread_start(thread4_pointer);
//  k_sleep(K_MSEC(1));
k_thread_start(thread1_pointer);



//  printf("All threads have started\n");
k_thread_join(thread1_pointer, K_FOREVER);
k_thread_join(thread2_pointer, K_FOREVER);
k_thread_join(thread3_pointer, K_FOREVER);
k_thread_join(thread4_pointer, K_FOREVER);
k_thread_join(thread5_pointer, K_FOREVER);



// while(1){
//      k_sleep(K_SECONDS(1));
//    }

//  printf("All threads have finished execution\n");

 if(k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER)==0){

// printf("final value of mutex_valuee is %d\n", mutex_valuee);
 assert(mutex_valuee==MAX_LOOP*4);

   k_mutex_ceiling_floor_unlock(&my_mutex);

 }
 else{
    printf("Failed to lock my_mutex in main thread\n");
 }
 if(k_mutex_ceiling_floor_lock(&my_mutex2, K_FOREVER)==0){

assert(mutex_valuee2==MAX_LOOP*3);
   k_mutex_ceiling_floor_unlock(&my_mutex2);

 }
 else{
    printf("Failed to lock my_mutex2 in main\n") ;}


 if(k_mutex_ceiling_floor_lock(&my_mutex3, K_FOREVER)==0){

assert(mutex_valuee3==MAX_LOOP*1);
   k_mutex_ceiling_floor_unlock(&my_mutex3);

 }
 else{
    printf("Failed to lock my_mutex3 in main\n") ;}    


  //       while(1){
  //    k_sleep(K_SECONDS(1));
  //  }

// //	k_thread_start(task1);

// //k_thread_join(task1, K_FOREVER);
            printf("All threads are done\n");

	return 0;
}
