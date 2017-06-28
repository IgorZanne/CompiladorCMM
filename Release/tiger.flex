/* scanner for a toy Pascal-like language */
  
%{
#define MAX_STR_SIZE 1024

/* need this for the call to atof() below */
#include <math.h>
#include "util.h"
#include "parser.tab.h"
#include "errormsg.h"

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
HEX		  [a-fA-F0-9]{2}
SPECIAL   [-_.,;:?^~!@#$%&´`+/§¨=*()¹²³£¢¬\{\}\[\]àáãäâèéẽëêìíĩïîòóöôõùúûũüçÀÁÂÃÄÈÉẼÊËÌÍÏĨÎÒÓÕÔÖÙÚŨÛÜÇ]
     
%%

array {
	adjust();
	return ARRAY;
}

if {
	adjust();
	return IF;
}

then {
	adjust();
	return THEN;
}

else {
	adjust();
	return ELSE;
}

while {
	adjust();
	return WHILE;
}

for {
	adjust();
	return FOR;
}

to {
	adjust();
	return TO;
}

do {
	adjust();
	return DO;
}

let {
	adjust();
	return LET;
}

in {
	adjust();
	return IN;
}

end {
	adjust();
	return END;
}

of {
	adjust();
	return OF;
}

break {
	adjust();
	return BREAK;
}

nil {
	adjust();
	return NIL;
}

function {
	adjust();
	return FUNCTION;
}

var {
	adjust();
	return VAR;
}

type {
	adjust();
	return TYPE;
}


{ID} {
	char* name = (char*)malloc((yyleng+1)*sizeof(char));
	strcpy(name, yytext);

	yylval.sval = name;

	adjust();
	return ID;
}
     
{DIGIT}+ {
	adjust();
	yylval.ival = atoi(yytext);
	return INT;
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

		yylval.sval = (char*)str;

		BEGIN(INITIAL);
		return STRING;
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
			yyerror("string error 1");
		}
		
		*string_ptr++ = (char)i;
	}
	
	\\x{HEX} {/*FAZER \xNUM -> Conversão de hex para ASCII*/} 
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
		yyerror("string error 2");
	}
	. {
		adjust();
		yyerror("string error 3");
	}
}

"," {
	adjust();
	return COMMA;
}

":" {
	adjust();
	return COLON;
}

";" {
	adjust();
	return SEMICOLON;
}

"(" {
	adjust();
	return LPAREN;
}

")" {
	adjust();
	return RPAREN;
}

"[" {
	adjust();
	return LBRACK;
}

"]" {
	adjust();
	return RBRACK;
}

"{" {
	adjust();
	return LBRACE;
}

"}" {
	adjust();
	return RBRACE;
}

"." {
	adjust();
	return DOT;
}

"+" {
	adjust();
	return PLUS;
}

"-" {
	adjust();
	return MINUS;
}

"*" {
	adjust();
	return TIMES;
}

"/" {
	adjust();
	return DIVIDE;
}

"=" {
	adjust();
	return EQ;
}

"<>" {
	adjust();
	return NEQ;
}

"<" {
	adjust();
	return LT;
}

"<=" {
	adjust();
	return LE;
}

">" {
	adjust();
	return GT;
}

">=" {
	adjust();
	return GE;
}

"&" {
	adjust();
	return AND;
}

"|" {
	adjust();
	return OR;
}

":=" {
	adjust();
	return ASSIGN;
}
   
     
[ \t\n]+ {
	adjust();
}          /* eat up whitespace */
     
.           printf( "Unrecognized character: %s\n", yytext );
     
%%

void adjust(){
	if(strcmp(yytext, "\n") == 0){
		EM_newline();
		EM_tokPos = 0;
	} else {
		EM_tokPos++;
		//EM_tokPos += (int)strlen(yytext);
	}
}
     
int yywrap() {}

