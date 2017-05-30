%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "table.h"
#include "errormsg.h"

int yylex(void); /* function prototype */

int yyparse(void);

void yyerror(char *s)
{
 EM_error(EM_tokPos, "%s", s);
}

A_exp absyn_root;

%}


%union {
	int pos;
	int ival;
	char *sval;
        float  fval;
        struct A_exp_ *A_exp;
        struct A_var_ *A_var;
        struct A_dec_ *A_dec;
        struct A_expList_ *A_expList;
        struct A_decList_ *A_decList;
	}

%token <sval> ID STRING
%token <ival> INT
%token <fval> FLOAT 
%type  <A_exp> exp OP ifExp
%type  <A_var> lvalue
%type  <A_dec> dec vardec
%type  <A_expList> ExpList parametro exps parametro2
%type  <A_decList> decs

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK 
  LBRACE RBRACE DOT 
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF 
  BREAK NIL 
  FUNCTION VAR TYPE PROCEDURE
  PALAVRA_RESER IMPORT PRIMITIVO


%start program

%%

/* This is a skeleton grammar file, meant to illustrate what kind of
 * declarations are necessary above the %% mark.  Students are expected
 *  to replace the two dummy productions below with an actual grammar. 
 */

program:	exp                                                       {absyn_root=$1;}
;
exp:          STRING                                                      {$$=A_StringExp(@1.first_line, $1);}
             |INT                                                         {$$=A_IntExp(@1.first_line, $1);}
             |ID LBRACK exp RBRACK OF exp                                 {$$=A_ArrayExp(@1.first_line, S_Symbol($1), $3, $6);}	
             |lvalue                                                      {$$=A_VarExp(@1.first_line, $1);}
             |ID LPAREN ExpList LPAREN                                    {$$=A_CallExp(@1.first_line, S_Symbol($1), $3);}
             |OP                                                          {$$=$1;}
             |lvalue ASSIGN exp                                           {$$=A_AssignExp(@1.first_line, $1, $3);}
             |ifExp                                                       {$$=$1;}
             |WHILE exp DO exp                                            {$$=A_WhileExp(@1.first_line, $2, $4);}
             |FOR ID ASSIGN exp TO exp DO exp                             {$$=A_ForExp(@1.first_line, S_Symbol($2), $4, $6, $8);}
             |LET decs IN exps END                                        {$$=A_LetExp(@1.first_line, $2, A_SeqExp(@1.first_line, $4));}
;
lvalue:       ID                                                          {$$=A_SimpleVar(@1.first_line, S_Symbol($1));}
             |lvalue DOT ID                                               {$$=A_FieldVar(@1.first_line,$1,S_Symbol($3));}
             |lvalue LBRACK exp RBRACK                                    {$$=A_SubscriptVar(@1.first_line,$1,$3);}
;
ExpList:                                                                  {$$=NULL;}
             |exp parametro                                               {$$=A_ExpList($1, $2);}
;
parametro:                                                                {$$=NULL;}
             |COMMA exp parametro                                         {$$=A_ExpList($2, $3);}
;

ifExp:        IF exp THEN exp                                              {$$=A_IfExp(@1.first_line, $2, $4, NULL);}
             |IF exp THEN exp ELSE exp                                     {$$=A_IfExp(@1.first_line, $2, $4, $6);}
;

exps:                                                                     {$$=NULL;} 

             |exp parametro2                                              {$$=A_ExpList($1, $2);}
;

dec:          vardec                                                      {$$=$1;}
;

vardec:       VAR ID ASSIGN exp                                           {$$=A_VarDec(@1.first_line, S_Symbol($2), NULL, $4);}
             |VAR ID COLON ID ASSIGN exp                                  {$$=A_VarDec(@1.first_line, S_Symbol($2), S_Symbol($2), $6);}
;

parametro2:                                                               {$$=NULL;}
             |SEMICOLON exp parametro2                                    {$$=A_ExpList($2, $3);}
;

decs:                                                                     {$$=NULL;}
             |dec decs                                                    {$$=A_DecList($1, $2);}
;

OP:           exp TIMES exp                                               {$$=A_OpExp(@1.first_line, A_timesOp, $1, $3);}
             |exp DIVIDE exp                                              {$$=A_OpExp(@1.first_line, A_divideOp, $1, $3);}
             |MINUS exp                                                   {$$=A_OpExp(@1.first_line, A_minusOp, A_IntExp(@1.first_line, 0), $2);}
             |exp MINUS exp                                               {$$=A_OpExp(@1.first_line, A_minusOp, $1, $3);}
             |exp PLUS exp                                                {$$=A_OpExp(@1.first_line, A_plusOp, $1, $3);}
             |exp EQ exp                                                  {$$=A_OpExp(@1.first_line, A_eqOp, $1, $3);}
             |exp NEQ exp                                                 {$$=A_OpExp(@1.first_line, A_neqOp, $1, $3);}
             |exp LT exp                                                  {$$=A_OpExp(@1.first_line, A_ltOp, $1, $3);}
             |exp LE exp                                                  {$$=A_OpExp(@1.first_line, A_leOp, $1, $3);}
             |exp GT exp                                                  {$$=A_OpExp(@1.first_line, A_gtOp, $1, $3);}
             |exp GE exp                                                  {$$=A_OpExp(@1.first_line, A_geOp, $1, $3);}
;


%%

