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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>


#if OMPD_SUPPORT

void ompd_load();
extern __UINT64_TYPE__ ompd_state;


#define OMPD_FOREACH_ACCESS(OMPD_ACCESS) \
OMPD_ACCESS(gomp_thread_pool, threads) \
OMPD_ACCESS(gomp_thread, handle) \
OMPD_ACCESS(gomp_thread, thread_pool)

#define OMPD_FOREACH_SIZEOF(OMPD_SIZEOF) \
OMPD_SIZEOF(int) \
OMPD_SIZEOF(char) \
OMPD_SIZEOF(ompd_state) \
OMPD_SIZEOF(ompd_thread_id_t) \
OMPD_SIZEOF(gomp_tls_key) \
OMPD_SIZEOF(gomp_tls_data) \



#endif /* OMPD_SUPPORT */
#endif /* _OMPD_SUPPORT_H */
