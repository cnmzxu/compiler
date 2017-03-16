/*************************************************************************
    > File Name: main.c
    > Author: xumz
 ************************************************************************/
#include <stdio.h>
#include "syntax.tab.h"
extern int yyparse();
extern int yyrestart(FILE* f);
extern int yylineno;

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
			yyparse();
			fclose(f);
		}
	}
	printf("Parsing End.\n");
	return 0;
}

