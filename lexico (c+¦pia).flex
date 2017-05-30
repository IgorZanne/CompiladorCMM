/* scanner for a toy Pascal-like language */
  
%{
/* need thi	s for the call to atof() below */
#include <math.h>
#include <string.h>
#include "tiger.tab.h"
#include "util.h"
#include "errormsg.h"

int char_pos = 1;

int countCom;

int yywrap(void);

void adjust(int QuebraLinha);

extern YYLTYPE yylloc;

extern YYSTYPE yylval;


int yywrap(void) {
   char_pos=1;
   return 1;
     }

void adjust(int QuebraLinha) {
   EM_tokPos=char_pos;
   char_pos+=yyleng;
   
   if (QuebraLinha = 0){
     yylloc.first_column =(yylloc.first_column + 1);
   }
   else
   {
     yylloc.first_line = (yylloc.first_line + 1);
     yylloc.first_column = 1;
   }
 }



%}

%x IN_STRING
%x IN_COMMENT
     
DIGITO          [0-9]
ID              [a-z][a-z0-9]*

     
%%    

" "           {      
                     adjust(0);
                     continue;
              }
 
\n            { 
                     adjust(1);
                     EM_newline;
                     continue;
              }

\r            { 
                     adjust(1);
                     EM_newline;
                     continue;
              }


\n\r          { 
                     adjust(1);
                     EM_newline;
                     continue;
              }

\r\n          { 
                     adjust(1);
                     EM_newline;
                     continue;
              }

"array"       {     adjust(0);
                     return (ARRAY);
              }

"if"          {      adjust(0);
                     return (IF);
               }

"then"        {      adjust(0);         
                     return (THEN);
                     }

"else"        {      adjust(0);
                     return (ELSE);
                     }

"while"       {      adjust(0);
                     return (WHILE);
                     }

"for"         {      adjust(0);
                     return (FOR);
                     }

"to"          {
                     adjust(0);
                     return (TO);
                     }

"do"          {
                     adjust(0);
                     return (DO);
                     }

"let"         {
                     adjust(0);
                     return (LET);
                     }

"in"          {
                     adjust(0);
                     return (IN);
                     }

"end"         {
                     adjust(0);
                     return (END);
                     }


"of"          {
                     adjust(0);
                     return (OF);
                     }

"break"       {
                     adjust(0);
                     return (BREAK);
                     }

"nil"         {
                     adjust(0);
                     return (NIL);
                     }

"procedure"   {
                     adjust(0);
                     return (PROCEDURE);
                     }

"funcion"     {
                     adjust(0);
                     return (FUNCTION);
                     }

"var"         {
                     adjust(0);
                     return (VAR);
                     }

"type"        {
                     adjust(0);
                     return (TYPE);
                     }


","        {
                     adjust(0);
                     return (COMMA);
                     }

":"        {
                     adjust(0);
                     return (COLON);
                     }

";"        {
                     adjust(0);
                     return (SEMICOLON);
                     }

"("        {
                     adjust(0);
                     return (LPAREN);
                     }
")"        {
                     adjust(0);
                     return (RPAREN);
                     }

"["        {
                     adjust(0);
                     return (LBRACK);
                     }

"]"        {
                     adjust(0);
                     return (RBRACK);
                     }


"{"        {
                     adjust(0);
                     return (LBRACE);
                     }

"}"        {
                     adjust(0);
                     return (RBRACE);
                     }

"."        {
                     adjust(0);
                     return (DOT);
                     }

"+"        {
                     adjust(0);
                     return (PLUS);
                     }

"-"        {
                     adjust(0);
                     return (MINUS);
                     }

"*"        {
                     adjust(0);
                     return (TIMES);
                     }

"/"        {
                     adjust(0);
                     return (DIVIDE);
                     }

"="        {
                     adjust(0);
                     return (EQ);
                     }

"<>"       {
                     adjust(0);
                     return (NEQ);
                     }

"<"        {
                     adjust(0);
                     return (LT);
                     }

"<="       {
                     adjust(0);
                     return (LE);
                     }

">"        {
                     adjust(0);
                     return (GT);
                     }

">="       {
                     adjust(0);
                     return (GE);
                     }


"&"        {
                     adjust(0);
                     return (AND);
                     }


"|"        {
                     adjust(0);
                     return (OR);
                     }

":="       {
                     adjust(0);
                     return (ASSIGN);
                     }
"import"       {
                     adjust(0);
                     return (IMPORT);
                     }

"primitive"        {
                     adjust(0);
                     return (PRIMITIVO);
                     }

{DIGITO}+    { yylval.ival = atoi(yytext); 
             adjust(0);
             return (INT);
             }
     
{DIGITO}+"."{DIGITO}*        {
                    adjust(0);
                    return (FLOAT);
                     }
     
     
{ID}                { adjust(0);
                      yylval.sval = strdup(yytext);                       
                      return (ID);
                     }

"\""                { adjust(0);
                      BEGIN(IN_STRING);
                    }

"/*"                { adjust(0);
                      countCom++;
                      BEGIN(IN_COMMENT);
                    }
     
<IN_COMMENT>{
    "/*" {countCom++; adjust(0);}
     .   {adjust(0);}
    "*/" {adjust(0); if (--countCom == 0) {BEGIN(INITIAL);}}
}

<IN_STRING>{
     adjust(0);
     yylval.sval=strdup(yytext);
     "\"" {BEGIN(INITIAL); return(STRING);}
     
}
	     
%%



