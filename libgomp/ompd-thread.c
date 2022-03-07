/* Copyright (C) 2021 Free Software Foundation, Inc.
   Contributed by Ahmed Sayed <ahmedsayedmousse@gmail.com>.
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



// This file contains the implementation of functions defined in
/* ---------------------Section 5.5 ThreadHandles--------------------- */
#include "ompd-helper.h"
#include "pthread.h"

ompd_rc_t
ompd_get_thread_in_parallel(ompd_parallel_handle_t * parallel_handle,
	int thread_num, ompd_thread_handle_t ** thread_handle)
{

// I made an assumption that the th of a parallel region refears to it's thread pool but it could be one of two possibilites 1- the thread pool, 2- the master thread.
// if its the master thread then i first need to get the thread pool then get the thread with thread num.
// TODO: check which one of the above assumptions is true and fix the function.
	if (!parallel_handle)
		return ompd_rc_stale_handle;
	if (!parallel_handle->ah)
		return ompd_rc_stale_handle;

	ompd_address_space_context_t *context = parallel_handle->ah->context;
	ompd_rc_t ret;

	if (!context)
		return ompd_rc_stale_handle;

	if (!callbacks) {
		return ompd_rc_callback_error;
	}

	ompd_word_t team_size_var;
	ret = ompd_get_icv_from_scope ((void *)parallel_handle, ompd_scope_parallel,
		ompd_icv_team_size_var, &team_size_var);
	if (ret != ompd_rc_ok)
		return ret;
	if (thread_num < 0 || thread_num >= team_size_var)
		return ompd_rc_bad_input;

	//get the address of gomp_thread **threads;
	ompd_size_t tmpOffset, fieldOffset;
	ompd_address_t tmpAddr;
	ompd_address_t taddr = {OMPD_SEGMENT_UNSPECIFIED,0};

	const char * symbolName = "ompd_access_gomp_thread_pool_threads";
	ret = callbacks->symbol_addr_lookup(context, NULL, symbolName, &taddr, NULL);
	ret = callbacks->read_memory(context,NULL,&taddr,
															target_sizes.sizeof_long_long, &(tmpOffset));
	if (ret != ompd_rc_ok)
		return ret;
  ret = callbacks->device_to_host(context,&(tmpOffset),
																target_sizes.sizeof_long_long,1, &fieldOffset);
	if (ret != ompd_rc_ok) {
		return ret;
	}
	taddr.address += fieldOffset; // now it points to **threads.

	// derefrencing
	ret = callbacks->read_memory(context, NULL, &taddr,
															target_sizes.sizeof_pointer,&(tmpAddr.address));
	ret = callbacks->device_to_host(context, &(tmpAddr.address),
																	target_sizes.sizeof_pointer,1,
																	&(taddr.address));

	if (ret != ompd_rc_ok) {
		return ret;
	}
	// pointer to the (thread_num)th thread.
	taddr.address += thread_num*target_sizes.sizeof_pointer;

	ret = callbacks->read_memory(context, NULL, &taddr,
															target_sizes.sizeof_pointer,&(tmpAddr.address));
	ret = callbacks->device_to_host(context, &(tmpAddr.address),
																	target_sizes.sizeof_pointer,1,
																	&(taddr.address));
	if (ret != ompd_rc_ok) {
		return ret;
	}
	//TODO: check whether is should stop here or not <create an if check>.
	//accessing the thread to get the pthread_t handle field still not sure if he wants the address of the gomp_thread struct or the handle field inside it;
	/*symbolName = "ompd_access_gomp_thread_handle"
	ret = callbacks->symbol_addr_lookup(context, NULL, symbolName,
																							&taddr, NULL);
	ret = callbacks->read_memory(context, NULL,&taddr,
															target_sizes.sizeof_long_long, &tmpOffset);
	if (ret != ompd_rc_ok)
		return ret;
	ret = callbacks>device_to_host(context,&(tmpOffset),
																target_sizes.sizeof_long_long,1, &fieldOffset);
	if (ret != ompd_rc_ok) {
		return ret;
	}
	// getting the target_handle for the (thread_num)th thread .
	taddr.address += fieldOffset;
	*/
	ret = callbacks->alloc_memory(sizeof(ompd_thread_handle_t),
																 (void **)(thread_handle));
	if (ret != ompd_rc_ok)
		return ret;

	(*thread_handle)->th = &taddr;
	(*thread_handle)->ah = parallel_handle->ah;
	return ret;
}
// The ompd_get_thread_handle function maps a native thread to an
//		OMPD thread handle.
ompd_rc_t
ompd_get_thread_handle(ompd_address_space_handle_t * handle,
	ompd_thread_id_t kind, ompd_size_t sizeof_thread_id,
	const void * thread_id, ompd_thread_handle_t ** thread_handle)
{
	if(!handle)
		return ompd_rc_stale_handle;

	ompd_address_space_context_t *context = handle->context;
	ompd_rc_t ret;

	if (!context)
    return ompd_rc_stale_handle;

  if (!callbacks) {
    return ompd_rc_callback_error;
  }

	ompd_thread_context_t *tcontext;
	ret =	callbacks->get_thread_context_for_thread_id(context, kind,
		sizeof_thread_id, thread_id, &tcontext);
	if(ret != ompd_rc_ok)
		return ret;


	//now at each thread start a gomp_thread struct is created and it's fields are assigned so now i want to reach that struct
	// the sruct is created in two ways 1- directrly gomp_tls_data
	// 2- by creating a local struct gomp_thread thr and then using
	// pthread_set_specific(gomp_tls_key,	thr)
	// should mimic the runtime call to gomp_thread() which returns a pointer to
	// a gomp_thread struct.
	// struct gomp_thread *thr = gomp_thread ();
	// returns &gomp_tls_data or
	// pthread_getspecific(gomp_tls_key);

	ompd_address_t tmpAddr,symbolAddr = {OMPD_SEGMENT_UNSPECIFIED,0};
	#if defined HAVE_TLS || defined USE_EMUTLS
		ret = callbacks->symbol_addr_lookup(context, tcontext,
																				"gomp_tls_data", &symbolAddr, NULL);
		ret = callbacks->read_memory(context, tcontext, &symbolAddr,
																target_sizes.sizeof_long_long, &(tmpAddr.address));
		ret = callbacks->device_to_host(context, &(tmpAddr.address),
																		target_sizes.sizeof_long_long, 1, &(symbolAddr.address));
	#else
		ret = callbacks->symbol_addr_lookup(context, tcontext,
																				"gomp_tls_key", &symbolAddr, NULL);
		ret = callbacks->read_memory(context, tcontext, &symbolAddr,
																target_sizes.sizeof_long_long, &(tmpAddr.address));
		ret = callbacks->device_to_host(context, &(tmpAddr.address),
																		target_sizes.sizeof_long_long, 1, &(symbolAddr.address));
		//TODO:
		//pthread_key_t *key = symbolAddr.address;
		symbolAddr.address = (ompd_addr_t)pthread_getspecific (*((pthread_key_t *) symbolAddr.address));

	#endif

  if (ret != ompd_rc_ok)
    return ret;


	ret = callbacks->alloc_memory(sizeof(ompd_thread_handle_t),
                                 (void **)(thread_handle));

	if(ret != ompd_rc_ok)
		return ret;

	(*thread_handle)->ah = handle;
	(*thread_handle)->th = &symbolAddr;
	(*thread_handle)->thread_context = tcontext;
	return ret;
};



// The ompd_rel_thread_handle function releases a thread handle.
ompd_rc_t
ompd_rel_thread_handle(ompd_thread_handle_t * thread_handle)
{
	if(!thread_handle)
		return ompd_rc_stale_handle;
	ompd_rc_t ret;
	ret = callbacks->free_memory((void *)thread_handle);
	if(ret != ompd_rc_ok)
		return ret;
	return ompd_rc_ok;
}


// eturn -1, 0 or 1 for thread_handle_1 <, == or > thread_handle_2.
ompd_rc_t
ompd_thread_handle_compare(ompd_thread_handle_t * thread_handle_1,
	ompd_thread_handle_t * thread_handle_2,	int * cmp_value )
{

	if (!thread_handle_1)
    return ompd_rc_stale_handle;
  if (!thread_handle_2)
    return ompd_rc_stale_handle;
  if (!cmp_value)
    return ompd_rc_bad_input;
  if (thread_handle_1->ah->kind != thread_handle_2->ah->kind)
    return ompd_rc_bad_input;

  *cmp_value = thread_handle_1->th->address - thread_handle_2->th->address;

	return ompd_rc_ok;
}

// The ompd_get_thread_id function maps an OMPD thread handle to a native thread.

/*
	TODO: ompd_rc_bad_input if a different value in sizeof_thread_id is expected for a thread kind of kind.
	TODO: ompd_rc_unsupported if the kind of thread is not supported.

*/
ompd_rc_t
ompd_get_thread_id(ompd_thread_handle_t * thread_handle, ompd_thread_id_t kind,
	ompd_size_t sizeof_thread_id, void * thread_id)
{
	// if (kind != OMPD_THREAD_ID_PTHREAD)
	// 	return ompd_rc_unsupported;
	// if (!thread_id)
	// 	return ompd_rc_bad_input;
	// if (!thread_handle)
	// 	return ompd_rc_stale_handle;
	// if (!thread_handle->ah)
	// 	return ompd_rc_stale_handle;
	// ompd_address_space_context_t *context = thread_handle->ah->context;
	// if (!context)
	// 	return ompd_rc_stale_handle;
	// ompd_rc_t ret;
	//
	// ompd_address_t symbolAddr;
	// ompd_size_t tmpSize,size;
	//
	// const char * symbolName = "ompd_sizeof_gomp_thread"
	// ret = callbacks->symbol_addr_lookup(context, NULL, symbolName,
	// 																			&symbolAddr, NULL);
	// ret = callbacks->read_memory(context, NULL, &symbolAddr,
	// 														1 * target_sizes.sizeof_long_long, &(tmpSize));
	// ret = callbacks->device_to_host(context,
	// 																&tmpSize, target_sizes.sizeof_long_long, 1,
	// 																&(size));
	// if (ret != ompd_rc_ok)
	// 	return ret;
	// if (sizeof_thread_id != size)
	// 	return ompd_rc_bad_input;
	//
	// return ret;
	return ompd_rc_unsupported;

}
