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

#if OMPD_SUPPORT

/**
 * Declaration of symbols to hold struct size and member offset information
 */

#define ompd_declare_access(t,m) __UINT64_TYPE__ ompd_access__##t##__##m;
OMPD_FOREACH_ACCESS(ompd_declare_access)
#undef ompd_declare_access

#define ompd_declare_sizeof_member(t,m) __UINT64_TYPE__ ompd_sizeof__##t##__##m;
OMPD_FOREACH_ACCESS(ompd_declare_sizeof_member)
#undef ompd_declare_sizeof_member

#define ompd_declare_bitfield(t,m) __UINT64_TYPE__ ompd_bitfield__##t##__##m;
OMPD_FOREACH_BITFIELD(ompd_declare_bitfield)
#undef ompd_declare_bitfield

#define ompd_declare_sizeof(t) __UINT64_TYPE__ ompd_sizeof__##t;
OMPD_FOREACH_SIZEOF(ompd_declare_sizeof)
#undef ompd_declare_sizeof

const char **ompd_dll_locations = NULL;
__UINT64_TYPE__ ompd_state;

void
ompd_load()
{
	static int ompd_initialized = 0;
	if(ompd_initialized)
		return;
	fprintf(stderr, "OMP OMPD active\n");
	static const char *tmp_ompd_dll_locations[2]
		= {"libgompd" SONAME_SUFFIX(1) , NULL};
	ompd_state |= OMPD_ENABLED;
	ompd_initialized = 1;
	ompd_dll_locations = (const char **)malloc(2 * sizeof(const char *));
	ompd_dll_locations[0] = tmp_ompd_dll_locations[0];
	ompd_dll_locations[1] = tmp_ompd_dll_locations[1];
	ompd_dll_locations_valid();

	/**
   * Calculate member offsets for structs and unions
   */

#define ompd_init_access(t,m) ompd_access__##t##__##m = (__UINT64_TYPE__)&(((t*)0)->m);
  OMPD_FOREACH_ACCESS(ompd_init_access)
#undef ompd_init_access

  /**
   * Create bit mask for bitfield access
   */

#define ompd_init_bitfield(t,m) ompd_bitfield__##t##__##m=0; ((t*)(&ompd_bitfield__##t##__##m))->m = 1;
  OMPD_FOREACH_BITFIELD(ompd_init_bitfield)
#undef ompd_init_bitfield

  /**
   * Calculate type size information
   */

#define ompd_init_sizeof_member(t,m) ompd_sizeof__##t##__##m = sizeof(((t*)0)->m);
  OMPD_FOREACH_ACCESS(ompd_init_sizeof_member)
#undef ompd_init_sizeof_member

#define ompd_init_sizeof(t) ompd_sizeof__##t = sizeof(t);
  OMPD_FOREACH_SIZEOF(ompd_init_sizeof)
#undef ompd_init_sizeof

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

#endif /* OMPD_SUPPORT */
