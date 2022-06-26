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
ompd_get_curr_task_handle( ompd_thread_handle_t *thread_handle,
   ompd_task_handle_t ** task_handle)
{
   if(thread_handle == NULL )
      return ompd_rc_bad_input ;

   CHECK(thread_handle->ah) ; 

   ompd_address_t symbol_addr = thread_handle->th;
   ompd_word_t temp_offset;
   ompd_address_t temp_sym_addr;
   ompd_addr_t temp_addr;
   ompd_address_space_context_t *context = thread_handle->ah->context;
   ompd_thread_context_t *t_context = thread_handle->thread_context;
   ompd_rc_t ret;
   ACCESS_VALUE (context, t_context, "gompd_access_gomp_thread_task",
    temp_offset, 1, ret, symbol_addr, temp_sym_addr, temp_addr);


   // allocate the handle 
   ret = callbacks->alloc_memory(sizeof(ompd_task_handle_t),
			(void **)(task_handle));
   CHECK_RET(ret) ; 

   (*task_handle)->ah = thread_handle->ah ;
   (*task_handle)->th = symbol_addr ;

   return ret;

}


ompd_rc_t
ompd_get_generating_task_handle( ompd_task_handle_t *task_handle,
   ompd_task_handle_t ** generating_task_handle)
{
   if(task_handle == NULL )
      return ompd_rc_bad_input ;

   CHECK(task_handle->ah) ; 

   ompd_address_t symbol_addr = task_handle->th;
   ompd_word_t temp_offset;
   ompd_address_t temp_sym_addr;
   ompd_addr_t temp_addr;
   ompd_address_space_context_t *context = task_handle->ah->context;
   ompd_thread_context_t *t_context = NULL;
   ompd_rc_t ret;

   ACCESS_VALUE (context, t_context, "gompd_access_gomp_task_parent", 
    temp_offset, 1, ret, symbol_addr, temp_sym_addr, temp_addr);


   // allocate the handle 
   ret = callbacks->alloc_memory(sizeof(ompd_task_handle_t),
			(void **)(generating_task_handle));
   CHECK_RET(ret) ; 

   (*generating_task_handle)->th = symbol_addr;
   (*generating_task_handle)->ah = task_handle->ah;

   return ret;
}


// TODO: should check for the thread_num <= threads in paralell region

ompd_rc_t
ompd_get_task_in_parallel(
    ompd_parallel_handle_t *parallel_handle, 
    int thread_num, 
    ompd_task_handle_t **task_handle 
) {


   if(task_handle == NULL )
      return ompd_rc_bad_input ;

   CHECK(parallel_handle->ah) ; 

  
   ompd_address_t task_addr = parallel_handle->th;
   ompd_word_t temp_offset;
   ompd_address_t temp_sym_addr;
   ompd_addr_t temp_addr;
   ompd_address_space_context_t *context = parallel_handle->ah->context;
   ompd_thread_context_t *t_context = NULL;
   ompd_rc_t ret; 
   ompd_word_t nthreads; 
   ompd_word_t task_size = 0;
   ompd_address_t symbol_addr = {OMPD_SEGMENT_UNSPECIFIED, 0};

   ret = gompd_get_team_size(parallel_handle , &nthreads );
   CHECK_RET(ret) ; 
   if (thread_num >= nthreads)
      return ompd_rc_incompatible ;  

   ACCESS_VALUE (context, t_context, "gompd_access_gomp_team_implicit_task", 
    temp_offset, 1, ret, task_addr, temp_sym_addr, temp_addr);

   GET_VALUE (context, t_context, "gompd_sizeof_gomp_task", task_size, 
      task_size,  target_sizes.sizeof_short, 1, ret, symbol_addr);

   task_addr.address += (thread_num * task_size) ;    

   // allocate the handle 
   ret = callbacks->alloc_memory(sizeof(ompd_task_handle_t),
			(void **)(task_handle));
   CHECK_RET(ret) ; 

   (*task_handle)->th = task_addr;
   (*task_handle)->ah = parallel_handle->ah;


   return ret; 

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

ompd_rc_t
ompd_task_handle_compare(
   ompd_task_handle_t *task_handle_1,
   ompd_task_handle_t *task_handle_2,
   int *cmp_value)
{
   if (!task_handle_1)
      return ompd_rc_stale_handle;
   if (!task_handle_2)
      return ompd_rc_stale_handle;
   if (!cmp_value)
      return ompd_rc_bad_input;

   //get the start addresses of both handles and compare them together
   *cmp_value = task_handle_1->th.address - task_handle_2->th.address ;

   return ompd_rc_ok;
}



ompd_rc_t
ompd_get_task_function ( ompd_task_handle_t * task_handle,
			 ompd_address_t * entry_point)
{
	if(!task_handle || !task_handle->ah )
		return ompd_rc_bad_input ;

   CHECK(task_handle->ah) ; 

   ompd_address_t symbol_addr = task_handle->th;
   ompd_word_t temp_offset;
   ompd_address_t temp_sym_addr;
   ompd_addr_t temp_addr;
   ompd_address_space_context_t *context = task_handle->ah->context;
   ompd_rc_t ret;
   ACCESS_VALUE (context, NULL, "gompd_access_gomp_task_fn",
    temp_offset, 1, ret, symbol_addr, temp_sym_addr, temp_addr);

   ret = callbacks->alloc_memory(sizeof(ompd_address_t),
			(void **)(entry_point));
   CHECK_RET(ret) ; 
   entry_point->address = symbol_addr.address;  
    

	return ompd_rc_ok ;
}

// ompd_rc_t 
// ompd_get_scheduling_task_handle( ompd_task_handle_t *task_handle, 
//             ompd_task_handle_t **scheduling_task_handle )
// {

// }
  

ompd_rc_t
ompd_get_task_frame (  ompd_task_handle_t *task_handle, 
         ompd_frame_info_t *exit_frame, 
         ompd_frame_info_t *enter_frame)
{
   if(task_handle == NULL )
      return ompd_rc_bad_input ;

   CHECK(task_handle->ah) ; 


   ompd_address_t exit_addr = task_handle->th;
   ompd_address_t enter_addr = task_handle->th;
   ompd_word_t temp_offset;
   ompd_address_t temp_sym_addr;
   ompd_addr_t temp_addr;
   ompd_address_space_context_t *context = task_handle->ah->context;
   ompd_rc_t ret;

   ACCESS_VALUE (context, NULL, "gompd_access_gomp_task_exit_frame",
    temp_offset, 1, ret, exit_addr, temp_sym_addr, temp_addr);

   ACCESS_VALUE (context, NULL, "gompd_access_gomp_task_enter_frame",
    temp_offset, 1, ret, enter_addr, temp_sym_addr, temp_addr);

   ret = callbacks->alloc_memory(sizeof(ompd_frame_info_t),
			(void **)(exit_frame));
   CHECK_RET(ret) ; 

   ret = callbacks->alloc_memory(sizeof(ompd_frame_info_t),
			(void **)(enter_frame));
   CHECK_RET(ret); 

   exit_frame->frame_address = exit_addr ;
   exit_frame->frame_flag =  ompt_frame_runtime | ompt_frame_framepointer ; 
   enter_frame->frame_address = enter_addr ;  
   enter_frame->frame_flag =  ompt_frame_runtime | ompt_frame_framepointer ; 

   return ompd_rc_ok ;  

}
