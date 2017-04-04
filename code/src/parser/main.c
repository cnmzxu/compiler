/*************************************************************************
    > File Name: main.c
    > Author: xumz
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parser/datastruct.h"
#include "syntax.tab.h"
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
	int x = 0;
	switch(treenode->token_type){
		case 1:
			break;
		case 2:
			sscanf(treenode->value, "%d", &x);
			printf(": %d", x);
			break;
		case 3:
			sscanf(treenode->value, "%x", &x);
			printf(": %d", x);
			break;
		case 4:
			sscanf(treenode->value, "%o", &x);
			printf(": %d", x);
			break;
		case 5:
			printf(": %f", atof(treenode->value));
			break;
		case 6:
		case 7:
			printf(": %s", treenode->value);
			break;
		default:
			printf(" (%d)", treenode->lineno);
			break;
	}

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
			yyparse();
			fclose(f);
		}
	}
	if (!parser_error_happen && !lex_error_happen)
		print(head, 0);

	return 0;
}

