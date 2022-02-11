/* Copyright (C) 2021 Free Software Foundation, Inc.
   Contributed by Mohamed Atef <mohamedatef1698@gmail.com>.
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



/*This file contains the helper functions for OMPD.  */

#ifndef _OMPD_HELPER_H
#define _OMPD_HELPER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "omp-tools.h"
#include "ompd-types.h"
#include <stdio.h>
#include <stddef.h>

#define stringize(x)  stringize1(x)
#define stringize1(x) #x

#define VERSION 202011

extern const ompd_callbacks_t *callbacks;
extern __UINT64_TYPE__ ompd_state;
extern ompd_device_type_sizes_t target_sizes;


ompd_rc_t get_sizes(ompd_address_space_context_t *);



struct _ompd_aspace_handle
{
	ompd_address_space_context_t *context;
	ompd_device_t kind;
};




#ifdef __cplusplus
} // extern C
#endif

#endif /* OMPD_HELPER_H */
