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

   Copyright 2003, 2004, Pierre Ganty
 */


#ifndef __MINIMIZEINVARIANTEXACT_H
#define __MINIMIZEINVARIANTEXACT_H

#include "proc.h"
#include "def.h"
#include "transsystem.h"



/* This modules prunes path of an IST, w.r.t a structural invariant of the transtion system */
ISTSharingTree *ist_intersection_with_invar(ISTSharingTree *ST1, invariant_t *invariant, integer16 height);
ISTSharingTree *ist_remove_with_all_invar_exact(ISTSharingTree *S, transition_system_t *system) ;

#endif
