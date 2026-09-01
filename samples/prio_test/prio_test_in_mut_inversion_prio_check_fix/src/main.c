


#include <zephyr/kernel.h>
// #include <zephyr/random/random.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

/* Define Priorities: Lower number = Higher Priority */
#define PRIO_H 5
#define PRIO_M 8
#define PRIO_L 12

#define STACK_SIZE 1024

/* Define the mutex with Priority Inheritance enabled by default */
struct k_mutex_ceiling_floor shared_mutex;



/* Allocate stacks for the threads */
K_THREAD_STACK_DEFINE(stack_l, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_m, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_h, STACK_SIZE);

/* Thread data structures */
struct k_thread tid_l;
struct k_thread tid_m;
struct k_thread tid_h;

/* Forward declarations of thread entry points */
extern void thread_low(void *, void *, void *);
extern void thread_medium(void *, void *, void *);
extern void thread_high(void *, void *, void *);



// K_MUTEX_DEFINE(shared_mutex);

/* Thread L: Low Priority */
void thread_low(void *p1, void *p2, void *p3) {
   if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
    printk("[L] Locked mutex. Starting work (500ms)...\n");
    
  k_busy_wait(500000); // 
    
    printk("[L] Releasing mutex.\n");
        k_mutex_ceiling_floor_unlock(&shared_mutex);

 k_busy_wait(5000000);
  printk("[L] Finished work.\n");      
}
}

/* Thread M: Medium Priority */
void thread_medium(void *p1, void *p2, void *p3) {
    // k_msleep(100); // Wait for L to grab the lock
    printk("[M] Waking up. Starting heavy CPU task (2s)...\n");
    
    // Using busy_wait to hog the CPU without yielding
    k_busy_wait(2000000); 
    
    printk("[M] Finished CPU task.\n");
}

/* Thread H: High Priority */
void thread_high(void *p1, void *p2, void *p3) {
    k_msleep(200); // Wake up while L has the lock and M is running
    printk("[H] Waking up. Attempting to lock mutex...\n");
    
    if(k_mutex_ceiling_floor_lock(&shared_mutex, K_FOREVER)==0){
        printk("[H] Success! High Priority thread now has the mutex.\n");
        k_mutex_ceiling_floor_unlock(&shared_mutex);
    }

    printk("[H] Finished work.\n");
}

int main(void) {
    printk("Priority Inversion Demo starting on Zephyr RTOS...\n");
    printk("-----------------------------------------------\n");

    /* Initialize your mutex before threads try to use it */
    // Note: Ensure your Zephyr version/config supports ceiling/floor if using this specific API
    k_mutex_ceiling_floor_init(&shared_mutex, PRIO_H, 0); 

    /* Start Low Priority Thread */
    k_thread_create(&tid_l, stack_l, K_THREAD_STACK_SIZEOF(stack_l),
                    thread_low, NULL, NULL, NULL,
                    PRIO_L, 0, K_NO_WAIT);
    
     k_msleep(5); 

    /* Start Medium Priority Thread */
    k_thread_create(&tid_m, stack_m, K_THREAD_STACK_SIZEOF(stack_m),
                    thread_medium, NULL, NULL, NULL,
                    PRIO_M, 0, K_NO_WAIT);
    
    // k_msleep(50); 

    /* Start High Priority Thread */
    k_thread_create(&tid_h, stack_h, K_THREAD_STACK_SIZEOF(stack_h),
                    thread_high, NULL, NULL, NULL,
                    PRIO_H, 0, K_NO_WAIT);

    /* Background loop */
    while (1) {
        k_msleep(1000);
    }
    return 0;
}