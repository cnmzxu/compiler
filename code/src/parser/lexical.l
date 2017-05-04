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

#define MY_LEXSETUP(name) \
	Tree_Node *lexval = (Tree_Node *)malloc(sizeof(Tree_Node)); \
	yylval = lexval;\
	yylval->lineno = yylineno;\
	yylval->token_type = LEX_TOKEN;\
	strcpy(yylval->type, name)
	

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
INT_DEC [+-]?([1-9][0-9]*)|0
INT_HEX [+-]?0[xX][0-9a-fA-F]+
INT_OCT [+-]?0[0-7]+
ERROR_HEX 0[xX][0-9a-zA-Z]+
ERROR_OCT 0[0-9]+
FLOAT_NOR [0-9]+\.[0-9]+
FLOAT_EXP (([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))(E|e)[+-]?[0-9]+
FLOAT [+-]?({FLOAT_NOR}|{FLOAT_EXP})
ERROR_FLOAT_EXP [0-9]*\.[0-9]*(E|e)[+-]?[0-9]*
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
	MY_LEXSETUP("STRUCT");
    return STRUCT;
}
{RETURN} {
	MY_LEXSETUP("RETURN");
    return RETURN;
}
{IF} {
	MY_LEXSETUP("IF"); 
	return IF;
}
{ELSE} {
	MY_LEXSETUP("ELSE");
	return ELSE;
}
{WHILE} {
	MY_LEXSETUP("WHILE");
    return WHILE;
}
{SEMI} {
	MY_LEXSETUP("SEMI");
    return SEMI;
}
{COMMA} {
	MY_LEXSETUP("COMMA");
	return COMMA;
}
{ASSIGNOP} {
	MY_LEXSETUP("ASSIGNOP");
    return ASSIGNOP;
}
{TYPE} {
	MY_LEXSETUP("TYPE");
	strcpy(yylval->value, yytext);
	yylval->token_type = LEX_TYPE;
    return TYPE;
}
{INT_DEC} {
	MY_LEXSETUP("INT");
	strcpy(yylval->value, yytext);
	yylval->token_type = LEX_INT;
    return INT;
}
{INT_HEX} {
	MY_LEXSETUP("INT");
	strcpy(yylval->value, yytext);
	yylval->token_type = LEX_INT;
    return INT;
}
{INT_OCT} {
	MY_LEXSETUP("INT");
	strcpy(yylval->value, yytext);
	yylval->token_type = LEX_INT;
    return INT;
}
{FLOAT} {
	MY_LEXSETUP("FLOAT");
	strcpy(yylval->value, yytext);
	yylval->token_type = LEX_FLOAT;
	return FLOAT;
}
{RELOP} {
	MY_LEXSETUP("RELOP");
    return RELOP;
}
{PLUS} {
	MY_LEXSETUP("PLUS");
    return PLUS;
}
{MINUS} {
	MY_LEXSETUP("MINUS");
    return MINUS;
}
{STAR} {
	MY_LEXSETUP("STAR");
    return STAR;
}
{DIV} {
	MY_LEXSETUP("DIV");
    return DIV;
}
{AND} {
	MY_LEXSETUP("AND");
    return AND;
}
{OR} {
	MY_LEXSETUP("OR");
    return OR;
}
{DOT} {
	MY_LEXSETUP("DOT");
    return DOT;
}
{NOT} {
	MY_LEXSETUP("NOT");
    return NOT;
}
{LP} {
	MY_LEXSETUP("LP");
    return LP;
}
{RP} {
	MY_LEXSETUP("RP");
    return RP;
}
{LB} {
	MY_LEXSETUP("LB");
    return LB;
}
{RB} {
	MY_LEXSETUP("RB");
    return RB;
}
{LC} {
	MY_LEXSETUP("LC");
    return LC;
}
{RC} {
	MY_LEXSETUP("RC");
    return RC;
}
{ID} {
	MY_LEXSETUP("ID");
	strcpy(yylval->value, yytext);
	yylval->token_type = LEX_ID;
    return ID;
}

{COMMENT_TYPE1} {
    char c = input();
	while (c != '\n')
		c = input();
}
{COMMENT_TYPE2} {
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
{ERROR_FLOAT_EXP} {
	lex_error_happen = 1;
	printf("Error Type A at Line %d: Illegal floating number '%s'.\n", yylineno, yytext);
	return FLOAT;
}
. {
	lex_error_happen = 1;
	printf("Error Type A at Line %d: Mysterious characters '%s'.\n", yylineno, yytext);
}
%%

