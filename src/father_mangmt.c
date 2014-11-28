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

#include "father_mangmt.h"
#include "computesim.h"
#include <stdlib.h>

/*remove the father Father of Son*/
void ist_remove_father(Father, Son)
	ISTNode *Father, *Son;
{
	ISTSon *RemovedFather, *PrecFather;

	PrecFather = NULL;
	RemovedFather = Son->FirstFather;
	while (RemovedFather != NULL) {
		if (RemovedFather->Son == Father) {
			if (PrecFather == NULL)
				Son->FirstFather = RemovedFather->Next;
			else
				PrecFather->Next = RemovedFather->Next;
			Father->NbSons--;
			ist_dispose_son(RemovedFather);
			RemovedFather = NULL;
		} else {
			PrecFather = RemovedFather;
			RemovedFather = RemovedFather->Next;
		}
	}
}


void ist_add_father(Father, Child)
	ISTNode *Father, *Child;
{
	/*this procedure is inspired from ist_add_son*/
	ISTSon *s, *sp;
	boolean stop;

	s = ist_new_son();   /*s will be used as a pointer towards the father*/
	s->Son = Father;
	Father->NbSons++;
	if (Child->FirstFather == NULL) {
		s->Next = NULL;
		Child->FirstFather = s;
	} else if (ist_greater_or_equal_interval(Child->FirstFather->Son->Info, s->Son->Info)) {
		s->Next = Child->FirstFather;
		Child->FirstFather = s;

	} else {
		sp = Child->FirstFather;
		stop = false;
		while (!stop) {
			if (sp->Next == NULL) {
				stop = true;
			} else {
				if (ist_greater_or_equal_interval(sp->Next->Son->Info,s->Son->Info))
					stop = true;
				else
					sp = sp->Next;
			}
		}
		s->Next = sp->Next;
		sp->Next = s;
	}
}

ISTNode *st_AddNodeF(layer, node)
	ISTLayer *layer;
	ISTNode *node;
	/* The call to RemoveSonsF, it's the only diff with ist_add_node */
{
	ISTNode  *nodep, *nodeq;
	if (layer->FirstNode == NULL) {
		layer->FirstNode = node;
		layer->LastNode = node;
	} else if (ist_greater_interval(layer->FirstNode->Info,node->Info)){
		node->Next = layer->FirstNode;
		layer->FirstNode = node;
	} else if (ist_less_interval(layer->LastNode->Info,node->Info)){
		layer->LastNode->Next = node;
		layer->LastNode = node;
	} else {
		nodep = layer->FirstNode;
		nodeq = NULL;
		while (nodep != NULL && ist_less_or_equal_interval(nodep->Info,node->Info)){
			if (ist_equal_interval(nodep->Info,node->Info)){
				/* We check for the validity of the condition 6 of [DRV02] */
				if (ist_same_sons(node, nodep)) {
					ist_remove_sons_fathers(node);
					ist_dispose_node(node);
					return nodep;
				}
			}
			nodeq = nodep;
			nodep = nodep->Next;
		}
		node->Next = nodep;
		nodeq->Next = node;
		if (layer->LastNode->Next != NULL)
			layer->LastNode = node;
	}
	return node;
}

void ist_add_son_father(node, child)
	ISTNode *node, *child;
{
	ISTSon *s, *sp;
	boolean stop;

	s = ist_new_son();
	s->Son = child;
	child->NbFathers++;
	if (node->FirstSon == NULL) {
		s->Next = NULL;
		node->FirstSon = s;
	} else if (ist_greater_or_equal_interval(node->FirstSon->Son->Info, child->Info)) {
		s->Next = node->FirstSon;
		node->FirstSon = s;
	} else {
		sp = node->FirstSon;
		stop = false;
		while (!stop) {
			if (sp->Next == NULL) {
				stop = true;
			} else {
				if (ist_greater_interval(sp->Next->Son->Info, child->Info))
					stop = true;
				else
					sp = sp->Next;
			}
		}

		s->Next = sp->Next;
		sp->Next = s;
	}
	ist_add_father(node, child);/*for on the fly computation of fathers links*/
}


void ist_remove_sons_fathers(node)
	ISTNode *node;
{
	ISTSon *s, *sn;

	s = node->FirstSon;
	while (s != NULL) {
		sn = s->Next;
		s->Son->NbFathers--;
		ist_remove_father(node, s->Son);/*for on the fly computation of fathers links*/
		ist_dispose_son(s);
		s = sn;
	}
	node->FirstSon = NULL;
}


void ist_copy_sons_fathers(orgnode, tgtnode)
	ISTNode *orgnode, *tgtnode;
{
	ISTSon *orgson, *tgtson, *lastson;

	orgson = orgnode->FirstSon;
	lastson = NULL;
	while (orgson != NULL) {
		tgtson = ist_new_son();
		tgtson->Son = orgson->Son;
		tgtson->Next = NULL;
		orgson->Son->NbFathers++;
		if (lastson == NULL)
			tgtnode->FirstSon = tgtson;
		else
			lastson->Next = tgtson;
		lastson = tgtson;
		ist_add_father(tgtnode, tgtson->Son);/*for on the fly computation of fathers links*/
		orgson = orgson->Next;
	}

}


void ist_remove_son_father(node, child)
	ISTNode *node, *child;
{
	ISTSon *s, *sp;

	sp = NULL;
	s = node->FirstSon;
	while (s != NULL && s->Son != child) {
		sp = s;
		s = s->Next;
	}

	if (sp == NULL)
		node->FirstSon = s->Next;
	else
		sp->Next = s->Next;
	ist_dispose_son(s);
	ist_remove_father(node, child);/*for on the fly computation of fathers links*/
	child->NbFathers--;
}


void ist_remove_fathers_sons(node)
	ISTNode *node;
{
	ISTSon *s, *sn;

	s = node->FirstFather;
	while (s != NULL) {
		sn = s->Next;
		ist_remove_son_father(s->Son, node);/*for on the fly computation of fathers links*/
		s = sn;
	}
	node->FirstFather = NULL;
}


void ist_remove_node_fathers_sons(layer, node)
	ISTLayer *layer;
	ISTNode *node;
{
	ISTNode *nodep;

	if (layer->FirstNode == node) {
		if (layer->LastNode == node) {
			layer->FirstNode = NULL;
			layer->LastNode = NULL;
		} else
			layer->FirstNode = node->Next;
	} else {
		nodep = layer->FirstNode;
		while (nodep->Next != node)
			nodep = nodep->Next;
		if (layer->LastNode == node) {
			nodep->Next = NULL;
			layer->LastNode = nodep;
		} else
			nodep->Next = node->Next;
	}
	/*for on the fly computation of fathers links*/
	ist_remove_sons_fathers(node);
	ist_remove_fathers_sons(node);
	/*Remove all the list used for the computation of the simulation relations*/
	DisposeRel(node);
	DisposeBackRel(node);
	ist_dispose_node(node);
}


void ist_construct_fathers_info(S)
	ISTSharingTree *S;
{
	ISTLayer *Layer, *PreviousLayer;
	ISTNode *Node;
	ISTSon *Son;

	/*Treatment of the first layer*/
	Layer = S->FirstLayer;
	Node = Layer->FirstNode;
	while (Node != NULL)
	{  /*each node of the first layer has the root as only father*/
		Node->FirstFather = ist_new_son();
		Node->FirstFather->Son = S->Root;


		S->Root->NbSons++;/*for on the fly computation of fathers links*/
		Node->FirstFather->Next = NULL;
		Node = Node->Next;
	}
	/*Treatment of the second layer and the remaining ones*/
	PreviousLayer = S->FirstLayer;
	Layer = PreviousLayer->Next;
	while (Layer != NULL) {
		Node = PreviousLayer->FirstNode;
		while (Node != NULL) {
			Son = Node->FirstSon;
			while (Son != NULL) {
				ist_add_father(Node, Son->Son);
				Son = Son->Next;
			}
			Node = Node->Next;
		}
		PreviousLayer = Layer;
		Layer = Layer->Next;
	}
}


void ist_dispose_fathers_info(S)
	ISTSharingTree *S;
{
	ISTLayer *Layer;
	ISTNode *Node;
	ISTSon *so;   /*pointer to a Father*/


	S->Root->NbSons = 0; /*for on the fly comutation of fathers links*/
	Layer = S->FirstLayer;
	while (Layer != NULL) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			/*for computation of simulation*/
			Node->NbSons = 0;
			/*---------------------------------------*/

			while (Node->FirstFather != NULL) {
				so = Node->FirstFather->Next;
				ist_dispose_son(Node->FirstFather);
				Node->FirstFather = so;
			}
			Node = Node->Next;
		}
		Layer = Layer->Next;
	}
}


void ist_merge_sons_fathers(Source, Target)
	ISTNode *Source, *Target;
{
	ISTSon *S;

	S = Source->FirstSon;
	while (S != NULL) {
		if (!ist_has_son(Target, S->Son))
			ist_add_son_father(Target, S->Son);
		S = S->Next;
	}
}


void ist_adjust_first_condition_sons_fathers(S)
	ISTSharingTree *S;
{
	ISTLayer *CurLayer, *NextLayer;
	ISTNode *CurN, *CopyN, *CopyS, *SaveN, *NewN;
	ISTInterval* val;

	CurLayer = NULL;
	NextLayer = S->FirstLayer;
	CurN = S->Root;
	while (NextLayer != S->LastLayer) {
		while (CurN != NULL) {
			if (CurN != S->Root && CurN->NbFathers == 0) {
				SaveN = CurN;
				CurN = CurN->Next;
				ist_remove_node_fathers_sons(CurLayer, SaveN);
			}
			else {
				CopyN = ist_create_node(CurN->Info);
				val = CurN->FirstSon->Son->Info;
				CopyS = ist_create_node(val);
				ist_copy_sons_fathers(CurN->FirstSon->Son, CopyS);
				ist_remove_son_father(CurN, CurN->FirstSon->Son);
				while (CurN->FirstSon != NULL) {
					if (ist_equal_interval(val,CurN->FirstSon->Son->Info)){
						ist_merge_sons_fathers(CurN->FirstSon->Son, CopyS);
						ist_remove_son_father(CurN, CurN->FirstSon->Son);
					} else {
						ist_add_son_father(CopyN, st_AddNodeF(NextLayer, CopyS));
						val = CurN->FirstSon->Son->Info;
						CopyS = ist_create_node(val);
						ist_copy_sons_fathers(CurN->FirstSon->Son, CopyS);
						ist_remove_son_father(CurN, CurN->FirstSon->Son);
					}
				}
				NewN = st_AddNodeF(NextLayer, CopyS);
				ist_add_son_father(CopyN, NewN);
				ist_copy_sons_fathers(CopyN,CurN);
				ist_remove_sons_fathers(CopyN);
				/*
				 * We cannot do the same operation
				 * than in the case without the father field
				 * (see the 2 intructions in commentar above)
				 * We have to modify also the father field
				 * so we use more complex operation
				 */
				ist_dispose_node(CopyN);
				CurN = CurN->Next;
			}
		}
		CurLayer = NextLayer;
		CurN = CurLayer->FirstNode;
		NextLayer = NextLayer->Next;
	}
	while (CurN != NULL) {
		if (CurN->NbFathers == 0) {
			SaveN = CurN;
			CurN = CurN->Next;
			ist_remove_node_fathers_sons(CurLayer, SaveN);
		} else
			CurN = CurN->Next;
	}
}

void ist_adjust_second_condition_sons_fathers(S)
	ISTSharingTree *S;
{
	ISTLayer *CurLayer, *NextLayer;
	ISTNode *CurN, *OtherN, *SaveN;
	ISTSon *CurS;
	boolean stop;

	/*The auxiliary pointers of NextLayer must always be well-positioned*/
	/*at the beginning NextLayer contains only bottom*/

	CurLayer = S->LastLayer->Previous;
	NextLayer = S->LastLayer;
	NextLayer->FirstNode->AuxP = NULL;

	while (CurLayer != S->FirstLayer) {
		CurN = CurLayer->FirstNode;
		while (CurN != NULL) {
			CurS = CurN->FirstSon;
			while (CurS != NULL) {
				if (CurS->Son->AuxP != NULL) {
					CurS->Son->NbFathers--;

					/*for on the fly computation of fathers*/
					ist_remove_father(CurN, CurS->Son);
					/*-------------------------------------*/

					CurS->Son = CurS->Son->AuxP;
					CurS->Son->NbFathers++;

					/*for on the fly computation of fathers*/
					ist_add_father(CurN, CurS->Son);
					/*-------------------------------------*/

				}
				CurS = CurS->Next;
			}
			CurN = CurN->Next;
		}

		/*Second Step: removal of nodes in NextLayer with AuxP<>nil*/
		CurN = NextLayer->FirstNode;
		while (CurN != NULL) {
			if (CurN->AuxP != NULL) {
				SaveN = CurN;
				CurN = CurN->Next;
				ist_remove_node_fathers_sons(NextLayer, SaveN);
			} else
				CurN = CurN->Next;
		}

		/*Third Step: set the pointers of the nodes of CurLayer*/

		CurN = CurLayer->FirstNode;
		while (CurN != NULL) {
			stop = false;
			CurN->AuxP = NULL;   /*all pointers must be well-positioned!*/
			OtherN = CurLayer->FirstNode;
			while (CurN != OtherN && !stop) {
				if (ist_equal_interval(CurN->Info, OtherN->Info) & ist_same_sons(CurN, OtherN)) {
					CurN->AuxP = OtherN;
					stop = true;
				} else
					OtherN = OtherN->Next;
			}
			CurN = CurN->Next;
		}

		NextLayer = CurLayer;
		CurLayer = CurLayer->Previous;
	}
	/*First step: redirect pointers of the sons of nodes of CurLayer*/


	/*
	 * Here CurLayer = S->FirstLayer.
	 * We process NextLayer but we don't have to
	 * set the pointer of nodes in FirstLayer.
	 * In fact, we assume that I condition holds:
	 * there cannot be two sons of the root with same label!
	 */

	/*First step: redirect pointers of the sons of nodes of CurLayer*/
	CurN = CurLayer->FirstNode;
	while (CurN != NULL) {
		CurS = CurN->FirstSon;
		while (CurS != NULL) {
			if (CurS->Son->AuxP != NULL) {
				CurS->Son->NbFathers--;

				/*for on the fly computation of fathers*/
				ist_remove_father(CurN, CurS->Son);
				/*-------------------------------------*/

				CurS->Son = CurS->Son->AuxP;
				CurS->Son->NbFathers++;

				/*for on the fly computation of fathers*/
				ist_add_father(CurN, CurS->Son);
				/*-------------------------------------*/

			}
			CurS = CurS->Next;
		}
		CurN = CurN->Next;
	}

	/*Second Step: removal of nodes in NextLayer with AuxP<>nil*/

	CurN = NextLayer->FirstNode;
	while (CurN != NULL) {
		if (CurN->AuxP != NULL) {
			SaveN = CurN;
			CurN = CurN->Next;
			ist_remove_node_fathers_sons(NextLayer, SaveN);
		} else
			CurN = CurN->Next;
	}

}


void ist_normalize_sons_fathers(S)
	ISTSharingTree *S;
{
	ist_adjust_first_condition_sons_fathers(S);
	ist_adjust_second_condition_sons_fathers(S);
}

/*Remove and dispose the nodes without fathers*/
void ist_dispose_node_without_father(S)
	ISTSharingTree *S;
{
	ISTLayer *Layer;
	ISTNode *Node, *NextNode;

	Layer = S->FirstLayer;
	while (Layer != NULL) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			if (Node->NbFathers == 0) {
				NextNode = Node->Next;
				ist_remove_node_fathers_sons(Layer, Node);
				Node = NextNode;
			} else
				Node = Node->Next;
		}
		Layer = Layer->Next;
	}
}


/*Eliminate the nodes in S that have no sons*/
void ist_dispose_node_without_son(S)
	ISTSharingTree *S;
{
	ISTLayer *Layer;
	ISTNode *Node, *NextNode;

	Layer = S->LastLayer->Previous;
	while (Layer != NULL) {
		Node = Layer->FirstNode;
		while (Node != NULL) {
			if (Node->NbSons == 0) {
				NextNode = Node->Next;
				ist_remove_node_fathers_sons(Layer, Node);
				Node = NextNode;
			} else {
				Node = Node->Next;
			}
		}
		Layer = Layer->Previous;
	}
}

