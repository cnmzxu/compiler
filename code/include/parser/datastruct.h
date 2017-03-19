/*************************************************************************
    > File Name: datastruct.h
    > Author: xumz
 ************************************************************************/
#ifndef PARSER_DATASTRUCT
#define PARSER_DATASTRUCT

typedef struct node {
	char type[20];
	char value[20];
	struct node *child; 
	struct node *sibling;
	int lineno;
} Node;

#endif
