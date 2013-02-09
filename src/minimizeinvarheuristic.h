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


#ifndef __MINIMIZEINVARHEURISTIC_H
#define __MINIMIZEINVARHEURISTIC_H

#include "proc.h"
#include "def.h"
#include "transsystem.h"


/*
 * Same as minimizeinvarexact but in this case, algorithms are based on a heuristic
 */
void ist_minimize_invar_heuristic(ISTSharingTree *S, invariant_t *invariant);
void ist_remove_with_invar_heuristic(ISTSharingTree *S, long NuRule, transition_system_t *system);
void ist_remove_with_all_invar_heuristic(ISTSharingTree *S, transition_system_t *system);

#endif
