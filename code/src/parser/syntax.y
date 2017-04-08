%locations
%{
#include <string.h>
#include <parser/datastruct.h>
void yyerror(const char *msg);

#define YYSTYPE_IS_DECLARED 1
typedef Node* YYSTYPE;
Node space_for_head;
YYSTYPE head = &space_for_head;
int parser_error_happen = 0;

#include "lex.yy.c"

#define EMPTY_SETUP \
	Node *node = (Node *)malloc(sizeof(Node));\
	(yyval) = node; \
	(yyval)->child = (Node *)NULL; \
	(yyval)->sibling = (Node *)NULL; \
	strcpy((yyval)->type, "empty"); \
	(yyval)->lineno = (yyloc).first_line; \
	(yyval)->token_type = 0;

#define ERROR_SETUP \
	Node *node = (Node *)malloc(sizeof(Node));\
	(yyval) = node; \
	(yyval)->child = (Node *)NULL; \
	strcpy((yyval)->type, "error");\
	(yyval)->lineno = (yyloc).first_line;

#define MY_SYNSETUP(str, l) \
	Node *node = (Node *)malloc(sizeof(Node)); \
	(yyval) = node; \
	(yyval)->child = (yyvsp[-(l-1)]); \
	strcpy((yyval)->type, str);\
	(yyval)->lineno = (yyloc).first_line;\
	(yyval)->token_type = 0;

#define SIBLING

#define SIBLING2 \
	SIBLING\
	(yyvsp[-1])->sibling = (yyvsp[0]);

#define SIBLING3 \
	SIBLING2 \
	(yyvsp[-2])->sibling = (yyvsp[-1]);

#define SIBLING4 \
	SIBLING3 \
	(yyvsp[-3])->sibling = (yyvsp[-2]);

#define SIBLING5 \
	SIBLING4 \
	(yyvsp[-4])->sibling = (yyvsp[-3]);

#define SIBLING6 \
	SIBLING5 \
	(yyvsp[-5])->sibling = (yyvsp[-4]);

#define SIBLING7 \
	SIBLING6\
	(yyvsp[-6])->sibling = (yyvsp[-5]);

#define MY_SYNSETUP_END
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

%nonassoc ERROR_TYPE
%nonassoc RIGHT_TYPE

%%
Program : ExtDefList {
		$$ = head;
		if ($$){
			strcpy($$->type, "Program");
			$$->child = $1;
			$$->lineno = @$.first_line;
			$$->token_type = 0;
		}
	}
		;
ExtDefList : ExtDef ExtDefList {
		MY_SYNSETUP("ExtDefList", 2)
		SIBLING2
		MY_SYNSETUP_END
	}
		   | /* empty */ {
		EMPTY_SETUP
	}
		   ;
ExtDef : Specifier ExtDecList SEMI %prec RIGHT_TYPE {
		MY_SYNSETUP("ExtDef", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	   | Specifier ExtDecList %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("Missing ';'");
	}
	   | Specifier SEMI %prec RIGHT_TYPE {
		MY_SYNSETUP("ExtDef", 2)
		SIBLING2
		MY_SYNSETUP_END
	}
	   | Specifier %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("Missing ';'");
	}
	   | Specifier FunDec CompSt {
		MY_SYNSETUP("ExtDef", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	   ;
ExtDecList : VarDec {
		MY_SYNSETUP("ExtDecList", 1)
		SIBLING
		MY_SYNSETUP_END
	}
		   | VarDec COMMA ExtDecList {
		MY_SYNSETUP("ExtDecList", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
		   ;
/* Specifiers */
Specifier : TYPE {
		MY_SYNSETUP("Specifier", 1)
		SIBLING
		MY_SYNSETUP_END
	}
		  | StructSpecifier {
		MY_SYNSETUP("Specifier", 1)
		SIBLING
		MY_SYNSETUP_END
	}
		  ;
StructSpecifier : STRUCT OptTag LC DefList RC %prec RIGHT_TYPE {
		MY_SYNSETUP("StructSpecifier", 5)
		SIBLING5
		MY_SYNSETUP_END
	}
				| STRUCT OptTag LC DefList %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing '}'");
	}
				| STRUCT OptTag DefList RC %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing '{'");
	}
				| STRUCT Tag {
		MY_SYNSETUP("StructSpecifier", 2)
		SIBLING2
		MY_SYNSETUP_END
	} 
				;
OptTag : ID {
		MY_SYNSETUP("OptTag", 1)
		SIBLING
		MY_SYNSETUP_END
	}
	   | /* empty */ {
		EMPTY_SETUP
	}
	   ;
Tag : ID {
		MY_SYNSETUP("Tag", 1)
		SIBLING
		MY_SYNSETUP_END
	}
	;
/* Declarators */
VarDec : ID {
		MY_SYNSETUP("VarDec", 1)
		SIBLING
		MY_SYNSETUP_END
	}
	   | VarDec LB INT RB %prec RIGHT_TYPE {
		MY_SYNSETUP("VarDec", 4)
		SIBLING4
		MY_SYNSETUP_END
	}
	   | VarDec LB INT %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing ']'");
	}
	   | VarDec INT RB %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing '['");
	}
	   ;
FunDec : ID LP VarList RP %prec RIGHT_TYPE {
		MY_SYNSETUP("FunDec", 4)
		SIBLING4
		MY_SYNSETUP_END
	}
	   | ID LP VarList %prec ERROR_TYPE{
		ERROR_SETUP
		yyerror("missing ')'");
	}
	   | ID VarList RP %prec ERROR_TYPE{
		ERROR_SETUP
		yyerror("missing '('");
	}
	   | ID LP RP{
		MY_SYNSETUP("FunDec", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	   ;
VarList : ParamDec COMMA VarList {
		MY_SYNSETUP("VarList", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
		| ParamDec {
		MY_SYNSETUP("VarList", 1)
		SIBLING
		MY_SYNSETUP_END
	}
		;
ParamDec : Specifier VarDec {
		MY_SYNSETUP("ParamDec", 2)
		SIBLING2
		MY_SYNSETUP_END
	}
		 ;
/*Statements*/
CompSt : LC DefList StmtList RC %prec RIGHT_TYPE {
		MY_SYNSETUP("CompSt", 4)
		SIBLING4
		MY_SYNSETUP_END
	}
	   | DefList StmtList RC %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing '{'");
	}
	   | LC DefList StmtList %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing '}'");
	}
	   | error RC {
		ERROR_SETUP
	}
	   ;
StmtList : Stmt StmtList {
		MY_SYNSETUP("StmtList", 2)
		SIBLING2
		MY_SYNSETUP_END
	}
		 | /* empty */{
		EMPTY_SETUP
	}
		 ;
Stmt : Exp SEMI %prec RIGHT_TYPE {
		MY_SYNSETUP("Stmt", 2)
		SIBLING2
		MY_SYNSETUP_END
	}
	 | Exp %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing ';'");
	}
	 | CompSt {
		MY_SYNSETUP("Stmt", 1)
		SIBLING
		MY_SYNSETUP_END
	}
	 | RETURN Exp SEMI %prec RIGHT_TYPE {
		MY_SYNSETUP("Stmt", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	 | RETURN Exp %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing ';'");
	}
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
		MY_SYNSETUP("Stmt", 5)
		SIBLING5
		MY_SYNSETUP_END
	}
	 | IF LP Exp RP Stmt ELSE Stmt {
		MY_SYNSETUP("Stmt", 7)
		SIBLING7
		MY_SYNSETUP_END
	}
	 | WHILE LP Exp RP Stmt {
		MY_SYNSETUP("Stmt", 5)
		SIBLING5
		MY_SYNSETUP_END
	}
	 | error SEMI {
		ERROR_SETUP
	}
	 ;
/* Local Definitions */
DefList : Def DefList {
		MY_SYNSETUP("DefList", 2)
		SIBLING2
		MY_SYNSETUP_END
	}
		| /* empty */{
		EMPTY_SETUP
	}
		;
Def : Specifier DecList SEMI %prec RIGHT_TYPE {
		MY_SYNSETUP("Def", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| Specifier DecList %prec ERROR_TYPE {
		ERROR_SETUP
		yyerror("missing ';'");
	}
	;
DecList : Dec {
		MY_SYNSETUP("DecList", 1)
		SIBLING
		MY_SYNSETUP_END
	}
		| Dec COMMA DecList {
		MY_SYNSETUP("DecList", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
		;
Dec : VarDec {
		MY_SYNSETUP("Dec", 1)
		SIBLING
		MY_SYNSETUP_END
	}
	| VarDec ASSIGNOP Exp {
		MY_SYNSETUP("Dec", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	;
/* Expressions */
Exp : Exp ASSIGNOP Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| Exp AND Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| Exp OR Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| Exp RELOP Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| Exp PLUS Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	} 
	| Exp MINUS Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| Exp STAR Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	} 
	| Exp DIV Exp {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	} 
	| LP Exp RP {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| MINUS Exp {
		MY_SYNSETUP("Exp", 2)
		SIBLING2
		MY_SYNSETUP_END
 	} 
	| NOT Exp {
		MY_SYNSETUP("Exp", 2)
		SIBLING2
		MY_SYNSETUP_END
	} 
	| ID LP Args RP {
		MY_SYNSETUP("Exp", 4)
		SIBLING4
		MY_SYNSETUP_END
	} 
	| ID LP RP {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	| Exp LB Exp RB {
		MY_SYNSETUP("Exp", 4)
		SIBLING4
		MY_SYNSETUP_END
	} 
	| Exp DOT ID {
		MY_SYNSETUP("Exp", 3)
		SIBLING3
		MY_SYNSETUP_END
	} 
	| ID {
		MY_SYNSETUP("Exp", 1)
		SIBLING
		MY_SYNSETUP_END
	} 
	| INT { 
		MY_SYNSETUP("Exp", 1)
		SIBLING
		MY_SYNSETUP_END
	} 
	| FLOAT {
		MY_SYNSETUP("Exp", 1)
		SIBLING
		MY_SYNSETUP_END
	}
	;
Args : Exp COMMA Args {
		MY_SYNSETUP("Args", 3)
		SIBLING3
		MY_SYNSETUP_END
	}
	 | Exp {
		MY_SYNSETUP("Args", 1)
		SIBLING
		MY_SYNSETUP_END
	}
	 ;
%%
void yyerror(const char *msg){
	parser_error_happen = 1;
	fprintf(stderr, "Error type B at Line %d: %s.\n", yylineno, msg);
	return;
}
