#include "proc.h"
#include "listnode.h"
#include "transsystem.h"
#include "abstraction.h"
#include "precone.h"
#include "normalize.h"
#include "xmalloc.h"

void ist_add_variables(ISTSharingTree *S,int nb_var);
ISTSharingTree * ist_projection(ISTSharingTree * S, int *mask);
ISTSharingTree * ist_concretisation(ISTSharingTree *S, transition_system_t *sys,transition_system_t *abstract_sys, abstraction_t * abs);
