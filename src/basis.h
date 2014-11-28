// vim:sw=4:ts=4
/*
   This file is part of mist.

   mist is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   mist is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mist; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Copyright 2003, 2004, Pierre Ganty, 2006, Laurent Van Begin
 */

#ifndef __BASIS_H
#define __BASIS_H

#include "proc.h"
#include "def.h"
#include "interval.h"
#include "listnode.h"
#include "normalize.h"
#include <stdio.h>
#include <stddef.h>

/*
 * This file contains all the basic functions on ISTs as union, intersection,
 * etc ...  Essentially manipulations on ISTs, statistics, printing.
 */


/*
 * Most of the time algorithms are recursive.  To keep the code readable,
 * instead of passing a plenty of parameters, we pass a data stucture of the
 * following kind where all the parameters are wrapped in.  This stucture is
 * used by some methods (e.g. copy union intersection etc). All these function
 * does not require all the fields (e.g. memo is useless for ist_copy).
 */
struct LOC_ist_method {
    ISTSharingTree *STR;
    ISTLayer *rlayer;
    TMemo1 *memo;
    ISTInterval *intersect;
} ;

boolean ist_is_empty(ISTSharingTree *ST) ;
boolean ist_add(ISTSharingTree *ST_, ISTInterval **Info_, integer16 LInfo_) ;
boolean ist_is_member(ISTSharingTree *ST, ISTInterval **Info, integer16 LInfo) ;
boolean ist_equal(ISTSharingTree *ST1, ISTSharingTree *ST2) ;
boolean ist_included(ISTSharingTree *ST1, ISTSharingTree *ST2) ;
ISTSharingTree *ist_copy(ISTSharingTree *ST) ;
ISTSharingTree *ist_union(ISTSharingTree *ST1, ISTSharingTree *ST2) ;
ISTSharingTree *ist_intersection(ISTSharingTree *ST1, ISTSharingTree *ST2) ;
ISTSharingTree *ist_minus(ISTSharingTree *ST1, ISTSharingTree *ST2) ;
ISTSharingTree *ist_projection(ISTSharingTree *S, integer16 *mask);
ISTSharingTree *ist_downward_closure(ISTSharingTree *ST);

int ist_nb_sons(ISTSharingTree *ST) ;
int ist_nb_nodes(ISTSharingTree *ST) ;
int ist_nb_layers(ISTSharingTree *ST) ;
long ist_nb_elements(ISTSharingTree *S) ;


/* I/O primitives */
void ist_write(ISTSharingTree *S) ;
void ist_fill_tree_from_file(FILE* file_spec, ISTSharingTree **S, size_t height, size_t width);
ISTInterval **ist_firstpath2array(ISTSharingTree *S) ;
long ist_nb_tuples(ISTSharingTree *ST);

#endif
