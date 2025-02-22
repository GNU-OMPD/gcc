/* Header file for gimple ranger SSA cache.
   Copyright (C) 2017-2022 Free Software Foundation, Inc.
   Contributed by Andrew MacLeod <amacleod@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_SSA_RANGE_CACHE_H
#define GCC_SSA_RANGE_CACHE_H

#include "gimple-range-gori.h" 
#include "gimple-range-side-effect.h"

// This class manages a vector of pointers to ssa_block ranges.  It
// provides the basis for the "range on entry" cache for all
// SSA names.

class block_range_cache
{
public:
  block_range_cache ();
  ~block_range_cache ();

  bool set_bb_range (tree name, const_basic_block bb, const irange &r);
  bool get_bb_range (irange &r, tree name, const_basic_block bb);
  bool bb_range_p (tree name, const_basic_block bb);

  void dump (FILE *f);
  void dump (FILE *f, basic_block bb, bool print_varying = true);
private:
  vec<class ssa_block_ranges *> m_ssa_ranges;
  ssa_block_ranges &get_block_ranges (tree name);
  ssa_block_ranges *query_block_ranges (tree name);
  irange_allocator *m_irange_allocator;
  bitmap_obstack m_bitmaps;
};

// This global cache is used with the range engine as markers for what
// has been visited during this incarnation.  Once the ranger evaluates
// a name, it is typically not re-evaluated again.

class ssa_global_cache
{
public:
  ssa_global_cache ();
  ~ssa_global_cache ();
  bool get_global_range (irange &r, tree name) const;
  bool set_global_range (tree name, const irange &r);
  void clear_global_range (tree name);
  void clear ();
  void dump (FILE *f = stderr);
private:
  vec<irange *> m_tab;
  class irange_allocator *m_irange_allocator;
};

// This class provides all the caches a global ranger may need, and makes 
// them available for gori-computes to query so outgoing edges can be
// properly calculated.

class ranger_cache : public range_query
{
public:
  ranger_cache (int not_executable_flag, bool use_imm_uses);
  ~ranger_cache ();

  virtual bool range_of_expr (irange &r, tree name, gimple *stmt);
  virtual bool range_on_edge (irange &r, edge e, tree expr);
  bool block_range (irange &r, basic_block bb, tree name, bool calc = true);

  bool get_global_range (irange &r, tree name) const;
  bool get_global_range (irange &r, tree name, bool &current_p);
  void set_global_range (tree name, const irange &r);

  void propagate_updated_value (tree name, basic_block bb);

  void apply_side_effects (gimple *s);
  gori_compute m_gori;
  side_effect_manager m_exit;

  void dump_bb (FILE *f, basic_block bb);
  virtual void dump (FILE *f) override;
private:
  ssa_global_cache m_globals;
  block_range_cache m_on_entry;
  class temporal_cache *m_temporal;
  void fill_block_cache (tree name, basic_block bb, basic_block def_bb);
  void propagate_cache (tree name);

  enum rfd_mode
    {
      RFD_NONE,		// Only look at current block cache.
      RFD_READ_ONLY,	// Scan DOM tree, do not write to cache.
      RFD_FILL		// Scan DOM tree, updating important nodes.
    };
  bool range_from_dom (irange &r, tree name, basic_block bb, enum rfd_mode);
  void range_of_def (irange &r, tree name, basic_block bb = NULL);
  void entry_range (irange &r, tree expr, basic_block bb, enum rfd_mode);
  void exit_range (irange &r, tree expr, basic_block bb, enum rfd_mode);
  bool edge_range (irange &r, edge e, tree name, enum rfd_mode);

  vec<basic_block> m_workback;
  class update_list *m_update;
};

#endif // GCC_SSA_RANGE_CACHE_H
