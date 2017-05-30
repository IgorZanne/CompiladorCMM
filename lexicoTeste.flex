/* scanner for a toy Pascal-like language */
  
%{
/* need thi	s for the call to atof() below */
#include <math.h>
#include <string.h>
#include "tiger.tab.h"
#include "errormsg.h"

int char_pos = 1;

int countCom;

char string[1000];

int yywrap(void);

void adjust(int QuebraLinha);

//extern YYLTYPE yylloc;

//extern YYSTYPE yylval;


int yywrap(void) {
   char_pos=1;
   //return 1;
     }

void adjust(int QuebraLinha) {
 }



%}

%x IN_STRING
%x IN_COMMENT
     
DIGITO          [0-9]+
ID              [a-z][a-z0-9]*

     
%%    

" "           {
                 adjust(0);
              }

\n            { 
                     adjust(1);
                     ////EM_newline;
                     //continue;
              }

\r            { 
                     adjust(1);
                     ////EM_newline;
                     //continue;
              }


\n\r          { 
                     adjust(1);
                     ////EM_newline;
                     //continue;
              }

\r\n          { 
                     adjust(1);
                     ////EM_newline;
                     //continue;
              }

\t          { 
                     adjust(1);
                     ////EM_newline;
                     //continue;
              }


"array"       {     adjust(0);
                     printf("ARRAY ");
                     //return (ARRAY);
              }

"if"          {      adjust(0);
                     printf("IF ");
                     //return (IF);
               }

"then"        {      adjust(0);         
                     printf("THEN ");
                     //return (THEN);
                     }

"else"        {      adjust(0);
                     printf("ELSE ");
                     //return (ELSE);
                     }

"while"       {      adjust(0);
                     printf("WHILE ");
                     //return (WHILE);
                     }

"for"         {      adjust(0);
                     printf("FOR ");
                     //return (FOR);
                     }

"to"          {
                     adjust(0);
                     printf("TO ");
                     //return (TO);
                     }

"do"          {
                     adjust(0);
                     printf("DO ");
                     //return (DO);
                     }

"let"         {
                     adjust(0);
                     printf("LET ");
                     //return (LET);
                     }

"in"          {
                     adjust(0);
                     printf("IN ");
                     //return (IN);
                     }

"end"         {
                     adjust(0);
                     printf("END ");
                     //return (END);
                     }


"of"          {
                     adjust(0);
                     printf("OF ");
                     //return (OF);
                     }

"break"       {
                     adjust(0);
                     printf("BREAK ");
                     //return (BREAK);
                     }

"nil"         {
                     adjust(0);
                     printf("NIL ");
                     //return (NIL);
                     }

"procedure"   {
                     adjust(0);
                     printf("PROCEDURE ");
                     //return (PROCEDURE);
                     }

"funcion"     {
                     adjust(0);
                     printf("FUNCTION ");
                     //return (FUNCTION);
                     }

"var"         {
                     adjust(0);
                     printf("VAR ");
                     //return (VAR);
                     }

"type"        {
                     adjust(0);
                     printf("TYPE ");
                     //return (TYPE);
                     }


","        {
                     adjust(0);
                     printf("COMMA ");
                     //return (COMMA);
                     }

":"        {
                     adjust(0);
                     printf("COLON ");
                     //return (COLON);
                     }

";"        {
                     adjust(0);
                     printf("SEMICOLON ");
                     //return (SEMICOLON);
                     }

"("        {
                     adjust(0);
                     printf("LPAREN ");
                     //return (LPAREN);
                     }
")"        {
                     adjust(0);
                     printf("RPAREN ");
                     //return (RPAREN);
                     }

"["        {
                     adjust(0);
                     printf("LBRACK ");
                     //return (LBRACK);
                     }

"]"        {
                     adjust(0);
                     printf("RBRACK ");
                     //return (RBRACK);
                     }


"{"        {
                     adjust(0);
                     printf("LBRACE ");
                     //return (LBRACE);
                     }

"}"        {
                     adjust(0);
                     printf("RBRACE ");
                     //return (RBRACE);
                     }

"."        {
                     adjust(0);
                     printf("DOT ");
                     //return (DOT);
                     }

"+"        {
                     adjust(0);
                     printf("PLUS ");
                     //return (PLUS);
                     }

"-"        {
                     adjust(0);
                     printf("MINUS ");
                     //return (MINUS);
                     }

"*"        {
                     adjust(0);
                     printf("TIMES ");
                     //return (TIMES);
                     }

"/"        {
                     adjust(0);
                     printf("DIVIDE ");
                     //return (DIVIDE);
                     }

"="        {
                     adjust(0);
                     printf("EQ ");
                     //return (EQ);
                     }

"<>"       {
                     adjust(0);
                     printf("NEQ ");
                     //return (NEQ);
                     }

"<"        {
                     adjust(0);
                     printf("LT ");
                     //return (LT);
                     }

"<="       {
                     adjust(0);
                     printf("LE ");
                     //return (LE);
                     }

">"        {
                     adjust(0);
                     printf("GT ");
                     //return (GT);
                     }

">="       {
                     adjust(0);
                     printf("GE ");
                     //return (GE);
                     }


"&"        {
                     adjust(0);
                     printf("AND ");
                     //return (AND);
                     }


"|"        {
                     adjust(0);
                     printf("OR ");
                     //return (OR);
                     }

":="       {
                     adjust(0);
                     printf("ASSIGN ");
                     //return (ASSIGN);
                     }
"import"       {
                     adjust(0);
                     printf("IMPORT ");
                     //return (IMPORT);
                     }

"primitive"        {
                     adjust(0);
                     printf("PRIMITIVO ");
                     //return (PRIMITIVO);
                     }

{DIGITO}{ID} {   //yyerror...
	             printf("Erro lexico:Token Invalido");

}

{ID}{DIGITO}+"."{DIGITO} {   //yyerror...
	             printf("Erro lexico:Token Invalido");

}                      

{DIGITO}    { //y//yylval.ival = atoi(yytext); 
             adjust(0);
             printf("INT ");
             //return (INT);
             }
     
{DIGITO}+"."{DIGITO}+        {
                    adjust(0);
                    printf("FLOAT ");
                    //return (FLOAT);
                     }
     
     
{ID}                { adjust(0);
                      //yylval.sval = strdup(yytext);                       
                      printf("ID ");
                      //return (ID);
                     }

"\""                { adjust(0);
	                  strcat(string,yytext);
                      BEGIN(IN_STRING);                      
                    }

"/*"                { adjust(0);
                      countCom++;                      
                      BEGIN(IN_COMMENT);
                    }
     
<IN_COMMENT>{
    "/*" {countCom++; adjust(0);}
    \n   {adjust(0);}   
     .   {adjust(0);}
    "*/" {adjust(0); if (--countCom == 0) { BEGIN(INITIAL);}}
}

<IN_STRING>{     
     "\"" {adjust(0); strcat(string,yytext); printf("%s",string); strcpy(string,""); BEGIN(INITIAL);  printf("STRING");}
     .    {adjust(0); strcat(string,yytext);}
     
     
}
     


. { printf("Erro lexico: Token Invalido");}
	     
%%

int main (int argc, char **argv) {  
  yyin = fopen(argv[1],"r");
  yylex();
}

