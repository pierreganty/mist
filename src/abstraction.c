// vim:sw=4:ts=4:cindent
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

   Copyright 2006, Pierre Ganty, 2007 Laurent Van Begin, 2015 Pedro Valero
 */

#include "abstraction.h"
#include "xmalloc.h"
#include "checkup.h"
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <sys/resource.h>

extern FILE *file;
extern int iterations;

// Global vars to compute the time used on each iteration.
struct rusage time_before, time_after;
int time_init=0;

transition_system_t * build_sys_using_abs(sys,abs)
	transition_system_t *sys;
	abstraction_t *abs;
{
	transition_system_t *retval;
	size_t i,j,k;

	retval=(transition_system_t *)xmalloc(sizeof(transition_system_t));
	retval->limits.nbr_rules = sys->limits.nbr_rules;
	retval->limits.nbr_variables = abs->nbV;
	/* Assignment for sanity of data structure */
	retval->limits.nbr_invariants=0;
	retval->transition=(transition_t *)xmalloc(retval->limits.nbr_rules*sizeof(transition_t));
	for(i=0;i<retval->limits.nbr_rules;++i){
		retval->transition[i].cmd_for_place = (gd_command_t *)xmalloc(abs->nbV*sizeof(gd_command_t));
		/* Assignment for sanity of data structure */
		retval->transition[i].nbr_transfers=0;
		for(j=0;j<retval->limits.nbr_variables;++j){
			/* We maintain for each place in the abstract net how many concrete ones it represent */
			retval->transition[i].cmd_for_place[j].places_abstracted=0;
			/* Start from bottom values for delta and guard */
			retval->transition[i].cmd_for_place[j].delta =0L;
			ist_assign_values_to_interval(&retval->transition[i].cmd_for_place[j].guard, 0L,  INFINITY);
			for(k=0;k<sys->limits.nbr_variables;++k){
				if(abs->A[j][k]>0){
					retval->transition[i].cmd_for_place[j].delta+= sys->transition[i].cmd_for_place[k].delta;
					ist_add_value_to_interval(&retval->transition[i].cmd_for_place[j].guard, sys->transition[i].cmd_for_place[k].guard.Left);
					++retval->transition[i].cmd_for_place[j].places_abstracted;
				}
			}
		}
	}
	return retval;
}

void dispose_abstraction(abstraction_t *abs)
{
	size_t i;
	if(abs->nbV > 0) {
		for(i=0;i<abs->nbV;++i)
			xfree(abs->A[i]);
		xfree(abs->bound);
	}
	xfree(abs);
}

void print_abstraction(abs)
	abstraction_t *abs;
{
	size_t i,j;
	printf("abs: %d variables.\n",abs->nbV);
	puts("abs: For the merging:");
	for(i=0;i<abs->nbV;++i){
		for(j=0;j<abs->nbConcreteV;++j)
			printf("%d",(int) abs->A[i][j]);
		puts("");
	}
	puts("abs: For the bounds:");
	for(i=0;i<abs->nbV;++i)
		printf("%d",(int) abs->bound[i]);
	puts("");

}

abstraction_t *glb(abstraction_t *abs1, abstraction_t *abs2)
{
	abstraction_t *retval;
	size_t i,j,k,rows,sum;
	assert(abs1->nbConcreteV==abs2->nbConcreteV);

	/* Count the number of sets in the resulting partition */
	rows=0;
	for (i=0;i<abs1->nbV;++i) {
		for (j=0;j<abs2->nbV;++j) {
			/* We look for an common place */
			for(k=0; k<abs1->nbConcreteV && abs1->A[i][k]+abs2->A[j][k]< 2; ++k);
			if(k != abs1->nbConcreteV)
				++rows;

		}

	}
	/* Memory allocation */
	retval=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* We copy the number of places of the original system into the abstraction */
	retval->nbConcreteV=abs1->nbConcreteV;
	/* We start with a unique abstract place */
	retval->nbV=rows;
	retval->bound=(integer16 *)xmalloc(retval->nbV*sizeof(integer16));
	retval->A=(integer16 **)xmalloc(retval->nbV*sizeof(integer16));
	for(i=0;i<retval->nbV;++i) {
		retval->A[i]=(integer16 *)xmalloc(retval->nbConcreteV*sizeof(integer16));
		retval->bound[i]=1;
	}
	rows=0;
	for (i=0;(i<abs1->nbV) && (rows < retval->nbV);++i) {
		for (j=0;(j<abs2->nbV) && (rows < retval->nbV);++j) {
			for(k=0; (k<abs1->nbConcreteV) && (rows < retval->nbV); ++k) {
				/* set to 1 if k is a common place of abs1[i] and abs2[j] */
				retval->A[rows][k]=(abs1->A[i][k]+abs2->A[j][k]>= 2 ? 1 : 0);
			}
			for(sum=0,k=0;k<retval->nbConcreteV;sum+=retval->A[rows][k++]);
			rows+=(sum>=1?1:0);
			assert(rows<=retval->nbV);
		}
	}
	return retval;
}

static int **partition_lines(int **matrix,int max_line, int max_row, int line1, int line2)
{
	int nb_var_ij =0;
	int i,j,l;
	int ** result;

	for(i=0;i<max_row;++i)
		nb_var_ij += matrix[line1][i] + matrix[line2][i];

	//allocation
	result = (int **)xmalloc((max_row - nb_var_ij + 1) * sizeof(int *));
	for(i = 0;i< max_row - nb_var_ij + 1;++i)
		result[i] = (int *)xmalloc(max_row * sizeof(int));

	//create the fuision of line1 and line2
	for(i=0; i < max_row;++i)
		result[0][i] = matrix[line1][i] + matrix[line2][i];
	//forall variables not in line1 and line2,
	//we create a singleton element in the partition
	for(i=0,l=1;i< max_row;++i) {
		if (result[0][i] == 0) {
			for(j=0;j < max_row;++j)
				result[l][j] = 0;
			result[l][i] = 1;
			++l;
		}
	}
	return result;
}

static int **line_fusion(int **matrix,int max_line, int max_row, int line1, int line2)
{
	int ** result;
	int i,j;//l;

	//allocation of memory
	result = (int **)xmalloc((max_line -1) * sizeof(int*));
	for(i=0;i < max_line -1;++i) {
		result[i] = (int *)xmalloc(max_row * sizeof(int));
	}
	// we assume that line1 < line2
	// and copy all the line < line 1
	for(i=0;i < line1;i++) {
		for(j=0;j<max_row;j++)
			result[i][j] = matrix[i][j];
	}
	//we replace line1 by the fusion of line 1 and line 2
	for(j=0;j<max_row;j++)
		result[line1][j] = matrix[line1][j] + matrix[line2][j];
	//we copy the lines between line 1 and line 2
	for(i=line1+1;i<line2;i++) {
		for(j=0;j< max_row;j++) {
			result[i][j] = matrix[i][j];
		}
	}
	//we copy the lines after line2
	for(i=line2+1;i < max_line;i++) {
		for(j=0;j<max_row;j++) {
			result[i-1][j] = matrix[i][j];
		}
	}
	return result;
}



//build a more general partition that defines an abstraction that allow the represent S exactly (naive implementation see ATPN'07)
abstraction_t *naive_new_abstraction(ISTSharingTree *S,int nb_var)
{
	int **result;
	abstraction_t * result_abs;
	int **tmp;
	ISTSharingTree * alpha_S;
	ISTSharingTree * approx_S;
	int  max_line,max_row, i,j,l, nb_var_ij;
	boolean found;
	abstraction_t abs;

	abs.nbConcreteV = nb_var;

	//allocation of memory + initialisation
	result = (int **)xmalloc(nb_var * sizeof(int*));
	for(i=0;i < nb_var;++i) {
		result[i] = (int *)xmalloc(nb_var * sizeof(int));
		for(j = 0;j< nb_var;++j)
			result[i][j] = 0;
		result[i][i] = 1;
	}
	max_line = nb_var;
	max_row = nb_var;

	found = true;
	while (found == true) {
		found = false;
		//we check lines pairwise to find a pair to fusion
		for(i = 0; (i < max_line) && (found == false);++i)
			for(j=i+1;(j<max_line) && (found == false);++j)
				if (i!=j) {

					tmp = partition_lines(result,max_line,max_row,i,j);
					nb_var_ij = 0;
					for(l=0;l<max_row;++l)
						nb_var_ij += result[i][l] + result[j][l];

					abs.A = tmp;
					abs.nbV = nb_var - nb_var_ij +1;


					alpha_S = ist_abstraction(S,&abs);
					approx_S = ist_concretisation(alpha_S,&abs);
					for(l = 0;l < abs.nbV;++l)
						xfree(abs.A[l]);
					xfree(abs.A);


					if (ist_exact_subsumption_test(approx_S,S) == true) {
						//we can build a more general partition, the old one useless
						//computation of the new partition
						tmp = line_fusion(result,max_line,max_row,i,j);
						for(l = 0;l< max_line;++l)
							xfree(result[l]);
						xfree(result);
						result = tmp;
						--max_line;
						found = true;


					}
					ist_dispose(alpha_S);
					ist_dispose(approx_S);
				}

	}
	result_abs = (abstraction_t *)xmalloc(sizeof(abstraction_t));
	result_abs->nbConcreteV = max_row;
	result_abs->nbV = max_line;
	result_abs->A = result;
	result_abs->bound = (int *)xmalloc(result_abs->nbV * sizeof(int));
	for(i=0;i < result_abs->nbV;++i)
		result_abs->bound[i] = 1;
	return result_abs;
}


static int *new_element_in_partition(int **matrix,int max_line, int max_row, int line1, int line2)
{
	size_t i;
	int *result;

	//allocation
	result = (int *)xmalloc(max_row * sizeof(int));

	//create the fuision of line1 and line2
	for(i=0; i < max_row;++i)
		result[i] = matrix[line1][i] + matrix[line2][i];
	return result;
}

static int *FindInfinitePlaces(ISTSharingTree *S,int nb_var)
{
	size_t i;
	ISTLayer * L;
	int *result=(int *)malloc(nb_var*(sizeof(int)));
	for(i=0,L = S->FirstLayer;i<nb_var;i++,L=L->Next)
		result[i]=(L->FirstNode->Info->Right == INFINITY) ? 1 : 0;
	return result;
}

static int *FindUnconstrainedPlaces(ISTSharingTree *S,int nb_var)
{
	size_t i;
	ISTLayer * L;
	int *result=(int *)malloc(nb_var*(sizeof(int)));
	for(i=0,L = S->FirstLayer;i<nb_var;i++,L=L->Next)
		result[i]=(L->LastNode->Info->Left == 0) ? 1 : 0;
	return result;
}


static boolean can_be_merged(int elem1, int elem2, abstraction_t *old_abs, int **new_abs) {
	int val1,val2;
	int old_elem1,old_elem2;

	//find a place p1 in the element 1 of the new partition
	for(val1=0;(val1< old_abs->nbConcreteV) && (new_abs[elem1][val1] == 0);val1++);
	//find a place in the element 2 of the new partition
	for(val2=0;(val2<old_abs->nbConcreteV) && (new_abs[elem2][val2] == 0);val2++);
	//find the element that contains p1 in the old partition
	for(old_elem1=0;(old_elem1< old_abs->nbV) && (old_abs->A[old_elem1][val1] == 0);old_elem1++);
	//find the element that contains p2 in the old partition
	for(old_elem2=0;(old_elem2< old_abs->nbV) && (old_abs->A[old_elem2][val2] == 0);old_elem2++);

	return (old_elem1 == old_elem2);
}

static boolean mergeable(int i,abstraction_t *old_abs,int **new_abs)
{
	boolean result;
	int k,l;

	//find a place of elem i
	for(k=0;(k< old_abs->nbConcreteV) && (new_abs[i][k] == 0);k++);

	//find the elem in old_abs that contains that place
	for(l=0;(l < old_abs->nbV) && (old_abs->A[l][k] == 0);l++);

	//test if it is possible to add places in elem i
	for(k=0,result=false;(k < old_abs->nbConcreteV) && (result == false);k++)
		if ((old_abs->A[l][k] == 1) && (new_abs[i][k] == 0))
			result = true;

	return result;
}


//Function that returns a path of the IST given as parameter
//Redundant w/ ist_firstpath2array
static ISTInterval **GiveMeAPath(ISTSharingTree *S)
{
	int nbvar = 0;
	ISTInterval ** result;
	ISTLayer * L;
	ISTNode * N;
	int i;

	for(L = S->FirstLayer;L != S->LastLayer;nbvar++,L = L->Next);
	result = (ISTInterval **)malloc(nbvar * sizeof(ISTInterval *));
	for(i=0,N = S->FirstLayer->FirstNode;N->FirstSon != NULL;N = N->FirstSon->Son,i++)
		result[i] = ist_copy_interval(N->Info);
	return result;
}

// Another binomial coefficient implementation (got it from the internet, http://rosettacode.org/wiki/Evaluate_binomial_coefficients#C).
typedef unsigned long ULONG;

ULONG choose(ULONG n, ULONG k)
{
	ULONG r = 1, d = n - k;

	/* choose the smaller of k and n - k */
	if (d > k) { k = d; d = n - k; }

	while (n > k) {
		if (r >= UINT_MAX / n) assert("binonial coefficient computation overflows" && false); /* overflown */
		r *= n--;

		/* divide (n - k)! as soon as we can to delay overflows */
		while (d > 1 && !(r % d)) r /= d--;
	}
	return r;
}


//that function tests if for all the paths there is no node in a layer given by Component
//that does not have INFINITY as right bound.
//return true if it holds
//return false otherwise
static boolean testINFINITY(ISTSharingTree * S,int *Component)
{
	ISTLayer * L;
	int i;
	boolean ok = true;

	i=0;
	L=S->FirstLayer;
	while((L->Next!=NULL) && ok){
		if (Component[i] > 0)
			ok = (L->FirstNode->Info->Right == INFINITY);
		i++;
		L = L->Next;
	}
	return ok;
}

//compute the sum of value appearing in the layers given by Component
//Component[i] = 0 == the layer is not considered
//Component[i] > 0 == the layer is considered
static int ValueInComponent(ISTInterval **V,int *Component,int dim)
{

	int i;
	int result = 0;

	for(i=0;i < dim;i++) {
		if (Component[i] > 0)
			result =  ist_add_value(result,V[i]->Right);
	}
	return result;
}

//used to compute all the path such that the sum of values appearing in layers given by Component is equal to val
static ISTNode *PathWithValueInComponent(ISTSharingTree * S, ISTNode * N,ISTLayer *L,int NuLayer,int * Component,int val,int sum)
{
	ISTNode *result;
	ISTSon *s;
	ISTNode *node;
	TMemo1 * memo;


	if (ist_equal_interval(N->Info,&IST_end_of_list)) {
		if(val == sum)
	        	result = ist_add_node(L, ist_create_node(&IST_end_of_list));
		else
			result = NULL;
	}else {
		L = L->Next;
		if(L == NULL) {
			L = ist_add_last_layer(S);
		}
		result = ist_create_node(ist_copy_interval(N->Info));


		for(s = N->FirstSon;s != NULL;s = s->Next) {
			if (Component[NuLayer] > 0) {
				if (ist_less_or_equal_value(ist_add_value(N->Info->Right,sum),val)){
					memo = ist_get_memoization1(s->Son,(ISTNode *) ist_add_value(N->Info->Right,sum));
					if (memo != NULL)
						node = memo->r;
					else {
						node = PathWithValueInComponent(S,s->Son,L,NuLayer+1,Component,val,ist_add_value(N->Info->Right,sum));
					}
				} else
					node = NULL;
			} else {
				memo = ist_get_memoization1(s->Son, (ISTNode *) sum);
				if (memo != NULL)
					node = memo->r;
				else
					node = PathWithValueInComponent(S,s->Son,L,NuLayer + 1,Component,val,sum);

			}
			if (node != NULL) {
				ist_add_son(result,node);
			}
		}

		L = L->Previous;
		if (result->FirstSon == NULL) {
			ist_dispose_node(result);
			result = NULL;
		} else
			result = ist_add_node(L,result);
	}
	ist_put_memoization1(N,(ISTNode *) sum,result);
	return result;
}


//That function returns an IST such that the paths are those of S such that the sum of the values
//in the layers given by Component is equal to val (we only consider right bound)
//Assumption: the left bound is equal to 0 for all the nodes
static ISTSharingTree *ist_PathsWithValueInComponent(ISTSharingTree * S,int * Component,int val)
{

	ISTSon *son;
	ISTSharingTree *result;
	ISTLayer *L;
	ISTNode *N;

	ist_new_magic_number();
    	ist_new_memo1_number();
	ist_new(&result);
	L = ist_add_last_layer(result);

	for(son = S->Root->FirstSon; son != NULL;son = son->Next) {
		N = PathWithValueInComponent(result,son->Son,L,0,Component,val,0);
		if (N != NULL)
			ist_add_son(result->Root,N);
	}
	return result;
}


//return true iff the partition of places that contains the set given by component and the simgletons
//for all the other places is precise enough to represent the tuple of S.
static boolean CanIRepresentExactlyTheDcSet(ISTSharingTree *S, int *Component)
{

	ISTSharingTree *Scopy, *T, *Q, *tmp;
	ISTInterval **Path;
	size_t dim, DimComp, i;
	int val;
	integer16 *complementComponent;
	boolean ok = true;

	Scopy=ist_copy(S);
	dim=ist_nb_layers(Scopy)-1;
	//compute the size of the set of places given by Component
	for(i=0,DimComp = 0; i< dim;i++) {
		if (Component[i] > 0)
			DimComp++;
	}

	while ((ist_is_empty(Scopy) == false) && ok) {
		Path = GiveMeAPath(Scopy);
		val = ValueInComponent(Path,Component,dim);
		//we take all the paths of Scopy where the places corresponding to Componenent contains val tokens
		T = ist_PathsWithValueInComponent(Scopy,Component,val);
		//tmp contains the other paths that must be still managed
		tmp=ist_minus(Scopy,T);
		ist_dispose(Scopy);
		Scopy=tmp;
		//if the number of tokens is INFINITY, then all the places in Component must contains
		//INFINITY tokens, otherwise the partition cannot represent the tuple of S
		if (val == INFINITY) {
			ok = (testINFINITY(T,Component) == true);
		} else {
			//otherwise we compute the number of possibilities to have val tokens in the places
			//given by component and we compute the number of possible sub-markings obtained by
			//removing the places of Component
			//Then, the product of the two values gives the number of tuples we must have
			complementComponent = (integer16 *)malloc((dim + 1) * sizeof(integer16));
			for(i=0;i<dim;i++)
				complementComponent[i] = (Component[i] > 0) ? 0 : 1;
			complementComponent[dim] = 1;
			Q = ist_projection(T,complementComponent);
			xfree(complementComponent);
			ok = (ist_nb_elements(Q) * choose(val + DimComp-1,DimComp-1) == ist_nb_elements(T));
			ist_dispose(Q);
		}
		ist_dispose(T);
	}
	ist_copy(Scopy);
	return ok;
}


//build a more general partition that defines an abstraction that allow the represent S exactly
//works for dc-sets only
abstraction_t *new_abstraction_dc_set(ISTSharingTree *S,int nb_var)
{
	int **result;
	abstraction_t * result_abs;
	int **tmp;
	int  max_line,max_row, i,j,l;
	boolean found;
	int *Component;
	int *infcomponent;

	//allocation of memory + initialisation
	result = (int **)xmalloc(nb_var * sizeof(int*));
	for(i=0;i < nb_var;++i) {
		result[i] = (int *)xmalloc(nb_var * sizeof(int));
		for(j = 0;j< nb_var;++j)
			result[i][j] = 0;
		result[i][i] = 1;
	}
	max_line = nb_var;
	max_row = nb_var;

	infcomponent = FindInfinitePlaces(S,nb_var);
	for(i=nb_var-1;(i >= 0) && (infcomponent[i] == 0);i--);
	if (i >= 0) {
		j=i-1;
		while(j>=0) {
			if (infcomponent[j] == 1) {
				tmp = line_fusion(result,max_line,max_row,j,i);
				for(l = 0;l< max_line;++l)
                                	xfree(result[l]);
                                xfree(result);
                                result = tmp;
				--max_line;
				i=j;
			}
			j--;
		}
	}
	xfree(infcomponent);

	found = true;
	while (found == true) {
		found = false;
		//we check lines pairwise to find a pair to fusion
		for(i = 0; (i < max_line);++i)
			for(j=i+1;(j<max_line);)
				if (i!=j) {
					Component=new_element_in_partition(result,max_line,max_row,i,j);
					if (CanIRepresentExactlyTheDcSet(S,Component) == true) {
						//we can build a more general partition, the old one useless
						//computation of the new partition
						tmp = line_fusion(result,max_line,max_row,i,j);
						for(l = 0;l< max_line;++l)
							xfree(result[l]);
						xfree(result);
						result = tmp;
						--max_line;
						found = true;

					} else j++; //when line i and line j are fusionned,
						//  the result is put at line i. Hence, in that case we do not have
						//  to increaese j
				}
	}
	result_abs = (abstraction_t *)xmalloc(sizeof(abstraction_t));
	result_abs->nbConcreteV = max_row;
	result_abs->nbV = max_line;
	result_abs->A = result;
	result_abs->bound = (int *)xmalloc(result_abs->nbV * sizeof(int));
	for(i=0;i < result_abs->nbV;++i)
		result_abs->bound[i] = 1;

	return result_abs;
}

//return true iff the partition of places that contains the set given by component and the simgletons
//for all the other places is precise enough to represent the tuple of S.
static boolean CanIRepresentExactlyTheFiniteSet(ISTSharingTree *S, int *Component)
{

	ISTSharingTree *Scopy, *T, *Q, *tmp;
	ISTInterval **Path;
	size_t dim, DimComp, i;
	int val;
	integer16 *complementComponent;
	boolean ok = true;

	Scopy=ist_copy(S);
	dim=ist_nb_layers(Scopy)-1;
	//compute the size of the set of places given by Component
	for(i=0,DimComp = 0; i< dim;i++) {
		if (Component[i] > 0)
			DimComp++;
	}

	while ((ist_is_empty(Scopy) == false) && ok) {
		Path = GiveMeAPath(Scopy);
		val = ValueInComponent(Path,Component,dim);
		//we take all the paths of Scopy where the places corresponding to Componenent contains val tokens
		T = ist_PathsWithValueInComponent(Scopy,Component,val);
		//tmp contains the other paths that must be still managed
		tmp=ist_minus(Scopy,T);
		ist_dispose(Scopy);
		Scopy=tmp;
		//otherwise we compute the number of possibilities to have val tokens in the places
		//given by component and we compute the number of possible sub-markings obtained by
		//removing the places of Component
		//Then, the product of the two values gives the number of tuples we must have
		complementComponent = (integer16 *)malloc((dim + 1) * sizeof(integer16));
		for(i=0;i<dim;i++)
			complementComponent[i] = (Component[i] > 0) ? 0 : 1;
		complementComponent[dim] = 1;
		Q = ist_projection(T,complementComponent);
		xfree(complementComponent);
		ok = (ist_nb_tuples(Q) * choose(val + DimComp-1,DimComp-1) == ist_nb_tuples(T));
		ist_dispose(Q);
		ist_dispose(T);
	}
	ist_copy(Scopy);
	return ok;
}

//build a more general partition that defines an abstraction that allow the represent S exactly
//works for finite sets only
abstraction_t *new_abstraction_finite_set(ISTSharingTree *S,int nb_var)
{
	int **result;
	abstraction_t * result_abs;
	int **tmp;
	int  max_line,max_row, i,j,l;
	boolean found;
	int *Component;


	//allocation of memory + initialisation
	result = (int **)xmalloc(nb_var * sizeof(int*));
	for(i=0;i < nb_var;++i) {
		result[i] = (int *)xmalloc(nb_var * sizeof(int));
		for(j = 0;j< nb_var;++j)
			result[i][j] = 0;
		result[i][i] = 1;
	}
	max_line = nb_var;
	max_row = nb_var;

	found = true;
	while (found == true) {
		found = false;
		//we check lines pairwise to find a pair to fusion
		for(i = 0; (i < max_line);++i)
			for(j=i+1;(j<max_line);)
				if (i!=j) {
					Component=new_element_in_partition(result,max_line,max_row,i,j);
					if (CanIRepresentExactlyTheFiniteSet(S,Component) == true) {
						//we can build a more general partition, the old one useless
						//computation of the new partition
						tmp = line_fusion(result,max_line,max_row,i,j);
						for(l = 0;l< max_line;++l)
							xfree(result[l]);
						xfree(result);
						result = tmp;
						--max_line;
						found = true;

					} else j++; //when line i and line j are fusionned,
						//  the result is put at line i. Hence, in that case we do not have
						//  to increaese j
				}
	}
	result_abs = (abstraction_t *)xmalloc(sizeof(abstraction_t));
	result_abs->nbConcreteV = max_row;
	result_abs->nbV = max_line;
	result_abs->A = result;
	result_abs->bound = (int *)xmalloc(result_abs->nbV * sizeof(int));
	for(i=0;i < result_abs->nbV;++i)
		result_abs->bound[i] = 1;

	return result_abs;
}


//build a more general partition that defines an abstraction that allow the represent S exactly
//precond: S is a uc-set!
abstraction_t *new_abstraction_lub(ISTSharingTree *S, int nb_var, abstraction_t *old_abs)
{
	ISTSharingTree *tmp;
	ISTLayer *layer;
	ISTNode *node;
	int **result, **tmpline, *Component, *infcomponent, max_line,max_row, i,j,l;
	abstraction_t *result_abs;

	puts("call to new_abstraction_lub");

	//allocation of memory + initialisation
	result = (int **)xmalloc(nb_var * sizeof(int*));
	for(i=0;i < nb_var;++i) {
		result[i] = (int *)xmalloc(nb_var * sizeof(int));
		for(j = 0;j< nb_var;++j)
			result[i][j] = (i==j) ? 1 : 0;
	}
	max_line = nb_var;
	max_row = nb_var;

	infcomponent = FindUnconstrainedPlaces(S,nb_var);
	for(i=nb_var-1;(i >= 0) && (infcomponent[i] == 0);i--);
	if (i >= 0) {
		j=i-1;
		while(j>=0) {
			if (infcomponent[j] == 1) {
				tmpline = line_fusion(result,max_line,max_row,j,i);
				for(l = 0;l< max_line;++l)
					xfree(result[l]);
				xfree(result);
				result = tmpline;
				--max_line;
				i=j;
			}
			j--;
		}
	}
	xfree(infcomponent);

	for(i = 0; (i < max_line);++i)
		if (mergeable(i,old_abs,result) == true) {
			for(j=i+1;(j<max_line);)
				if (can_be_merged(i,j,old_abs,result) == true) {
					Component=new_element_in_partition(result,max_line,max_row,i,j);
					tmp=ist_copy(S);
					/* we take the minimal elements of the outcoming uc-set */
					layer=tmp->FirstLayer;
					while(layer!=tmp->LastLayer) {
						node=layer->FirstNode;
						while(node!=NULL){
							node->Info->Right=node->Info->Left;
							node=node->Next;
						}
						layer=layer->Next;
					}
					/* ist_normalize is not necessary */
					ist_normalize(tmp);
					if (CanIRepresentExactlyTheFiniteSet(tmp,Component) == true) {
						//we can build a more general partition, the old one useless
						//computation of the new partition
						tmpline = line_fusion(result,max_line,max_row,i,j);
						for(l = 0;l< max_line;++l)
							xfree(result[l]);
						xfree(result);
						result = tmpline;
						--max_line;

					} else
						j++; //when line i and line j are fusionned,
					ist_dispose(tmp);
					//  the result is put at line i. Hence, in that case we do not have
					//  to increaese j
				} else j++;
		}

	result_abs = (abstraction_t *)xmalloc(sizeof(abstraction_t));
	result_abs->nbConcreteV = max_row;
	result_abs->nbV = max_line;
	result_abs->A = result;
	result_abs->bound = (int *)xmalloc(result_abs->nbV * sizeof(int));
	for(i=0;i < result_abs->nbV;++i)
		result_abs->bound[i] = 1;

	// DEBUG
	assert(old_abs->nbV < result_abs->nbV);


	return result_abs;
}

static void ist_add_variables(ISTSharingTree *S,integer16 nb_var)
{
	size_t i;
	ISTLayer * L;
	ISTNode * N;
	ISTNode * N_tmp;
	ISTInterval * inter;

	inter = ist_build_interval(0,0);
	N = S->LastLayer->FirstNode;
	N->Info = inter;
	for (i=0;i< nb_var;i++) {
		L = ist_add_last_layer(S);
		N_tmp = ist_create_node(inter);
		ist_add_son(N,N_tmp);
		ist_add_node(L,N_tmp);
		N = N_tmp;
	}
	ist_dispose_info(S->LastLayer->FirstNode->Info);
	S->LastLayer->FirstNode->Info = &IST_end_of_list;
}



ISTSharingTree *ist_abstraction(S,abs)
	ISTSharingTree *S;
	abstraction_t *abs;
{

	size_t i, j;
	ISTSharingTree *temp, *temp2 = NULL, *result;
	transition_t * t = (transition_t *)xmalloc(sizeof(transition_t));
	int *mask;

	if (ist_is_empty(S) == false) {
		/* adding of abstract variables */
		temp = ist_copy(S);
		ist_add_variables(temp,abs->nbV);
		/*
		 * construction of the transfers that defines the mapping from concrete
		 * variables to abstract variables
		 */
		t->nbr_transfers = abs->nbV;
		for(i=0;i< abs->nbV;i++) {
			t->transfers[i].target = abs->nbConcreteV+i;
			t->transfers[i].origin = (integer16 *)xmalloc((abs->nbConcreteV + abs->nbV)*sizeof(integer16));
			for(j=0;j < abs->nbConcreteV;j++)
				t->transfers[i].origin[j] = abs->A[i][j];
			for(j=0; j < abs->nbV;j++)
				t->transfers[i].origin[abs->nbConcreteV + j] = 0;
		}
		/* Computation of the abstract values */
		for(i=0; i<t->nbr_transfers;++i) {
			temp2 = ist_post_of_transfer(temp,&t->transfers[i]);
			ist_dispose(temp);
			temp = temp2;
		}

		for(i=0; i < abs->nbV;i++)
			xfree(t->transfers[i].origin);
		xfree(t);
		/* projection to only keep the concrete variables */
		mask = (integer16 *) xmalloc((abs->nbConcreteV+abs->nbV+1)*sizeof(integer16));
		for(i = 0; i < abs->nbConcreteV;i++)
			mask[i] = 0;
		/* i = abs->nbConcreteV */
		for(; i < abs->nbV + abs->nbConcreteV; i++)
			mask[i] = 1;
		/* by convention */
		mask[abs->nbV + abs->nbConcreteV] = 1;
		result = ist_projection(temp2,mask);
		ist_dispose(temp2);
		xfree(mask);
	} else ist_new(&result);
	return result;
}

// should work on any set since pre_of_transfer has been coded for intervals
ISTSharingTree *ist_concretisation(ISTSharingTree *S, abstraction_t *abs)
{
	size_t i, j;
	ISTSharingTree *temp;
	ISTSharingTree *temp2;
	ISTSharingTree *result;
	ISTLayer *L;
	ISTNode *N;
	transition_t *t = (transition_t *)xmalloc(sizeof(transition_t));
	boolean *in_abs = (boolean *)xmalloc(abs->nbConcreteV*sizeof(boolean));
	integer16* mask;

	if (ist_is_empty(S) == false) {

		//ist_write(S);

		/* initialisation */
		for(i = 0; i < abs->nbConcreteV;i++)
			in_abs[i] = false;

		/* adding of concrete variables */
		temp = ist_copy(S);
		ist_add_variables(temp,abs->nbConcreteV);

		/*
		 * construction of the transfers that defines the mapping from concrete
		 * variables to abstract variables
		 */
		t->nbr_transfers = abs->nbV;
		for(i=0;i< abs->nbV;i++) {
			t->transfers[i].target = i;
			t->transfers[i].origin = (integer16 *)xmalloc\
						 ((abs->nbConcreteV + abs->nbV)*sizeof(integer16));
			for(j=0;j < abs->nbV;j++)
				t->transfers[i].origin[j] = 0;
			for(j=0; j < abs->nbConcreteV;j++) {
				t->transfers[i].origin[abs->nbV + j] = abs->A[i][j];
				if (abs->A[i][j] != 0) {
					/* Place j is not ignored by the abstraction */
					in_abs[j] = true;
				} else t->transfers[i].origin[abs->nbV + j] = 0;
			}
		}
		/* Computation of the concrete values */
		temp2 = ist_pre_of_all_transfer_for_concretisation(temp,t);
		ist_dispose(temp);

		//ist_write(temp2);

		for(i=0; i < abs->nbV;i++)
			xfree(t->transfers[i].origin);
		xfree(t);

		/* projection to only keep the concrete variables */
		mask = (integer16 *) xmalloc(
				(abs->nbConcreteV + abs->nbV+1) * sizeof(integer16));
		for(i = 0; i < abs->nbV;i++)
			mask[i] = 0;
		/* i = abs->nbV */
		for(; i < abs->nbV + abs->nbConcreteV; i++)
			mask[i] = 1;
		/* by convention */
		mask[abs->nbV + abs->nbConcreteV] = 1;
		result = ist_projection(temp2,mask);
		ist_dispose(temp2);
		xfree(mask);
		/*
	 	* assignment of variables not in abstraction (viz. each entry of the column equals to 0)
	 	*/
		for(i = 0, L = result->FirstLayer; i < abs->nbConcreteV; i++,L=L->Next) {
			if (in_abs[i] == false) {
				N = L->FirstNode;
				while (N != NULL) {
					ist_assign_values_to_interval(N->Info,0,INFINITY);
					N = N->Next;
				}
			}
		}
	} else ist_new(&result);
	return result;
}

ISTSharingTree
*ist_abstract_post_of_rules(ISTSharingTree *S, void (*approx)(ISTSharingTree
			*S, integer16 *b), integer16 *bound, transition_t *t)
{
   ISTSharingTree *tmp;
   ISTSharingTree *res = ist_symbolic_post_of_rules(S,t);
   if (ist_is_empty(res) == false) {
      tmp = ist_downward_closure(res);
	  ist_dispose(res);
      if(approx)
		  approx(tmp,bound);
      ist_normalize(tmp);
      res = ist_minimal_form(tmp);
      ist_dispose(tmp);
   }
   return res;
}

ISTSharingTree
*ist_abstract_post(ISTSharingTree *S, void (*approx)(ISTSharingTree
			*S, integer16 *b), integer16 *bound, transition_system_t *t)
{
	size_t i;
	ISTSharingTree *res, *tmp, *_tmp;

	float comp_s;

	ist_new(&res);
	for(i=0;i< t->limits.nbr_rules;i++) {
		tmp = ist_abstract_post_of_rules(S,approx,bound,&t->transition[i]);
		_tmp = ist_remove_subsumed_paths(tmp,res);
		ist_dispose(tmp);
		if (ist_is_empty(_tmp)==false) {
			tmp = ist_remove_subsumed_paths(res,_tmp);
			ist_dispose(res);
			res = ist_union(tmp,_tmp);
			ist_dispose(tmp);
		}
		ist_dispose(_tmp);
	}
	ist_stat(res);
	if (file != NULL) ist_stat_plot(res, file);
	if(time_init == 0){
		time_init = 1;
		if(file != NULL) fprintf(file, ",\t 0");
		getrusage(RUSAGE_SELF, &time_before);
	} else {
		getrusage(RUSAGE_SELF, &time_after);
		comp_s =((float)time_after.ru_utime.tv_usec +(float)time_after.ru_utime.tv_sec*1000000 - (float)time_before.ru_utime.tv_sec*1000000- (float)time_before.ru_utime.tv_usec);
		if(file != NULL) fprintf(file, ",\t %f", (float)comp_s /(float)1000);
		getrusage(RUSAGE_SELF, &time_before);
	}
	return res;
}

ISTSharingTree
*ist_abstract_post_transtree(ISTSharingTree *S, void (*approx)(ISTSharingTree
			*S, integer16 *b), integer16 *bound, transition_system_t *t)
{
	ISTSharingTree *res, *tmp;

	res=ist_post_of_rules(t->tree_of_transitions, S);
//	ist_stat(res);
	if (ist_is_empty(res) == false) {
		tmp = ist_downward_closure(res);
		ist_dispose(res);
		if(approx)
			approx(tmp,bound);
		ist_normalize(tmp);
		res = ist_minimal_form(tmp);
		ist_dispose(tmp);
	}
	return res;
}



/* Assume initial_marking is a downward closed marking and the ist is minimal */
ISTSharingTree *ist_abstract_post_star(ISTSharingTree *initial_marking, void
		(*approx)(ISTSharingTree *S, integer16* b), integer16 *bound,
		transition_system_t *t)
{
	ISTSharingTree *S, *tmp;
	ISTSharingTree *Frontier;

	S = ist_copy(initial_marking);
	if(approx)
		approx(S,bound);
	ist_normalize(S);
	Frontier = ist_copy(S);
	while (true) {
		//printf("iteration dans abstract post star\n");
		if (file != NULL) fprintf(file, "%d,",iterations++);
		tmp = ist_abstract_post(Frontier,approx,bound,t);
		if (file != NULL) fprintf(file, "\n");
		//tmp = ist_abstract_post_transtree(S,approx,bound,t);
		ist_dispose(Frontier);
		Frontier = ist_remove_subsumed_paths(tmp,S);
		ist_dispose(tmp);
		if (ist_is_empty(Frontier)==false) {
			tmp = ist_remove_subsumed_paths(S,Frontier);
			ist_dispose(S);
			S = ist_union(tmp,Frontier);
			ist_dispose(tmp);
		} else {
			break;
		}
//		ist_write(S);

	}
	return S;
}



/* Assume initial_marking is a downward closed marking and the ist is minimal */
ISTSharingTree *ist_abstract_post_star_until_reach_bad(ISTSharingTree *initial_marking, void
		(*approx)(ISTSharingTree *S, integer16* b), integer16 *bound,
		transition_system_t *t, ISTSharingTree *bad)
{
	ISTSharingTree *S, *tmp;
	ISTSharingTree *Frontier;
	ISTSharingTree *inter;

	S = ist_copy(initial_marking);
	if(approx)
		approx(S,bound);
	ist_normalize(S);
	Frontier = ist_copy(S);
	inter = ist_intersection(Frontier,bad);
	if (ist_is_empty(inter) == true) {
		ist_dispose(inter);
		while (true) {
			//printf("iteration dans abstract post star\n");
			if(file!=NULL) fprintf(file, "%d,",iterations++);
			tmp = ist_abstract_post(Frontier,approx,bound,t);
			if(file!=NULL) fprintf(file, "\n");
			//tmp = ist_abstract_post_transtree(S,approx,bound,t);
			ist_dispose(Frontier);
			Frontier = ist_remove_subsumed_paths(tmp,S);
			ist_dispose(tmp);
			if (ist_is_empty(Frontier)==false) {
				//Test for termination
				inter = ist_intersection(Frontier,bad);
				if (ist_is_empty(inter) == true) {
					ist_dispose(inter);
					tmp = ist_remove_subsumed_paths(S,Frontier);
					ist_dispose(S);
					S = ist_union(tmp,Frontier);
					ist_dispose(tmp);
				} else {
					ist_dispose(S);
					S = inter;
					break;
				}
			} else {
				break;
			}
//		ist_write(S);

		}
	} else ist_dispose(inter);
	return S;
}


/*
 * compute the adhoc pretild for one transition t
 */
ISTSharingTree *adhoc_pretild_rule(ISTSharingTree *S, transition_t *t)
{
	ISTSharingTree *result = NULL;
	ISTSharingTree *temp;
	size_t i;
	boolean top;

	if (ist_is_empty(S) == false) {
		for (i=0, top = false; (i < ist_nb_layers(S)-1) && (top == false);i++) {
			if ((t->cmd_for_place[i].guard.Left > 0) &&
			(t->cmd_for_place[i].places_abstracted > 1))
				top = true;
		}
		if (top == false) {
			temp = ist_copy(S);
			ist_complement(temp,ist_nb_layers(S)-1);
			result = ist_pre_of_rule_plus_transfer(temp,t);
			ist_dispose(temp);
			/* as specified in ist_pre_of_rule_plus_transfer the result is not
			 * supposed to be in normal form. */
			if (!ist_is_empty(result))
				ist_normalize(result);
			ist_complement(result,ist_nb_layers(S)-1);
		} else {
			ist_new(&result);
			ist_complement(result,ist_nb_layers(S)-1);
		}
	} else
		ist_new(&result);
	return result;
}


/*
 * compute the adhoc pretild for all transitions
 */
ISTSharingTree *adhoc_pretild(ISTSharingTree *S, transition_system_t *t)
{
	ISTSharingTree *result, *temp1, *temp2;
	size_t i;

	ist_new(&result);
	ist_complement(result,ist_nb_layers(S)-1);
	for(i = 0; i < t->limits.nbr_rules; i++) {
		temp1 = adhoc_pretild_rule(S,&t->transition[i]);
		temp2 = ist_intersection(result,temp1);
		ist_dispose(result);
		ist_dispose(temp1);
		result= temp2;
	}
	return result;
}

// compute the adhoc pre for one transition t
ISTSharingTree *adhoc_pre_rule(ISTSharingTree *S, transition_t *t)
{
	ISTSharingTree *result = NULL;
	ISTSharingTree *temp;
	size_t i;
	boolean bottom;

	if (ist_is_empty(S) == false) {
		for (i=0, bottom = false; (i < ist_nb_layers(S)-1) && (bottom == false);i++) {
			if ((t->cmd_for_place[i].guard.Left > 0) &&\
					(t->cmd_for_place[i].places_abstracted > 1))
				bottom = true;
		}
		if (bottom == false) {
			temp = ist_copy(S);
			result = ist_pre_of_rule_plus_transfer(temp,t);
			ist_dispose(temp);
			/* as specified in ist_pre_of_rule_plus_transfer the result is not
			 * supposed to be in normal form. */
			if (!ist_is_empty(result))
				ist_normalize(result);
		} else {
			ist_new(&result);
		}
	} else
		ist_new(&result);
	return result;
}


//compute the adhoc pre for all transitions
ISTSharingTree *adhoc_pre(ISTSharingTree *S, transition_system_t *t)
{
	ISTSharingTree *result, *temp1, *temp2;
	size_t i;

	ist_new(&result);
	for(i = 0; i < t->limits.nbr_rules; i++) {
		temp1 = adhoc_pre_rule(S,&t->transition[i]);
		temp2 = ist_union(result,temp1);
		ist_dispose(result);
		ist_dispose(temp1);
		result= temp2;
	}
	return result;
}

ISTSharingTree *adhoc_pre_star_pruned_unless_hit_m0(ISTSharingTree *S, ISTSharingTree *cutter, transition_system_t *sys, ISTSharingTree *initial_marking)
{
	ISTSharingTree *tmp, *result = NULL, *frontier, *inter;
	int iter, i, j;
	boolean *maskpre;


	tmp=ist_prune_a_uc_ist_with_a_dc_ist(S,cutter);
	if(ist_is_empty(tmp)==true){
		ist_dispose(tmp);
		return result;
	}
	frontier=tmp;
	result=ist_copy(frontier);


	/* mask for pre (for the pre*_\hat{N}) */
	maskpre=(boolean *)xmalloc(sys->limits.nbr_rules*sizeof(boolean));
	for(i=0;i<sys->limits.nbr_rules;++i) {
		maskpre[i]=true;
		for (j=0; (j < sys->limits.nbr_variables) && (maskpre[i] == true);j++) {
			if ((sys->transition[i].cmd_for_place[j].guard.Left > 0) &&
					(sys->transition[i].cmd_for_place[j].places_abstracted > 1))
				maskpre[i]=false;
		}
	}
	from_transitions_to_tree(sys, maskpre);
	xfree(maskpre);
	puts("Tree of transitions:");
	ist_stat(sys->tree_of_transitions);
	ist_write(sys->tree_of_transitions);
	if(ist_is_empty(sys->tree_of_transitions)==true)
		return result;

	iter=0;
	while(true) {
		iter++;
		printf("adhoc_pre_star_pruned_unless_hit_m0: iteration %d\n",iter);
	//	tmp = adhoc_pre(frontier,sys);
		tmp = ist_pre_of_rules(sys->tree_of_transitions,frontier);
		ist_dispose(frontier);
		frontier = ist_prune_a_uc_ist_with_a_dc_ist(tmp,cutter);
		assert(ist_checkup(frontier)==true);
		ist_dispose(tmp);
		tmp=ist_remove_subsumed_paths(frontier, result);
		ist_dispose(frontier);
		frontier=tmp;

		if (ist_is_empty(frontier)==false) {
			inter = ist_intersection(initial_marking,frontier);
			if(ist_is_empty(inter) == false) {
				ist_dispose(inter);
				ist_dispose(result);
				result = NULL;
				puts("hit m0");
				break;
			}
			tmp = ist_remove_subsumed_paths(result,frontier);
			ist_dispose(result);
			result = ist_union(frontier,tmp);
			ist_dispose(tmp);
		} else {
			ist_dispose(frontier);
			puts("frontier is empty");
			break;
		}
	}
	return result;
}
