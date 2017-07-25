%{
#include <stdio.h>
#include <stdlib.h>

#include "absyn.h"
#include "errormsg.h"
#include "symbol.h" 
#include "util.h"

int yyparse(void); /* function prototype */

extern FILE * yyin;

A_exp absyn_root;

void yyerror(char *s)
{
 EM_error(EM_tokPos, "%s", s);
}
%}


%union {
  int pos;
  int ival;
  string sval;
  struct A_exp_       *A_exp;
  }

%token <sval> ID STRING
%token <ival> INTEGER

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK
  LBRACE RBRACE DOT
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE PERC
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO IN END OF 
  BREAK NIL STR INT
  FUNCTION VAR TYPE
  READ WRITE BOOL RETURN NOT INC DEC MULT MOD REST QUESTION

%type <A_exp> expr oper atrib lite

%nonassoc ASSIGN

%left OR AND

%nonassoc GT LT NEQ EQ LE GE

%left PLUS MINUS
%left TIMES DIVIDE

%left UMINUS

%start program

%%

program: expr {absyn_root=$1;}

expr:
    ID atrib oper {$$=A_AssignExp(EM_tokPos, $1, $3);}

lite:
    INTEGER    {$$=A_IntExp(EM_tokPos, $1);}
  | STRING     {$$=A_StringExp(EM_tokPos, $1);}

oper:
    lite TIMES lite   {$$=A_OpExp(EM_tokPos, A_timesOp, $1, $3);}
  | lite DIVIDE lite  {$$=A_OpExp(EM_tokPos, A_divideOp, $1, $3);}
  | lite MINUS lite   {$$=A_OpExp(EM_tokPos, A_minusOp, $1, $3);}
  | lite PLUS lite    {$$=A_OpExp(EM_tokPos, A_plusOp, $1, $3);}
  | lite GT lite      {$$=A_OpExp(EM_tokPos, A_gtOp, $1, $3);}
  | lite LT lite      {$$=A_OpExp(EM_tokPos, A_ltOp, $1, $3);}
  | lite NEQ lite     {$$=A_OpExp(EM_tokPos, A_neqOp, $1, $3);}
  | lite EQ lite      {$$=A_OpExp(EM_tokPos, A_eqOp, $1, $3);}
  | lite LE lite      {$$=A_OpExp(EM_tokPos, A_leOp, $1, $3);}
  | lite GE lite      {$$=A_OpExp(EM_tokPos, A_geOp, $1, $3);}
  | MINUS lite        {$$=A_OpExp(EM_tokPos, A_minusOp, NULL, $2);}
  | lite              {$$=$1}

atrib:  
    ASSIGN        {$$=NULL;}
  | PLUS ASSIGN   {$$=NULL;}
  | MINUS ASSIGN  {$$=NULL;}
  | TIMES ASSIGN  {$$=NULL;}
  | DIVIDE ASSIGN {$$=NULL;}
  | PERC ASSIGN   {$$=NULL;}

%% 

A_exp parse(string filename)
{
  EM_reset(filename);
  if (yyparse() == 0)
    return absyn_root;
  else
    return NULL;
}