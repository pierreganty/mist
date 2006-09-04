// vim:sw=4:ts=4
/*
   This file is part of mist2.

   mist2 is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   mist2 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mist2; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Copyright 2003, 2004, Pierre Ganty
 */

#ifndef __INTERVAL_H
#define __INTERVAL_H

#include "def.h"

/*
 * In this file we define the most usual functions that we need
 * on intervals. Sometimes in others files we will work directly on 
 * Left and Right bound of the ISTInterval data structure, at a glance this does not
 * seem clean but in that way :
 * - we achieve a better performance
 * - the source code is still readily understandable.
 * Don't forget that we are only interested in dealing with intervals
 * with their bounds >= 0. Normally we should ensure that this cond holds but
 * to achieve performance we omit checks.
 * 
 * I put this limitation because I though only in terms of applying ISTs to verification
 * of Multi-Transfer Nets. Obviously we can relax that condition if needed.
 *
 * The author has to confess that the interval module is a weak point of the library.
 * As a matter of fact, better performance (it should be a gain of 2 upto 3 following some tests)
 * could be achieved by rewriting this module. A first step could be to set INFINITY to the 
 * greatest value (e.g. 2^32-1) like that we should save a lot of tests. 
 * A second should be to put this code in macro. However modifying this is not so obvious
 * because it has implication everywhere in the library. So if you do it, be very carefull.
 */ 


/* This structure is the basic information that we manipulate */
typedef struct ISTInterval {
    integer32 Left, Right;
	struct ISTInterval *next; /* Private field for memory mangement. See
								 ist_dispose_info, ist_new_info for instance.
								 */
} ISTInterval;

#ifndef LOCAL
extern ISTInterval IST_beg_of_list;
extern ISTInterval IST_end_of_list;
#endif

/*
 * Declaration of functions. Most of them are defined inline.
 */
boolean ist_greater_value(integer32 Value1, integer32 Value2) ;
boolean ist_greater_or_equal_value(integer32 Value1, integer32 Value2) ;
boolean ist_less_value(integer32 Value1, integer32 Value2) ;
boolean ist_less_or_equal_value(integer32 Value1, integer32 Value2) ;
integer32 ist_add_value(integer32 Value1, integer32 Value2) ;
integer32 ist_sub_value(integer32 Value1, integer32 Value2) ;
integer32 max(integer32 Value1, integer32 Value2) ;
integer32 min(integer32 Value1, integer32 Value2);

boolean ist_greater_interval(ISTInterval *Value1, ISTInterval *Value2) ;
boolean ist_greater_or_equal_interval(ISTInterval *Value1, ISTInterval *Value2) ;
boolean ist_less_interval(ISTInterval *Value1, ISTInterval *Value2) ;
boolean ist_less_or_equal_interval(ISTInterval *Value1, ISTInterval *Value2) ;
boolean ist_equal_interval(ISTInterval *Value1, ISTInterval *Value2) ;
boolean ist_not_equal_interval(ISTInterval *Value1, ISTInterval *Value2) ;
boolean ist_is_unbounded(ISTInterval *Value);

void ist_assign_interval_to_interval(ISTInterval* receiver,ISTInterval *sender); 
void ist_assign_values_to_interval(ISTInterval* receiver,integer32 leftbound,integer32 rightbound);

ISTInterval* ist_build_interval(integer32 leftbound, integer32 rightbound); 
ISTInterval* ist_copy_interval(ISTInterval *interv);

/* If Value2 \subset Value1 the function returns TRUE */
boolean ist_include_interval(ISTInterval *Value1, ISTInterval *Value2) ;
/* If the intersection is empty the function returns NULL */
ISTInterval* ist_intersect_intervals(ISTInterval *interv1,ISTInterval *interv2);
/*
 * If the ist_convex_union of interv1 and interv2 exists the function computes it,
 * put the result in interv1 and return true. False otherwise !
 */ 
boolean ist_convex_union(ISTInterval *interv1,ISTInterval *interv2);
void ist_add_value_to_interval(ISTInterval *interv,integer32 Value);
void ist_add_interval_to_interval(ISTInterval *interv1,ISTInterval *interv2);
/*
 * Use ist_sub_interval_to_interval at your own risks, this function 
 * doesn't prevent to get negative bounds after a call to it
 */
void ist_sub_interval_to_interval(ISTInterval *interv1,ISTInterval *interv2);
/* Same remarks for this one */
void ist_multiply_left_and_right_bound_by_value(ISTInterval *interv,integer32 value);
#endif
