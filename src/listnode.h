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


#ifndef __LISTNODE_H
#define __LISTNODE_H

#include "proc.h"
/*
* This modules provides methods and a data structure to manipulate lists of IST
* nodes.  This is used  for instance in the modules: determinize, complement,
* predtrans.
*/

/* Declaration of structures */
typedef struct ISTHeadListNode {
    ISTSon *FirstElem;
} ISTHeadListNode;

/* Declaration of functions */
void ist_init_list_node(ISTHeadListNode **List) ;
void ist_insert_list_node(ISTHeadListNode *List, ISTNode *Node) ;
boolean ist_insert_list_node_without_redundancy(ISTHeadListNode *List, ISTNode *Node) ;
ISTNode *ist_remove_first_elem_list_node(ISTHeadListNode *List) ;
boolean ist_is_empty_list_node(ISTHeadListNode *List) ;

#endif
