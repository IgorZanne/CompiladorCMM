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
	struct A_var_ 		 	*A_var;
	struct A_exp_ 		 	*A_exp;
	struct A_dec_ 		 	*A_dec;
	struct A_ty_ 		 	*A_ty;
	struct A_field_ 	 	*A_field;
	struct A_fieldList_  	*A_fieldList;
	struct A_expList_ 	 	*A_expList;
	struct A_decList_ 	 	*A_decList;
	struct A_efield_ 	 	*A_efield;
	struct A_efieldList_ 	*A_efieldList;
	}

%token <sval> ID STRING
%token <ival> INT

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK
  LBRACE RBRACE DOT
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF 
  BREAK NIL
  FUNCTION VAR TYPE

%type <A_var> lvalue
%type <A_exp> exp exps pri_exp or_exp and_exp rel_exp add_exp times_exp
%type <A_dec> function-dec nametype-dec var-dec dec
%type <A_ty> type
%type <A_field> type-field
%type <A_fieldList> type-fields function-fields
%type <A_expList> function-args exp_list

%type <A_decList> decs
%type <A_efield> record-field
%type <A_efieldList> record-fields

%nonassoc ASSIGN

%left OR AND

%nonassoc GT LT NEQ EQ LE GE

%left PLUS MINUS
%left TIMES DIVIDE

%left UMINUS

%start program

%%

/* This is a skeleton grammar file, meant to illustrate what kind of
 * declarations are necessary above the %% mark.  Students are expected
 *  to replace the two dummy productions below with an actual grammar. 
 * printf("INTEGERS: %i - %i\n", $1, $2);
 */

program: exp {absyn_root=$1;}

pri_exp: NIL {$$=A_NilExp(EM_tokPos);}
	| INT {$$=A_IntExp(EM_tokPos, $1);}
	| STRING {$$=A_StringExp(EM_tokPos, $1);}
	| ID LPAREN function-args RPAREN {$$=A_CallExp(EM_tokPos, S_Symbol($1), $3);} 
	| lvalue {$$=A_VarExp(EM_tokPos, $1);}
	| MINUS pri_exp %prec UMINUS {$$=A_OpExp(EM_tokPos, A_minusOp, A_IntExp(EM_tokPos, 0), $2);}
	| exps {$$=$1;}

exps: LPAREN exp_list RPAREN {$$=A_SeqExp(EM_tokPos, $2);}

exp_list: {$$=NULL;}
	| exp {$$=A_ExpList($1, NULL);}
	| exp SEMICOLON exp_list {$$=A_ExpList($1, $3);}

exp: ID LBRACK exp RBRACK OF exp {$$=A_ArrayExp(EM_tokPos, S_Symbol($1), $3, $6);}
	| ID LBRACE record-fields RBRACE {$$=A_RecordExp(EM_tokPos, S_Symbol($1), $3);}
	| lvalue ASSIGN exp {$$=A_AssignExp(EM_tokPos, $1, $3);}
	| IF exp THEN exp {$$=A_IfExp(EM_tokPos, $2, $4, NULL);}
	| IF exp THEN exp ELSE exp {$$=A_IfExp(EM_tokPos, $2, $4, $6);}
	| WHILE exp DO exp {$$=A_WhileExp(EM_tokPos, $2, $4);}
	| FOR ID ASSIGN exp TO exp DO exp {$$=A_ForExp(EM_tokPos, S_Symbol($2), $4, $6, $8);}
	| BREAK {$$=A_BreakExp(EM_tokPos);}
	| LET decs IN exp_list END {$$=A_LetExp(EM_tokPos, $2, A_SeqExp(EM_tokPos, $4));}
	| or_exp {$$=$1;}

or_exp: and_exp {$$=$1;}
	| or_exp OR and_exp {$$=A_OpExp(EM_tokPos, A_orOp, $1, $3);}

and_exp: rel_exp {$$=$1;}
	| and_exp AND rel_exp {$$=A_OpExp(EM_tokPos, A_andOp, $1, $3);}

rel_exp: add_exp {$$=$1;}
	| rel_exp GT add_exp {$$=A_OpExp(EM_tokPos, A_gtOp, $1, $3);}
	| rel_exp LT add_exp {$$=A_OpExp(EM_tokPos, A_ltOp, $1, $3);}
	| rel_exp NEQ add_exp {$$=A_OpExp(EM_tokPos, A_neqOp, $1, $3);}
	| rel_exp EQ add_exp {$$=A_OpExp(EM_tokPos, A_eqOp, $1, $3);}
	| rel_exp LE add_exp {$$=A_OpExp(EM_tokPos, A_leOp, $1, $3);}
	| rel_exp GE add_exp {$$=A_OpExp(EM_tokPos, A_geOp, $1, $3);}

add_exp: times_exp {$$=$1;}
	| add_exp PLUS times_exp {$$=A_OpExp(EM_tokPos, A_plusOp, $1, $3);}
	| add_exp MINUS times_exp {$$=A_OpExp(EM_tokPos, A_minusOp, $1, $3);}

times_exp: pri_exp {$$=$1;}
	| times_exp TIMES pri_exp {$$=A_OpExp(EM_tokPos, A_timesOp, $1, $3);}
	| times_exp DIVIDE pri_exp {$$=A_OpExp(EM_tokPos, A_divideOp, $1, $3);}

record-fields: {$$=NULL;}
	| record-field {$$=A_EfieldList($1, NULL);}
	| record-field COMMA record-fields {$$=A_EfieldList($1, $3);}

record-field: ID EQ exp {$$=A_Efield(EM_tokPos, S_Symbol($1), $3);}

lvalue:	ID {$$=A_SimpleVar(EM_tokPos, S_Symbol($1));}
	| lvalue DOT ID {$$=A_FieldVar(EM_tokPos, $1, S_Symbol($3));}
	| lvalue LBRACK exp RBRACK {$$=A_SubscriptVar(EM_tokPos, $1, $3);}
	| ID LBRACK exp RBRACK {$$=A_SubscriptVar(EM_tokPos, A_SimpleVar(EM_tokPos,S_Symbol($1)), $3);}

function-args: {$$=NULL;}
	| exp {$$=A_ExpList($1, NULL);}
	| exp COMMA function-args {$$=A_ExpList($1, $3);}

decs:	dec {$$=A_DecList($1, NULL);}
	| dec decs {$$=A_DecList($1, $2);}

dec:  nametype-dec {$$=$1;} //type dec
	| var-dec {$$=$1;}
	| function-dec {$$=$1;}

function-dec: FUNCTION ID LPAREN function-fields RPAREN EQ exp {$$=A_FunctionDec(EM_tokPos, S_Symbol($2), $4, NULL, $7);}
	| FUNCTION ID LPAREN function-fields RPAREN COLON ID EQ exp {$$=A_FunctionDec(EM_tokPos, S_Symbol($2), $4, S_Symbol($7), $9);}
	
function-fields: {$$=NULL;}
	| type-fields {$$=$1;}

var-dec: VAR ID ASSIGN exp {$$=A_VarDec(EM_tokPos, S_Symbol($2), NULL, $4);} //var dec without type
	| VAR ID COLON ID ASSIGN exp {$$=A_VarDec(EM_tokPos, S_Symbol($2), S_Symbol($4), $6);} //var dec with type

nametype-dec: TYPE ID EQ type {$$=A_TypeDec(EM_tokPos, S_Symbol($2), $4);}

type: ID {$$=A_NameTy(EM_tokPos, S_Symbol($1));}
	| LBRACE type-fields RBRACE {$$=A_RecordTy(EM_tokPos, $2);}
	| ARRAY OF ID {$$=A_ArrayTy(EM_tokPos, S_Symbol($3));}

type-fields: type-field {$$=A_FieldList($1, NULL);}
	| type-field COMMA type-fields {$$=A_FieldList($1, $3);}

type-field: ID COLON ID {$$=A_Field(EM_tokPos, S_Symbol($1), S_Symbol($3));}

%% 

A_exp parse(string filename)
{
	EM_reset(filename);
	if (yyparse() == 0)
		return absyn_root;
	else
		return NULL;
	
}