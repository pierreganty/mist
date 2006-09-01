/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INIT = 258,
     RULES = 259,
     TARGET = 260,
     INVARIANTS = 261,
     VARS = 262,
     ID = 263,
     NB = 264,
     EQUAL = 265,
     TRUE = 266,
     ARROW = 267,
     GTE = 268,
     COMMA = 269,
     IN = 270,
     TERMINATOR = 271
   };
#endif
/* Tokens.  */
#define INIT 258
#define RULES 259
#define TARGET 260
#define INVARIANTS 261
#define VARS 262
#define ID 263
#define NB 264
#define EQUAL 265
#define TRUE 266
#define ARROW 267
#define GTE 268
#define COMMA 269
#define IN 270
#define TERMINATOR 271




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 32 "parser.y"
typedef union YYSTYPE {
  int integer;
  char* string;
  T_PTR_tbsymbol_entry tbsymbol_entry;
  T_PTR_tree tree;
} YYSTYPE;
/* Line 1447 of yacc.c.  */
#line 77 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



