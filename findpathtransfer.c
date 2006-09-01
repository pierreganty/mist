// vim:sw=4:ts=4
/*
   This file is part of mist2.

   mist2 is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   mist2 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mist2; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Copyright 2004, Laurent Van Begin, Pierre Ganty
 */

#include "findpathtransfer.h"
#include "xmalloc.h"

ISTNode * StISTIntersectionNode(ISTNode * node1, ISTNode * node2, ISTLayer * rlayer, ISTSharingTree * STR) {
  ISTSon * s1, * s2;
  ISTNode * rnode, * rchild;
  TMemo1 * memo;
  
  if(ist_equal_interval(node1->Info,&IST_end_of_list)) {
    rnode = ist_add_node(rlayer,ist_create_node(&IST_end_of_list));
  } else {
         rnode = ist_create_node(node1->Info);
         rlayer = rlayer->Next;
         if (rlayer == NULL) {
           rlayer = ist_add_last_layer(STR);
	 }
         s1 = node1->FirstSon;
         while (s1 != NULL) {
           s2 = node2->FirstSon;
           while (s2 != NULL) {
             if (s1->Son->Info->Left >= s2->Son->Info->Left) {
                    memo = ist_get_memoization1(s1->Son,s2->Son);
                    if (memo != NULL) {
                      rchild = memo->r;
	            }
                    else {
                      rchild = StISTIntersectionNode(s1->Son,s2->Son,rlayer,STR);
		    }
                    if (rchild != NULL) {
                      ist_add_son(rnode,rchild);
		    }
                    s2 = s2->Next;
	     } else {
               s2 = NULL;
	     }
	   }
           s1 = s1->Next;
         } 
         rlayer = rlayer->Previous;
         if (rnode->FirstSon != NULL) {
           rnode = ist_add_node(rlayer,rnode);
	 } else {
           ist_dispose_node(rnode);
           rnode = NULL;
         }
       }
  ist_put_memoization1(node1,node2,rnode);
  return rnode;
}


//Compute the intersection between a sharing tree and an interval sharing tree
ISTSharingTree * StISTIntersection(ISTSharingTree * St, ISTSharingTree * IST) {
  ISTSharingTree * STR;
  ISTLayer * rlayer;
  ISTSon * s1, * s2;
  ISTNode * rchild;
  int stop;

  ist_new(&STR);
  ist_new_memo1_number();
  rlayer = ist_add_last_layer(STR);
  s1 = St->Root->FirstSon;
  while (s1 != NULL) {
    s2 = IST->Root->FirstSon;
    while (s2 != NULL) {
      if (s1->Son->Info->Left >= s2->Son->Info->Left) {
             rchild = StISTIntersectionNode(s1->Son,s2->Son,rlayer,STR);
             if (rchild != NULL) {
                    ist_add_son(STR->Root,rchild);
             }
             s2 = s2->Next;
      } else s2 = NULL;
    }
    s1 = s1->Next;
  } 

  stop = 0;
  while (stop == 0) {
    if (STR->LastLayer == NULL) { 
      stop = 1;
    } else 
    if (STR->LastLayer->FirstNode != NULL) {
      stop = 1;
    } else {
      ist_delete_last_layer(STR);
    }
  }
  STR->NbElements = STR->Root->AuxI;
  return STR;
}


//put a path from the initial marking to a marking of the last covering sharing tree such that the ith intermediate marking is included in the ith covering sharing tree
void PutPathOnScreen(ISTInterval **initial_marking,THeadListIST * list_ist, transition_system_t * rules) {
  ISTSharingTree * S;
  ISTSharingTree * post;
  ISTSharingTree * Siter;
  ISTSharingTree * intersect;
  int b;
  int Continue;
  int i;

  printf("Path to reach the set of target states:\n");
  
  ist_new(&S);
  b = ist_add(S,initial_marking,rules->limits.nbr_variables);
  Siter = ist_first_element_list_ist(list_ist);
  while (Siter != NULL) {
    i = 0;
    Continue = 1;
    while ((i < rules->limits.nbr_rules) && (Continue == 1)) {
      post = ist_post_transition(S,rules,i);
      intersect = StISTIntersection(post,Siter);
      if (!ist_is_empty(intersect)) { 
        printf(" --> %d",i);
        ist_dispose(S);
        S = intersect;
        Continue = 0;
      } else { 
        ist_dispose(intersect);
        ist_dispose(post);
        i = i + 1;
      } 
    } 
    if (i == rules->limits.nbr_rules) { 
      printf("\nNo Path Found\n");
      Siter = NULL;
    } else {
      Siter = ist_next_element_list_ist(list_ist);
    }
  }
}


ISTInterval ** InstanciateParametricMarking(ISTInterval ** parametric_marking, ISTSharingTree * S) { 
  ISTLayer * layer;
  ISTNode * node;
  ISTInterval ** instance;
  int nu_layer;
  ISTSon *son;
  int b; 
  ISTSharingTree * initial_marking_ist;
  int old_magic_number;
  int ok;
  int height;
  
  ist_new_magic_number();
  old_magic_number = ist_get_magic_number();
  S->Root->Mark = old_magic_number;
  nu_layer = 0;
  ist_is_initial_marking_reached_node(parametric_marking[nu_layer],S->Root);
  nu_layer++;
  layer = S->FirstLayer;
  height = ist_nb_layers(S) - 1;
  while (nu_layer < height) {
	  ist_is_initial_marking_reached_layer(parametric_marking[nu_layer], layer);
	  nu_layer++;
	  layer = layer->Next;
  }

  ist_new(&initial_marking_ist);
  b = ist_add(initial_marking_ist,parametric_marking,nu_layer);
  ist_new_magic_number();
  node = S->LastLayer->Previous->FirstNode;
  while (node != NULL) {
	  if (node->Mark == old_magic_number) {
		  node->Mark = ist_get_magic_number();
		  node = NULL;
	  }
	  else {
		  node = node->Next;
	  }
  } 
  layer = S->LastLayer->Previous->Previous;
  instance = (ISTInterval **)xmalloc((nu_layer) * sizeof(ISTInterval *));
  nu_layer = nu_layer - 2;
  while (layer != NULL) {
    node = layer->FirstNode;
    while (node != NULL) {
      if (node->Mark == old_magic_number) {

	      son = node->FirstSon;
	      ok = 0;
	      while (son != NULL) {
		      if (son->Son->Mark == ist_get_magic_number()) {
			      ok = 1;
			      node->Mark = ist_get_magic_number();
			      instance[nu_layer] = ist_copy_interval(node->Info);
			      instance[nu_layer]->Right = instance[nu_layer]->Left;
			      son = NULL;
		      } else {
			      son = son->Next;
		      }
	      }
	      if (ok == 1) {
		      node = NULL;
	      } else {
		      node = node->Next;
	      }
      } else {
	      node = node->Next;
      }
    }
    nu_layer--;
    layer = layer->Previous;
  }
  return instance;
}


void ist_put_path_on_screen_with_parameterized_initial_marking(ISTSharingTree * initial_marking,THeadListIST * list_ist, transition_system_t * rules) {
  ISTSharingTree * S;
  ISTInterval ** instance;
  THeadListIST * copy;
  int i;
  ISTInterval ** marking;
  ISTLayer * layer;

  layer = initial_marking->FirstLayer;
  marking = (ISTInterval **)xmalloc(rules->limits.nbr_variables * (sizeof(ISTInterval *)));
  for(i = 0; i < rules->limits.nbr_variables; i++) {
    marking[i] = ist_copy_interval(layer->FirstNode->Info);
    layer = layer->Next;
  } 
  copy = ist_copy_list_ist(list_ist);
  S = ist_first_element_list_ist(copy);
  while (!ist_is_initial_marking_reached(initial_marking,S)) {
    S = ist_first_element_list_ist(copy);
    ist_remove_list_ist(copy,S);
  }
  instance = InstanciateParametricMarking(marking,S);
  PutPathOnScreen(instance,copy,rules);
  printf("\n");
}

