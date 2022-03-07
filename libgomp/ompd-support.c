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

#include "ompd-support.h"


#define ompd_declare_access(t, m) __UINT64_TYPE__ ompd_access_##t##_##m;
	OMPD_FOREACH_ACCESS(ompd_declare_access)
#undef ompd_declare_access


#define ompd_declare_sizeof_members(t, m) __UINT64_TYPE__ ompd_sizeof_##t##_##m;
	OMPD_FOREACH_ACCESS(ompd_declare_sizeof_members)
#undef ompd_declare_sizeof_members


#define ompd_declare_sizes(t) __UINT64_TYPE__ ompd_sizeof_##t;
	OMPD_SIZES(ompd_declare_sizes)
#undef ompd_declare_sizes


const char **ompd_dll_locations = NULL;
__UINT64_TYPE__ ompd_state;

void 
ompd_load()
{
	static int ompd_initialized = 0;
	if(ompd_initialized)
		return;

	/* Get the offset of the struct members.  */
	#define ompd_init_access(t, m)                \
  		ompd_access_##t##_##m = (__UINT64_TYPE__) & (((struct t *)0)->m);
 		OMPD_FOREACH_ACCESS(ompd_init_access)
	#undef ompd_init_access


	/* Get sizeof members.  */

	#define ompd_init_sizeof_members(t, m) \
		ompd_sizeof_##t##_##m = sizeof(((struct t *)NULL)->m);
		OMPD_FOREACH_ACCESS(ompd_init_sizeof_members)
	#undef ompd_declare_sizeof_members


	#define ompd_init_sizes(t) ompd_sizeof_##t = sizeof(struct t);
		OMPD_SIZES(ompd_init_sizes)
	#undef ompd_init_sizes


	fprintf(stderr, "OMP OMPD active\n");
	static const char *tmp_ompd_dll_locations[2] 
		= {"libgompd" SONAME_SUFFIX(1) , NULL};
	ompd_state |= OMPD_ENABLED;
	ompd_initialized = 1;
	ompd_dll_locations = (const char **)malloc(2 * sizeof(const char *));
	ompd_dll_locations[0] = tmp_ompd_dll_locations[0];
	ompd_dll_locations[1] = tmp_ompd_dll_locations[1];
	ompd_dll_locations_valid();
}


/*Dummy functions. they shoud not be optimized.  */

void __attribute__ ((noinline))
ompd_dll_locations_valid()
{
	asm("");
}


void __attribute__ ((noinline))
ompd_bp_parallel_begin()
{
	asm("");
}

void __attribute__ ((noinline))
ompd_bp_parallel_end()
{
	asm("");
}



void __attribute__ ((noinline))
ompd_bp_task_begin()
{
	asm("");
}


void __attribute__ ((noinline))
ompd_bp_task_end()
{
	asm("");
}



void __attribute__ ((noinline))
ompd_bp_thread_begin()
{
	asm("");
}

void __attribute__ ((noinline))
ompd_bp_thread_end()
{
	asm("");
}



void __attribute__ ((noinline))
ompd_bp_device_begin()
{
	asm("");
}

void __attribute__ ((noinline))
ompd_bp_device_end()
{
	asm("");
}
