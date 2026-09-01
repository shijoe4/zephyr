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

LOG_MODULE_REGISTER(my_app, 4);

#define LOCK_ITER 1


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

int count=0;
while (count<LOCK_ITER) {



LOG_DBG("t1:starting lock");
    k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);
LOG_DBG("t1:lock acquired");    

    mutex_valuee++;

   k_busy_wait(2000);
LOG_DBG("t1:starting unlock");   
    k_mutex_ceiling_floor_unlock(&my_mutex);
LOG_DBG("t1:unlock done");

    count++;
}
}


void thread_function2(void *ignored, void *iter, void *interv)
{
while (true) {


  k_busy_wait(20000);



  
}
}

void thread_function3(void *ignored, void *iter, void *interv)
{
int count=0;
while (count<LOCK_ITER) {

LOG_DBG("t3:starting lock");
    k_mutex_ceiling_floor_lock(&my_mutex, K_FOREVER);
LOG_DBG("t3:lock acquired");


    mutex_valuee++;

    k_busy_wait(20000);
LOG_DBG("t3:starting unlock");
    k_mutex_ceiling_floor_unlock(&my_mutex);
LOG_DBG("t3:unlock done");

    count++;
}
}




int main(void)
{
	//k_sleep(K_SECONDS(2));
	k_mutex_ceiling_floor_init(&my_mutex, 5, 10);
//k_mutex_init(&my_mutex2);
	k_sleep(K_SECONDS(2));

  // printf("Starting threads\n");

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


k_thread_start(thread3_pointer);


k_sleep(K_MSEC(1));
k_thread_start(thread1_pointer);
k_thread_start(thread2_pointer);

k_thread_join(thread1_pointer, K_FOREVER);
k_thread_join(thread2_pointer, K_FOREVER);
k_thread_join(thread3_pointer, K_FOREVER);



	return 0;
}
