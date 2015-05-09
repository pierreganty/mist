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

#include "minimizeinvarheuristic.h"
#include <stdlib.h>
#include "basis.h"
#include "remove.h"
#include "normalize.h"
/* DEPRECATED #include "globvar.h" */
/*return true is if the number of marks in a place constrained by the invariant is modified*/
#ifdef TRANSFERT
static boolean UseInvariant(NuInvar, NuRule, system)
	long NuInvar, NuRule;
	transition_system_t *system;
{
	size_t i;
	boolean Sol;
	ISTInterval *tau = ist_build_interval(0,INFINITY);

	Sol = false;
	i = 0;
	while (i < system->limits.nbr_variables && !Sol) {
		if (( system->transition[NuRule].cmd_for_place[i].delta != 0
					|| ist_not_equal_interval(&system->transition[NuRule].cmd_for_place[i].guard,tau))
				&& system->invariants[NuInvar].weight_on_place[i] != 0)
			Sol = true;
		else
			i++;
	}
	ist_dispose_info(tau);
	return Sol;
}
#endif

static void InitializeMinUp(Layer)
	ISTLayer *Layer;
{
	ISTNode *Node;

	Node = Layer->FirstNode;
	while (Node != NULL) {
		Node->MinUp = 0L;
		Node = Node->Next;
	}
}

static void InitializeMaxUp(Layer)
	ISTLayer *Layer;
{
	ISTNode *Node;

	Node = Layer->FirstNode;
	while (Node != NULL) {
		Node->MaxUp = 0;
		Node = Node->Next;
	}
}

/*
 * We assume that the sizes of S and invariant are the same (i.e. nbr_variables)
 */
static void ComputeMinUp(S, invariant)
	ISTSharingTree *S;
	invariant_t *invariant;
{
	size_t LastLayer, CurrentLayer;
	ISTLayer *Layer;
	ISTNode *Node;
	ISTSon *Son;
	long Mark;

	ist_new_magic_number();
	Mark = ist_get_magic_number();
	for (LastLayer = ist_nb_layers(S)-2; invariant->weight_on_place[LastLayer] == 0; LastLayer--)
		;

	Layer = S->FirstLayer;
	for (CurrentLayer = 0; invariant->weight_on_place[CurrentLayer] == 0; CurrentLayer++)
		Layer = Layer->Next;

	InitializeMinUp(Layer);

	while (CurrentLayer < LastLayer) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			Node->MinUp += invariant->weight_on_place[CurrentLayer] * Node->Info->Left;
			Son = Node->FirstSon;
			while (Son != NULL) {
				if (Son->Son->Mark != Mark) {
					Son->Son->Mark = Mark;
					Son->Son->MinUp = Node->MinUp;
				} else
					Son->Son->MinUp = min(Son->Son->MinUp, Node->MinUp);
				Son = Son->Next;
			}
			Node = Node->Next;
		}
		CurrentLayer++;
		Layer = Layer->Next;
	}

	Node = Layer->FirstNode;
	while (Node != NULL) {
		Node->MinUp += invariant->weight_on_place[LastLayer] * Node->Info->Left;
		Node = Node->Next;
	}

}

/*
 * We assume that the sizes of S and invariant are the same (i.e. nbr_variables)
 */
static  void ComputeMaxUp(S, invariant)
	ISTSharingTree *S;
	invariant_t *invariant;
{
	size_t LastLayer, CurrentLayer;
	ISTLayer *Layer;
	ISTNode *Node;
	ISTSon *Son;
	long Mark;

	ist_new_magic_number();
	Mark = ist_get_magic_number();
	for (LastLayer = ist_nb_layers(S)-2; invariant->weight_on_place[LastLayer] == 0; LastLayer--)
		;

	Layer = S->FirstLayer;
	for (CurrentLayer = 0; invariant->weight_on_place[CurrentLayer] == 0; CurrentLayer++)
		Layer = Layer->Next;

	InitializeMaxUp(Layer);

	while (CurrentLayer < LastLayer) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			if (Node->Info->Right != INFINITY)
				Node->MaxUp += invariant->weight_on_place[CurrentLayer] * Node->Info->Right;
			else
				Node->MaxUp = INFINITY;
			Son = Node->FirstSon;
			while (Son != NULL) {
				if (Son->Son->Mark != Mark) {
					Son->Son->Mark = Mark;
					Son->Son->MaxUp = Node->MaxUp;
				} else
					Son->Son->MaxUp = max(Son->Son->MaxUp, Node->MaxUp);
				Son = Son->Next;
			}
			Node = Node->Next;
		}
		CurrentLayer++;
		Layer = Layer->Next;
	}

	Node = Layer->FirstNode;
	while (Node != NULL) {
		if (Node->Info->Right != INFINITY)
			Node->MaxUp += invariant->weight_on_place[LastLayer] * Node->Info->Right;
		else
			Node->MaxUp = INFINITY;
		Node = Node->Next;
	}

}

static long ComputeMinMinDownSon(Node)
	ISTNode *Node;
{
	ISTSon *Son;
	long Sol;

	Son = Node->FirstSon;
	Sol = Son->Son->MinDown;
	Son = Son->Next;
	while (Son != NULL) {
		Sol = min(Sol, Son->Son->MinDown);
		Son = Son->Next;
	}
	return Sol;
}

static long ComputeMaxMaxDownSon(Node)
	ISTNode *Node;
{
	ISTSon *Son;
	long Sol;

	Son = Node->FirstSon;
	Sol = Son->Son->MaxDown;
	Son = Son->Next;
	while (Son != NULL) {
		Sol = max(Sol, Son->Son->MaxDown);
		Son = Son->Next;
	}
	return Sol;
}

static void ComputeMinDown(S, invariant)
	ISTSharingTree *S;
	invariant_t *invariant;
{
	/* CurrentLayer is not a size_t type because in some cases it can go under 0 */
	integer16 CurrentLayer, FirstLayer;
	ISTLayer *Layer;
	ISTNode *Node;

	for (FirstLayer = 0; invariant->weight_on_place[FirstLayer] == 0; FirstLayer++)
		;

	Layer = S->LastLayer->Previous;
	/* Minus 1 because there is one layer more than variable, minus 2 because in vectors we start from 0 */
	for (CurrentLayer = ist_nb_layers(S)-2; invariant->weight_on_place[CurrentLayer] == 0; CurrentLayer--)
		Layer = Layer->Previous;

	Node = Layer->FirstNode;
	while (Node != NULL) {
		Node->MinDown = Node->Info->Left * invariant->weight_on_place[CurrentLayer];
		Node = Node->Next;
	}

	CurrentLayer--;
	Layer = Layer->Previous;

	while (CurrentLayer >= FirstLayer) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			Node->MinDown = Node->Info->Left * invariant->weight_on_place[CurrentLayer] + ComputeMinMinDownSon(Node);
			Node = Node->Next;
		}
		CurrentLayer--;
		Layer = Layer->Previous;
	}
}

static void ComputeMaxDown(S, invariant)
	ISTSharingTree *S;
	invariant_t *invariant;
{
	integer16 CurrentLayer, FirstLayer;
	ISTLayer *Layer;
	ISTNode *Node;

	for (FirstLayer = 0; invariant->weight_on_place[FirstLayer] == 0; FirstLayer++)
		;

	Layer = S->LastLayer->Previous;
	for (CurrentLayer = ist_nb_layers(S)-2; invariant->weight_on_place[CurrentLayer] == 0; CurrentLayer--)
		Layer = Layer->Previous;


	Node = Layer->FirstNode;
	while (Node != NULL) {
		if (Node->Info->Right != INFINITY)
			Node->MaxDown = invariant->weight_on_place[CurrentLayer] * Node->Info->Right;
		else
			Node->MaxDown = INFINITY;
		Node = Node->Next;
	}

	CurrentLayer--;
	Layer = Layer->Previous;

	while (CurrentLayer >= FirstLayer) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			if (Node->Info->Right != INFINITY)
				Node->MaxDown = (invariant->weight_on_place[CurrentLayer] * Node->Info->Right) + ComputeMaxMaxDownSon(Node);
			else
				Node->MaxDown = INFINITY;
			Node = Node->Next;
		}
		CurrentLayer--;
		Layer = Layer->Previous;
	}
}


static boolean ist_reduce_with_invar_heuristic(S, invariant)
	ISTSharingTree *S;
	invariant_t *invariant;
{
	boolean Reduce;
	ISTLayer *Layer;
	ISTNode *Node;
	ISTSon *Son, *NextSon;

	size_t FirstLayer, LastLayer, CurrentLayer;

	Reduce = false;

	Layer = S->FirstLayer;
	for (FirstLayer = 0; invariant->weight_on_place[FirstLayer] == 0; FirstLayer++)
		Layer = Layer->Next;

	for (LastLayer = ist_nb_layers(S)-2; invariant->weight_on_place[LastLayer] == 0; LastLayer--)
		;

	CurrentLayer = FirstLayer;
	while (CurrentLayer < LastLayer) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			Son = Node->FirstSon;
			while (Son != NULL) {

				if (ist_greater_value(Node->MinUp + Son->Son->MinDown , invariant->m0_p->Right)
						|| (ist_less_value(ist_add_value(Node->MaxUp , Son->Son->MaxDown) , invariant->m0_p->Left))){
					/*
					 * If one of these two conditions holds
					 * min_<(e) + min_>(e) > p^T . m_o
					 * max_<(e) + max_>(e) < p^T . m_o
					 * We can remove the edge
					 */
					Reduce = true;
					NextSon = Son->Next;
					ist_remove_son(Node, Son->Son);
					Son = NextSon;
				} else
					Son = Son->Next;
			}
			Node = Node->Next;
		}
		CurrentLayer++;
		Layer = Layer->Next;
	}
	ist_remove_node_without_father(S);
	ist_remove_node_without_son(S);
	/* take care of first or second condition that might be violated */
	return Reduce;
}


void ist_minimize_invar_heuristic(S, invariant)
	ISTSharingTree *S;
	invariant_t *invariant;
{
	if (ist_is_empty(S) == false) {
		ComputeMinUp(S, invariant);
		ComputeMaxUp(S, invariant);
		ComputeMinDown(S, invariant);
		ComputeMaxDown(S, invariant);
		ist_reduce_with_invar_heuristic(S, invariant);
	}
}


/*procedure removing the markings greater than a marking satisfying an invariant*/
void ist_remove_with_invar_heuristic(S, NuRule, system)
	ISTSharingTree *S;
	long NuRule;
	transition_system_t *system;
{
	size_t NuInv;

	NuInv = 0;
	while (NuInv < system->limits.nbr_invariants) {
#ifdef TRANSFERT
		if (UseInvariant(NuInv, NuRule, system) == true) {
			ist_minimize_invar_heuristic(S, &system->invariants[NuInv]);
			if (ist_is_empty(S) == false)
				NuInv++;
			else
				NuInv = system->limits.nbr_invariants;
		} else
			NuInv++;
#endif
	}
	if (ist_is_empty(S) == false)
		ist_normalize(S);
}


void ist_remove_with_all_invar_heuristic(S, system)
	ISTSharingTree *S;
	transition_system_t *system;
{
	size_t NuInv;

	NuInv = 0;
	while ((NuInv < system->limits.nbr_invariants) && (ist_is_empty(S) == false)) {
		ist_minimize_invar_heuristic(S, &system->invariants[NuInv]);
		NuInv++;
	}
}


