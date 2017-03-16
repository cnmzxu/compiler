%{
#include <stdio.h>
#include "syntax.tab.h"
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
BLANK [\n\t ]+
INT_DEC [+-]?(([1-9][0-9]*)|0)
INT_HEX [+-]?0x([0-9]|[a-f]|[A-F])+
INT_OCT [+-]?0[0-7]+
INT {INT_DEC}|{INT_HEX}|{INT_OCT}
FLOAT_NOR [+-]?[0-9]+\.[0-9]+
FLOAT_EXP [+-]?(([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))(E|e)[+-]?[0-9]+
FLOAT {FLOAT_NOR}|{FLOAT_EXP}
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
COMMENT1 "//"
LCOMMENT "/*"
RCOMMENT "*/"
ID [_a-zA-Z][_a-zA-Z0-9]*
%%
{STRUCT} {
    return STRUCT;
}
{RETURN} {
    return RETURN;
}
{IF} {
    return IF;
}
{ELSE} {
    return ELSE;
}
{WHILE} {
    return WHILE;
}
{SEMI} {
    return SEMI;
}
{COMMA} {
    return COMMA;
}
{ASSIGNOP} {
    return ASSIGNOP;
}
{TYPE} {
    return TYPE;
}
{INT} {
    return INT;
}
{FLOAT} {
    return FLOAT;
}
{RELOP} {
    return RELOP;
}
{PLUS} {
    return PLUS;
}
{MINUS} {
    return MINUS;
}
{STAR} {
    return STAR;
}
{DIV} {
    return DIV;
}
{AND} {
    return AND;
}
{OR} {
    return OR;
}
{DOT} {
    return DOT;
}
{NOT} {
    return NOT;
}
{LP} {
    return LP;
}
{RP} {
    return RP;
}
{LB} {
    return LB;
}
{RB} {
    return RB;
}
{LC} {
    return LC;
}
{RC} {
    return RC;
}
{ID} {
    return ID;
}

{COMMENT1} {
    printf("%s\n", "COMMENT_TYPE1");
    char c = input();
	while (c != '\n')
		c = input();
}
{LCOMMENT} {
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
{BLANK} { }

. {
	printf("Error Type A at Line %d: Mysterious characters '%s''\n", yylineno, yytext);
	return ERROR;
}
%%

