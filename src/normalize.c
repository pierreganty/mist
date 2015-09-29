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

#include "normalize.h"
#include "checkup.h"

void ist_merge_sons(Source, Target)
	ISTNode *Source, *Target;
{
	ISTSon *S;

	S = Source->FirstSon;
	while (S != NULL) {
		if (!ist_has_son(Target, S->Son))
			ist_add_son(Target, S->Son);
		S = S->Next;
	}
}


void ist_adjust_first_condition(S)
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
				ist_remove_node(CurLayer, SaveN);
			} else {
				CopyN = ist_create_node(CurN->Info);
				val = CurN->FirstSon->Son->Info;
				CopyS = ist_create_node(val);
				ist_copy_sons(CurN->FirstSon->Son, CopyS);
				ist_remove_son(CurN, CurN->FirstSon->Son);
				while (CurN->FirstSon != NULL) {
					if (ist_equal_interval(val,CurN->FirstSon->Son->Info)){
						ist_merge_sons(CurN->FirstSon->Son, CopyS);
						ist_remove_son(CurN, CurN->FirstSon->Son);
					} else {
						ist_add_son(CopyN, ist_add_node(NextLayer, CopyS));
						val = CurN->FirstSon->Son->Info;
						CopyS = ist_create_node(val);
						ist_copy_sons(CurN->FirstSon->Son, CopyS);
						ist_remove_son(CurN, CurN->FirstSon->Son);
					}
				}
				NewN = ist_add_node(NextLayer, CopyS);
				ist_add_son(CopyN, NewN);
				CurN->FirstSon = CopyN->FirstSon;
				CopyN->FirstSon = NULL;
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
			ist_remove_node(CurLayer, SaveN);
		} else
			CurN = CurN->Next;
	}
}


void ist_adjust_second_condition(S)
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
					CurS->Son = CurS->Son->AuxP;
					CurS->Son->NbFathers++;
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
				ist_remove_node(NextLayer, SaveN);
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
				if (ist_equal_interval(CurN->Info,OtherN->Info) & ist_same_sons(CurN, OtherN)) {
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


	/*Here CurLayer = S->FirstLayer. We process NextLayer but we don't have to*/
	/*set the pointer of nodes in FirstLayer. In fact, we assume that I condition holds:*/
	/*there cannot be two sons of the root with same label!*/

	/*First step: redirect pointers of the sons of nodes of CurLayer*/

	CurN = CurLayer->FirstNode;
	while (CurN != NULL) {
		CurS = CurN->FirstSon;
		while (CurS != NULL) {
			if (CurS->Son->AuxP != NULL) {
				CurS->Son->NbFathers--;
				CurS->Son = CurS->Son->AuxP;
				CurS->Son->NbFathers++;
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
			ist_remove_node(NextLayer, SaveN);
		} else
			CurN = CurN->Next;
	}

}


void ist_normalize(S)
	ISTSharingTree *S;
{
	ist_adjust_first_condition(S);
	ist_adjust_second_condition(S);
}
