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

#include "determinize.h"
#include "interval.h"
#include "normalize.h"
#include "listnode.h"
#include "remove.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <stdio.h>


/* Local data structure */
typedef struct list_val_t {
	struct list_val_t *next;
	float val;
} list_val_t;

static ISTHeadListNode* generate_new_sons_for_node(ISTNode *Node){
	list_val_t *head_list, *p, *pnext, **q, *new_elem;
	ISTNode *new_node;
	ISTSon *son, *copy_all_sons, *sons_of_new_node ;
	float bound;
	long new_left, new_right;
	ISTHeadListNode *list_node;
	boolean stop, authorized ;

	head_list = NULL;
	son = Node->FirstSon;
	while (son != NULL){
		bound = (float) son->Son->Info->Left - .5;
		/*
		 * Now we insert left bound sortedly and uniquely in the list,
		 * remind that the left bound is always finite
		 */
		p = head_list;
		q = &head_list;
		while (p != NULL && p->val != INFINITY && p->val < bound ){
			/* In this case, we can have INFINITY in the list p but not for bound */
			q = &(p->next);
			p = p->next;
		}
		if ((p != NULL && (p->val == INFINITY || p->val > bound)) || p == NULL){
			new_elem = (list_val_t *)xmalloc(sizeof(list_val_t));
			new_elem->val =  bound;
			new_elem->next = p;

			*q = new_elem;
		}
		/* As left_bound <= right_bound, we continue from this place */
		p = *q;
		if (son->Son->Info->Right != INFINITY){
			bound = (float) son->Son->Info->Right + .5;
		} else {
			bound = INFINITY;
		}
		stop = false;
		while (!stop){
			if (p == NULL){
				/* If we are at the end of the list */
				stop = true;
			} else if (p->val != INFINITY && ((bound != INFINITY && p->val < bound) || bound == INFINITY)){
				/* If we have to go farer in the list */
				q = &(p->next);
				p = p->next;
			} else if (p->val == INFINITY || p->val >= bound) {
				/* If we are at the end or at the right place */
				stop = true;
			}
		}
		if ((p != NULL &&  p->val != bound) || p == NULL){
			new_elem = (list_val_t *)xmalloc(sizeof(list_val_t));
			new_elem->val =  bound;
			new_elem->next = p;
			*q = new_elem;
		}
		son = son->Next;
	}
	ist_init_list_node(&list_node);
	p = head_list;
	while (p != NULL){
		new_left = (long) (p->val + .5);
		pnext = p->next;
		xfree(p);
		p = pnext;
		if (p != NULL){
			if (p->val != INFINITY){
				new_right = (long) (p->val - .5);
			} else {
				new_right = INFINITY;
			}
			new_node = ist_create_node(ist_build_interval(new_left,new_right));
			son = Node->FirstSon;
			while (son != NULL){
				if (ist_include_interval(son->Son->Info,new_node->Info)){
					copy_all_sons = son->Son->FirstSon;
					while (copy_all_sons != NULL){
						/* We assign to new_node all the sons of son->Son */
						sons_of_new_node = new_node->FirstSon;
						authorized = true;
						while (sons_of_new_node != NULL && authorized ){
							if (sons_of_new_node->Son == copy_all_sons->Son){
								authorized = false;
							}
							sons_of_new_node = sons_of_new_node->Next;
						}
						if (authorized)
							ist_add_son(new_node,copy_all_sons->Son);
						copy_all_sons = copy_all_sons->Next;
					}
				}
				son = son->Next;
			}
			/*
			 * We can also avoid to use ISTHeadListNode adding new_node
			 * directly in the layer without father, afterwards we upgrade
			 * the the sons of Node
			 */
			if (new_node->FirstSon != NULL) {
				ist_insert_list_node(list_node,new_node) ;
			} else {
				/*
				   It can happen that you generate a son which is not included in any succ(Node)
				   For instance take a Node with <0,0> and <3,3> as sons, you will generate
				   <0,0> <1,2> and <3,3> with this procedure, so you have to get rid of <1,2>.
				 */
				ist_dispose_node(new_node);
			}
		}
	}
	return list_node;
}


void ist_determinize(ISTSharingTree *S){
	ISTLayer *Layer;
	ISTNode *Node, *new_node;
	ISTHeadListNode *list_node;

	list_node = generate_new_sons_for_node(S->Root);
	ist_remove_sons(S->Root);
	ist_remove_node_without_father_layer(S->FirstLayer);
	new_node = ist_remove_first_elem_list_node(list_node);
	while (new_node != NULL){
		/*ist_add_son(S->Root,new_node);
		  ist_add_node_star(S->FirstLayer,new_node);*/
		ist_add_son(S->Root,ist_add_node(S->FirstLayer,new_node));
		new_node = ist_remove_first_elem_list_node(list_node);
	}
	xfree(list_node);
	ist_adjust_first_condition(S);

	/* Now the general case */
	Layer = S->FirstLayer;
	while (Layer != S->LastLayer->Previous){
		Node = Layer->FirstNode;
		while (Node != NULL){
			list_node = generate_new_sons_for_node(Node);
			ist_remove_sons(Node);
			ist_remove_node_without_father_layer(Layer->Next);
			new_node = ist_remove_first_elem_list_node(list_node);
			while (new_node != NULL){
				ist_add_son(Node,ist_add_node(Layer->Next,new_node));
				new_node = ist_remove_first_elem_list_node(list_node);
			}
			xfree(list_node);
			Node = Node->Next;
		}
		ist_adjust_first_condition(S);
		ist_adjust_second_condition(S);
		Layer = Layer->Next;
	}
}
