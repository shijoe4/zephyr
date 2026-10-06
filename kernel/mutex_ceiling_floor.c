/*
 *  Copyright (c) 2026 Instituto Superior de Engenharia do Porto (ISEP)
 *  SPDX-License-Identifier: Apache-2.0
 */



#ifndef CONFIG_SMP 

#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>
#include <zephyr/toolchain.h>
#include <ksched.h>
#include <kthread.h>
#include <wait_q.h>
#include <errno.h>
#include <zephyr/init.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/tracing/tracing.h>
#include <zephyr/sys/check.h>
#include <zephyr/logging/log.h>
#include <zephyr/llext/symbol.h>
#include <zephyr/sys/dlist.h>

#define MAX_ITEMS 100

bool reshedule_flag = false;
bool reschedule_unlock_flag = false;

int stack_size = 0;
//LOG_LEVEL_NONE
//
LOG_MODULE_DECLARE(os, LOG_LEVEL_DBG);

static struct k_spinlock lock;

/**
 * @brief Initialization guard flag for the global synchronization stack.
 *
 * Ensures that the underlying global stack infrastructure is initialized exactly 
 * once, even when multiple ceiling/floor mutexes are instantiated concurrently 
 * or sequentially across the system.
 */

 


atomic_t stack_init_flag = ATOMIC_INIT(0);
/**
 * @brief Structure for tracking mutex ownership and priority boundaries.
 *
 * This structure encapsulates the operational metadata of a locked mutex under 
 * the Immediate Priority Ceiling Protocol (IPCP). It is designed to be pushed 
 * onto and popped from the global synchronization stack to maintain state 
 * during nested or multi-resource lock acquisitions.
 */

struct ceiling_floor_mutex_owner{

     struct k_thread *owner;      /**pointer to the owner thread tid */
    int ceiling_prio;           /*ceiling priority of the mutex */
    #ifdef CONFIG_SCHED_DEADLINE
    int64_t floor_prio;        /* floor priority of the mutex */
    #endif

   sys_dnode_t node ;  
};

/* Define the capacity of the memory slab */
#define ITEM_COUNT 10

/* Static memory slab to manage stack items safely */
K_MEM_SLAB_DEFINE(stack_slab, sizeof(struct ceiling_floor_mutex_owner), ITEM_COUNT, 4);

/* Stack head (Assumes external global synchronization protects list modifications) */
static sys_dlist_t ceiling_floor_stack = SYS_DLIST_STATIC_INIT(&ceiling_floor_stack);


/**
 * @brief Global wait queue for threads blocked on the priority ceiling/floor mutex.
 *
 * This system-wide queue manages threads that are currently suspended because 
 * their priority does not satisfy the immediate priority ceiling or floor requirements. 
 * Threads are enqueued here to prevent priority inversion and are unpended (awakened) 
 * dynamically as mutexes are released and system resource ceilings drop.
 */

_wait_q_t wait_q_mutex_ceiling_floor;



/**
 * @brief Pushes a mutex owner record onto the global stack.
 *
 * This function is responsible for adding a new mutex ownership record to the 
 * global stack, which tracks the current system ceiling and floor priorities. 
 * It is assumed that external synchronization (e.g., spinlock) protects this call.
 *
 * @param[in] input_item Pointer to the mutex owner data to be pushed.
 * @return true if the push operation was successful, false otherwise.
 */

int ceiling_floor_stack_push(const struct ceiling_floor_mutex_owner *input_item) {
    if (input_item == NULL) {
        return -EINVAL;
    }

    struct ceiling_floor_mutex_owner *item;

    /* Allocate block from memory slab */
    if (k_mem_slab_alloc(&stack_slab, (void **)&item, K_NO_WAIT) != 0) {
        return -ENOMEM; // Slab is full
    }

    /* Copy data fields into the slab-allocated block */
    item->owner = input_item->owner;
    item->ceiling_prio = input_item->ceiling_prio;
#ifdef CONFIG_SCHED_DEADLINE
    item->floor_prio = input_item->floor_prio;
#endif

    /* Prepend to the intrusive list */
    sys_dlist_prepend(&ceiling_floor_stack, &item->node);


    return 0;
}


/**
 * @brief Peeks at the top mutex owner record on the global stack without removing it.
 *
 * This function retrieves the current system ceiling and floor priorities by 
 * inspecting the top item of the global stack. It does not modify the stack state.
 *
 * @param[out] out_item Pointer to a structure where the top item's data will be copied.
 * @return 0 on success, -EINVAL if out_item is NULL, -ENOENT if the stack is empty.
 */

int ceiling_floor_stack_peek(struct ceiling_floor_mutex_owner *out_item) {


    if (out_item == NULL) {
        return -EINVAL;
    }

    /* Fixed: Use sys_dnode_t * instead of struct sys_dnode * */
    sys_dnode_t *head = sys_dlist_peek_head(&ceiling_floor_stack);
    if (!head) {
        return -ENOENT; // Stack is empty
    }

    struct ceiling_floor_mutex_owner *item = CONTAINER_OF(head, struct ceiling_floor_mutex_owner, node);

    /* Copy data out without removing */
    out_item->owner = item->owner;
    out_item->ceiling_prio = item->ceiling_prio;
#ifdef CONFIG_SCHED_DEADLINE
    out_item->floor_prio = item->floor_prio;
#endif

    return 0;
}

/**
 * @brief Pops the top mutex owner record from the global stack and frees its memory.
 *
 * This function removes the top item from the global stack, copies its data to 
 * the provided output structure, and returns the memory block back to the slab. 
 * It is assumed that external synchronization protects this call.
 *
 * @param[out] out_item Pointer to a structure where the popped item's data will be copied.
 *                     Can be NULL if the caller does not need the data.
 * @return 0 on success, -ENOENT if the stack is empty.
 */


int ceiling_floor_stack_pop(struct ceiling_floor_mutex_owner *out_item) {
    /* Fixed: Use sys_dnode_t * instead of struct sys_dnode * */
    sys_dnode_t *node = sys_dlist_get(&ceiling_floor_stack);
    if (!node) {
        return -ENOENT; // Stack is empty
    }

    struct ceiling_floor_mutex_owner *item = CONTAINER_OF(node, struct ceiling_floor_mutex_owner, node);

    /* Copy data out to caller's buffer */
    if (out_item != NULL) {
        out_item->owner = item->owner;
        out_item->ceiling_prio = item->ceiling_prio;
#ifdef CONFIG_SCHED_DEADLINE
        out_item->floor_prio = item->floor_prio;
#endif
    }

    /* Automatically return the memory block back to the slab */
    k_mem_slab_free(&stack_slab, (void *)item);
    

    return 0;
}




/**
 * @brief Initializes a Priority Ceiling/Floor mutex with specified resource limits.
 *
 * Configures the operational bounds of a mutex using the Immediate Priority 
 * Ceiling Protocol (IPCP) with deadline floor. 
 *
 * @param[in,out] mutex            Pointer to the mutex structure to be initialized.
 * @param[in]  resource_ceiling The maximum priority level assigned to this resource.
 * @param[in]  resource_floor   The baseline floor priority level (applicable for deadline scheduling).
 *
 * @return int 0 on successful initialization.
 * @return int -ENOMEM if there is insufficient memory to allocate the mutex resources.
 */

int z_impl_k_mutex_ceiling_floor_init(struct k_mutex_ceiling_floor *mutex, int resource_ceiling, int64_t resource_floor)
{

    
    k_spinlock_key_t key;
    
	key = k_spin_lock(&lock);

    struct ceiling_floor_mutex_owner current_mutex_owner;
     current_mutex_owner.owner = NULL;
     current_mutex_owner.ceiling_prio = INT_MAX; 
    
     #ifdef CONFIG_SCHED_DEADLINE
     current_mutex_owner.floor_prio = INT64_MAX;
    #endif
    int stack_status = atomic_get(&stack_init_flag);
    if(stack_status==0)
    {

   if(ceiling_floor_stack_push(&current_mutex_owner)==0)
   {
      atomic_set(&stack_init_flag, 1);
   }
        else
        {
LOG_ERR("Failed to initialize the stack for ceiling floor mutex");
            k_spin_unlock(&lock, key);
            return -ENOMEM;
     }

    }


  LOG_DBG("initializing mutex %p with ceiling %d and floor %lld\n", mutex, resource_ceiling, resource_floor);  
	mutex->owner = NULL;
	mutex->lock_count = 0U;
	mutex->resource_ceiling = resource_ceiling;
	mutex->resource_floor = resource_floor;

	z_waitq_init(&wait_q_mutex_ceiling_floor);
	k_object_init(mutex);

#ifdef CONFIG_OBJ_CORE_MUTEX_CEILING_FLOOR
	k_obj_core_init_and_link(K_OBJ_CORE(mutex), &obj_type_mutex_ceiling_floor);
#endif /* CONFIG_OBJ_CORE_MUTEX_CEILING_FLOOR */


 
SYS_PORT_TRACING_OBJ_INIT(k_mutex_ceiling_floor, mutex,resource_ceiling, resource_floor,0);
LOG_DBG("initialized mutex %p with ceiling %d and floor %lld\n", mutex, resource_ceiling, resource_floor);
	 k_spin_unlock(&lock, key);
    return 0;
}

/**
 * @brief Implements a  Immediate Priority Ceiling Protocol (IPCP) mutex lock with multi-resource 
 * support and deadline-floor for deadline based scheduling.
 *
 * This function enforces a strict resource ceiling protocol to prevent priority 
 * inversion. When a thread successfully acquires the mutex, its execution priority 
 * is dynamically elevated to the resource's predefined ceiling priority.
 *
 * @param[in,out] mutex   Pointer to the ceiling mutex structure to be locked.
 * @param[in]     timeout The maximum time to wait for acquiring the mutex.
 * @return int            Status code (e.g., success code or error indicating acquisition failure).
 *
 * @note Algorithm Behavior:
 * 1. **Snapshot Current State:** Reads the current mutex owner information from the 
 * system/thread stack into a local variable.
 * 2. **Strictly Higher Priority Case:** If the calling thread's priority is strictly 
 * higher than the current system ceiling priority AND the lock count is 0:
 * - The lock is successfully acquired.
 * - The mutex owner metadata is updated and pushed onto the stack.
 * 3. **Acquisition Failure:** If the conditions for locking are not met, the function 
 * fails immediately or after the timeout, returning the appropriate error code to the caller.
 * 4. **Equal Priority / Multi-Resource Case:** If the calling thread's priority equals 
 * the system ceiling, the owner of the lock is checked:
 * - **Different Resource Acquisition:** If the calling thread is attempting to lock a 
 * *different* resource that shares the exact same ceiling priority, acquisition is allowed. 
 * The owner metadata is updated and pushed to the stack.
 * 5. **Deadline Scheduling Case (EDF Extension):** When priorities are equal under 
 * deadline-based scheduling, a thread is permitted to enter the critical section if:
 * - The new thread has a shorter (closer) deadline AND the lock is currently unowned, OR
 * - The calling thread is already the current owner of the system ceiling.
 */

int z_impl_k_mutex_ceiling_floor_lock(struct k_mutex_ceiling_floor *mutex, k_timeout_t timeout)
{
 
k_spinlock_key_t key;
struct ceiling_floor_mutex_owner current_mutex_owner ;


unpend_success: 


    if(reshedule_flag || reschedule_unlock_flag)
    {
        LOG_DBG("rescheduling after initializing mutex %p\n", mutex);
        k_reschedule();
  reshedule_flag = false;
  reschedule_unlock_flag = false;
      
    }

 key = k_spin_lock(&lock);    

LOG_DBG("entering lock mutex thread %s mutex %p\n", 	k_thread_name_get (k_current_get()), mutex);
    #ifdef CONFIG_SCHED_DEADLINE
LOG_DBG("current prio of thread %d, current deadline of thread : %lld therad name: %s\n", _current->base.prio, _current->base.prio_deadline, k_thread_name_get(k_current_get()));
#else
LOG_DBG("current prio of thread %d thread name: %s\n", _current->base.prio, k_thread_name_get(k_current_get()));
#endif
__ASSERT(!arch_is_in_isr(), "mutexes cannot be used inside ISRs");
    

LOG_DBG("Get the latest mutex owner information from stack\n");
	 SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mutex_ceiling_floor, lock, mutex,timeout);


   if(ceiling_floor_stack_peek(&current_mutex_owner)!=0)
   {
        LOG_ERR("Failed to peek the mutex owner from stack\n");
    k_spin_unlock(&lock, key);
    return -ENOMEM;
   }

     #ifdef CONFIG_SCHED_DEADLINE
        LOG_DBG("current system ceiling %d, current owner of the lock %p, current stack floor %lld\n", current_mutex_owner.ceiling_prio, current_mutex_owner.owner, current_mutex_owner.floor_prio);
     #endif
        LOG_DBG("current system ceiling %d, current owner of the lock %p\n", current_mutex_owner.ceiling_prio, current_mutex_owner.owner);
     LOG_DBG("current tid %p\n", _current);
     LOG_DBG("base prio of the thread %d\n", _current->base.prio);



     if(mutex->resource_ceiling >_current->base.prio)
      {

/**
 * @brief Error Handling: Invalid Priority Ceiling Configuration.
 *
 * Under the Immediate Priority Ceiling Protocol (IPCP), a thread is strictly 
 * prohibited from acquiring a lock if the resource's ceiling priority is 
 * lower than the thread's current priority. 
 *
 * Valid configurations require the resource ceiling to be equal to or higher 
 * than the priority of any thread that can acquire it. Failing this check 
 * indicates a critical misconfiguration, and an error is returned to the caller.
 */
LOG_DBG("resource ceiling =%d\n", mutex->resource_ceiling);
LOG_DBG("error condition for locking\n ");
LOG_DBG("thread %s is trying to acquire mutex %p with ceiling %d and floor %lld but its priority is lower than the ceiling\n", k_thread_name_get(k_current_get()), mutex, mutex->resource_ceiling, mutex->resource_floor);
    SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex_ceiling_floor, lock, mutex, timeout, -EINVAL);
    k_spin_unlock(&lock, key);
    return -EINVAL;
      }



#ifdef CONFIG_SCHED_DEADLINE
    if ((_current->base.prio < current_mutex_owner.ceiling_prio) ||((_current->base.prio == current_mutex_owner.ceiling_prio )&&(_current->base.prio_deadline < current_mutex_owner.floor_prio))\
    ||((_current->base.prio == current_mutex_owner.ceiling_prio )&&(_current->base.prio_deadline == current_mutex_owner.floor_prio)\
    &&(current_mutex_owner.owner==NULL || current_mutex_owner.owner==_current)))

    #else
    if (((_current->base.prio < current_mutex_owner.ceiling_prio) || ((_current->base.prio==current_mutex_owner.ceiling_prio)&&(current_mutex_owner.owner==NULL || current_mutex_owner.owner==_current))) && (mutex->lock_count == 0U))  
    
#endif   
    {    
    /*
      if two threads have same priority then are  allowded inside they are allowed only if 
      they have short deadline 
      if same therad trying to aquire the same lock or different lock
      with same priority then they can aquire the lock 
      and change or keep the system ceiling as the same value 
      */
#ifdef CONFIG_SCHED_DEADLINE
      
       LOG_DBG("base prio %d resource ceiling %d lock %d deadline of task %lld\n", _current->base.prio, mutex->resource_ceiling, mutex->lock_count,_current->base.prio_deadline);
#endif
       LOG_DBG("base prio %d resource ceiling %d lock %d \n", _current->base.prio, mutex->resource_ceiling, mutex->lock_count);


      if(mutex->lock_count == 0U)
      
    //   if(mutex->lock_count == 0U &&((mutex->resource_ceiling <_current->base.prio)||(current_mutex_owner.owner==NULL || current_mutex_owner.owner==_current)))
    {
        LOG_DBG("thread %s is acquiring mutex %p with ceiling %d and floor %lld\n", k_thread_name_get(k_current_get()), mutex, mutex->resource_ceiling, mutex->resource_floor);
        
        //  bool result=false;
        mutex->owner_orig_prio = _current->base.prio;
        #ifdef CONFIG_SCHED_DEADLINE
        mutex->owner_original_floor_prio = _current->base.prio_deadline;
        LOG_DBG("orginal deadline of the mutex value=%lld\n", mutex->owner_original_floor_prio);
        #endif

        current_mutex_owner.owner = _current;
        current_mutex_owner.ceiling_prio = mutex->resource_ceiling;
        #ifdef CONFIG_SCHED_DEADLINE
            LOG_DBG("current cycle count %lld\n", k_cycle_get_64());
        current_mutex_owner.floor_prio =MIN(mutex->resource_floor+k_cycle_get_64(),_current->base.prio_deadline);
        #endif

    if(ceiling_floor_stack_push(&current_mutex_owner)!=0)
   {
                LOG_ERR("Failed to push the mutex owner to stack\n");
            k_spin_unlock(&lock, key);
              return -ENOMEM;
       }

        mutex->owner = _current;
        mutex->lock_count=1U;
        if(mutex->resource_ceiling!=_current->base.prio)
        {

       reshedule_flag = z_thread_prio_set(_current, mutex->resource_ceiling);
        }
       #ifdef CONFIG_SCHED_DEADLINE
       if(current_mutex_owner.floor_prio!=_current->base.prio_deadline)
       {
          k_thread_absolute_deadline_set_64(mutex->owner, current_mutex_owner.floor_prio);
       }
     
       LOG_DBG("deadline of the thread after setting deadline %lld\n", _current->base.prio_deadline);
       reshedule_flag =true;
       #endif
        LOG_DBG("thread %s has acquired mutex %p with ceiling %d and floor %lld\n", k_thread_name_get(k_current_get()), mutex, mutex->resource_ceiling, mutex->resource_floor);
       SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex_ceiling_floor, lock, mutex, timeout, 0);

          k_spin_unlock(&lock, key);       
        //  if(result)
        //  {
        //     LOG_DBG("rescheduling after acquiring mutex %p\n", mutex);
          
        //      k_reschedule();
        //     //  z_reschedule(&lock, key);           
        // }  
          
        return 0;
      }
 
    }

 /**
 * @brief Thread Priority Insufficient: Block and Pend on System Ceiling.
 *
 * The calling thread's priority is lower than the current system ceiling.
 * To prevent priority inversion, the thread must yield execution and block 
 * until the system ceiling drops to an acceptable level.
 *
 */

      LOG_DBG("therad %s is waiting for mutex %p\n", k_thread_name_get(k_current_get()), mutex); 
    SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mutex_ceiling_floor, lock, mutex, timeout);
// int got_mutex = z_pend_curr(&lock, key, &wait_q_mutex_ceiling_floor, timeout);
z_pend_curr(&lock, key, &wait_q_mutex_ceiling_floor, timeout);
// if(got_mutex == 0)
// {
    LOG_DBG("therad %s is free after pending \n", k_thread_name_get(k_current_get()));
    goto unpend_success;
// }

    LOG_DBG("something bad happened when locking mutex %p thread name: %s\n", mutex, k_thread_name_get(k_current_get()));
     SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex_ceiling_floor, lock, mutex, timeout, -EINVAL);
     k_spin_unlock(&lock, key);
     return -EINVAL;
}


/**
 * @brief Unlocks a priority ceiling/floor mutex and restores system state.
 *
 * @param[in,out] mutex Pointer to the mutex to be unlocked.
 * @return 0 on success, -EPERM if the calling thread is not the owner.
 *
 * @note Algorithm:
 * 1. **Verify Owner:** Return -EPERM if the caller does not own the lock.
 * 2. **Restore State:** Pop the tracking stack to restore the thread's 
 * original priority/deadline and the system's ceiling/floor levels.
 * 3. **Reschedule:**
 * rescheduling is needed in two cases:
 * - when you change the deadline of the thread after unlocking the mutex
 * also when there is a thread waiting in the wait queue of the mutex because 
 * when the mutex is released the waiting thread will be unpended and it can run 
 *if it has higher priority or shorter deadline than the current running thread.
 */


int z_impl_k_mutex_ceiling_floor_unlock(struct k_mutex_ceiling_floor *mutex)
{
   
    k_spinlock_key_t key = k_spin_lock(&lock);

    #ifdef CONFIG_SCHED_DEADLINE
   LOG_DBG("deadline value of the mutex %lld\n", mutex->owner_original_floor_prio);
    #endif
    struct ceiling_floor_mutex_owner current_mutex_owner ;

    LOG_DBG("Entering unlock mutex %p thread name: %s\n", mutex, k_thread_name_get(k_current_get()));
SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mutex_ceiling_floor, unlock, mutex);
	struct k_thread *new_owner= NULL;


 if (mutex->owner == NULL) {

    LOG_DBG("unlocking mutex whose owner is null %p thread name: %s\n", mutex, k_thread_name_get(k_current_get()));
        k_spin_unlock(&lock, key);
        return -EINVAL;
    }

 if (mutex->owner ==_current) {
   
        if (mutex->lock_count > 0U) {
             LOG_DBG("unlocking mutex %p, current prio: %d thread name: %s\n", mutex, _current->base.prio, k_thread_name_get(k_current_get()));
             
                #ifdef CONFIG_SCHED_DEADLINE
                  LOG_DBG("unlocking mutex %p, current prio: %d  ,current deadline %lld ,thread name: %s\n", mutex, _current->base.prio,_current->base.prio_deadline, k_thread_name_get(k_current_get()));
                #endif  
            
             mutex->lock_count=0U;
            mutex->owner = NULL;
  
           
        if(ceiling_floor_stack_pop(&current_mutex_owner)==0)
       {
                current_mutex_owner.owner = NULL;
                current_mutex_owner.ceiling_prio = INT_MAX;
                #ifdef CONFIG_SCHED_DEADLINE
                current_mutex_owner.floor_prio = INT64_MAX;
                #endif
             }
             else
             {
                LOG_ERR("Failed to pull the mutex owner from stack\n");
                k_spin_unlock(&lock, key);
                return -ENOMEM;
             }
              
             if(mutex->owner_orig_prio != _current->base.prio)
             {
   reschedule_unlock_flag =  z_thread_prio_set(_current, mutex->owner_orig_prio);
             }
     
          
                #ifdef CONFIG_SCHED_DEADLINE
        if(mutex->owner_original_floor_prio != _current->base.prio_deadline)
        {
         k_thread_absolute_deadline_set_64(_current,mutex->owner_original_floor_prio);

            res=1;
        }
            LOG_DBG("released mutex %p, current prio: %d,  deadline: %lld thread name: %s\n", mutex, _current->base.prio, _current->base.prio_deadline, k_thread_name_get(k_current_get()));
            #else
             LOG_DBG("released mutex %p, current prio: %d thread name: %s\n", mutex, _current->base.prio, k_thread_name_get(k_current_get()));
            #endif
             LOG_DBG("looking for new owner of mutex %p \n", mutex);

             bool reschedule_needed=false;
            new_owner = z_unpend_first_thread_locked(&wait_q_mutex_ceiling_floor);
           if(new_owner != NULL)
           {

                       arch_thread_return_value_set(new_owner, 0);
                		z_sched_ready_locked(new_owner);
                          reschedule_needed=true;
                        reschedule_unlock_flag = true;
                        LOG_DBG("old therad is is returning \n");
           } 
      
        LOG_DBG("there is no one to run \n");


        #ifdef CONFIG_SCHED_DEADLINE
        LOG_DBG("unlocking mutex %p is done, current prio: %d  ,deadline %lld thread name: %s\n", mutex, _current->base.prio, _current->base.prio_deadline, k_thread_name_get(k_current_get()));       
        #else
        LOG_DBG("unlocking mutex %p is done, current prio: %d thread name: %s\n", mutex, _current->base.prio, k_thread_name_get(k_current_get()));
        
        #endif
        
            SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex_ceiling_floor, unlock, mutex, 0);
              k_spin_unlock(&lock, key);
        //     if(reschedule_needed)
        //     {
            
        //      k_reschedule();
        // //  z_reschedule(&lock, key);     
           
        // }
    
            return 0;
           
        }
    }
    else{
        LOG_DBG("unlocking mutex which is owned by another thread %p thread name: %s\n", mutex, k_thread_name_get(k_current_get()));
      
            SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex_ceiling_floor, unlock, mutex,-EINVAL);
         k_spin_unlock(&lock, key);
            return -EACCES;

       }

	
 LOG_DBG("something bad happened when unlocking mutex\n" );

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex_ceiling_floor, unlock, mutex, 0);
    k_spin_unlock(&lock, key);

	return -EINVAL;
}




int z_impl_k_mutex_ceiling_init(struct k_mutex_ceiling_floor *mutex, int resource_ceiling)
{
    return z_impl_k_mutex_ceiling_floor_init(mutex, resource_ceiling,INT64_MAX);

}

int z_impl_k_mutex_ceiling_lock(struct k_mutex_ceiling_floor *mutex, k_timeout_t timeout)
{
    return z_impl_k_mutex_ceiling_floor_lock(mutex, timeout);
}

int z_impl_k_mutex_ceiling_unlock(struct k_mutex_ceiling_floor *mutex)
{
 return z_impl_k_mutex_ceiling_floor_unlock(mutex);

}


#ifdef CONFIG_OBJ_CORE_MUTEX_CEILING_FLOOR
K_OBJ_TYPE_DEFINE(obj_type_mutex_ceiling_floor, k_mutex_ceiling_floor, K_OBJ_TYPE_MUTEX_CEILING_FLOOR_ID, NULL);
#endif /* CONFIG_OBJ_CORE_MUTEX_CEILING_FLOOR */


#endif /* CONFIG_SMP */