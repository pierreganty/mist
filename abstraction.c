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

   Copyright 2006, Pierre Ganty, 2007 Laurent Van Begin
 */

#include"abstraction.h"
#include"xmalloc.h"
#include"checkup.h"
#include<assert.h>

static boolean CanIRepresentExactlyTheSet(ISTSharingTree * S,int *Component);
/* Only works for Petri Nets w/o invariants */
transition_system_t *
build_sys_using_abs(sys,abs)
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

/////////////////////////////////////////////////////////////////////////////////////////
// precieuse methods																   //	
/////////////////////////////////////////////////////////////////////////////////////////

int ** partition_lines(int **matrix,int max_line, int max_row, int line1, int line2) 
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

int ** line_fusion(int **matrix,int max_line, int max_row, int line1, int line2) 
{
	int ** result;
	int i,j;//l;

	//allocation of memory
	result = (int **)xmalloc((max_line -1) * sizeof(int*));	
	for(i=0;i < max_line -1;++i) {
		result[i] = (int *)xmalloc(max_row * sizeof(int));
	}

/*
	//copy of all the lines that are not fusioned
	for(i = 0, l=0; i < max_line;++i) {
		if (i != line1 && i != line2) {
			for(j=0;j< max_row;++j) {
				result[l][j] = matrix[i][j];
			}
			++l;
		}
	}
	//fusion of line1 and line2
	for(i = 0; i < max_row;++i){
		result[max_line -2][i] = matrix[line1][i] + matrix[line2][i];
	}
	return result;
*/
	// we assume that line1 < line2
	// and copy all the line < line 1
	for(i=0;i < line1;i++) {
		for(j=0;j<max_row;j++) {
			result[i][j] = matrix[i][j];
		}
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

//	int a,b;
	
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


int *new_element_in_partition(int **matrix,int max_line, int max_row, int line1, int line2) 
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

int *FindInfinitePlaces(ISTSharingTree *S,int nb_var) 
{
	size_t i;
	ISTLayer * L;
	int *result=(int *)malloc(nb_var*(sizeof(int)));

	for(i=0,L = S->FirstLayer;i<nb_var;i++,L=L->Next) {
		if (L->FirstNode->Info->Right == INFINITY)
			result[i] = 1;
		else
			result[i] = 0;
	}	
	return result;
}

//build a more general partition that defines an abstraction that allow the represent S exactly
//works for dc-sets only ?
abstraction_t *new_abstraction(ISTSharingTree *S,int nb_var) 
{
	int **result;
	abstraction_t * result_abs;
	int **tmp;
	int  max_line,max_row, i,j,l; 
	boolean found;
	abstraction_t abs;
	int *Component;
	int *infcomponent;

	abs.nbConcreteV = nb_var;


	printf("Construction of a new abstraction\n");
	
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
	free(infcomponent);

	found = true;
	while (found == true) {
		found = false;
		//we check lines pairwise to find a pair to fusion
		for(i = 0; (i < max_line);++i) 
			for(j=i+1;(j<max_line);)
				if (i!=j) {
					Component=new_element_in_partition(result,max_line,max_row,i,j);
					if (CanIRepresentExactlyTheSet(S,Component) == true) {
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

	printf("end construction of a new abstraction\n");

	return result_abs;
}



//Function that returns a path of the IST given as parameter
//Redundant w/ ist_firstpath2array
ISTInterval **GiveMeAPath(ISTSharingTree *S) 
{
	int nbvar = 0;
	ISTInterval ** result;
	ISTLayer * L;
	ISTNode * N;
	int i;

	for(L = S->FirstLayer;L != S->LastLayer;nbvar++,L = L->Next);

	result = (ISTInterval **)malloc(nbvar * sizeof(ISTInterval *));
	for(i=0,N = S->FirstLayer->FirstNode;N->FirstSon != NULL;N = N->FirstSon->Son,i++){
		result[i] = ist_copy_interval(N->Info);		
	}
	return result;
}


//compute the sum of value appearing in the layers given by Component
//Component[i] = 0 == the layer is not considered
//Component[i] > 0 == the layer is considered
int ValueInComponent(ISTInterval **V,int *Component,int dim) 
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
ISTNode *PathWithValueInComponent(ISTSharingTree * S, ISTNode * N,ISTLayer *L,int NuLayer,int * Component,int val,int sum) 
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
ISTSharingTree *ist_PathsWithValueInComponent(ISTSharingTree * S,int * Component,int val) 
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


//used to compute all the path such that the sum of values appearing in layers given by Component is equal to val
int nb_PathWithValueInComponent(ISTNode * N,ISTLayer *LayerCache,int NuLayer,int * Component,int sum,int val) 
{
	int result;
	ISTSon *s;
	ISTNode *node;
	TMemo1 * memo;


	if (ist_equal_interval(N->Info,&IST_end_of_list)) {
		if(val == sum)
	        	result = 1;
		else
			result = 0;
	}else {
		result = 0;


		for(s = N->FirstSon;s != NULL;s = s->Next) {
			if (Component[NuLayer] > 0) {
				if (ist_less_or_equal_value(ist_add_value(N->Info->Right,sum),val)){
					memo = ist_get_memoization1(s->Son,(ISTNode *) ist_add_value(N->Info->Right,sum));
					if (memo != NULL)
						result += memo->r->Info->Right;
					else { 
						result += nb_PathWithValueInComponent(s->Son,LayerCache,NuLayer+1,Component,ist_add_value(N->Info->Right,sum),val);
					}
				} else 
					result += 0;
			} else { 
				memo = ist_get_memoization1(s->Son, (ISTNode *) sum);
				if (memo != NULL)
					result += memo->r->Info->Right;
				else
					result += nb_PathWithValueInComponent(s->Son,LayerCache,NuLayer + 1,Component,sum,val);

			}
		}			
	
	}
	node = ist_add_node(LayerCache,ist_create_node(ist_build_interval(0,result)));
	ist_put_memoization1(N,(ISTNode *) sum,node);
	return result;
}



//That function returns an IST such that the paths are those of S such that the sum of the values
//in the layers given by Component is equal to val (we only consider right bound)
//Assumption: the left bound is equal to 0 for all the nodes
 int ist_nb_PathsWithValueInComponent(ISTSharingTree * S,int * Component,int val) 
{

	ISTSon *son;
	int result =0;
	ISTLayer LayerCache;
	ISTNode * n, * m;

	ist_new_magic_number();
    	ist_new_memo1_number();

	for(son = S->Root->FirstSon; son != NULL;son = son->Next) {
		result += nb_PathWithValueInComponent(son->Son,&LayerCache,0,Component,0,val);
		
	}

	n = LayerCache.FirstNode;
	while (n != NULL) {
		m = n->Next;
		ist_dispose_node(n);
		n = m;
	}

	return result;
}


/*

//that function compute a choose b
int choose(int a,int b) 
{
   int v = 1;
   int facaminusb = 1;
   int i;

   //we first compute a!/b!, assuming that a>= b
   for(i=a;i>b;i--)
       v *= i;

   for(i=1;i<= a-b;i++)
       facaminusb *= i;

   return v/facaminusb;
}
*/

#define MAXN    100     /* largest n or m */

long choose(n,m)
int n,m;            /* computer n choose m */
{
    int i,j;        /* counters */
    long bc[MAXN][MAXN];    /* table of binomial coefficient values */

    for (i=0; i<=n; i++) bc[i][0] = 1;

    for (j=0; j<=n; j++) bc[j][j] = 1;

    for (i=1; i<=n; i++)
        for (j=1; j<i; j++)
            bc[i][j] = bc[i-1][j-1] + bc[i-1][j];

    return(bc[n][m]);
}

#define	MAXN	100		/* largest n or m */

long binomial_coefficient(n,m)
int n,m;			/* computer n choose m */
{
	int i,j;		/* counters */
	long bc[MAXN][MAXN];	/* table of binomial coefficient values */

	for (i=0; i<=n; i++) bc[i][0] = 1;

	for (j=0; j<=n; j++) bc[j][j] = 1;

	for (i=1; i<=n; i++)
		for (j=1; j<i; j++)
			bc[i][j] = bc[i-1][j-1] + bc[i-1][j];

	return(bc[n][m]);
}




//that function tests if for all the paths there is no node in a layer given by Component
//that does not have INFINITY as right bound.
//return true if it holds
//return false otherwise
boolean testINFINITY(ISTSharingTree * S,int *Component) 
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

//return true iff the partition of places that contains the set given by component and the simgletons
//for all the other places is precise enough to represent the tuple of S.
static boolean CanIRepresentExactlyTheSet(ISTSharingTree *S, int *Component) 
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
			for(i=0;i<dim;i++) {
				if(Component[i] > 0)
					complementComponent[i] = 0;
				else
					complementComponent[i] = 1;
			}
			complementComponent[dim] = 1;
			Q = ist_projection(T,complementComponent);
			free(complementComponent);
			ok = (ist_nb_elements(Q) * choose(val + DimComp-1,DimComp-1) == ist_nb_elements(T));
			ist_dispose(Q);
		}
		ist_dispose(T);
	}
	ist_copy(Scopy);
	return ok;	
}


/////////////////////////////////////////////////////////////////////////////////////////

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
	ISTSharingTree *temp, *temp2, *result;
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
		temp2 = ist_pre_of_all_transfer(temp,t);
		ist_dispose(temp);
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

/* ASSUME row has at least two entries greater or equal than 1 */
static integer16 *refined_variables(integer16 *row, ISTSharingTree *S, ISTSharingTree *cpreS)
{
	size_t length, j;
	integer16 *NewB;
	length=ist_nb_layers(S)-1;
	ISTLayer *L,*_L;
	ISTNode *N,*_N;
	boolean matched;

	/* We allocate and initialize the value to return */
	NewB = (integer16 *)xmalloc(length*sizeof(integer16));
	for(j=0;j<length;NewB[j++]=0); 

	L = S->FirstLayer;
	_L = cpreS->FirstLayer;
	j=0;
	while(L!=NULL && _L!=NULL){
		if(row[j]==1){
			/* Does a new bound appear in cpreS ? */
			_N=_L->FirstNode;
			while(_N!=NULL) {
				/* Take a node in cpreS layer j and search for a samed
				 * node in S layer j if so NewB[j]==1 */
				N=L->FirstNode;
				matched=false;
				while(N!=NULL){
					if(ist_equal_interval(_N->Info,N->Info))
						matched=true;
					N=N->Next;
				}
				if (matched==false)
					NewB[j]=1;
				_N=_N->Next;
			}
			/* Does a bound disappear in S ? */
			N=L->FirstNode;
			while(N!=NULL) {
				/* Take a node in cpreS layer j and search for a samed
				 * node in S layer j if so NewB[j]==1 */
				_N=L->FirstNode;
				matched=false;
				while(_N!=NULL){
					if(ist_equal_interval(_N->Info,N->Info))
						matched=true;
					_N=_N->Next;
				}
				if (matched==false)
					NewB[j]=1;
				N=N->Next;
			}
		}
		L=L->Next;
		_L=_L->Next;
		++j;
	}
	return NewB;
}

static abstraction_t *add_row(abstraction_t *abs, size_t nb_row, integer16 *NewB)
{

	size_t j,k;
	abstraction_t *retval;

	retval = (abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* we add exactly one row  */
	retval->nbV=abs->nbV+1;
	retval->nbConcreteV = abs->nbConcreteV;
	retval->A=(integer16 **)xmalloc(retval->nbV*sizeof(integer16));
	retval->bound=(integer16 *)xmalloc(retval->nbV*sizeof(integer16));
	/* we copy abs into retval */
	for(k=0;k<retval->nbV;++k) {
		retval->bound[k]=1;
		retval->A[k]=(integer16 *)xmalloc(retval->nbConcreteV*sizeof(integer16));
		for(j=0;j<retval->nbConcreteV;++j)
			/* the added row is set to 0's. */
			retval->A[k][j]=(k < abs->nbV ? abs->A[k][j] : 0);
	}
	/* we split the nb_row according to NewB */
	if (NewB != NULL) {
		/* If NewB is NULL we do nothing */
		for(j=0;j<retval->nbConcreteV;++j){
			if(NewB[j]==1) {
				retval->A[nb_row][j] = 0;
				retval->A[retval->nbV-1][j]= 1;
			}
		}
	}
	return retval;
}


abstraction_t *refine_abs(cur_abs, S, cpreS, safe, sys)
	abstraction_t *cur_abs;
	ISTSharingTree *S, *cpreS, *safe;
	transition_system_t *sys;
{
	abstraction_t *refined_abs, *p;
	ISTSharingTree *cpre_t_S, *tmp, *_tmp;
	integer16 *NewB, sum, sumB;
	size_t i, j, k, splitted, nb_row;

	p=cur_abs;

	/* For each row i in the current abstraction */
	for(i=0;i<cur_abs->nbV;++i) {
		/* compute sum */
		for(sum=0,j=0;j<p->nbConcreteV;sum+=cur_abs->A[i][j++]);
		if(sum > 1){
			/* there are places to separate on row i */
			NewB = refined_variables(p->A[i],S,cpreS);
			for(sumB=0,j=0;j<p->nbConcreteV;sumB+=NewB[j++]);
			if (0 < sumB) {
				/* We have to refine this row */
				if (sumB < sum)	{
					/* CASE 1: Some but not all bounds are refined */
					refined_abs = add_row(p,i,NewB);
					/* to not modify cur_abs which is a in parameter */
					if (p!= cur_abs)
						dispose_abstraction(p);
					p = refined_abs;
				} 
				xfree(NewB);	
				if (sumB == sum) {
					/* CASE 2: All the bounds of row i are refined. */
					nb_row = p->nbV;
					/* We remember how many rows counts the abstraction p */
					for (k=0; nb_row==p->nbV && k<sys->limits.nbr_rules;++k){
						tmp = adhoc_pretild_rule(S,&sys->transition[k]);
						_tmp = ist_intersection(tmp,safe);
						ist_dispose(tmp);
						tmp = ist_downward_closure(_tmp);
						ist_normalize(tmp);
						ist_dispose(_tmp);
						cpre_t_S = ist_minimal_form(tmp);
						ist_dispose(tmp);
						assert(ist_checkup(cpre_t_S)==true);
						/* This time using cpre_t_S */
						NewB = refined_variables(p->A[i],S,cpre_t_S);
						for(sumB=0,j=0;j<p->nbConcreteV;sumB+=NewB[j++]);
						if (0 < sumB && sumB < sum)	{
							/* Some but not all bounds are refined */
							refined_abs = add_row(p,i,NewB);
							/* to not modify cur_abs which is a in parameter */
							if (p!= cur_abs)
								dispose_abstraction(p);
							p = refined_abs;
							/* recompute sum since p->A[i] has been modified */
							for(sum=0,j=0;j<p->nbConcreteV;sum+=p->A[i][j++]);
						}
						xfree(NewB);
						ist_dispose(cpre_t_S);
					}
					if (nb_row == p->nbV) {
						/* CASE 2a: No clue about separating places of row i.
						 * So, we carry on an arbitrary split of row i */
						refined_abs=add_row(p,i,NULL);
						/* refined_abs is p w/ one row more */
						for(splitted=0,j=0;j<refined_abs->nbConcreteV;++j) {
							if(refined_abs->A[i][j]==1 && splitted < (integer16) sum/2 ) {
								refined_abs->A[i][j] = 0;
								refined_abs->A[refined_abs->nbV-1][j] = 1;
								++splitted;
							}
						}
						/* to not modify cur_abs which is a in parameter */
						if (p!= cur_abs)
							dispose_abstraction(p);
						p = refined_abs;
					} else
						printf("%d rows added by the trans/trans \
								refinement\n",p->nbV-nb_row);
				}
			} else
				/* We do nothing but release the NewB array. */
				xfree(NewB);
		}
	}
	if (p==cur_abs) {
		/* So far no refinenement has been proposed. So we split a non singleton row */
		for(i=0;i<cur_abs->nbV;++i) {
			/* compute sum */
			for(sum=0,j=0;j<cur_abs->nbConcreteV;sum+=cur_abs->A[i][j++]);
			if(sum > 1) {
				p=add_row(cur_abs,i,NULL);
				/* we add a row and we split row i into two sets. */
				for(splitted=0,j=0;j<p->nbConcreteV;++j) {
					if(p->A[i][j]==1 && splitted < (integer16) sum/2 ) {
						p->A[i][j] = 0;
						p->A[p->nbV-1][j] = 1;
						++splitted;
					}
				}
			}
		}
	}
	/* A sanity check */
	for(i=0;i<p->nbV;++i) {
		for(sum=0,j=0;j<p->nbConcreteV;sum+=p->A[i][j++]);
		assert(sum>0);
	}
	return p;
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

//ISTSharingTree 
//*ist_abstract_post(ISTSharingTree *S, void (*approx)(ISTSharingTree
//			*S, integer16 *b), integer16 *bound, transition_system_t *t) 
//{
//	size_t i;
//	ISTSharingTree *res, *tmp, *_tmp;
//
//	ist_new(&res);
//	for(i=0;i< t->limits.nbr_rules;i++) {
//		tmp = ist_abstract_post_of_rules(S,approx,bound,&t->transition[i]);
//		_tmp = ist_remove_subsumed_paths(tmp,S);
//		ist_dispose(tmp);
//		if (ist_is_empty(_tmp)==false) {
//			tmp = ist_remove_subsumed_paths(S,_tmp);
//			res = ist_union(tmp,_tmp);
//			ist_dispose(tmp);
//		}
//		ist_dispose(_tmp);
//	}
//	return res;
//}

ISTSharingTree
*ist_abstract_post(ISTSharingTree *S, void (*approx)(ISTSharingTree
			*S, integer16 *b), integer16 *bound, transition_system_t *t)
{
	size_t i;
	ISTSharingTree *res, *tmp, *_tmp;

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
	return res;
}

/* Assume initial_marking is a downward closed marking and the ist is minimal */
ISTSharingTree 
*ist_abstract_post_star(ISTSharingTree *initial_marking, void
		(*approx)(ISTSharingTree *S, integer16* b), integer16 *bound,
		transition_system_t *t) 
{
	ISTSharingTree *S, *tmp, *_tmp;

	S = ist_copy(initial_marking);
	if(approx)
		approx(S,bound);
	ist_normalize(S);
	while (true) {
		tmp = ist_abstract_post(S,approx,bound,t);
		_tmp =  ist_remove_subsumed_paths(tmp,S);
		ist_dispose(tmp);
		if (ist_is_empty(_tmp)==false) {		
			tmp = ist_remove_subsumed_paths(S,_tmp);
			ist_dispose(S);
			S = ist_union(tmp,_tmp);
			ist_dispose(tmp);
			ist_dispose(_tmp);
		} else {
			ist_dispose(_tmp);
			break;
		}
	}
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

ISTSharingTree * ist_symbolic_pre_tild_of_rule(Prec, transition)
   ISTSharingTree *Prec;
   transition_t *transition;
{
	ISTSharingTree *NegGuard;
	ISTSharingTree *tmp;
	ISTSharingTree * pre;
	ISTSharingTree * result;
	ISTInterval **path;
	int i;
	int nbvar = (ist_nb_layers(Prec)-1);

	path = 	(ISTInterval **)malloc(sizeof(ISTInterval*) * nbvar);
	for(i=0;i< nbvar;i++) 
		path[i] = &(transition->cmd_for_place[i].guard);
	ist_new(&NegGuard);
	ist_add(NegGuard,path,nbvar);
	free(path);

	ist_complement(NegGuard,nbvar);
	tmp = ist_downward_closure(NegGuard);
	ist_dispose(NegGuard);
	NegGuard = tmp;
	pre = ist_symbolic_pre_of_rule(Prec,transition);
	tmp = ist_downward_closure(pre);
	ist_dispose(pre);
	pre = ist_union(tmp,NegGuard);
	ist_dispose(tmp);
	ist_dispose(NegGuard);
	result = ist_minimal_form(pre);
	ist_dispose(pre);
	return result;	
}



ISTSharingTree * ist_enumerative_pre_tild_of_rule(Prec, sys,trans)
   ISTSharingTree *Prec;
   transition_system_t *sys;
   int trans;
{
	ISTSharingTree *NegGuard;
	ISTSharingTree *tmp;
	ISTSharingTree * pre;
	ISTSharingTree * result;
	ISTInterval **path;
	int i;
	int nbvar = (ist_nb_layers(Prec)-1);


	path = 	(ISTInterval **)malloc(sizeof(ISTInterval*) * nbvar);

	for(i=0;i< nbvar;i++) 
		path[i] = &(sys->transition[trans].cmd_for_place[i].guard);
	ist_new(&NegGuard);
	ist_add(NegGuard,path,nbvar);
	free(path);
	ist_complement(NegGuard,nbvar);
	pre = ist_enumerative_pre_transition(Prec,sys,trans);
	tmp = ist_downward_closure(pre);
	ist_dispose(pre);
	pre = ist_union(tmp,NegGuard);
	ist_dispose(tmp);
	ist_dispose(NegGuard);
	result = ist_minimal_form(pre);
	ist_dispose(pre);
	return result;	
}

ISTSharingTree * ist_symbolic_pre_tild(Prec, sys)
   ISTSharingTree * Prec;
   transition_system_t * sys;
{
	ISTSharingTree * result;
	ISTSharingTree * tmp1;
	ISTSharingTree * tmp2;
	int i;

	ist_new(&result);
	ist_complement(result,ist_nb_layers(Prec)-1);

	for(i=0;i< sys->limits.nbr_rules;i++) {
		tmp1 = ist_symbolic_pre_tild_of_rule(Prec,&sys->transition[i]);
		tmp2 = ist_intersection(tmp1,result);
		ist_dispose(tmp1);
		ist_dispose(result);
		result = ist_minimal_form(tmp2);
		ist_dispose(tmp2);
	}	
	return result;
}

ISTSharingTree * ist_symbolic_abstract_pre_tild(Prec, sys)
   ISTSharingTree * Prec;
   transition_system_t * sys;
{
	ISTSharingTree * result;
	ISTSharingTree * cpre;
	ISTSharingTree * tmp;
	int i;
	int j;
	boolean top;

	ist_new(&result);
	ist_complement(result,ist_nb_layers(Prec)-1);

	for(j=0;j< sys->limits.nbr_rules;j++) {
		for (i=0, top = false; (i < sys->limits.nbr_variables) && (top == false);i++) {
			if ((sys->transition[j].cmd_for_place[i].guard.Left > 0) && 
			(sys->transition[j].cmd_for_place[i].places_abstracted > 1))
				top = true;
		}
		if (top == false) {
			cpre = ist_symbolic_pre_tild_of_rule(Prec,&sys->transition[j]);
			tmp = ist_intersection(cpre,result);
			ist_dispose(cpre);
			ist_dispose(result);
			result = ist_minimal_form(tmp);
			ist_dispose(tmp);

		}
	}
	return result;
}

