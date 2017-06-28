%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "symbol.h"
#include "absynCMM.h"
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
%type  <A_exp> exp OP ifExp 
%type  <A_dec> dec
%type  <A_expList>  parametro
%type  <A_decList> sdec

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK 
  LBRACE RBRACE DOT 
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE OP
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF 
  BREAK NIL 
  FUNCTION VAR TYPE PROCEDURE
  PALAVRA_RESER IMPORT PRIMITIVO
  PERC BOOL READ WRITE RETURN


%start program

%%

program:
	dec meth {absyn_root=$1;}
;

exp:  
	  typ
	| ID  
	| com
;

lit: 
	  INT
    | STRING
;

expList: 
      exp parametro
;

parametro: 
      COMMA exp parametro
;

dec:
	  TYPE ID SEMICOLON
	| TYPE ID sdec SEMICOLON
	| TYPE ID ASSIGN lit sdec SEMICOLON
;

sdec:
	  COLON ID sdec
	| COLON ID ASSIGN lit sdec
;

com:
      ID atrib 
	| ifExp	
	| WHILE LBRACK exp RBRACK LBRACE exp RBRACE
	| FOR LPAREN dec SEMICOLON op SEMICOLON  RPAREN
	| ID LPAREN expList RPAREN
	| READ ID
	| WRITE expList
;

ifExp:  
	  IF exp THEN bloc
	| IF exp THEN bloc ELSE bloc
;

bloc: 
	  dec com
    | BREAK
    | RETURN
;

typ:
	  INT
	| STRING
	| BOOL
;

meth:
	  func
	| proc
;

proc: 
      ID LPAREN dec RPAREN LBRACE bloc RBRACE
;

func:
      typ ID LPAREN dec RPAREN LBRACE bloc RBRACE
;            

op:   
	  exp TIMES exp
    | exp DIVIDE exp
    | MINUS exp
    | exp MINUS exp
    | exp PLUS exp
    | exp EQ exp
    | exp NEQ exp
    | exp LT exp
    | exp LE exp
    | exp GT exp
    | exp GE exp
;

atrib:  
	  ASSIGN
    | PLUS ASSIGN
    | MINUS ASSIGN
    | TIMES ASSIGN
    | DIVIDE ASSIGN
    | PERC ASSIGN
;


 %%

