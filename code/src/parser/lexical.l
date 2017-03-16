%{
#include <stdio.h>
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
    printf("%s\n", "STRUCT");
}
{RETURN} {
    printf("%s\n", "RETURN");
}
{IF} {
    printf("%s\n", "IF");

}
{ELSE} {
    printf("%s\n", "ELSE");
}
{WHILE} {
    printf("%s\n", "WHILE");
}
{SEMI} {
    printf("%s\n", "SEMI");
}
{COMMA} {
    printf("%s\n", "COMMA");
}
{ASSIGNOP} {
    printf("%s\n", "ASSIGNOP");
}
{TYPE} {
    printf("%s: ", "TYPE");
    printf("%s\n", yytext);
}
{INT} {
    printf("%s: ", "INT");
    printf("%s\n", yytext);
}
{FLOAT} {
    printf("%s: ", "FLOAT");
    printf("%s\n", yytext);
}
{RELOP} {
    printf("%s\n", "RELOP");
}
{PLUS} {
    printf("%s", "PLUS");
}
{MINUS} {
    printf("%s\n", "MINUS");
}
{STAR} {
    printf("%s\n", "STAR");
}
{DIV} {
    printf("%s\n", "DIV");
}
{AND} {
    printf("%s\n", "AND");
}
{OR} {
    printf("%s\n", "OR");
}
{DOT} {
    printf("%s\n", "DOT");
}
{NOT} {
    printf("%s\n", "NOT");
}
{LP} {
    printf("%s\n", "LP");
}
{RP} {
    printf("%s\n", "RP");
}
{LB} {
    printf("%s\n", "LB");
}
{RB} {
    printf("%s\n", "RB");
}
{LC} {
    printf("%s\n", "LC");
}
{RC} {
    printf("%s\n", "RC");
}
{COMMENT1} {
    printf("%s\n", "COMMENT_TYPE1");
    char c = input();
	while (c != '\n')
		c = input();
}
{LCOMMENT} {
    printf("%s\n", "COMMENT_TYPE2");
    char c  = input();
	while (1){
		if (c == '*'){
			c = input();
			if (c == '/')
				break;
		}
		c = input();
	}
}
{BLANK} { }
{ID} {
    printf("%s: ", "ID");
    printf("%s\n", yytext);
}
. {
	printf("Error Type A at Line %d:%s\n", yylineno, yytext);
}
%%
int main(int argc,char** argv){
	printf("Parsering Start...\n");
	if (argc < 2){
		yylex();
	}
	else {
		int i = 0;
		for (i = 1; i < argc; i++){
			FILE *f = fopen(argv[i], "r");
			if (!f){
				perror(argv[i]);
				return 1;
			}
			yyrestart(f);
			yylineno = 1;
			yylex();
			fclose(f);
		}
	}
	printf("Parsering End.\n");
	return 0;
}
