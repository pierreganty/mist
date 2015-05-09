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

   Copyright 2003, 2004, Pierre Ganty, Anthony Piron
 */


#include "codegeninvar.h"
#include "xmalloc.h"
#include "tbsymbol.h"
#include "laparser.h"
#include "interval.h"

static void invariant(T_PTR_tree entry);
static void invariantor(T_PTR_tree entry);
static void invariantand(T_PTR_tree entry);

static size_t last; /* actual # of invariants coz some could be irrelevant */

static transition_system_t *_system;
static ISTInterval **_initM;

void
invariantscode_produce(T_PTR_tree entry, transition_system_t *system, ISTSharingTree *init)
{
  /* We need the initial marking to compute m0 times p */
  _initM = ist_firstpath2array(init);
  _system = system;
  invariant(entry);
}

static
void
invariant(T_PTR_tree entry) {
	size_t nbrinv;
	char* info;

	if (entry) {
		last = 0;
		info = (char*) tree_getinfo(entry);
		if (strcmp(info,"or") == 0) {
			_system->invariants = (invariant_t *)xmalloc(tree_nbrsubtrees(entry) *sizeof(invariant_t));
			for (nbrinv = 0 ; nbrinv < tree_nbrsubtrees(entry) ; nbrinv++) {
				_system->invariants[last].weight_on_place=NULL;
				invariantor(tree_subtree(entry,nbrinv));
			}

		} else if (strcmp(info,"and") == 0) {
			nbrinv = 0;
			_system->invariants = (invariant_t *)xmalloc(sizeof(invariant_t));
			_system->invariants[last].weight_on_place=NULL;
			invariantor(entry);
			nbrinv = 1;
		}
		_system->limits.nbr_invariants = last;
		/* We do not free the unsed invariant, it is useless to save such little memory */
	}
}


static
void
invariantor(T_PTR_tree entry) {
	size_t i;
	boolean overwrite;
	integer16 place;
	ISTInterval *Product = NULL;


	if (entry) {
		/*
		 * Note the use of realloc, this is useful if last is not incremented
		 * between 2 call to invariantor(). We should ensure the pointer
		 * passed in parameter is null at the first call
		 */
		_system->invariants[last].weight_on_place = (integer16 *)
			xrealloc(_system->invariants[last].weight_on_place, nbr_var*sizeof(integer16));
		/* We initialize or REinitialize to 0 */
		for (i = 0 ; i < nbr_var; i++)
			_system->invariants[last].weight_on_place[i] = 0;
		for (i = 0 ; i < tree_nbrsubtrees(entry) ; i++)
			invariantand(tree_subtree(entry,i));
		/* We check wether the invariant make sense or not: if _initM * p is defined then ++last */
		Product = ist_new_info();
		overwrite = false;
		/* A small memory leak here */
		_system->invariants[last].m0_p = ist_new_info();
		_system->invariants[last].m0_p->Left = 0;
		_system->invariants[last].m0_p->Right = 0;
		for (i = 0; i < nbr_var; ++i) {
			place = _system->invariants[last].weight_on_place[i];
			/*
			 * IST_m0_p[relevant] += _initM[j] * place;
			 * We test that the weight is not null AND we skip if the initial marking is
			 * PARAMETRIC for that place
			 */
			if (place > 0 && overwrite == false ) {
				if (_initM[i]->Right != INFINITY){
					ist_assign_interval_to_interval(Product,_initM[i]);
					ist_multiply_left_and_right_bound_by_value(Product,place);
					ist_add_interval_to_interval(_system->invariants[last].m0_p,Product);
				} else { /* We skip this invariants due to parametric marking */
					overwrite = true;
				}

			}
		}
		if (overwrite == false)
			++last;
	}
	ist_dispose_info(Product);
}


static
void
invariantand(T_PTR_tree entry) {
	T_PTR_tbsymbol_info infoid, infonb;
	char *info;

	if (entry) {
		if (tree_nbrsubtrees(entry)){
			info = (char *) tree_getinfo(entry);
			infoid = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 0)));
			infonb = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 1)));
			if (strcmp(info,"=") == 0){
				_system->invariants[last].weight_on_place[infoid->info.id.addr] = infonb->info.nb.value;
			}
		}
	}
}
