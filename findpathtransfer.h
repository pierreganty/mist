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

   Copyright 2004, Laurent Van Begin
 */

#ifndef __FINDPATHTRANSFER_H
#define __FINDPATHTRANSFER_H

#include "proc.h"
#include "transsystem.h"
#include "basis.h"
#include "list_ist.h"
#include "interval.h"
#include "father_mangmt.h"
#include "initialmarking.h"
#include "initialmarking.h"
#include "precone.h"

void ist_put_path_on_screen_with_parameterized_initial_marking(ISTSharingTree * initial_marking,THeadListIST * list_ist, transition_system_t * rules);
#endif
