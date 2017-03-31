%{
#include <stdio.h>
#include <stdlib.h>
#include "syntax.tab.h"

int yycolumn = 0;
#define YY_USER_ACTION \
	yylloc.first_line = yylloc.last_line = yylineno; \
	yylloc.first_column = yycolumn; \
	yylloc.last_column = yycolumn + yyleng - 1; \
	yycolumn += yyleng - 1;

#define MY_LEXSETUP \
	Node *lexval = (Node *)malloc(sizeof(Node)); \
	yylval = lexval;\
	yylval->lineno = yylineno;\
	yylval->is_token = 1;

#define MY_LEXSETUP_END
int lex_error_happen;
%}
%option yylineno


STRUCT "struct"
RETURN "return"
IF "if"
ELSE "else"
WHILE "while"
SEMI ";"
COMMA ","
ASSIGNOP "="
TYPE "int"|"float"
BLANK [\n\t\r\f\v ]+
INT_DEC ([1-9][0-9]*)|0
INT_HEX 0[xX][0-9a-fA-F]+
INT_OCT 0[0-7]+
INT [+-]?({INT_DEC}|{INT_HEX}|{INT_OCT})
ERROR_HEX 0[xX][0-9a-zA-Z]+
ERROR_OCT 0[0-9]+
FLOAT_NOR [0-9]+\.[0-9]+
FLOAT_EXP (([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))(E|e)[+-]?[0-9]+
FLOAT [+-]?({FLOAT_NOR}|{FLOAT_EXP})
RELOP >|<|>=|<=|==|!=
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
COMMENT_TYPE1 "//"
COMMENT_TYPE2 "/*"
ID [_a-zA-Z][_a-zA-Z0-9]*
%%
{STRUCT} {
	MY_LEXSETUP
	strcpy(yylval->type, "STRUCT");
	MY_LEXSETUP_END
    return STRUCT;
}
{RETURN} {
	MY_LEXSETUP
	strcpy(yylval->type, "RETURN");
	MY_LEXSETUP_END
    return RETURN;
}
{IF} {
	MY_LEXSETUP
	strcpy(yylval->type, "IF");
	MY_LEXSETUP_END
    return IF;
}
{ELSE} {
	MY_LEXSETUP
	strcpy(yylval->type, "ELSE");
	MY_LEXSETUP_END
    return ELSE;
}
{WHILE} {
	MY_LEXSETUP
	strcpy(yylval->type, "WHILE");
	MY_LEXSETUP_END
    return WHILE;
}
{SEMI} {
	MY_LEXSETUP
	strcpy(yylval->type, "SEMI");
	MY_LEXSETUP_END
    return SEMI;
}
{COMMA} {
	MY_LEXSETUP
	strcpy(yylval->type, "COMMA");
	MY_LEXSETUP_END
    return COMMA;
}
{ASSIGNOP} {
	MY_LEXSETUP
	strcpy(yylval->type, "ASSIGNOP");
	MY_LEXSETUP_END
    return ASSIGNOP;
}
{TYPE} {
	MY_LEXSETUP
	strcpy(yylval->type, "TYPE");
	strcpy(yylval->value, yytext);
	MY_LEXSETUP_END
    return TYPE;
}
{INT} {
	MY_LEXSETUP
	strcpy(yylval->type, "INT");
	strcpy(yylval->value, yytext);
	MY_LEXSETUP_END
    return INT;
}
{FLOAT} {
	MY_LEXSETUP
	strcpy(yylval->type, "FLOAT");
	strcpy(yylval->value, yytext);
	MY_LEXSETUP_END
	return FLOAT;
}
{RELOP} {
	MY_LEXSETUP
	strcpy(yylval->type, "RELOP");
	MY_LEXSETUP_END
    return RELOP;
}
{PLUS} {
	MY_LEXSETUP
	strcpy(yylval->type, "PLUS");
	MY_LEXSETUP_END
    return PLUS;
}
{MINUS} {
	MY_LEXSETUP
	strcpy(yylval->type, "MINUS");
	MY_LEXSETUP_END
    return MINUS;
}
{STAR} {
	MY_LEXSETUP
	strcpy(yylval->type, "STAR");
	MY_LEXSETUP_END
    return STAR;
}
{DIV} {
	MY_LEXSETUP
	strcpy(yylval->type, "DIV");
	MY_LEXSETUP_END
    return DIV;
}
{AND} {
	MY_LEXSETUP
	strcpy(yylval->type, "AND");
	MY_LEXSETUP_END
    return AND;
}
{OR} {
	MY_LEXSETUP
	strcpy(yylval->type, "OR");
	MY_LEXSETUP_END
    return OR;
}
{DOT} {
	MY_LEXSETUP
	strcpy(yylval->type, "DOT");
	MY_LEXSETUP_END
    return DOT;
}
{NOT} {
	MY_LEXSETUP
	strcpy(yylval->type, "NOT");
	MY_LEXSETUP_END
    return NOT;
}
{LP} {
	MY_LEXSETUP
	strcpy(yylval->type, "LP");
	MY_LEXSETUP_END
    return LP;
}
{RP} {
	MY_LEXSETUP
	strcpy(yylval->type, "RP");
	MY_LEXSETUP_END
    return RP;
}
{LB} {
	MY_LEXSETUP
	strcpy(yylval->type, "LB");
	MY_LEXSETUP_END
    return LB;
}
{RB} {
	MY_LEXSETUP
	strcpy(yylval->type, "RB");
	MY_LEXSETUP_END
    return RB;
}
{LC} {
	MY_LEXSETUP
	strcpy(yylval->type, "LC");
	MY_LEXSETUP_END
    return LC;
}
{RC} {
	MY_LEXSETUP
	strcpy(yylval->type, "RC");
	MY_LEXSETUP_END
    return RC;
}
{ID} {
	MY_LEXSETUP
	strcpy(yylval->type, "ID");
	strcpy(yylval->value, yytext);
	MY_LEXSETUP_END
    return ID;
}

{COMMENT_TYPE1} {
    printf("%s\n", "COMMENT_TYPE1");
    char c = input();
	while (c != '\n')
		c = input();
}
{COMMENT_TYPE2} {
    printf("%s\n", "COMMENT_TYPE2");
    char c  = input(), d = input();
	while (1){
		if (c == '*' && d == '/'){
				break;
		}
		c = d;
		d = input();
	}
}
[\t\r\f\v ]+ { }
"\n" { yycolumn = 1; }

{ERROR_HEX} {
	lex_error_happen = 1;
	printf("Error Type A at Line %d: Illegal hexadecimal number '%s'.\n", yylineno, yytext);
	return INT;
}
{ERROR_OCT} {
	lex_error_happen = 1;
	printf("Error Type A at Line %d: Illegal octal number '%s'.\n", yylineno, yytext);
	return INT;
}
. {
	lex_error_happen = 1;
	printf("Error Type A at Line %d: Mysterious characters '%s'.\n", yylineno, yytext);
	return ERROR;
}
%%

