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

#ifndef __ABSTRACTION_H
#define __ABSTRACTION_H

#include"def.h"
#include"transsystem.h"

typedef struct abstraction_t {
		integer16 nbV;
		integer16 **A;
		integer16 *bound;
} abstraction_t;

/* Only works for Petri Nets w/o invariants */
transition_system_t *build_sys_using_abs(transition_system_t *sys, abstraction_t *abs);
#endif
