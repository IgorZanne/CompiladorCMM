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
  struct A_var_       *A_var;
  struct A_exp_       *A_exp;
  struct A_dec_       *A_dec;
  struct A_ty_      *A_ty;
  struct A_field_     *A_field;
  struct A_fieldList_   *A_fieldList;
  struct A_expList_     *A_expList;
  struct A_decList_     *A_decList;
  struct A_efield_    *A_efield;
  struct A_efieldList_  *A_efieldList;
  }

%token <sval> ID STRING
%token <ival> INT

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK
  LBRACE RBRACE DOT
  PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO IN END OF 
  BREAK NIL
  FUNCTION VAR TYPE

%type <A_var> lvalue
%type <A_exp> exp exps pri_exp or_exp and_exp rel_exp add_exp times_exp
%type <A_dec> method_dec nametype_dec var_dec dec
%type <A_ty> type
%type <A_field> type_field
%type <A_fieldList> type_fields function_fields
%type <A_expList> function_args exp_list

%type <A_decList> decs
%type <A_efield> record_field
%type <A_efieldList> record_fields

%nonassoc ASSIGN

%left OR AND

%nonassoc GT LT NEQ EQ LE GE

%left PLUS MINUS
%left TIMES DIVIDE

%left UMINUS

%start program

%%

program: decs {absyn_root=$1;}

dec: <varDec>
       | id '(' <paramList> ')' '{' <block> '}'
       | <type> id '(' <paramList> ')' '{' <block> '}'

<varDec>: <type> <varSpecSeq> ';'

<varSpec>: id
           | id '=' <literal>
           | id '[' num ']'
           | id '[' num ']' '=' '{' <literalSeq> '}'

<type>: "int"
        | "string"
        | "bool"

<param>: <type> id
         | <type> id '[' ']'

<block>: <varDecList> <stmtList>

<stmt>: <ifStmt>
        | <whileStmt>
        | <forStmt>
        | <breakStmt>
        | <returnStmt>
        | <readStmt>
        | <writeStmt>
        | <assign> ';'
        | <subCall> ';'

<ifStmt>: "if" '(' <exp> ')' '{' <block> '}'
          | "if" '(' <exp> ')' '{' <block> '}' "else" '{' <block> '}'

<whileStmt>: "while" '(' <exp> ')' '{' <block> '}'

<forStmt>: "for" '(' <assign> ';' <exp> ';' <assign> ')' '{' <block> '}'

<breakStmt>: "break" ';'

<readStmt>: "read" <var> ';'

<writeStmt>: "write" <expList> ';'

<returnStmt>: "return" ';'
              | "return" <exp> ';'

<subCall>: id '(' <expList> ')'

<assign>: <var> '='  <exp>
          | <var> "+=" <exp>
          | <var> "-=" <exp>
          | <var> "*=" <exp>
          | <var> "/=" <exp>
          | <var> "%=" <exp>

<var>: id
       | id '[' <exp> ']'

<exp>: <exp> '+'   <exp>
       | <exp> '-'   <exp>
       | <exp> '*'   <exp>
       | <exp> '/'   <exp>
       | <exp> '%'   <exp>
       | <exp> "=="  <exp>
       | <exp> "!="  <exp>
       | <exp> "<="  <exp>
       | <exp> ">="  <exp>
       | <exp> '>'   <exp>
       | <exp> '<'   <exp>
       | <exp> "&&"  <exp>
       | <exp> "||"  <exp>
       | '!' <exp>
       | '-' <exp>
       | <exp> '?' <exp> ':' <exp>
       | <subCall>
       | <var>
       | <literal>
       | '(' <exp> ')'

<literal>: num
           | str
           | logic

<paramList>: <paramSeq>
             | ε

<paramSeq>: <param> ',' <paramSeq>
            | <param>

<varDecList>: <varDec> <varDecList>
              | ε

<varSpecSeq>: <varSpec> ',' <varSpecSeq>
              | <varSpec>

decs: dec decs
          | dec

<stmtList>: <stmt> <stmtList>
            | ε

<literalSeq>: <literal> <literalSeq>
              | <literal>

<expList>: <expSeq>
           | ε

<expSeq>: <exp> ',' <expSeq>
          | <exp>