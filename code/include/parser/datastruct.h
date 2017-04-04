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
	int token_type;
	/*
	 * token_type:
	 * 0: syntax token
	 * 1: normal lexical token
	 * 2: decimal int
	 * 3: hexadecimal int
	 * 4: octal int
	 * 5: float
	 * 6: ID
	 * 7: TYPE
	 * */
} Node;

#endif
