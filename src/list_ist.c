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

   Copyright 2003, Pierre Ganty
 */

#include "list_ist.h"
#include "xmalloc.h"

//Initialise the list of sharing trees
void ist_init_list_ist(THeadListIST *list) {
  list->FirstElem = NULL;
}

//insert S at the beginning of the list
void ist_insert_at_the_beginning_list_ist(THeadListIST *list,ISTSharingTree * S) {
  TListIST * new_elem;

  new_elem = (TListIST *)xmalloc(sizeof(TListIST));
  new_elem->Info = S;
  new_elem->Next = list->FirstElem;
  list->FirstElem = new_elem;
}

int ist_count_elem_list_ist(THeadListIST *list) {
	int size=0;
	TListIST *tmp;
	for(tmp=list->FirstElem; tmp!=NULL; tmp=tmp->Next,++size);
	return size;
}

//insert S at the end of the list
void ist_insert_at_the_end_list_ist(THeadListIST *list, ISTSharingTree * S) {
  TListIST * new_elem;
  TListIST * elem;

  new_elem = (TListIST *)xmalloc(sizeof(TListIST));
  new_elem->Info = S;
  new_elem->Next = NULL;
  if (list->FirstElem == NULL) {
    list->FirstElem = new_elem;
  } else {
    elem = list->FirstElem;
    while (elem->Next != NULL) {
      elem = elem->Next;
    }
    elem->Next = new_elem;
  }
}

//empty the list of sharing trees and dispose the sharing trees
void ist_empty_list_ist_with_info(THeadListIST * list){
  TListIST * elem_list;

  while (list->FirstElem != NULL) {
    elem_list = list->FirstElem;
    list->FirstElem = elem_list->Next;
    ist_dispose(elem_list->Info);
    xfree(elem_list);
  }
}

//empty the list of sharing trees
void ist_empty_list_ist(THeadListIST * list) {
  TListIST * elem_list;

  while (list->FirstElem != NULL) {
    elem_list = list->FirstElem;
    list->FirstElem = elem_list->Next;
    xfree(elem_list);
  }
}


void ist_remove_list_ist(THeadListIST * list, ISTSharingTree * S){
  TListIST * elem;
  TListIST * next_elem;

  elem = list->FirstElem;
  next_elem = list->FirstElem;
  while (next_elem->Info != S) {
    elem = next_elem;
    next_elem = next_elem->Next;
  }
  if (elem == next_elem) {
    list->FirstElem = elem->Next;
    xfree(next_elem);
  } else {
    elem->Next = next_elem->Next;
    xfree(next_elem);
  }
}

//return the first sharing tree of the list and initialize the data for the list
ISTSharingTree *ist_first_element_list_ist(THeadListIST * list) {
  ISTSharingTree *first_elem;

  list->NextElem = list->FirstElem;
  if (list->NextElem == NULL) {
    first_elem = NULL;
  } else {
    first_elem = list->NextElem->Info;
    list->NextElem = list->NextElem->Next;
  }
  return first_elem;
}

//return the next sharing tree of the list
ISTSharingTree *ist_next_element_list_ist(THeadListIST * list) {
  ISTSharingTree *next_elem;

  if (list->NextElem == NULL) {
    next_elem = NULL;
  } else {
    next_elem = list->NextElem->Info;
    list->NextElem = list->NextElem->Next;
  }
  return next_elem;
}


//return true if the list is empty and false elsewhere
boolean ist_is_empty_list_ist(THeadListIST * list) {
  boolean empty;
  empty = (list->FirstElem == NULL) ? true : false;
  return empty;
}

THeadListIST * ist_copy_list_ist(THeadListIST * list) {
  THeadListIST * new_copy;
  ISTSharingTree * S;

  new_copy = (THeadListIST *)xmalloc(sizeof(THeadListIST));
  ist_init_list_ist(new_copy);
  S = ist_first_element_list_ist(list);
  while (S != NULL) {
    ist_insert_at_the_end_list_ist(new_copy,S);
    S = ist_next_element_list_ist(list);
  }
  return new_copy;
}
