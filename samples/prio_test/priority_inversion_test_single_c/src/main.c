/*
 * Copyright (c) 2024 Instituto Superior de Engenharia do Porto (ISEP).
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <zephyr/mutex_floor/mutex_ceiling_floor.h>

/*
zephyr priority inversion test cases 


 */
  uint32_t mutex_valuee=0;


struct k_mutex_ceiling_floor my_mutex;



K_THREAD_STACK_DEFINE(stack1, 1024);
K_THREAD_STACK_DEFINE(stack2, 1024);
K_THREAD_STACK_DEFINE(stack3, 1024);

static struct k_thread therad1_data;
static struct k_thread therad2_data;
static struct k_thread therad3_data;

void thread_function1(void *ignored, void *iter, void *interv);
void thread_function2(void *ignored, void *iter, void *interv);
void thread_function3(void *ignored, void *iter, void *interv);


void thread_function1(void *ignored, void *iter, void *interv)
{

while (true) {

k_thread_deadline_set	(k_current_get(),200);
    k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);

    mutex_valuee++;

   k_busy_wait(2000);
    k_mutex_ceiling_floor_unlock(&my_mutex);

    // k_sleep(K_MSEC(200));
}
}


void thread_function2(void *ignored, void *iter, void *interv)
{
while (true) {

//k_thread_deadline_set	(k_current_get(),400);
   //  k_mutex_lock(&my_mutex2, K_FOREVER);

   // mutex_valuee2++;
  k_busy_wait(20000);

//     k_mutex_unlock(&my_mutex2);

  
}
}

void thread_function3(void *ignored, void *iter, void *interv)
{
while (true) {
//k_thread_deadline_set(	k_current_get(),600);

    k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);

  //   k_mutex_lock(&my_mutex2, K_FOREVER);

    mutex_valuee++;
    //mutex_valuee2++;
    k_busy_wait(20000);
      //  k_sleep(K_MSEC(200));
   //k_mutex_unlock(&my_mutex2);
    k_mutex_ceiling_floor_unlock(&my_mutex);


}
}




int main(void)
{
	//k_sleep(K_SECONDS(2));
	k_mutex_ceiling_floor_init(&my_mutex, 5, 10);
//k_mutex_init(&my_mutex2);
	k_sleep(K_SECONDS(2));

  k_tid_t thread1_pointer, thread2_pointer, thread3_pointer;
thread1_pointer=k_thread_create(&therad1_data, stack1, 1024,
                thread_function1,
                NULL, NULL, NULL,
                5, 0, K_FOREVER);

        
k_thread_name_set	(	thread1_pointer,"therad_1" );       
 


thread2_pointer=k_thread_create(&therad2_data, stack2, 1024,
                thread_function2,
                NULL, NULL, NULL,
                6, 0, K_FOREVER);
k_thread_name_set	(	thread2_pointer,"therad_2" );   



thread3_pointer=k_thread_create(&therad3_data, stack3, 1024,
                thread_function3,
                NULL, NULL, NULL,
                7, 0, K_FOREVER);
k_thread_name_set	(	thread3_pointer,"therad_3" );   
// k_thread_deadline_set	(	thread3_pointer,600);
// k_thread_deadline_set	(	thread1_pointer,200);
// k_thread_deadline_set	(	thread2_pointer,400);

k_thread_start(thread3_pointer);


k_sleep(K_MSEC(1));
k_thread_start(thread1_pointer);
k_thread_start(thread2_pointer);

k_thread_join(thread1_pointer, K_FOREVER);
k_thread_join(thread2_pointer, K_FOREVER);
k_thread_join(thread3_pointer, K_FOREVER);

//	k_thread_start(task1);

//k_thread_join(task1, K_FOREVER);

	return 0;
}
