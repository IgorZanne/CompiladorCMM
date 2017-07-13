/* scanner for a toy Pascal-like language */
%option noyywrap  
%{
#define MAX_STR_SIZE 1024

/* need this for the call to atof() below */
#include <math.h>
#include <string.h>
#include "errormsg.h"
#include "util.h"


int nesting = 0;
char str[MAX_STR_SIZE];
char* string_ptr;

void adjust();

%}

%x IN_COMMENT
%x IN_STRING

LETTER    [a-zA-Z]     
DIGIT     [0-9]
ID        {LETTER}({LETTER}|{DIGIT}|"_")*
SPECIAL   [-_.,;:?^~!@#$%&´`+/§¨=*()¹²³£¢¬\{\}\[\]àáãäâèéẽëêìíĩïîòóöôõùúûũüçÀÁÂÃÄÈÉẼÊËÌÍÏĨÎÒÓÕÔÖÙÚŨÛÜÇ]
     
%%

array {
	adjust();
	printf("ARRAY ");
}

if {
	adjust();
	printf("IF ");
}

else {
	adjust();
	printf("ELSE ");
}

read {
	adjust();
	printf("READ ");
}

write {
	adjust();
	printf("WRITE ");
}

while {
	adjust();
	printf("WHILE ");
}

for {
	adjust();
	printf("FOR ");
}

bool {
	adjust();
	printf("BOOL ");
}

false {
	adjust();
	printf("FALSE ");
}

true {
	adjust();
	printf("TRUE ");
}

return {
	adjust();
	printf("RETURN ");
}

string {
	adjust();
	printf("STRING ");
}

int {
	adjust();
	printf("INT ");
}

break {
	adjust();
	printf("BREAK ");
}



{ID} {
	char* name = (char*)malloc((yyleng+1)*sizeof(char));
	strcpy(name, yytext);

	//yylval.sval = name;

	adjust();
	printf("ID ");
}
     
{DIGIT}+ {
	adjust();
	//yylval.ival = atoi(yytext);
	printf("INT ");
}

     
"/*" {
	adjust();
	nesting++;
	BEGIN(IN_COMMENT); 
}


<IN_COMMENT>
{
	"/*" {
		adjust();
		nesting++;
	}
	"*/" {
		adjust();
		if(--nesting == 0) BEGIN(INITIAL);
	}
	. {}
}

"\"" {
	adjust();
	string_ptr = str;
	BEGIN(IN_STRING);
}

<IN_STRING>
{
	\" {
		adjust();
		
		*string_ptr = '\0';

		//yylval.sval = (char*)str;

		BEGIN(INITIAL);
		printf("STRING ");
	}
	({LETTER}|{DIGIT}|{SPECIAL}|" ")* {
		int i;
		adjust();
		for(i = 0; i < strlen(yytext); i++) {
			*string_ptr++ = yytext[i];
		} 
	}
	\\a {
		adjust();
		*string_ptr++ = '\a';
	}
	\\b {
		adjust();
		*string_ptr++ = '\b';
	}
	\\f {
		adjust();
		*string_ptr++ = '\f';
	}
	\\n {
		adjust();
		*string_ptr++ = '\n';
	}
	\\r {
		adjust();
		*string_ptr++ = '\r';
	}
	\\t {
		adjust();
		*string_ptr++ = '\t';
	}
	\\v {
		adjust();
		*string_ptr++ = '\v';
	}
	\\{DIGIT}{3} {
		adjust();
		int i = atoi(&yytext[1]);
		if(i > 255){
			//yyerror("string error 1");
		}
		
		*string_ptr++ = (char)i;
	}

	\\ {
		adjust();
		*string_ptr++ = '\\';
	}
	\\\" {
		adjust();
		*string_ptr++ = '"';
	}
	<<EOF>> {
		adjust();
		//yyerror("string error 2");
	}
	. {
		adjust();
		//yyerror("string error 3");
	}
}

"(" {
	adjust();
	printf("LPAREN ");
}

")" {
	adjust();
	printf("RPAREN ");
}

"[" {
	adjust();
	printf("LBRACK ");
}

"]" {
	adjust();
	printf("RBRACK ");
}

"{" {
	adjust();
	printf("LBRACE ");
}

"}" {
	adjust();
	printf("RBRACE ");
}

"," {
	adjust();
	printf("COMMA ");
}

";" {
	adjust();
	printf("SEMICOLON ");
}

"." {
	adjust();
	printf("DOT ");
}

"+" {
	adjust();
	printf("PLUS ");
}

"-" {
	adjust();
	printf("MINUS ");
}

"*" {
	adjust();
	printf("TIMES ");
}

"/" {
	adjust();
	printf("DIVIDE ");
}


"==" {
	adjust();
	printf("EQ ");
}

"!=" {
	adjust();
	printf("NEQ ");
}

">" {
	adjust();
	printf("GT ");
}

">=" {
	adjust();
	printf("GE ");
}

"<" {
	adjust();
	printf("LT ");
}

"<=" {
	adjust();
	printf("LE ");
}

"||" {
	adjust();
	printf("OR ");
}

"&&" {
	adjust();
	printf("AND ");
}

"!" {
	adjust();
	printf("NOT ");
}

"=" {
	adjust();
	printf("ASSIGN ");
}

"+=" {
	adjust();
	printf("INC ");
}

"-=" {
	adjust();
	printf("DEC ");
}
	
"*=" {
	adjust();
	printf("MULT ");
}

"/=" {
	adjust();
	printf("MOD ");
}

"%=" {
	adjust();
	printf("REST ");
}

"?" {
	adjust();
	printf("QUESTION ");
}
   	
":" {
	adjust();
	printf("COLON ");
}
     
[ \t\n]+ {
	adjust();
}          /* eat up whitespace */
     
.           printf( "Unrecognized character: %s\n");
     
%%

void adjust(){
	if(strcmp(yytext, "\n") == 0){
		EM_newline();
		EM_tokPos = 0;
	} else {
		EM_tokPos++;
		EM_tokPos += (int)strlen(yytext);
	}
}
     
int main (int argc, char **argv) {  
  yyin = fopen(argv[1],"r");
  yylex();
}

