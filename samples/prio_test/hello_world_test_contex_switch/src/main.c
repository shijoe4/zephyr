/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdint.h>
#include <zephyr/kernel.h>
// #include <zephyr/random/random.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>


 uint32_t mutex_valuee=0;






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


// printf("current deadline t1: %d\n", _current->base.prio_deadline);
//printf("Thread 1 is running\n");
   k_busy_wait(20000);
  //  k_sleep(K_MSEC(2000));


}
}

void thread_function2(void *ignored, void *iter, void *interv)
{

while (true) {

  //  k_thread_deadline_set(_current,0);
     k_thread_absolute_deadline_set(_current,INT_MAX-1000);

 printf("current deadline t2: %d\n", _current->base.prio_deadline);

//  printf("Thread 2 is running\n");
    // k_busy_wait(2000);

//  k_sleep(K_MSEC(2000));
  k_thread_absolute_deadline_set(_current,INT_MAX);
   printf("current deadline t2: %d\n", _current->base.prio_deadline);
}
}


void thread_function3(void *ignored, void *iter, void *interv)
{

while (true) {

// printf("current deadline t3: %d\n", _current->base.prio_deadline);

//  printf("Thread 3 is running\n");
    k_busy_wait(20000);

// k_sleep(K_MSEC(2000));
}
}



int main(void)
{



k_sleep(K_SECONDS(1));
// printf("Starting threads\n");

  k_tid_t thread1_pointer, thread2_pointer, thread3_pointer;
thread1_pointer=k_thread_create(&therad1_data, stack1, 1024,
                thread_function1,
                NULL, NULL, NULL,
                100, 0, K_FOREVER);


thread2_pointer=k_thread_create(&therad2_data, stack2, 1024,
                thread_function2,
                NULL, NULL, NULL,
                100, 0, K_FOREVER);      

thread3_pointer=k_thread_create(&therad3_data, stack3, 1024,
                thread_function3,
                NULL, NULL, NULL,
                100, 0, K_FOREVER);
k_thread_name_set(thread1_pointer,"therad_1");  
k_thread_name_set(thread2_pointer,"therad_2");  
k_thread_name_set(thread3_pointer,"therad_3");  
// k_sleep(K_MSEC(10));
// k_thread_start(thread1_pointer);
// k_sleep(K_MSEC(10));
k_thread_start(thread2_pointer);
//  k_thread_start(thread3_pointer);

// while(1)
// {
// k_sleep(K_MSEC(100));
// }



// printf("Main thread is exiting\n");
k_thread_join(thread1_pointer, K_FOREVER);
k_thread_join(thread2_pointer, K_FOREVER);
k_thread_join(thread3_pointer, K_FOREVER);



	return 0;
}
