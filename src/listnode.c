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

   Copyright 2003, Pierre Ganty. 2006, Laurent Van Begin
 */

#include "listnode.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <stdio.h>


/*Initialise the list of nodes*/
void ist_init_list_node(List)
	ISTHeadListNode **List;
{
	*List = (ISTHeadListNode *)xmalloc(sizeof(ISTHeadListNode));
	(*List)->FirstElem = NULL;
}


/*Insert Node at the beginning of the list*/
void ist_insert_list_node(List, Node)
	ISTHeadListNode *List;
	ISTNode *Node;
{
	ISTSon *NewElem;

	NewElem = ist_new_son();
	NewElem->Son = Node;
	NewElem->Next = List->FirstElem;
	List->FirstElem = NewElem;
}

//add node only if it not in List yet
boolean ist_insert_list_node_without_redundancy(List,Node)
	ISTHeadListNode *List;
	ISTNode *Node;
{
	ISTSon *tmp;

	tmp = List->FirstElem;
    while (tmp != NULL)
		if (tmp->Son == Node)
			return false;
	    else
	   		tmp = tmp->Next;
	ist_insert_list_node(List,Node);
	return true;
}

ISTNode *ist_remove_first_elem_list_node(List)
	ISTHeadListNode *List;
{
	ISTSon *Elem;
	ISTNode *Sol;

	if (List->FirstElem == NULL) {
		Sol = NULL;
		return Sol;
	}
	Elem = List->FirstElem;
	List->FirstElem = List->FirstElem->Next;
	Sol = Elem->Son;
	ist_dispose_son(Elem);
	return Sol;
}


/*return true if the list is empty and false otherwise*/
boolean ist_is_empty_list_node(List)
	ISTHeadListNode *List;
{
	boolean Empty;

	if (List->FirstElem == NULL)
		Empty = true;
	else
		Empty = false;
	return Empty;
}

