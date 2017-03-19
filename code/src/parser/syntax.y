%locations
%{
#include <string.h>
#include <parser/datastruct.h>
void yyerror(char *msg);

#define YYSTYPE_IS_DECLARED 1
typedef Node* YYSTYPE;
Node head = {"", "", NULL, NULL, 0};

#include "lex.yy.c"

#define MY_SYNSETUP(str) \
	Node *node = (Node *)malloc(sizeof(Node)); \
	$$ = node; \
	$$->child = $1; \
	strcpy($$->type, str);\
	$$->lineno = @$.first_line;

#define SIBLING

#define SIBLING2 \
	SIBLING\
	$1->sibling = $2;

#define SIBLING3 \
	SIBLING2 \
	$2->sibling = $3;

#define SIBLING4 \
	SIBLING3 \
	$3->sibling = $4;

#define SIBLING5 \
	SIBLING4 \
	$4->sibling = $5;

#define SIBLING6 \
	SIBLING5 \
	$5->sibling = $6;

#define SIBLING7 \
	SIBLING6\
	$6->sibling = $7;

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
%%
Program : ExtDefList {
   $$ = &head;
   strcpy($$->type, "Program");
   $$->child = $1;
   $$->lineno = @$.first_line;
  }
  ;
ExtDefList : ExtDef ExtDefList {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "ExtDefList"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2; 
  }
     | {
   $$ = NULL;
  }
     ;
ExtDef : Specifier ExtDecList SEMI {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "ExtDef"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2; 
   $2->sibling = $3;
  
  }
    | Specifier SEMI {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "ExtDef"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2;
  
  }
    | Specifier FunDec CompSt {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "ExtDef"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2; 
   $2->sibling = $3; 
  }
    ;
ExtDecList : VarDec {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "ExtDecList"); 
   $$->lineno = @$.first_line;
  
  
  }
     | VarDec COMMA ExtDecList {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node;
   $$->child = $1;
   strcpy($$->type, "ExtDecList");
   $$->lineno = @$.first_line;
   $1->sibling = $2;
   $2->sibling = $3;
  }
     ;

Specifier : TYPE {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "Specifier"); 
   $$->lineno = @$.first_line;
  
  
  }
    | StructSpecifier {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "Specifier"); 
   $$->lineno = @$.first_line;
  }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "StructSpecifier"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2;
   $2->sibling = $3;
   $3->sibling = $4;
   $4->sibling = $5;
  
  }
    | STRUCT Tag {
   Node *node = (Node *)malloc(sizeof(Node));
   $$ = node;
   $$->child = $1;
   strcpy($$->type, "StructSpecifier");
   $$->lineno = @$.first_line;
   $1->sibling = $2;
  
  }
    ;
OptTag : ID {
	   Node *node = (Node *)malloc(sizeof(Node));
	   $$ = node;
	   $$->child = $1;
	  strcpy($$->type, "OptTag");
	  $$->lineno = @$.first_line; 
	}
    | /* empty */{
		$$ = NULL;
	}
    ;
Tag : ID {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "Tag"); 
   $$->lineno = @$.first_line;
  }
 ;

VarDec : ID {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "VarDec"); 
   $$->lineno = @$.first_line;
    }
    | VarDec LB INT RB {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; $$->child = $1; strcpy($$->type, "");
   $$->lineno = @$.first_line;
   $1->sibling = $2; 
   $2->sibling = $3; 
   $3->sibling = $4;
    }
    ;
FunDec : ID LP VarList RP {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "FunDec");
   $$->lineno = @$.first_line;
   $1->sibling = $2; 
   $2->sibling = $3; 
   $3->sibling = $4;
   }
    | ID LP RP{
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "FunDec"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2; 
   $2->sibling = $3;
  }
    ;
VarList : ParamDec COMMA VarList {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node; 
   $$->child = $1; 
   strcpy($$->type, "VarList"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2; 
   $2->sibling = $3;
  
  }
  | ParamDec {
   Node *node = (Node *)malloc(sizeof(Node)); 
   $$ = node;
   $$->child = $1; 
   strcpy($$->type, "VarList");
   $$->lineno = @$.first_line;
  
  
  }
  ;
ParamDec : Specifier VarDec {
   Node *node = (Node *)malloc(sizeof(Node));
   $$ = node;
   $$->child = $1;
   strcpy($$->type, "ParamDec"); 
   $$->lineno = @$.first_line;
   $1->sibling = $2;
  
  }
   ;

CompSt : LC DefList StmtList RC {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "CompSt"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3; $3->sibling = $4;
  
  }
    | LC DefList error RC { }
    ;
StmtList : Stmt StmtList {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "StmtList"); $$->lineno = @$.first_line;
   $1->sibling = $2;
  
  }
   | {
   $$ = NULL;
  }
   ;
Stmt : Exp SEMI {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Stmt"); $$->lineno = @$.first_line;
   $1->sibling = $2;
  
  }
  | CompSt {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Stmt"); $$->lineno = @$.first_line;
  
  
  }
  | RETURN Exp SEMI {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Stmt"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Stmt"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; $4->sibling = $5;
  
  }
  | IF LP Exp RP Stmt ELSE Stmt {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Stmt"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; $4->sibling = $5; $5->sibling = $6; $6->sibling = $7;
  
  }
  | WHILE LP Exp RP Stmt {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Stmt"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3; $3->sibling = $4; $4->sibling = $5;
  
  }
  | error SEMI { }
  | RETURN error SEMI { }
  ;

DefList : Def DefList {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "DefList"); $$->lineno = @$.first_line;
   $1->sibling = $2;
  
  }
  |
  ;
Def : Specifier DecList SEMI {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Def"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Specifier error SEMI { }
 ;
DecList : Dec {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "DecList"); $$->lineno = @$.first_line;
  
  
  }
  | Dec COMMA DecList {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "DecList"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
  ;
Dec : VarDec {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Dec"); $$->lineno = @$.first_line;
  
  
  }
 | VarDec ASSIGNOP Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Dec"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 ;

Exp : Exp ASSIGNOP Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp AND Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp OR Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp RELOP Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp PLUS Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp MINUS Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp STAR Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp DIV Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | LP Exp RP {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | MINUS Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2;
  
   }
 | NOT Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2;
  
  }
 | ID LP Args RP {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3; $3->sibling = $4;
  
  }
 | ID LP RP {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | Exp LB Exp RB {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3; $3->sibling = $4;
  
  }
 | Exp DOT ID {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
 | ID {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
  
  
  }
 | INT {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
  
  
  }
 | FLOAT {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Exp"); $$->lineno = @$.first_line;
  
  
  }
 | ID LP error RP { }
 | LP error RP { }
 ;
Args : Exp COMMA Args {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Args"); $$->lineno = @$.first_line;
   $1->sibling = $2; $2->sibling = $3;
  
  }
  | Exp {
   Node *node = (Node *)malloc(sizeof(Node)); $$ = node; $$->child = $1; strcpy($$->type, "Args"); $$->lineno = @$.first_line;
  
  
  }
  ;
%%
void yyerror(char *msg){
	fprintf(stderr, "Error Typr B at Line %d: ", yylineno);
	fprintf(stderr, "error: %s\n", msg);
	return;
}
