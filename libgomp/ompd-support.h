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



/*This file contains the runtime support for OMPD.  */

#ifndef _OMPD_SUPPORT_H
#define _OMPD_SUPPORT_H

#include "omp-tools.h"
#include "plugin-suffix.h"
#include "libgomp.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

void ompd_load();
extern __UINT64_TYPE__ ompd_state;

#define OMPD_ENABLED 0x1

#define OMPD_FOREACH_ACCESS(ompd_access) \
   ompd_access(gomp_task_icv, nthreads_var) \
   ompd_access(gomp_task_icv, run_sched_var) \
   ompd_access(gomp_task_icv, run_sched_chunk_size) \
   ompd_access(gomp_task_icv, default_device_var) \
   ompd_access(gomp_task_icv, thread_limit_var) \
   ompd_access(gomp_task_icv, dyn_var) \
   ompd_access(gomp_task_icv, bind_var) \
   ompd_access(gomp_thread, task)


#define OMPD_SIZES(ompd_size) \
   ompd_size(gomp_thread) \
   ompd_size(gomp_task_icv) \
   ompd_size(gomp_task) 


#endif /* _OMPD_SUPPORT_H */
