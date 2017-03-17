%{
#include "lex.yy.c"
void yyerror(char *msg);
%}
%union {
	int type_int;
	float type_float;
	double type_double;
}

%token<type_int> INT 
%token<type_float> FLOAT 
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
%%
/* High-level Definition */
Program : ExtDefList
		;
ExtDefList : ExtDef ExtDefList
		   | /* empty */
		   ;
ExtDef : Specifier ExtDecList SEMI 
	   | Specifier SEMI 
	   | Specifier FunDec CompSt
	   ;
ExtDecList : VarDec 
		   | VarDec COMMA ExtDecList
		   ;
/* Specifiers */
Specifier : TYPE 
		  | StructSpecifier
		  ;
StructSpecifier : STRUCT OptTag LC DefList RC
				| STRUCT Tag
				;
OptTag : ID 
	   | /* empty */
	   ;
Tag : ID
	;
/* Declarators */
VarDec : ID
	   | VarDec LB INT RB
	   ;
FunDec : ID LP VarList RP
	   | ID LP RP
	   ;
VarList : ParamDec COMMA VarList
		| ParamDec
		;
ParamDec : Specifier VarDec
		 ;
/*Statements*/
CompSt : LC DefList StmtList RC
	   | LC DefList error RC { }
	   ;
StmtList : Stmt StmtList
		 | /* empty */
		 ;
Stmt : Exp SEMI
	 | CompSt
	 | RETURN Exp SEMI
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
	 | IF LP Exp RP Stmt ELSE Stmt
	 | WHILE LP Exp RP Stmt
	 | error SEMI {}
	 | RETURN error SEMI {}
	 ;
/* Local Definitions */
DefList : Def DefList
		| /* empty */
		;
Def : Specifier DecList SEMI
	| Specifier error SEMI {}
	;
DecList : Dec
		| Dec COMMA DecList
		;
Dec : VarDec
	| VarDec ASSIGNOP Exp
	;
/* Expressions */
Exp : Exp ASSIGNOP Exp 
	| Exp AND Exp 
	| Exp OR Exp 
	| Exp RELOP Exp 
	| Exp PLUS Exp 
	| Exp MINUS Exp 
	| Exp STAR Exp 
	| Exp DIV Exp 
	| LP Exp RP 
	| MINUS Exp 
	| NOT Exp 
	| ID LP Args RP 
	| ID LP RP
	| Exp LB Exp RB 
	| Exp DOT ID 
	| ID 
	| INT 
	| FLOAT
	| ID LP error RP { }
	| LP error RP { }
	;
Args : Exp COMMA Args
	 | Exp
	 ;
%%
void yyerror(char *msg){
	fprintf(stderr, "line %d: ", yylineno);
	fprintf(stderr, "error: %s\n", msg);
	return;
}
