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


#ifndef __LIST_ST_H
#define __LIST_ST_H

#include "proc.h"

typedef struct TListSt {
ISTSharingTree * Info;
struct TListSt * Next;
} TListIST;

typedef struct THeadListSt {
TListIST * FirstElem;
TListIST * NextElem;
} THeadListIST;

void ist_init_list_ist(THeadListIST *list);
void ist_insert_at_the_beginning_list_ist(THeadListIST *list,ISTSharingTree * S);
void ist_insert_at_the_end_list_ist( THeadListIST *list, ISTSharingTree * S);
void ist_empty_list_ist_with_info(THeadListIST * List);
void ist_empty_list_ist(THeadListIST * List);
void ist_remove_list_ist(THeadListIST * ListSt, ISTSharingTree * S);
ISTSharingTree *ist_first_element_list_ist(THeadListIST * List);
ISTSharingTree *ist_next_element_list_ist(THeadListIST * List);
boolean ist_is_empty_list_ist(THeadListIST * List);
THeadListIST * ist_copy_list_ist(THeadListIST * List);
int ist_count_elem_list_ist(THeadListIST *list);

#endif
