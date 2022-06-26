/* Copyright (C) 2022 Free Software Foundation, Inc.
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

/*This file contains the runtime support for gompd.  */

#ifndef _OMPD_SUPPORT_H
#define _OMPD_SUPPORT_H

#include "omp-tools.h"
#include "plugin-suffix.h"
#include "libgomp.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#ifdef HAVE_ATTRIBUTE_VISIBILITY
#pragma GCC visibility push(hidden)
#endif

void gompd_load (void);
extern unsigned short gompd_state;

#define OMPD_ENABLED 0x1

#ifdef GOMP_NEEDS_THREAD_HANDLE
#define gompd_thread_handle_access gompd_access (gomp_thread, handle)
#else
#define gompd_thread_handle_access
#endif

#define GOMPD_FOREACH_ACCESS(gompd_access) \
  gompd_access (gomp_task_icv, nthreads_var) \
  gompd_access (gomp_task_icv, run_sched_var) \
  gompd_access (gomp_task_icv, run_sched_chunk_size) \
  gompd_access (gomp_task_icv, default_device_var) \
  gompd_access (gomp_task_icv, thread_limit_var) \
  gompd_access (gomp_task_icv, dyn_var) \
  gompd_access (gomp_task_icv, bind_var) \
  gompd_access (gomp_thread, task) \
  gompd_access (gomp_thread_pool, threads) \
  gompd_access (gomp_thread, ts) \
  gompd_access (gomp_team_state, team_id) \
  gompd_access (gomp_task, parent) \
  gompd_access (gomp_task, fn) \
  gompd_access (gomp_task, icv) \
  gompd_access (gomp_task, enter_frame) \
  gompd_access (gomp_task, exit_frame) \
  gompd_access (gomp_task, kind) \
  gompd_access (gomp_task, final_task) \
  gompd_access (gomp_team, nthreads) \
  gompd_access (gomp_team, implicit_task) \
  gompd_thread_handle_access

#define GOMPD_SIZES(gompd_size) \
  gompd_size (gomp_thread) \
  gompd_size (gomp_task_icv) \
  gompd_size (gomp_task)

#ifdef HAVE_ATTRIBUTE_VISIBILITY
#pragma GCC visibility pop
#endif

#endif /* _OMPD_SUPPORT_H */
