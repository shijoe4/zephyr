// /*
//  * Copyright (c) 2012-2014 Wind River Systems, Inc.
//  *
//  * SPDX-License-Identifier: Apache-2.0
//  */

// #include <stdio.h>
// #include <zephyr/mutex_floor/mutex_ceiling_floor.h>


// int main(void)
// {

// 	struct k_mutex_ceiling_floor my_mutex;
// 	k_mutex_ceiling_floor_init(&my_mutex, 5, 1);
// 	k_mutex_ceiling_floor_lock(&my_mutex, K_NO_WAIT);
// 	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);
// 	k_mutex_ceiling_floor_unlock(&my_mutex);

// 	return 0;
// }





/*
 * Copyright (c) 2024 Instituto Superior de Engenharia do Porto (ISEP).
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
//LOG_MODULE_REGISTER(priotest, 4);


/*
zephyr priority inversion test cases 


 */
  uint32_t mutex_valuee=0;

  uint32_t MAX_LOOP=100;

struct k_mutex_ceiling_floor my_mutex;



K_THREAD_STACK_DEFINE(stack1, 2048);
K_THREAD_STACK_DEFINE(stack2, 2048);
K_THREAD_STACK_DEFINE(stack3, 2048);

static struct k_thread therad1_data;
static struct k_thread therad2_data;
static struct k_thread therad3_data;

void thread_function1(void *ignored, void *iter, void *interv);
void thread_function2(void *ignored, void *iter, void *interv);
void thread_function3(void *ignored, void *iter, void *interv);


void thread_function1(void *ignored, void *iter, void *interv)
{

  uint32_t loop=0;
while (loop<MAX_LOOP) {
  loop+=1;
//k_thread_deadline_set	(k_current_get(),200);
    k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);

    mutex_valuee++;
  // printf("thread 1 is running and mutex_valuee is %d\n", mutex_valuee);
   //k_busy_wait(5);
      k_sleep(K_MSEC(sys_rand32_get() % 10));
    k_mutex_ceiling_floor_unlock(&my_mutex);

     k_sleep(K_USEC(sys_rand32_get() % 1000));
}
printf("thread 1 is done\n");
}


void thread_function2(void *ignored, void *iter, void *interv)
{
  uint32_t loop=0;
while (loop<MAX_LOOP) {
loop+=1;
//k_thread_deadline_set	(k_current_get(),400);
     k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);

   mutex_valuee++;
  //k_busy_wait(20000);
 k_sleep(K_MSEC(sys_rand32_get() % 10));
     k_mutex_ceiling_floor_unlock(&my_mutex);

 k_sleep(K_USEC(sys_rand32_get() % 1000));
  
}
 printf("thread 2 is done\n");
}

void thread_function3(void *ignored, void *iter, void *interv)
{

  uint32_t loop=0;

while (loop<MAX_LOOP) {
    loop+=1;

    k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);

    mutex_valuee++;
   // k_busy_wait(2000);
  // / printf("thread 3 is running and mutex_valuee is %d\n", mutex_valuee);
   k_sleep(K_MSEC(sys_rand32_get() % 10));
    k_mutex_ceiling_floor_unlock(&my_mutex);
    //printf("done unlock\n");

    k_sleep(K_USEC(sys_rand32_get() % 1000));

}
 printf("thread 3 is done\n");
}




int main(void)
{
	//k_sleep(K_SECONDS(2));
	k_mutex_ceiling_floor_init(&my_mutex, 100, 10);
//k_mutex_init(&my_mutex2);
k_sleep(K_SECONDS(1));
//printf("Starting threads\n");

  k_tid_t thread1_pointer, thread2_pointer, thread3_pointer;
thread1_pointer=k_thread_create(&therad1_data, stack1, 2048,
                thread_function1,
                NULL, NULL, NULL,
                100, 0, K_FOREVER);

        
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
// k_thread_deadline_set	(	thread3_pointer,600);
// k_thread_deadline_set	(	thread1_pointer,200);
// k_thread_deadline_set	(	thread2_pointer,400);

k_thread_start(thread3_pointer);

//printf("Started thread 3\n");

k_sleep(K_MSEC(1));
k_thread_start(thread1_pointer);
k_thread_start(thread2_pointer);

k_thread_join(thread1_pointer, K_FOREVER);
k_thread_join(thread2_pointer, K_FOREVER);
k_thread_join(thread3_pointer, K_FOREVER);


 k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);

// printf("final value of mutex_valuee is %d\n", mutex_valuee);
assert(mutex_valuee==MAX_LOOP*3);

  k_mutex_ceiling_floor_unlock(&my_mutex);


//	k_thread_start(task1);

//k_thread_join(task1, K_FOREVER);
  printf("All threads are done\n");

	return 0;
}
