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

   Copyright 2003, 2004, Pierre Ganty, Anthony Piron, 2014 Pedro Valero
 */
%{
#include <stdio.h>
#include "laparser.h"
#include "error.h"

extern int yylex();
extern char* yytext;
T_PTR_tree tmp_tree;
int yyerror(char *);


%}

%union{
  int integer;
  char* string;
  T_PTR_tbsymbol_entry tbsymbol_entry;
  T_PTR_tree tree;
}



%token INIT
%token RULES
%token TARGET
%token INVARIANTS
%token VARS
%token <string> ID
%token <tbsymbol_entry> NB
%token EQUAL
%token TRUE
%token ARROW
%token GTE
%token COMMA
%token IN
%token TERMINATOR

%type <tree> exprarithleft
%type <tree> exprarith
%type <tree> statement
%type <tree> statementlist
%type <tree> statementfollow
%type <tree> guard
%type <tree> guardlist
%type <tree> guardedcmd
%type <tree> guardedcmdlist
%type <tree> rulessection
%type <tree> constr
%type <tree> constrlistor
%type <tree> constrlistand
%type <tree> targetssection
%type <tree> initsection
%type <tree> invsection
%type <tree> equal
%type <tree> equallistor
%type <tree> equallistand
%type <tree> prog

%left '+' '-' COMMA

%%

prog: varsection rulessection initsection targetssection invsection{
  tmp_tree = tree_new4("program", $2, $3, $4, $5);
}
;

varsection: VARS varlist
;

varlist: varlist ID {
		 /* One should write varlist : ID varlist but in that
		    way the output has the inversed order comparing
		    to the order in the *.spec file */
  T_PTR_tbsymbol_info info;
  T_PTR_tbsymbol_entry entry;

  if (tbsymbol_select(tbsymbol, $2))
    err_quit("\nhey fieu don't put twice a symbol line %d on %s", linenumber, $2);
  entry = tbsymbol_insert(tbsymbol, $2);
  info = tbsymbol_info_new();
  info->tag = tbsymbol_INFO_ID;
  info->info.id.addr = nbr_var++;
  info->info.nb.read = -1; // Initialize with an invalid value
  tbsymbol_setinfo(entry, info, sizeof(T_tbsymbol_info));
}
| { }
;

initsection: INIT constrlistand {
  $$ = tree_new1("init", $2);
  // We must reset the table to be able to detect
  // if a symbol has been delimited twice or not
  tbsymbol_dump(tbsymbol, reset);
}
;

targetssection: TARGET constrlistor {
  $$ = tree_new1("target", $2);
}
;



constrlistor: constrlistand constrlistor {
  $$ = tree_merge("or",tree_new1("or", $1),$2);
}
| constrlistand {
  $$ = tree_new1("or", $1);
}
;

constrlistand: constr COMMA constrlistand {
  $$ = tree_merge("and",tree_new1("and",$1),$3);
}
| constr {
  $$ = tree_new1("and",$1);

  // We must reset the table to be able to detect
  // if a symbol has been delimited twice or not
  tbsymbol_dump(tbsymbol, reset);
}
;

constr: ID EQUAL NB {
  T_PTR_tbsymbol_entry entry;
  T_PTR_tbsymbol_info info;

  entry = tbsymbol_select(tbsymbol, $1);
  if (entry == NULL)
    err_quit("\nhey fieu undeclared symbol %s line %d", $1, linenumber);
  // Storing the constan value into the symbol table

  $$ = tree_new2("=", tree_new0(entry), tree_new0($3));

  info = tbsymbol_getinfo(entry);
  if (info->info.nb.read != -1)
    err_quit("\nhey fieu symbol %s has been limited twice in the same state, last time in line %d", $1, linenumber);
  info->info.nb.read = atoi($3->name); // Store the min value that the variable could have before applying this transition.

}
| ID GTE NB {
  T_PTR_tbsymbol_entry entry;
  T_PTR_tbsymbol_info info;

  entry = tbsymbol_select(tbsymbol, $1);
  if (entry == NULL)
    err_quit("\nhey fieu undeclared symbol %s line %d", $1, linenumber);
  // Storing the constan value into the symbol table

  $$ = tree_new2(">=", tree_new0(entry), tree_new0($3));

  info = tbsymbol_getinfo(entry);
  if (info->info.nb.read != -1)
    err_quit("\nhey fieu symbol %s has been limited twice in the same state, last time in line %d", $1, linenumber);
  info->info.nb.read = atoi($3->name); // Store the min value that the variable could have before applying this transition.

}
| ID IN '[' NB COMMA NB ']' {

  T_PTR_tbsymbol_entry entry;
  T_PTR_tbsymbol_info info;

  entry = tbsymbol_select(tbsymbol, $1);
  if (entry == NULL)
    err_quit("\nhey fieu undeclared symbol %s line %d", $1, linenumber);
  // Storing the constan value into the symbol table

  $$ = tree_new3("in", tree_new0(entry), tree_new0($4),tree_new0($6));

  info = tbsymbol_getinfo(entry);
  if (info->info.nb.read != -1)
    err_quit("\nhey fieu symbol %s has been limited twice in the same state, last time in line %d", $1, linenumber);
  info->info.nb.read = atoi($4->name); // Store the min value that the variable could have before applying this transition.

}
;

rulessection: RULES guardedcmdlist {
  $$ = $2;
}
;

guardedcmdlist: guardedcmd guardedcmdlist {
  $$ = tree_merge("rules",
		  tree_new1("firstrule", $1), $2);

}
| { $$ = NULL;}
;

guardedcmd: guardlist ARROW statementlist TERMINATOR {
	/* For bounded transfer an arrow -X-> with X a natural
	 * number should be added in the grammar. Up till now
	 * we only consider unbounded transfers
	 */
  $$ = tree_new2("guardedcmd", $1, $3);

  // We must reset the table to be able to detect
  // if a symbol has been delimited twice or not

  tbsymbol_dump(tbsymbol, reset);
}
;

guardlist: guard COMMA guardlist {
  $$ = tree_merge("guard", tree_new1("guard", $1), $3);
}
| guard {
  $$ = tree_new1("guard", $1);
}
;

guard : constr {
		   $$ = $1;
	   }
| TRUE {
  T_PTR_tbsymbol_info info;
  T_PTR_tbsymbol_entry entry;

  entry = tbsymbol_select(tbsymbol, "true");
  if (entry == NULL) {
    entry = tbsymbol_insert(tbsymbol, "true");
    info = tbsymbol_info_new();
    info->tag = tbsymbol_INFO_ID;
    info->info.id.addr = -1;        // useless data field
    tbsymbol_setinfo(entry, info, sizeof(T_tbsymbol_info));
  }

  $$ = tree_new0(entry);
}
;
/*
statementlist:  statement COMMA statementlist{

  $$ = tree_merge("statement", tree_new1("statement",$1), $3);
}
| statement {
  $$ = tree_new1("statement", $1);
}
;*/

statementlist:  statement statementfollow {

  $$ = tree_merge("statement", tree_new1("statement",$1), $2);
}
|  {
  $$ = NULL;
}
;

statementfollow : COMMA statement statementfollow {
    $$ = tree_merge("statement",tree_new1("statement",$2),$3);
}
|{
    $$ = NULL;
};


statement : ID '\'' EQUAL exprarith {
  T_PTR_tree tree;
  T_PTR_tbsymbol_entry entry;
  T_PTR_tbsymbol_info info;

  entry = tbsymbol_select(tbsymbol, $1);
  if (entry == NULL)
    err_quit("\nhey fieu undeclared symbol %s line %d", $1, linenumber);

  tree = tree_new2("=", tree_new0(entry), $4);

  $$ = tree;

  if (strcmp($4->info, "-") == 0) { //If we have a substraction
    info = tbsymbol_getinfo(entry);
    // Checking that the variable we are going to substract is lower or equal than the minimum value reached by the variable before the transition began so we can ensure that the variable never goes below 0.
    if(atoi($4->subtrees[1]->info) > info->info.nb.read && info->info.nb.read >= 0){
       err_quit("\nhey fieu invalid operation in line %d. %d must be lower than %d", linenumber, atoi($4->subtrees[1]->info), info->info.nb.read);
    }
  }
}
;

exprarith: exprarithleft '+' NB {
  T_PTR_tree tree;

  tree = tree_new2("+", $1, tree_new0($3));

  $$ = tree;
}
| exprarithleft '-' NB {
  T_PTR_tree tree;

  tree = tree_new2("-", $1, tree_new0($3));

  $$ = tree;
}
| NB {
  T_PTR_tree tree;

  tree = tree_new0($1);

  $$ = tree;
}
| exprarithleft {
	$$ = $1;
}
;

exprarithleft: exprarithleft '+' exprarithleft {
  T_PTR_tree tree;

  tree = tree_new2("+", $1, $3);

  $$ = tree;
}
| ID {
  T_PTR_tree tree;
  T_PTR_tbsymbol_entry entry;

  entry = tbsymbol_select(tbsymbol, $1);
  if (entry == NULL)
    err_quit("\nhey fieu undeclared symbol %s line %d", $1, linenumber);

  tree = tree_new0(entry);

  $$ = tree;
}
;

invsection : INVARIANTS equallistor {
  $$ = tree_new1("invariants", $2);
}
| {}
;

equallistor: equallistand equallistor {
  $$ = tree_merge("or",tree_new1("or", $1),$2);
}
| equallistand {
  $$ = tree_new1("or", $1);
}
;

equallistand: equal COMMA equallistand {
  $$ = tree_merge("and",tree_new1("and",$1),$3);
}
| equal {
  // We must reset the table to be able to detect
  // if a symbol has been delimited twice or not
  $$ = tree_new1("and",$1);
  tbsymbol_dump(tbsymbol, reset);
}
;

equal: ID EQUAL NB {
  T_PTR_tbsymbol_entry entry;
  T_PTR_tbsymbol_info info;

  entry = tbsymbol_select(tbsymbol, $1);
  if (entry == NULL)
    err_quit("\nhey fieu undeclared symbol %s line %d", $1, linenumber);

  $$ = tree_new2("=", tree_new0(entry), tree_new0($3));

  info = tbsymbol_getinfo(entry);
  if (info->info.nb.read != -1)
    err_quit("\nhey fieu symbol %s has been defined has invariant twice in line %d", $1, linenumber);

  info->info.nb.read = 0; // For the invariants the minimum value doesn't care
                          // We just have to distinguish two possible states
}
;

%%
#include "error.h"
#include "laparser.h"

// T_PTR_tree yyparse(void);

int
yyerror(char* s){
  err_quit("%s : line %d on '%s'",s, linenumber, yytext);
  return -1;
}

extern FILE *yyin;


int
my_yyparse(T_PTR_tree* tree, char* filename)
{
  int retval;
  // Input redirection for lex
  yyin = fopen(filename,"r");
  if (yyin == NULL)
    err_sys("fopen error");

  nbr_var = 0;

  retval = yyparse();
  *tree = tmp_tree;

  if (fclose(yyin) != 0)
    err_sys("fclose error");

  return retval;
}
// The field 'read' let us control if the variable has already been constrained
// in a block where this can happend just once or not.

// If we are working on a transition we also want to store the minimum value possible
// for the variable.

// To reset the symbol table before continue analyzing a different block we have to
// reset the field to the same value with which we initialize it.

void reset(T_PTR_tbsymbol_entry entry){
  T_PTR_tbsymbol_info info;

  info = tbsymbol_getinfo(entry);
  if(info->tag == tbsymbol_INFO_ID)
    info->info.nb.read = -1;
}