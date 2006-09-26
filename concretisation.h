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

   Copyright 2006, Laurent Van Begin
 */
#ifndef __CONCRETISATION_H
#define __CONCRETISATION_H

#include "proc.h"
#include "listnode.h"
#include "transsystem.h"
#include "abstraction.h"
#include "predtrans.h"
#include "normalize.h"
#include "xmalloc.h"
#include "basis.h"

void ist_add_variables(ISTSharingTree *S,integer16 nb_var);
ISTSharingTree * ist_concretisation(ISTSharingTree *S, abstraction_t * abs);

#endif
