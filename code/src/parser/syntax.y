%locations
%{
#include <string.h>
#include <stdlib.h>
#include "parser/parser.h"
void yyerror(const char *msg);

#define YYSTYPE_IS_DECLARED 1
typedef Tree_Node* YYSTYPE;
YYSTYPE head;
int parser_error_happen = 0;

#include "lex.yy.c"

#define EMPTY_SETUP \
	(yyval) = empty_setup((yyloc).first_line);

#define ERROR_SETUP \
	(yyval) = error_setup((yyloc).first_line);

#define MY_SYNSETUP(str) \
	(yyval) = common_setup(str, (yyloc).first_line

#define SIBLING \
	, (yyvsp[0]), NULL);

#define SIBLING2 \
	, (yyvsp[-1]) \
	SIBLING\

#define SIBLING3 \
	, (yyvsp[-2]) \
	SIBLING2 \

#define SIBLING4 \
	, (yyvsp[-3]) \
	SIBLING3 \

#define SIBLING5 \
	, (yyvsp[-4]) \
	SIBLING4 \

#define SIBLING6 \
	, (yyvsp[-5]) \
	SIBLING5 \

#define SIBLING7 \
	, (yyvsp[-6]) \
	SIBLING6\

%}



%token INT FLOAT 
%token ID ERROR
%token RELOP PLUS MINUS STAR DIV AND OR NOT DOT
%token LP RP LB RB LC RC
%token SEMI COMMA
%token TYPE STRUCT RETURN 
%token WHILE IF ELSE
%token ASSIGNOP

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT LP RP LB RB

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%error-verbose

%%
Program : ExtDefList {
		MY_SYNSETUP("Program")
		SIBLING
		head = $$;
	}
		;
ExtDefList : ExtDef ExtDefList {
		MY_SYNSETUP("ExtDefList")
		SIBLING2
	}
		   | /* empty */ {
		EMPTY_SETUP
	}
		   ;
ExtDef : Specifier ExtDecList SEMI {
		MY_SYNSETUP("ExtDef")
		SIBLING3
	}
	   | Specifier SEMI {
		MY_SYNSETUP("ExtDef")
		SIBLING2
	}
	   | Specifier FunDec CompSt {
		MY_SYNSETUP("ExtDef")
		SIBLING3
	}  | Specifier FunDec SEMI {
		MY_SYNSETUP("ExtDef")
		SIBLING3
	}
	   ;
ExtDecList : VarDec {
		MY_SYNSETUP("ExtDecList")
		SIBLING
	}
		   | VarDec COMMA ExtDecList {
		MY_SYNSETUP("ExtDecList")
		SIBLING3
	}
		   ;
/* Specifiers */
Specifier : TYPE {
		MY_SYNSETUP("Specifier")
		SIBLING
	}
		  | StructSpecifier {
		MY_SYNSETUP("Specifier")
		SIBLING
	}
		  ;
StructSpecifier : STRUCT OptTag LC DefList RC {
		MY_SYNSETUP("StructSpecifier")
		SIBLING5
	}
				| STRUCT Tag {
		MY_SYNSETUP("StructSpecifier")
		SIBLING2
	} 
				;
OptTag : ID {
		MY_SYNSETUP("OptTag")
		SIBLING
	}
	   | /* empty */ {
		EMPTY_SETUP
	}
	   ;
Tag : ID {
		MY_SYNSETUP("Tag")
		SIBLING
	}
	;
/* Declarators */
VarDec : ID {
		MY_SYNSETUP("VarDec")
		SIBLING
	}
	   | VarDec LB INT RB {
		MY_SYNSETUP("VarDec")
		SIBLING4
	}
	   ;
FunDec : ID LP VarList RP {
		MY_SYNSETUP("FunDec")
		SIBLING4
	}
	   | ID LP RP{
		MY_SYNSETUP("FunDec")
		SIBLING3
	}
	   ;
VarList : ParamDec COMMA VarList {
		MY_SYNSETUP("VarList")
		SIBLING3
	}
		| ParamDec {
		MY_SYNSETUP("VarList")
		SIBLING
	}
		;
ParamDec : Specifier VarDec {
		MY_SYNSETUP("ParamDec")
		SIBLING2
	}
		 ;
/*Statements*/
CompSt : LC DefList StmtList RC {
		MY_SYNSETUP("CompSt")
		SIBLING4
	}
	   | error RC {
		ERROR_SETUP
	}
	   ;
StmtList : Stmt StmtList {
		MY_SYNSETUP("StmtList")
		SIBLING2
	}
		 | /* empty */{
		EMPTY_SETUP
	}
		 ;
Stmt : Exp SEMI {
		MY_SYNSETUP("Stmt")
		SIBLING2
	}
	 | CompSt {
		MY_SYNSETUP("Stmt")
		SIBLING
	}
	 | RETURN Exp SEMI {
		MY_SYNSETUP("Stmt")
		SIBLING3
	}
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
		MY_SYNSETUP("Stmt")
		SIBLING5
	}
	 | IF LP Exp RP Stmt ELSE Stmt {
		MY_SYNSETUP("Stmt")
		SIBLING7
	}
	 | WHILE LP Exp RP Stmt {
		MY_SYNSETUP("Stmt")
		SIBLING5
	}
	 | error SEMI {
		ERROR_SETUP
	}
	 ;
/* Local Definitions */
DefList : Def DefList {
		MY_SYNSETUP("DefList")
		SIBLING2
	}
		| /* empty */{
		EMPTY_SETUP
	}
		;
Def : Specifier DecList SEMI {
		MY_SYNSETUP("Def")
		SIBLING3
	}
	;
DecList : Dec {
		MY_SYNSETUP("DecList")
		SIBLING
	}
		| Dec COMMA DecList {
		MY_SYNSETUP("DecList")
		SIBLING3
	}
		;
Dec : VarDec {
		MY_SYNSETUP("Dec")
		SIBLING
	}
	| VarDec ASSIGNOP Exp {
		MY_SYNSETUP("Dec")
		SIBLING3
	}
	;
/* Expressions */
Exp : LExp {
		MY_SYNSETUP("Exp")
		SIBLING
	}
	| RExp {
		MY_SYNSETUP("Exp")
		SIBLING
	}
	;

RExp : Exp ASSIGNOP Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	}
	| Exp AND Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	}
	| Exp OR Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	}
	| Exp RELOP Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	}
	| Exp PLUS Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	} 
	| Exp MINUS Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	}
	| Exp STAR Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	} 
	| Exp DIV Exp {
		MY_SYNSETUP("RExp")
		SIBLING3
	} 
	| LP Exp RP {
		MY_SYNSETUP("RExp")
		SIBLING3
	}
	| MINUS Exp {
		MY_SYNSETUP("RExp")
		SIBLING2
 	} 
	| NOT Exp {
		MY_SYNSETUP("RExp")
		SIBLING2
	} 
	| ID LP Args RP {
		MY_SYNSETUP("RExp")
		SIBLING4
	} 
	| ID LP RP {
		MY_SYNSETUP("RExp")
		SIBLING3
	}
	| INT { 
		MY_SYNSETUP("RExp")
		SIBLING
	} 
	| FLOAT {
		MY_SYNSETUP("RExp")
		SIBLING
	}
	;

LExp : ID {
		MY_SYNSETUP("LExp")
		SIBLING
	 }
	 | Exp LB Exp RB {
		MY_SYNSETUP("LExp")
		SIBLING4
	 }
	 | Exp DOT ID {
		MY_SYNSETUP("LExp")
		SIBLING3
	 }
	 ;

Args : Exp COMMA Args {
		MY_SYNSETUP("Args")
		SIBLING3
	}
	 | Exp {
		MY_SYNSETUP("Args")
		SIBLING
	}
	 ;
%%
void yyerror(const char *msg){
	parser_error_happen = 1;
	fprintf(stderr, "Error type B at Line %d: %s.\n", yylineno, msg);
	return;
}
