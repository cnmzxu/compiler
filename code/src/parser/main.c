/*************************************************************************
    > File Name: main.c
    > Author: xumz
 ************************************************************************/
#include <stdio.h>
#include "syntax.tab.h"
#include <string.h>
#include <parser/datastruct.h>

extern int yyparse();
extern int yyrestart(FILE* f);
extern int yylineno;
extern int parser_error_happen, lex_error_happen;
extern Node* head;
void print(Node *treenode, int indent){
	if(treenode == NULL || (strcmp(treenode->type, "empty") == 0))
		return;
	int i = 0, j = 0;
	for(i = 0; i < indent; i++)
		printf("  ");
	printf("%s", treenode->type);
	if (treenode->is_token == 1){
		if (strlen(treenode->value) > 0)
			printf(": %s", treenode->value);
	}
	else
		printf(" (%d)", treenode->lineno);
	printf("\n");
	Node *child = treenode->child;
	while(1){
		if(child == NULL)
			break;
		print(child, i + 1);
		child = child->sibling;
	}
}

int main(int argc,char** argv){
	printf("Parsing Start...\n");
	if (argc < 2){
		yyparse();
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
			//yydebug = 1;
			yyparse();
			fclose(f);
		}
	}
	printf("Parsing End.\n");
	if (!parser_error_happen && !lex_error_happen)
		print(head, 0);

	return 0;
}

