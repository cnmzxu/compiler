#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "parser/tree.h"

Tree_Node* common_setup(char* syntax_type, int lineno, Tree_Node *child, ...) {
	Tree_Node *parent = (Tree_Node *)malloc(sizeof(Tree_Node));
	if (parent == NULL)
		error("malloc error");
	
	strcpy(parent->type, syntax_type);
	parent->lineno =lineno;
	parent->token_type = SYNTAX_TOKEN;
	parent->sibling = NULL;

	va_list childlist;
	va_start(childlist, child);
	Tree_Node *nextchild = va_arg(childlist, Tree_Node *);
	parent->child = child;
	while(child){
		child->sibling = nextchild;
		if (nextchild != NULL)
			nextchild->rsibling = child;
		child = nextchild;
		nextchild = va_arg(childlist, Tree_Node *);
	}
	return parent;
}

Tree_Node* error_setup(int lineno){
	Tree_Node *error_node = (Tree_Node *)malloc(sizeof(Tree_Node));
	if(error_node == NULL)
		error("malloc error");
	error_node->child = NULL;
	error_node->sibling = NULL;
	error_node->lineno = lineno;
	strcpy(error_node->type, "error");
	return error_node;
}

Tree_Node *empty_setup(int lineno){
	Tree_Node *empty_node = (Tree_Node *)malloc(sizeof(Tree_Node));
	if (empty_node == NULL)
		error("malloc error");
	empty_node->child = NULL;
	empty_node->sibling = NULL;
	empty_node->lineno = lineno;
	strcpy(empty_node->type, "empty");
	return empty_node;
}

