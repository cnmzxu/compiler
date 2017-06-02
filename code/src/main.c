#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "parser/parser.h"
#include "parser/syntax.tab.h"
#include "semantic/semantic.h"
#include "intercode/intercode.h"
void error(char *msg) {
	printf("%s\n", msg);
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
	if (!parser_error_happen && !lex_error_happen){
		translate(head);
	}
		
	return 0;
}

