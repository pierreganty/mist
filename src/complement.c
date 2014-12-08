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

   Copyright 2003, 2004, 2006 Pierre Ganty, Laurent Van Begin
 */


#include "complement.h"
#include "xmalloc.h"
#include "interval.h"
#include "normalize.h"
#include "listnode.h"
#include "remove.h"
#include "determinize.h"
#include "basis.h"
#include <stdlib.h>


/*
 * As assumption we know that the IST passed in argument is determinized
 */
static ISTHeadListNode* complete_list_of_sons(ISTNode *Node){

	ISTSon *son;
	ISTNode *new_node;
	ISTHeadListNode *list_node;
	long current_value;

	son = Node->FirstSon;
	ist_init_list_node(&list_node);
	if (son == NULL) {
		new_node = ist_create_node(ist_build_interval(0L,INFINITY));
		ist_insert_list_node(list_node,new_node) ;
	} else {
		current_value = 0;

		while (son != NULL && current_value != INFINITY ) {
			if (son->Son->Info->Left > current_value){
				new_node = ist_create_node(ist_build_interval(current_value,son->Son->Info->Left-1));
				ist_insert_list_node(list_node,new_node) ;
			}
			current_value = (son->Son->Info->Right == INFINITY ) ? INFINITY : son->Son->Info->Right+1;
			son = son->Next;
		}
		if (current_value != INFINITY ) {
			new_node = ist_create_node(ist_build_interval(current_value,INFINITY));
			ist_insert_list_node(list_node,new_node) ;
		}

	}
	return list_node;
}

void ist_complement(ISTSharingTree *S, size_t dim)
{
	ISTLayer *Layer;
	ISTNode *Node, *new_node, *rnode;
	ISTHeadListNode *list_node;
    ISTInterval **tuple;
	int i;

	if (ist_is_empty(S) == false) {
		/*First step: determinisation */
		ist_determinize(S);
		/* Special case where we have only one variable */
		if(dim<=1) {
			/*complementation in itself */
			list_node = complete_list_of_sons(S->Root);
			new_node = ist_remove_first_elem_list_node(list_node);
			ist_remove_sons(S->Root);
			ist_remove_node_without_father_layer(S->FirstLayer);
			while (new_node != NULL){
				/*
				 * Caution, ist_add_node don't necesseraly return new_node ...
				 * e.g. imagine one node without son in layer and you want insert a same node without sons
				 */
				rnode=ist_add_node(S->FirstLayer,new_node);
				ist_add_son(S->Root,rnode);
				ist_add_son(rnode,S->LastLayer->FirstNode);
				new_node = ist_remove_first_elem_list_node(list_node);
			}
			xfree(list_node);

		} else {

			/*complementation in itself */
			list_node = complete_list_of_sons(S->Root);
			new_node = ist_remove_first_elem_list_node(list_node);
			while (new_node != NULL){
				/*
				 * Caution, ist_add_node don't necesseraly return new_node ...
				 * e.g. imagine one node without son in layer and you want insert a same node without sons
				 */
				ist_add_son(S->Root,ist_add_node(S->FirstLayer,new_node));
				new_node = ist_remove_first_elem_list_node(list_node);
			}
			xfree(list_node);

			/* Now the general case */
			Layer = S->FirstLayer;
			while (Layer != S->LastLayer->Previous){
				/* We have at least two variables in the system ! */
				Node = Layer->FirstNode;
				while (Node != NULL){
					list_node = complete_list_of_sons(Node);
					if (Layer->Next == S->LastLayer->Previous) {
						ist_remove_sons(Node);
						ist_remove_node_without_father_layer(Layer->Next);
					}
					new_node = ist_remove_first_elem_list_node(list_node);
					while (new_node != NULL){
						ist_add_son(Node,ist_add_node(Layer->Next,new_node));
						new_node = ist_remove_first_elem_list_node(list_node);
					}
					xfree(list_node);
					Node = Node->Next;
				}
				Layer = Layer->Next;
			}
			Node = Layer->FirstNode;
			while (Node != NULL) {
				ist_add_son(Node,S->LastLayer->FirstNode);
				Node = Node->Next;
			}
			ist_remove_node_without_son(S);
			/* Special case when true is given in input */
			if (S->Root->FirstSon == NULL) {
				while (S->LastLayer != NULL) {
					ist_remove_last_layer(S);
				}
			} else
				/* if you ist_remove_sons and that after your list_node is empty, this is the case */
				ist_adjust_second_condition(S);
		}
	} else {
		/* complement of the empty sef of dimension dim*/

		/*First: construction of a tuple corresponding to N^{dim} */
		tuple = (ISTInterval **) xmalloc(dim * sizeof(ISTInterval *));
		for(i = 0;i < dim; i++)
			tuple[i] = ist_build_interval(0,INFINITY);

		/*adding tuple to S (which is empty) gives us an IST that contains any tuple over positive
		 * integer of dimension dim*/

		ist_add(S,tuple,dim);

		/*free of the tuple */
		for(i = 0; i < dim; i++) {
			ist_dispose_info(tuple[i]);
		}
		xfree(tuple);
	}
}
