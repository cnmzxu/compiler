/*************************************************************************
    > File Name: main.c
    > Author: xumz
 ************************************************************************/
#include <stdio.h>
extern int yylex();
extern int yyrestart(FILE* f);
extern int yylineno;

int main(int argc,char** argv){
	printf("Parsering Start...\n");
	if (argc < 2){
		yylex();
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
			yylex();
			fclose(f);
		}
	}
	printf("Parsering End.\n");
	return 0;
}

