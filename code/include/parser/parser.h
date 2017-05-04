#ifndef __PARSER_PARSER__
#define __PARSER_PARSER__

#include <stdio.h>
#include "parser/tree.h"
extern int yyparse();
extern void yyrestart(FILE* f);
extern int yylineno;
extern int parser_error_happen, lex_error_happen;
extern Tree_Node *head;

#endif
