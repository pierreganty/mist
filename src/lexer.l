%{
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

   Copyright 2003, 2004, Pierre Ganty
 */

#include "laparser.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

%}

%option noyywrap
%option nounput

alpha [a-zA-Z_]
digit [0-9]
alphanum ({alpha}|{digit})
id {alpha}{alphanum}*
%%
[ \t]        {}
[\n]         {linenumber++;}
#.*          {/* Comments */}

";"   		 {/* ECHO; */return TERMINATOR;}

"init"       {/* ECHO; */return INIT;}
"rules"      {/* ECHO; */return RULES;}
"invariants" {/* ECHO; */return INVARIANTS;}
"vars"       {/* ECHO; */return VARS;}
"target"     {/* ECHO; */return TARGET;}
"true"       {/* ECHO; */return TRUE;}
"->"         {/* ECHO; */return ARROW;}
">="         {/* ECHO; */return GTE;}
"="          {/* ECHO; */return EQUAL;}
","	     	 {/* ECHO; */return COMMA;}
"in"	     {/* ECHO; */return IN;}
{id}         {
               /* ECHO; */
               yylval.string = (char*)xmalloc(strlen(yytext)+1);
               strcpy(yylval.string, yytext);

               return ID;
             }
{digit}*     { /* Why not + instead of *  */
               T_PTR_tbsymbol_info info;

               /* ECHO; */
			   yylval.tbsymbol_entry = tbsymbol_select(tbsymbol,yytext);
			   if (yylval.tbsymbol_entry == NULL ){
				   yylval.tbsymbol_entry = tbsymbol_insert(tbsymbol, yytext);
				   info = tbsymbol_info_new();
				   info->tag = tbsymbol_INFO_NB;
				   sscanf(yytext, "%d", &(info->info.nb.value));
				   tbsymbol_setinfo(yylval.tbsymbol_entry,
						   info,
						   sizeof(T_tbsymbol_info));
			   }

               return NB;
             }
.            {/* ECHO; */return yytext[0];}
%%
/*int main()
{
  int val;

  while (val = yylex())
    {
      printf("Value : %d\n", val);
    }
}*/
