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

   Copyright 2003, 2004, Pierre Ganty, 2015, Pedro Valero
 */

#ifndef __CHECKUP_H
#define __CHECKUP_H

#include <stdio.h>
#include "proc.h"
#include "def.h"

/*
 * This module aims at verifying that an IST respects his definition
 * and print some stats about it.
 */

boolean ist_checkup(ISTSharingTree *S) ;
void ist_stat(ISTSharingTree *S) ;
void ist_stat_plot(ISTSharingTree *S, FILE *f);

#endif
