/* Copyright (C) 2021 Free Software Foundation, Inc.
   Contributed by youssef magdy <youssefmagdyy775@gmail.com>.
   This file is part of the GNU Offloading and Multi Processing Library
   (libgomp).
   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.
   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.
   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.
   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */



/* This file contains the implementation of functions defined in
	section 5.5.1  */

#include "ompd-helper.h"


ompd_rc_t
ompd_get_curr_task_handle( ompd_thread_handle_t * thread_handle,
   ompd_task_handle_t ** task_handle)
{
   if(thread_handle == NULL )
      return ompd_rc_bad_input ;

   ompd_address_space_context_t context = thread_handle->ah.context  ; 
   ompd_thread_context_t tcontext = thread_handle->thread_context ;   
   if(context == NULL )
      return ompd_rc_bad_input ;
   
   // gomp_thread_pool[t]->task 
   //  how to get the size 
   ompd_addr_t task_address ; 
   omd_rc_t ret = ompd_rc_ok ; 

   GET_VALUE(context, tcontext, "task", task_address, task_address,
		target_sizes.sizeof_long_long, 1, ret);
   
   ret = callbacks->alloc_memory(sizeof(ompd_task_handle_t),
			(void **)(task_handle));

   if(ret != ompd_rc_ok)
		return ret;

	if(handle == NULL)
		return ompd_rc_error;

   (*task_handle)->ah = thread_handle->ah ;
   (*task_handle)->th = task_address ;

   return ret;

}

ompd_rc_t
ompd_get_generating_task_handle( ompd_task_handle_t *task_handle,
   ompd_task_handle_t ** generating_task_handle)
{
   if (!task_handle)
      return ompd_rc_stale_handle;
  if (!task_handle->ah)
      return ompd_rc_stale_handle;
   
  ompd_address_space_context_t *context = task_handle->ah->context;
  
  if (!context)
      return ompd_rc_stale_handle;
  if (!callbacks) 
      return ompd_rc_callback_error;

   ompd_address_t *target_addr = task_handle->th ;
   ompd_address_t *temp_parent_address ;
   ompd_address_t *parent_address ;

   ompd_rc_t ret =  ompd_rc_stale_handle;
// required: accessing the task that the *task_handle points to, to get the parent task
// locate the address of that parent
// make the *generating_task_handle points to that location.

   //get the offset (address) of the parent task (gomp_task->parent) and store it in parent_address
   ret = callbacks->symbol_addr_lookup(context, NULL, "parent", target_addr, NULL);
   ret = callbacks->read_memory(context,NULL,target_addr,target_sizes.sizeof_long_long,temp_parent_address);
	ret = callbacks->device_to_host(context, temp_parent_address, target_sizes.sizeof_long_long,1, parent_address);

   ompd_rc_t ret = callbacks->alloc_memory(sizeof(ompd_task_handle_t),
                                 (void **)(generating_task_handle));

   if (ret != ompd_rc_ok)
      return ret;

   (*generating_task_handle)->th = parent_address;
   (*generating_task_handle)->ah = task_handle->ah;
   return ret;
}
/*
   input : *parallel_handle , thread_num (selects the required implicit task)
   output: make *task_handle pointer points to where the required implicit task will be located
   required : access gomp_team.implicit_tasks[thread_num]
*/

ompd_rc_t
ompd_get_task_in_parallel(
    ompd_parallel_handle_t *parallel_handle, 
    int thread_num, 
    ompd_task_handle_t **task_handle 
) {

   if (!parallel_handle)
      return ompd_rc_stale_handle;
   
   if (!parallel_handle->ah)
      return ompd_rc_stale_handle;

   ompd_address_space_context_t *context = parallel_handle->ah->context;

   ompd_address_t *target_addr = parallel_handle->th ; 
   if (!context)
      return ompd_rc_stale_handle;

  if (!callbacks) 
    return ompd_rc_callback_error;
  
   ompd_rc_t ret ;
   ompd_word_t team_threads ; //stores the number of threads of the input parallel handle
   
   ret = ompd_get_num_threads(parallel_handle, &team_threads);
   if(ret != ompd_rc_ok){
      return ret ;
   }
   //check if the input thread_num is in the range of 0 to team_threads
   if(thread_num <= 0 || thread_num > team_threads){
      return ompd_rc_bad_input ;
   }

   ompd_address_t *implicit_offset ; // stores the offset to get the location of gomp_team.implicit_task
   ompd_address_t *implicit_task_address ; //stores the output reading of device to host operation
   
   //looking up for implicit_task array in gomp_team
   ret = callbacks->symbol_addr_lookup(context, NULL, "implicit_task", target_addr, NULL);
   ret = callbacks->read_memory(context, NULL, target_addr, target_sizes.sizeof_long_long, implicit_offset);
   ret = callbacks->device_to_host(context, NULL, target_sizes.sizeof_long_long,1, implicit_task_address);

   if(ret != ompd_rc_ok){
      return ret;
   }

   // implicit_task_address is the address of the first element of the array 
   target_addr->address = implicit_task_address + thread_num* target_sizes.sizeof_pointer ;//address of implicit_tasks[thread_num]

   ret = callbacks->alloc_memory(sizeof(ompd_task_handle_t),
                                 (void **)(task_handle));

   if (ret != ompd_rc_ok){
      return ret;
   }

   (*task_handle)->th = target_addr;
   (*task_handle)->ah = parallel_handle->ah;
 
}

ompd_rc_t
ompd_rel_task_handle(
   ompd_task_handle_t *task_handle
){
   if (!task_handle)
      return ompd_rc_stale_handle;

   ompd_rc_t ret = callbacks->free_memory((void *)(task_handle));

   if (ret != ompd_rc_ok)
      return ret;
  return ompd_rc_ok;
}