%{
#include <stdio.h>
%}

INT_DEC [+-]?(([1-9][0-9]*)|0)
INT_HEX [+-]?0x([0-9]|[a-f]|[A-F])+
INT_OCT [+-]?0[0-7]+
INT {INT_DEC}|{INT_HEX}|{INT_OCT}
FLOAT_NOR [+-]?[0-9]+\.[0-9]+
FLOAT_EXP [+-]?(([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))(E|e)[+-]?[0-9]+
FLOAT {FLOAT_NOR}|{FLOAT_EXP}
ID [_a-zA-Z][_a-zA-Z0-9]*
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
typr int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
COMMENT1 //.*$
LCOMMENT /*
RCOMMENT */
%%
{INT} {
    printf("%s\n", "INT");
    printf("%s\n", yytext);
}
{FLOAT} {
    printf("%s\n", "FLOAT");
    printf("%s\n", yytext);
}
{ID} {
    printf("%s\n", "ID");
    printf("%s\n", yytext);
}
{SEMI} {
    printf("%s\n", "SEMI");
    printf("%s\n", yytext);
}
{COMMA} {
    printf("%s\n", "COMMA");
    printf("%s\n", yytext);
}
{ASSIGNOP} {
    printf("%s\n", "ASSIGNOP");
    printf("%s\n", yytext);
}
{RELOP} {
    printf("%s\n", "RELOP");
    printf("%s\n", yytext);
}
{PLUS} {
    printf("%s\n", "PLUS");
    printf("%s\n", yytext);
}
{MINUS} {
    printf("%s\n", "MINUS");
    printf("%s\n", yytext);
}
{STAR} {
    printf("%s\n", "STAR");
    printf("%s\n", yytext);
}
{DIV} {
    printf("%s\n", "DIV");
    printf("%s\n", yytext);
}
{AND} {
    printf("%s\n", "AND");
    printf("%s\n", yytext);
}
{OR} {
    printf("%s\n", "OR");
    printf("%s\n", yytext);
}
{DOT} {
    printf("%s\n", "DOT");
    printf("%s\n", yytext);
}
{NOT} {
    printf("%s\n", "NOT");
    printf("%s\n", yytext);
}
{typr} {
    printf("%s\n", "typr");
    printf("%s\n", yytext);
}
{LP} {
    printf("%s\n", "LP");
    printf("%s\n", yytext);
}
{RP} {
    printf("%s\n", "RP");
    printf("%s\n", yytext);
}
{LB} {
    printf("%s\n", "LB");
    printf("%s\n", yytext);
}
{RB} {
    printf("%s\n", "RB");
    printf("%s\n", yytext);
}
{LC} {
    printf("%s\n", "LC");
    printf("%s\n", yytext);
}
{RC} {
    printf("%s\n", "RC");
    printf("%s\n", yytext);
}
{STRUCT} {
    printf("%s\n", "STRUCT");
    printf("%s\n", yytext);
}
{RETURN} {
    printf("%s\n", "RETURN");
    printf("%s\n", yytext);
}
{IF} {
    printf("%s\n", "IF");
    printf("%s\n", yytext);
}
{ELSE} {
    printf("%s\n", "ELSE");
    printf("%s\n", yytext);
}
{WHILE} {
    printf("%s\n", "WHILE");
    printf("%s\n", yytext);
}
{COMMENT1} {
    printf("%s\n", "COMMENT1");
    printf("%s\n", yytext);
}
{LCOMMENT} {
    printf("%s\n", "LCOMMENT");
    printf("%s\n", yytext);
}
{RCOMMENT} {
    printf("%s\n", "RCOMMENT");
    printf("%s\n", yytext);
}

%%
int main(int argc,char** argv){
	printf("Parsering Start...");
	if (argc < 2){
		yylex();
	}
	else {
		int i = 0;
		for (i = 0; i < argc; i++){
			FILE *f = fopen(argv[i], "r");
			if (!f){
				perror(argv[i]);
				return 1;
			}
			yyrestart(f);
			yylex();
			fclose(f);
		}
	}
	printf("Parsering End.");
	return 0;
}
