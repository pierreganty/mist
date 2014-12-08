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

#include "basis.h"
#include "minimizeinvarexact.h"
#include "minimizeinvarheuristic.h"
#include <stdlib.h>
#include <stdio.h>


static ISTNode *IntersectionWithInvar(node, invariant, height, NuLayer, LINK)
    ISTNode *node;
	invariant_t *invariant;
    integer16 height;
    size_t  NuLayer;
    struct LOC_ist_method  *LINK;
{
	ISTSon *s1;
	ISTNode *rnode, *rchild;
	ISTInterval *Product, *IntersectWithInv;
	boolean stop;
	long temp;
	Product = ist_new_info();


	if (ist_equal_interval(node->Info,&IST_end_of_list)) {
		/*
		 * We redo the computation of the intersection, we made it before
		 * but we didn't pass the result in parameter
		 */
		IntersectWithInv = ist_intersect_intervals(LINK->intersect,invariant->m0_p);
		if (IntersectWithInv != NULL){
			rnode = ist_create_node(&IST_end_of_list);
			rnode = ist_add_node(LINK->rlayer, rnode);
			ist_dispose_info(IntersectWithInv);
		} else
			rnode = NULL;
	} else {
		rnode = ist_create_node(node->Info);
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s1 = node->FirstSon;
		stop = false;
		while (s1 != NULL && !stop) {
			if (NuLayer < height && invariant->weight_on_place[NuLayer] != 0){
				ist_assign_interval_to_interval(Product,s1->Son->Info);
				ist_multiply_left_and_right_bound_by_value(Product,invariant->weight_on_place[NuLayer]);
				ist_add_interval_to_interval(LINK->intersect,Product);
				/*
				 * We can only stop iff we are over the value and there is no intersection
				 */
				if (ist_greater_value(LINK->intersect->Left,invariant->m0_p->Right)) {
					stop = true;
				} else {
					/*
					 * We store the interval, in a univoque way inside a 32 bit field.
					 * We take as asumption that -> Left and ->Right < 2^16.
					 */
					temp = (0x0000ffff & LINK->intersect->Left);
					temp =  (LINK->intersect->Right == INFINITY ?
							0x0000ffff : 0x0000ffff & LINK->intersect->Right) | (temp << 16);
					LINK->memo = ist_get_memoization1(s1->Son, (ISTNode*)temp);
					/*
					 * Here, we play with fire ... I explain. We take as asumption that our values
					 * won´t be over 2^16 - 2 (2 because infinite is encoded as 2^16 - 1)
					 * which is a reasonable asumption.
					 * In the case of infinity we encode 0xffff as value. We take also as asumption
					 * that long is over 32 bit
					 */
					if (LINK->memo != NULL)
						rchild = LINK->memo->r;
					else

						rchild = IntersectionWithInvar(s1->Son, invariant, height, NuLayer + 1 , LINK);
					if (rchild != NULL)
						ist_add_son(rnode, rchild);
				}
				ist_sub_interval_to_interval(LINK->intersect,Product);
			} else {
				temp = (0x0000ffff & LINK->intersect->Left);
				temp =  (LINK->intersect->Right == INFINITY ?
						0x0000ffff : 0x0000ffff & LINK->intersect->Right) | (temp << 16);
				LINK->memo = ist_get_memoization1(s1->Son, (ISTNode*)temp);

				if (LINK->memo != NULL)
					rchild = LINK->memo->r;
				else
					rchild = IntersectionWithInvar(s1->Son, invariant, height, NuLayer + 1, LINK);
				if (rchild != NULL)
					ist_add_son(rnode, rchild);
			}
			s1 = s1->Next;
		}
		LINK->rlayer = LINK->rlayer->Previous;
		if (rnode->FirstSon != NULL)
			rnode = ist_add_node(LINK->rlayer, rnode);
		else {
			ist_dispose_node(rnode);
			rnode = NULL;
		}
	}
	ist_dispose_info(Product);
	temp = (0x0000ffff & LINK->intersect->Left);
	temp =  (LINK->intersect->Right == INFINITY ?
			0x0000ffff : 0x0000ffff & LINK->intersect->Right) | (temp << 16);
	ist_put_memoization1(node, (ISTNode *)temp, rnode);
	return rnode;
}

ISTSharingTree *ist_intersection_with_invar(ST1, invariant, height)
	ISTSharingTree *ST1;
	invariant_t *invariant;
	integer16 height;
{
	struct LOC_ist_method  V;
	ISTSon *s1;
	ISTNode *rchild;
	boolean stop;
	ISTSharingTree *WITH;
	ISTInterval *Product;

	ist_new(&V.STR);
	V.intersect = ist_build_interval(0,0);
	Product = ist_new_info();
	ist_new_memo1_number();
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = ST1->Root->FirstSon;
	stop = false;
	while (s1 != NULL && !stop) {
		if (invariant->weight_on_place[0] != 0){
			ist_assign_interval_to_interval(Product,s1->Son->Info);
			ist_multiply_left_and_right_bound_by_value(Product,invariant->weight_on_place[0]);
			ist_add_interval_to_interval(V.intersect,Product);
			/*
			 * We can only stop iff we are over  the value in this case there is no intersection
			 * In that case we know that it will be impossible to have an intersection afterwards
			 * because the list on sons is sorted !
			 */
			if (ist_greater_value(V.intersect->Left,invariant->m0_p->Right)) {
				stop = true;

			} else {

				rchild = IntersectionWithInvar(s1->Son, invariant, height, 1L, &V);
				if (rchild != NULL) {
					ist_add_son(V.STR->Root, rchild);
				}

			}
			if(!stop)
				ist_sub_interval_to_interval(V.intersect,Product);
		} else {
			rchild = IntersectionWithInvar(s1->Son, invariant, height, 1L, &V);
			if (rchild != NULL) {
				ist_add_son(V.STR->Root, rchild);
			}
		}
		s1 = s1->Next;
	}
	WITH = V.STR;
	stop = false;
	while (!stop) {
		if (WITH->LastLayer == NULL) {
			stop = true;
		} else {
			if (WITH->LastLayer->FirstNode != NULL)
				stop = true;
			else
				ist_remove_last_layer(V.STR);
		}
	}
	ist_dispose_info(V.intersect);
	ist_dispose_info(Product);
	return V.STR;
}


ISTSharingTree *ist_remove_with_all_invar_exact(S, system)
    ISTSharingTree *S;
	transition_system_t *system;
{
    long NuInv;
    ISTSharingTree *Sol, *Temp;

    Sol = ist_copy(S);
    NuInv = 0;
    while ((NuInv < system->limits.nbr_invariants) && (ist_is_empty(Sol) == false)) {
		/*
		 * IST_m0_p should be deleted and replaced by system->invariants[NuInv]->m0_p
		 */
		Temp = ist_intersection_with_invar(Sol, &system->invariants[NuInv], system->limits.nbr_variables);
		ist_dispose(Sol);
		Sol = Temp;
		NuInv++;
    }
    return Sol;
}
