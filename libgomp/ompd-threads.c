/* Copyright (C) The GNU Toolchain Authors.
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

/* This file contains the implementation of functions defined in
   Section 5.5 ThreadHandles. */


#include "ompd-helper.h"

ompd_rc_t
ompd_get_thread_in_parallel (ompd_parallel_handle_t *parallel_handle,
			     int thread_num,
			     ompd_thread_handle_t **thread_handle)
{

  if (parallel_handle == NULL)
    return ompd_rc_stale_handle;
  CHECK (parallel_handle->ah);

  ompd_address_space_context_t *context = parallel_handle->ah->context;
  ompd_rc_t ret;

  ompd_word_t team_size_var = 1;
  if (parallel_handle->th.address)
    gompd_get_team_size (parallel_handle, &team_size_var);

  if (thread_num < 0 || thread_num >= team_size_var)
    return ompd_rc_bad_input;

  ompd_word_t temp_offset;
  ompd_address_t temp_symbol_addr, symbol_addr = {OMPD_SEGMENT_UNSPECIFIED, 0};
  ompd_addr_t temp_addr;

  ACCESS_VALUE (context, NULL, "gompd_access_gomp_thread_pool_threads",
		temp_offset, 1, ret, symbol_addr, temp_symbol_addr, temp_addr);

  symbol_addr.address += thread_num * target_sizes.sizeof_pointer;

  DEREFERENCE (context, NULL, symbol_addr, target_sizes.sizeof_pointer, 1,
	       temp_addr, ret, 1);

  ret = callbacks->alloc_memory (sizeof (ompd_thread_handle_t),
				 (void **) thread_handle);

  CHECK_RET (ret);

  if (symbol_addr.address == 0)
    return ompd_rc_unsupported;

  (*thread_handle)->th = symbol_addr;
  (*thread_handle)->ah = parallel_handle->ah;
  return ret;
}

/* The ompd_get_thread_handle function that maps a native thread to an
   OMPD thread handle.  */

ompd_rc_t
ompd_get_thread_handle (ompd_address_space_handle_t *handle,
			ompd_thread_id_t kind, ompd_size_t sizeof_thread_id,
			const void *thread_id,
			ompd_thread_handle_t **thread_handle)
{
  CHECK (handle);
  if (kind != OMPD_THREAD_ID_PTHREAD)
    return ompd_rc_unsupported;

  ompd_address_space_context_t *context = handle->context;
  ompd_thread_context_t *tcontext;
  ompd_rc_t ret;

  ret = callbacks->get_thread_context_for_thread_id (context, kind,
						     sizeof_thread_id,
						     thread_id, &tcontext);
  CHECK_RET (ret);

  ompd_size_t temp_symbol_size, symbol_size;
  ompd_address_t temp_symbol_addr, symbol_addr = {OMPD_SEGMENT_UNSPECIFIED, 0};

  GET_VALUE (context, NULL, "gompd_sizeof_gomp_thread", symbol_size,
	     temp_symbol_size, target_sizes.sizeof_short, 1, ret,
	     temp_symbol_addr);

  GET_VALUE (context, tcontext, "gomp_tls_data", symbol_addr.address,
	     temp_symbol_addr.address, symbol_size, 1, ret, symbol_addr);

  ret = callbacks->alloc_memory (sizeof (ompd_thread_handle_t),
				 (void **) thread_handle);

  CHECK_RET (ret);

  (*thread_handle)->ah = handle;
  (*thread_handle)->th = symbol_addr;
  (*thread_handle)->thread_context = tcontext;
  return ret;
}


ompd_rc_t
ompd_rel_thread_handle (ompd_thread_handle_t *thread_handle)
{
  if (thread_handle == NULL)
    return ompd_rc_stale_handle;

  ompd_rc_t ret;
  ret = callbacks->free_memory ((void *) thread_handle);
  if (ret != ompd_rc_ok)
  return ret;

  return ompd_rc_ok;
}


/* Return -1, 0 or 1 for thread_handle_1 <, == or > thread_handle_2.  */
ompd_rc_t
ompd_thread_handle_compare (ompd_thread_handle_t *thread_handle_1,
			    ompd_thread_handle_t *thread_handle_2,
			    int	*cmp_value)
{

  if (thread_handle_1 == NULL || thread_handle_2 == NULL)
    return ompd_rc_stale_handle;
  if (cmp_value == NULL)
    return ompd_rc_bad_input;
  if (thread_handle_1->ah->kind != thread_handle_2->ah->kind)
    return ompd_rc_bad_input;

  if (thread_handle_1->th.address < thread_handle_2->th.address)
    *cmp_value = -1;
  else if (thread_handle_1->th.address > thread_handle_2->th.address)
    *cmp_value = 1;
  else
    *cmp_value = 0;

  return ompd_rc_ok;
}


ompd_rc_t
ompd_get_thread_id (ompd_thread_handle_t *thread_handle, ompd_thread_id_t kind,
		    ompd_size_t sizeof_thread_id, void *thread_id)
{
  if (kind != OMPD_THREAD_ID_PTHREAD)
    return ompd_rc_unsupported;
  if (thread_id == NULL)
    return ompd_rc_bad_input;
  if (thread_handle == NULL)
    return ompd_rc_stale_handle;

  CHECK (thread_handle->ah);
  ompd_address_space_context_t *context = thread_handle->ah->context;

  ompd_rc_t ret;
  ompd_address_t taddr = thread_handle->th;
  ompd_address_t temp_symbol_addr, symbol_addr = {OMPD_SEGMENT_UNSPECIFIED, 0};
  ompd_size_t temp_symbol_size, symbol_size;
  ompd_word_t temp_offset, offset;

  GET_VALUE (context, NULL, "gompd_sizeof_gomp_thread_handle", symbol_size,
	     temp_symbol_size, target_sizes.sizeof_short, 1, ret, symbol_addr);

  if (symbol_size == 0)
    {
      GET_VALUE (context, NULL, "gompd_thread_initial_tls_bias", offset,
		 temp_offset, target_sizes.sizeof_long, 1, ret, symbol_addr);

      ret = callbacks->symbol_addr_lookup (context, NULL,"gomp_tls_data",
					   &symbol_addr, NULL);
      ret = callbacks->device_to_host (context, &temp_symbol_addr.address,
				       target_sizes.sizeof_long_long, 1,
				       &symbol_addr.address);
      CHECK_RET (ret);

      taddr.address = symbol_addr.address + offset;
      ret = callbacks->read_memory (context, NULL, &taddr,
				    target_sizes.sizeof_long_long, thread_id);
    }
  else
    {
      if (sizeof_thread_id != symbol_size)
        return ompd_rc_bad_input;

      GET_VALUE (context, NULL, "gompd_access_gomp_thread_handle", offset,
		 temp_offset, target_sizes.sizeof_short, 1, ret, symbol_addr);
      taddr.address += offset;

      ret = callbacks->read_memory (context, NULL, &taddr, symbol_size,
				    thread_id);
    }
  return ret;
}


/* OMPD doesn't support GPUs for now.  */
ompd_rc_t ompd_get_device_from_thread (ompd_thread_handle_t *thread_handle,
				       ompd_address_space_handle_t **device)
{
  if (thread_handle == NULL)
    return ompd_rc_stale_handle;
  return ompd_rc_unsupported;
}
