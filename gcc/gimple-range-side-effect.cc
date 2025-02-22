/* Gimple range side effect implementation.
   Copyright (C) 2022 Free Software Foundation, Inc.
   Contributed by Andrew MacLeod <amacleod@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "insn-codes.h"
#include "tree.h"
#include "gimple.h"
#include "ssa.h"
#include "gimple-pretty-print.h"
#include "gimple-range.h"
#include "tree-cfg.h"
#include "target.h"
#include "attribs.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "cfganal.h"

// Adapted from infer_nonnull_range_by_dereference and check_loadstore
// to process nonnull ssa_name OP in S.  DATA contains a pointer to a
// stmt side effects instance.

static bool
non_null_loadstore (gimple *, tree op, tree, void *data)
{
  if (TREE_CODE (op) == MEM_REF || TREE_CODE (op) == TARGET_MEM_REF)
    {
      /* Some address spaces may legitimately dereference zero.  */
      addr_space_t as = TYPE_ADDR_SPACE (TREE_TYPE (op));
      if (!targetm.addr_space.zero_address_valid (as))
	{
	  tree ssa = TREE_OPERAND (op, 0);
	  ((stmt_side_effects *)data)->add_nonzero (ssa);
	}
    }
  return false;
}

// Add NAME and RANGE to the the side effect summary.

void
stmt_side_effects::add_range (tree name, irange &range)
{
  m_names[num_args] = name;
  m_ranges[num_args] = range;
  if (num_args < size_limit - 1)
    num_args++;
}

// Add a nonzero range for NAME to the side effect summary.

void
stmt_side_effects::add_nonzero (tree name)
{
  if (!gimple_range_ssa_p (name))
    return;
  int_range<2> nz;
  nz.set_nonzero (TREE_TYPE (name));
  add_range (name, nz);
}

// Process S for side effects and fill in the summary list.
// This is the routine where new side effects should be added.

stmt_side_effects::stmt_side_effects (gimple *s)
{
  num_args = 0;

  if (is_a<gphi *> (s))
    return;

  if (is_a<gcall *> (s) && flag_delete_null_pointer_checks)
    {
      tree fntype = gimple_call_fntype (s);
      bitmap nonnullargs = get_nonnull_args (fntype);
      // Process any non-null arguments
      if (nonnullargs)
	{
	  for (unsigned i = 0; i < gimple_call_num_args (s); i++)
	    {
	      if (bitmap_empty_p (nonnullargs)
		  || bitmap_bit_p (nonnullargs, i))
		{
		  tree op = gimple_call_arg (s, i);
		  if (POINTER_TYPE_P (TREE_TYPE (op)))
		    add_nonzero (op);
		}
	    }
	  BITMAP_FREE (nonnullargs);
	}
      // Fallthru and walk load/store ops now.
    }

  // Look for possible non-null values.
  if (flag_delete_null_pointer_checks && gimple_code (s) != GIMPLE_ASM
      && !gimple_clobber_p (s))
    walk_stmt_load_store_ops (s, (void *)this, non_null_loadstore,
			      non_null_loadstore);

}

// -------------------------------------------------------------------------

// This class is an element in list of side effect ranges.

class exit_range
{
public:
  tree name;
  irange *range;
  exit_range *next;
};

// If there is an element which matches SSA, return a pointer to the element.
// Otherwise return NULL.

exit_range *
side_effect_manager::exit_range_head::find_ptr (tree ssa)
{
  // Return NULL if SSA is not in this list.
  if (!m_names || !bitmap_bit_p (m_names, SSA_NAME_VERSION (ssa)))
    return NULL;
  for (exit_range *ptr = head; ptr != NULL; ptr = ptr->next)
    if (ptr->name == ssa)
      return ptr;
  // Should be unreachable.
  gcc_unreachable ();
  return NULL;
}

// Construct a side effects manager.  DO_SEARCH indicates whether an immediate
// use scan should be made the first time a name is processed.  This is for
// on-demand clients who may not visit every statement and may miss uses.

side_effect_manager::side_effect_manager (bool do_search)
{
  bitmap_obstack_initialize (&m_bitmaps);
  m_on_exit.create (0);
  m_on_exit.safe_grow_cleared (last_basic_block_for_fn (cfun) + 1);
  // m_seen == NULL indicates no scanning.  Otherwise the bit indicates a
  // scan has been performed on NAME.
  if (do_search)
    m_seen = BITMAP_ALLOC (&m_bitmaps);
  else
    m_seen = NULL;
  obstack_init (&m_list_obstack);
  // Non-zero elements are very common, so cache them for each ssa-name.
  m_nonzero.create (0);
  m_nonzero.safe_grow_cleared (num_ssa_names + 1);
}

// Destruct a side effects manager.

side_effect_manager::~side_effect_manager ()
{
  m_nonzero.release ();
  obstack_free (&m_list_obstack, NULL);
  m_on_exit.release ();
  bitmap_obstack_release (&m_bitmaps);
}

// Return a non-zero range value of the appropriate type for NAME from
// the cache, creating it if necessary.

const irange&
side_effect_manager::get_nonzero (tree name)
{
  unsigned v = SSA_NAME_VERSION (name);
  if (v >= m_nonzero.length ())
    m_nonzero.safe_grow_cleared (num_ssa_names + 20);
  if (!m_nonzero[v])
    {
      m_nonzero[v] = m_range_allocator.allocate (2);
      m_nonzero[v]->set_nonzero (TREE_TYPE (name));
    }
  return *(m_nonzero[v]);
}

// Return TRUE if NAME has a side effect range in block BB.

bool
side_effect_manager::has_range_p (tree name, basic_block bb)
{
  // Check if this is an immediate use search model.
  if (m_seen && !bitmap_bit_p (m_seen, SSA_NAME_VERSION (name)))
    register_all_uses (name);

  if (bb->index >= (int)m_on_exit.length ())
    return false;
  if (!m_on_exit[bb->index].m_names)
    return false;
  if (!bitmap_bit_p (m_on_exit[bb->index].m_names, SSA_NAME_VERSION (name)))
    return false;
  return true;
}

// Return TRUE if NAME has a side effect range in block BB, and adjust range R
// to include it.

bool
side_effect_manager::maybe_adjust_range (irange &r, tree name, basic_block bb)
{
  if (!has_range_p (name, bb))
    return false;
  exit_range *ptr = m_on_exit[bb->index].find_ptr (name);
  gcc_checking_assert (ptr);
  // Return true if this exit range changes R, otherwise false.
  return r.intersect (*(ptr->range));
}

// Add range R as a side effect for NAME in block BB.

void
side_effect_manager::add_range (tree name, basic_block bb, const irange &r)
{
  if (bb->index >= (int)m_on_exit.length ())
    m_on_exit.safe_grow_cleared (last_basic_block_for_fn (cfun) + 1);

  // Create the summary list bitmap if it doesn't exist.
  if (!m_on_exit[bb->index].m_names)
      m_on_exit[bb->index].m_names = BITMAP_ALLOC (&m_bitmaps);

  if (dump_file && (dump_flags & TDF_DETAILS))
   {
     fprintf (dump_file, "   on-exit update ");
     print_generic_expr (dump_file, name, TDF_SLIM);
     fprintf (dump_file, " in BB%d : ",bb->index);
     r.dump (dump_file);
     fprintf (dump_file, "\n");
   }

  // If NAME already has a range, intersect them and done.
  exit_range *ptr = m_on_exit[bb->index].find_ptr (name);
  if (ptr)
    {
      int_range_max cur = r;
      // If no new info is added, just return.
      if (!cur.intersect (*(ptr->range)))
	return;
      if (ptr->range->fits_p (cur))
	*(ptr->range) = cur;
      else
	ptr->range = m_range_allocator.allocate (cur);
      return;
    }

  // Otherwise create a record.
  bitmap_set_bit (m_on_exit[bb->index].m_names, SSA_NAME_VERSION (name));
  ptr = (exit_range *)obstack_alloc (&m_list_obstack, sizeof (exit_range));
  ptr->range = m_range_allocator.allocate (r);
  ptr->name = name;
  ptr->next = m_on_exit[bb->index].head;
  m_on_exit[bb->index].head = ptr;
}

// Add a non-zero side effect for NAME in block BB.

void
side_effect_manager::add_nonzero (tree name, basic_block bb)
{
  add_range (name, bb, get_nonzero (name));
}

// Follow immediate use chains and find all side effects for NAME.

void
side_effect_manager::register_all_uses (tree name)
{
  gcc_checking_assert (m_seen);

  // Check if we've already processed this name.
  unsigned v = SSA_NAME_VERSION (name);
  if (bitmap_bit_p (m_seen, v))
     return;
  bitmap_set_bit (m_seen, v);

  use_operand_p use_p;
  imm_use_iterator iter;

  // Loop over each immediate use and see if it has a side effect.
  FOR_EACH_IMM_USE_FAST (use_p, iter, name)
    {
      gimple *s = USE_STMT (use_p);
      stmt_side_effects se (s);
      for (unsigned x = 0; x < se.num (); x++)
	{
	  if (name == se.name (x))
	    add_range (name, gimple_bb (s), se.range (x));
	}
    }
}
