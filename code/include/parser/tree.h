#ifndef __PARSER_TREE__
#define __PARSER_TREE__

typedef struct tree_node_struct{
	char type[20], value[20];
	struct tree_node_struct *child, *sibling, *rsibling;
	int lineno;
	enum {SYNTAX_TOKEN, LEX_TOKEN, LEX_INT, LEX_FLOAT, LEX_ID, LEX_TYPE} token_type;
} Tree_Node;

Tree_Node *common_setup(char* syntax_type, int lineno, Tree_Node *child, ...);
Tree_Node *error_setup(int lineno);
Tree_Node *empty_setup(int lineno);

#endif
