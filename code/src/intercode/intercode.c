#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "intercode/intercode.h"
#include "parser/tree.h"
#include "semantic/semantic.h"

FILE *outfile;
int labelnum, tempvarnum;

int getlabel() {
	labelnum++;
	return labelnum;
}

int gettempvar() {
	tempvarnum++;
	return tempvarnum;
}

typedef struct {
	int flag;
	int ans;
} expans;

int translateExtDefList(Tree_Node *node);
int translateExtDef(Tree_Node *node);
int translateDefList(Tree_Node *node);
int translateStmtList(Tree_Node *node, int label);
int translateStmt(Tree_Node *node, int label);
expans analysisExp(Tree_Node *node);
symbol_type *translateExp(Tree_Node *node, int flag, char *dest);// flag = 0 : value, flag = 1 : address
int translateCon(Tree_Node *node, int truelabel, int falselabel2);


int translate(Tree_Node *node) {
	outfile = fopen("./translation.ir", "w+");
	translateExtDefList(node->child);
	return 0;
}

int translateExtDefList(Tree_Node *node) {
	//printf("extdeflist\n");
	if (strcmp(node->type, "ExtDefList") == 0){
		node = node->child;
		if (node != NULL){
			translateExtDef(node);
			translateExtDefList(node->sibling);
		}
	}

	return 0;
}

int translateExtDef(Tree_Node *extdef) {
	//printf("extdef\n");
	extdef = extdef->child;
	symbol_type *type = get_type(extdef);
	extdef = extdef->sibling;
	
	if (strcmp(extdef->type, "ExtDecList") == 0) {
		void translateVarDec(Tree_Node *vardec, symbol_type *ttype) {
			while(1) {
				vardec = vardec->child;
				if (strcmp(vardec->type, "ID") == 0)
					break;
				symbol_type *new_type = (symbol_type *)malloc(sizeof(symbol_type));
				new_type->type = TYPE_ARRAY;
				new_type->array_field.subtype = ttype;
				new_type->array_field.length = atoi(vardec->sibling->sibling->value);
				new_type->size = ttype->size * new_type->array_field.length;
				ttype = new_type;
			}
			if (ttype->type == TYPE_STRUCT || ttype->type == TYPE_ARRAY)
				fprintf(outfile, "DEC %s %d\n", vardec->value, ttype->size);
			add_symbol_entry(vardec->value, ttype, vardec->lineno, VARIABLE_TABLE, 1);
		}
		Tree_Node *vardec = extdef->child;
		while(1) {
			translateVarDec(vardec, type);
			vardec = vardec->sibling;
			if (vardec == NULL)
				break;
			vardec = vardec->sibling;
		}
	}
	else if (strcmp(extdef->type, "FunDec") == 0) {
		symbol_type *functype = (symbol_type *)malloc(sizeof(symbol_type));
		functype->function_field.return_type = type;
		functype->function_field.variable_length = 0;
		symbol_table_entry *variable = (symbol_table_entry *)malloc(sizeof(symbol_table_entry) * 20);
		void translateVarDec(Tree_Node *vardec, symbol_type *ttype) {
			while(1) {
				vardec = vardec->child;
				if (strcmp(vardec->type, "ID") == 0)
					break;
				symbol_type *new_type = (symbol_type *)malloc(sizeof(symbol_type));
				new_type->type = TYPE_ARRAY;
				new_type->array_field.subtype = ttype;
				new_type->array_field.length = atoi(vardec->sibling->sibling->value);
				new_type->size = ttype->size * new_type->array_field.length;
				ttype = new_type;
			}
			add_symbol_entry(vardec->value, ttype, vardec->lineno, VARIABLE_TABLE, 2);
			strcpy(variable[functype->function_field.variable_length].name, vardec->value);
			variable[functype->function_field.variable_length].type = ttype;
			functype->function_field.variable_length += 1;
			fprintf(outfile, "PARAM %s\n", vardec->value);
		}
		functype->function_field.variable = variable;
		Tree_Node *fundec = extdef->child;
		fprintf(outfile, "FUNCTION %s :\n", fundec->value);
		add_symbol_entry(fundec->value, functype, fundec->lineno, FUNCTION_TABLE, 1);
		fundec = fundec->sibling->sibling;
		Tree_Node *temp;
		creat_new_scope();
		if (strcmp(fundec->type, "VarList") == 0){
			while(1) {
				fundec = fundec->child;
				temp = fundec->child;
				translateVarDec(temp->sibling, get_type(temp));
				fundec = fundec->sibling;
				if (fundec == NULL)
					break;
				fundec = fundec->sibling;
			}
		}
		extdef = extdef->sibling->child->sibling;
		translateDefList(extdef);
		translateStmtList(extdef->sibling, 0);
		delete_local_scope();
	}
}

int translateDefList(Tree_Node *deflist) {
	//printf("deflist\n");
	if (strcmp(deflist->type, "empty") == 0)
		return 0;
	Tree_Node *def = deflist->child;
	deflist = def->sibling;
	def = def->child;
	symbol_type *type = get_type(def);
	void translateVarDec(Tree_Node *vardec, symbol_type *ttype) {
		Tree_Node *id = vardec->child;
		while(1) {
			if (strcmp(id->type, "ID") == 0)
				break;
			symbol_type *new_type = (symbol_type *)malloc(sizeof(symbol_type));
			new_type->type = TYPE_ARRAY;
			new_type->array_field.subtype = ttype;
			new_type->array_field.length = atoi(id->sibling->sibling->value);
			new_type->size = ttype->size * new_type->array_field.length;
			ttype = new_type;
			id = id->child;
		}
		if (ttype->type == TYPE_STRUCT || ttype->type == TYPE_ARRAY)
			fprintf(outfile, "DEC %s %d\n", id->value, ttype->size);
		add_symbol_entry(id->value, ttype, id->lineno, VARIABLE_TABLE, 1);
		if (vardec->sibling != NULL) {
			expans ans = analysisExp(vardec->sibling->sibling);
			if (ans.flag == 1)
				fprintf(outfile, "%s := #%d\n", id->value, ans.ans);
			else
				translateExp(vardec->sibling->sibling, 0, id->value);
		}
	}
	
	def = def->sibling;
	Tree_Node *vardec;
	while(1) {
		def = def->child;
		vardec = def->child;
		translateVarDec(vardec, type);
		def = def->sibling;
		if (def == NULL)
			break;
		def = def->sibling;
	}
	translateDefList(deflist);
	return 0;
}

int translateStmtList(Tree_Node *stmtlist, int label) {
	//printf("stmtlist\n");
	while (1) {
		if (strcmp(stmtlist->type, "empty") == 0){
			if (label != 0) {
				fprintf(outfile, "GOTO label%d\n", label);
			}
			break;
		}
		Tree_Node *stmt = stmtlist->child;
		stmtlist = stmt->sibling;
		translateStmt(stmt, 0);
	}
	return 0;	
}

int translateStmt(Tree_Node *stmt, int label) {
	//printf("stmt\n");
	stmt = stmt->child;
	if (strcmp(stmt->type, "Exp") == 0) {
		translateExp(stmt, 0, NULL);
		if (label != 0)
			fprintf(outfile, "GOTO label%d\n", label);
	}
	else if (strcmp(stmt->type, "CompSt") == 0) {
		stmt = stmt->child->sibling;
		translateDefList(stmt);
		translateStmtList(stmt->sibling, label);
	}
	else if (strcmp(stmt->type, "RETURN") == 0) {
		stmt = stmt->sibling;
		expans ans = analysisExp(stmt);
		if (ans.flag == 1)
			fprintf(outfile, "RETURN #%d\n", ans.ans);
		else {
			Tree_Node *id = stmt->child;
			char str[20];
			if (ans.flag == 2)
				strcpy(str, id->value);
			else {
				int tempvar = gettempvar();
				sprintf(str, "v%d", tempvar);
				translateExp(stmt, 0, str);
			}
			fprintf(outfile, "RETURN %s\n", str);
		}
	}
	else if (strcmp(stmt->type, "IF") == 0) {
		Tree_Node *exp = stmt->sibling->sibling;
		Tree_Node *stmt1 = exp->sibling->sibling;
		Tree_Node *stmt2 = stmt1->sibling;
		
		expans ans = analysisExp(exp);
		if (stmt2 == NULL) {
			if (ans.flag == 1) {
				if (ans.ans != 0)
					translateStmt(stmt1, label);
				else {
					if (label != 0)
						fprintf(outfile, "GOTO label%d\n", label);
				}
				return 0;
			}
			else {
				int newlabel;
				if (label == 0)
					newlabel = getlabel();
				else
					newlabel = label;
				translateCon(exp, 0, newlabel);
				translateStmt(stmt1, label);
				if (label == 0)
					fprintf(outfile, "LABEL label%d :\n", newlabel);
			}
		}
		else {
			stmt2 = stmt2->sibling;
			if (ans.flag == 1) {
				if (ans.ans != 0)
					translateStmt(stmt1, label);
				else
					translateStmt(stmt2, label);
				return 0;
			}
			else {
				int newlabel1 = getlabel();
				int newlabel2;
				if (label == 0)
					newlabel2 = getlabel();
				else
					newlabel2 = label;
				translateCon(exp, 0, newlabel1);
				translateStmt(stmt1, newlabel2);
				fprintf(outfile, "LABEL label%d :\n", newlabel1);
				translateStmt(stmt2, label);
				if (label == 0)
					fprintf(outfile, "LABEL label%d :\n", newlabel2);
			}
		}
	}
	else if (strcmp(stmt->type, "WHILE") == 0) {
		Tree_Node *exp = stmt->sibling->sibling;
		stmt = exp->sibling->sibling;
		int newlabel1, newlabel2;
		newlabel1 = getlabel();
		if (label == 0)
			newlabel2 = getlabel();
		else
			newlabel2 = label;
		fprintf(outfile, "LABEL label%d :\n", newlabel1);
		translateCon(exp, 0, newlabel2);
		translateStmt(stmt, newlabel1);
		fprintf(outfile, "LABEL label%d :\n", newlabel2);
	}
}

expans analysisExp(Tree_Node *node) {
	//printf("anaexp\n");
	expans ans;
	ans.flag = 0;
	if (node->value[0] == 1) {
		ans.flag = 1;
		ans.ans = (node->value[1] << 24 & 0xff000000) | (node->value[2] << 16 & 0xff0000) | (node->value[3] << 8 & 0xff00) | (node->value[4] & 0xff);
		return ans;
	}

	void split(int n) {
		node->value[0] = 1;
		node->value[1] = (n >> 24) & 0xff;
		node->value[2] = (n >> 16) & 0xff;
		node->value[3] = (n >> 8) & 0xff;
		node->value[4] = n & 0xff;
	}
	Tree_Node *node1 = node->child;
	if (strcmp(node1->type, "Exp") == 0) {
		Tree_Node *node2 = node1->sibling;
		Tree_Node *node3 = node2->sibling;
		if (strcmp(node2->type, "ASSIGNOP") == 0)
			return ans;
		else if (strcmp(node2->type, "LB") == 0) 
			return ans;
		
		else if (strcmp(node2->type, "DOT") == 0) 
			return ans;			
	
		else if (strcmp(node2->type, "AND") == 0) {	
			expans ans1 = analysisExp(node1),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1 && ans2.flag == 1){
				ans.flag = 1;
				ans.ans = ans1.ans && ans2.ans;
				split(ans.ans);
			}
			return ans;
		}
		else if (strcmp(node2->type, "OR") == 0) {
			expans ans1 = analysisExp(node1),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1 && ans2.flag == 1){
				ans.flag = 1;
				ans.ans = ans1.ans || ans2.ans;
				split(ans.ans);
			}
			return ans;
		}
		else if (strcmp(node2->type, "PLUS") == 0) {
			expans ans1 = analysisExp(node1),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1 && ans2.flag == 1){
				ans.flag = 1;
				ans.ans = ans1.ans + ans2.ans;
				split(ans.ans);
			}
			return ans;
		}
		else if (strcmp(node2->type, "MINUS") == 0) {
			expans ans1 = analysisExp(node1),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1 && ans2.flag == 1){
				ans.flag = 1;
				ans.ans = ans1.ans - ans2.ans;
				split(ans.ans);
			}
			return ans;
		}
		else if (strcmp(node2->type, "STAR") == 0) {
			expans ans1 = analysisExp(node1),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1 && ans2.flag == 1){
				ans.flag = 1;
				ans.ans = ans1.ans * ans2.ans;
				split(ans.ans);
			}
			return ans;
		}
		else if (strcmp(node2->type, "DIV") == 0) {
			expans ans1 = analysisExp(node1),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1 && ans2.flag == 1){
				ans.flag = 1;
				ans.ans = ans1.ans / ans2.ans;
				split(ans.ans);
			}
			return ans;
		}
		else if (strcmp(node2->type, "RELOP") == 0) {
			expans ans1 = analysisExp(node1),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1 && ans2.flag == 1) {
				ans.flag = 1;
				if (strcmp(node2->value, ">=") == 0)
					ans.ans = (ans1.ans >= ans2.ans);
				else if (strcmp(node2->value, ">") == 0)
					ans.ans = (ans1.ans > ans2.ans);
				else if (strcmp(node2->value, "<=") == 0)
					ans.ans = (ans1.ans <= ans2.ans);
				else if (strcmp(node2->value, "<") == 0)
					ans.ans = (ans1.ans < ans2.ans);
				else if (strcmp(node2->value, "==") == 0)
					ans.ans = (ans1.ans == ans2.ans);
				else if (strcmp(node2->value, "!=") == 0)
					ans.ans = (ans1.ans != ans2.ans);
				split(ans.ans);
			}
			return ans;
		}
	}
	else if (strcmp(node1->type, "ID") == 0) {
		if (node1->sibling == NULL)
			ans.flag = 2;
		return ans;
	}
	else if (strcmp(node1->type, "LP") == 0) {
		ans = analysisExp(node1->sibling);
		if (ans.flag == 1)
			split(ans.ans);
	}
	else if (strcmp(node1->type, "MINUS") == 0) {
		ans = analysisExp(node1->sibling);
		if (ans.flag == 1) {
			ans.ans = -ans.ans;
			split(ans.ans);
		}
		return ans;
	}
	else if (strcmp(node1->type, "NOT") == 0) {
		ans = analysisExp(node1->sibling);
		if (ans.flag == 1) {
			ans.ans = !ans.ans;
			split(ans.ans);
		}
		return ans;
	}
	else if (strcmp(node1->type, "INT") == 0) {
		ans.flag = 1;
		ans.ans = atoi(node1->value);
		split(ans.ans);
		return ans;
	}
}

int translateCon(Tree_Node *node, int truelabel, int falselabel) {
	//printf("con\n");
	node = node->child;
	if (truelabel == falselabel && truelabel != 0)
		fprintf(outfile, "GOTO label%d\n", truelabel);
	if (truelabel == 0 && falselabel != 0) {
		if (strcmp(node->type, "Exp") == 0) {
			Tree_Node *node2 = node->sibling;
			Tree_Node *node3 = node2->sibling;
			if (strcmp(node2->type, "LB") == 0) {
				int tempvar1 = gettempvar(),
					tempvar4 = gettempvar();
				expans ans = analysisExp(node3);
				char dest1[20];
				sprintf(dest1, "v%d", tempvar1);
				translateExp(node, 1, dest1);
				if (ans.flag == 0) {
					int tempvar2 = gettempvar(),
						tempvar3 = gettempvar();
					char dest2[20];
					sprintf(dest2, "v%d", tempvar2);
					translateExp(node3, 0, dest2);
					fprintf(outfile, "v%d := v%d * #4\n", tempvar3, tempvar2);
					fprintf(outfile, "v%d := v%d + v%d\n", tempvar4, tempvar1, tempvar3);
				}
				else
					fprintf(outfile, "v%d := v%d + #%d\n", tempvar4, tempvar1, 4 * ans.ans);
				fprintf(outfile, "IF *v%d == #0 GOTO label%d\n", tempvar4, falselabel);
			}
			else if (strcmp(node2->type, "DOT") == 0) {
				int tempvar = gettempvar(),
					tempvar2 = gettempvar();
				char dest1[20];
				sprintf(dest1, "v%d", tempvar);
				symbol_type *type = translateExp(node, 1, dest1);
				int size = 0, i = 0;
				for (i = 0; i < type->struct_field.table_length; i++) {
					if (strcmp(type->struct_field.field_table[i].name, node3->value) == 0) {
						fprintf(outfile, "v%d := v%d + #%d\n", tempvar2, tempvar, size);
						break;
					}
					size += type->struct_field.field_table[i].type->size;
				}
				fprintf(outfile, "IF *v%d == #0 GOTO label%d\n", tempvar2, falselabel);
			}
			else if (strcmp(node2->type, "AND") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag != 1 && ans2.flag != 1){
					translateCon(node, 0, falselabel);
					translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans == 0)
						fprintf(outfile, "GOTO label%d\n", falselabel);
					else
						translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans == 0)
						fprintf(outfile, "GOTO label%d\n", falselabel);
					else
						translateCon(node, truelabel, falselabel);
				}
				else {
					if (ans1.ans == 0 || ans2.ans == 0)	
						fprintf(outfile, "GOTO label%d\n", falselabel);
				}
			}
			else if (strcmp(node2->type, "OR") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag != 1 && ans2.flag != 1){
					int newlabel = getlabel();
					translateCon(node, newlabel, 0);
					translateCon(node3, truelabel, falselabel);
					fprintf(outfile, "LABEL label%d :\n", newlabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans == 0)
						translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans == 0)
						translateCon(node, truelabel, falselabel);
				}
				else {
					if (ans1.ans == 0 && ans2.ans == 0)	
						fprintf(outfile, "GOTO label%d\n", falselabel);
				}

			}
			else if (strcmp(node2->type, "RELOP") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag == 1 && ans2.flag == 1) {
					if ((strcmp(node2->value, ">=") == 0 && ans1.ans < ans2.ans)
					|| (strcmp(node2->value, ">") == 0 && ans1.ans <= ans2.ans)
					|| (strcmp(node2->value, "<=") == 0 && ans1.ans > ans2.ans)
					|| (strcmp(node2->value, "<") == 0 && ans1.ans >= ans2.ans)
					|| (strcmp(node2->value, "==") == 0 && ans1.ans != ans2.ans)
					|| (strcmp(node2->value, "!=") == 0 && ans1.ans == ans2.ans))
						fprintf(outfile, "GOTO label%d\n", falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					Tree_Node *id = node->child;
					char str[20];
					if (ans1.flag == 2)
						strcpy(str, id->value);
					else {
						int tempvar = gettempvar();
						sprintf(str, "v%d", tempvar);
						translateExp(node, 0, str);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s < #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s <= #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s > #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s >= #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s != #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s == #%d GOTO label%d\n", str, ans2.ans, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					Tree_Node *id = node3->child;
					char str[20];
					if (ans2.flag == 2)
						strcpy(str, id->value);
					else {
						int tempvar = gettempvar();
						sprintf(str, "v%d", tempvar);
						translateExp(node3, 0, str);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s > #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s >= #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s < #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s <= #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s != #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s == #%d GOTO label%d\n", str, ans1.ans, falselabel);
				}
				else {
					Tree_Node *id1 = node->child;
					char str1[20];
					if (strcmp(id1->type, "ID") == 0 && id1->sibling == NULL)
						strcpy(str1, id1->value);
					else {
						int tempvar1 = gettempvar();
						sprintf(str1, "v%d", tempvar1);
						translateExp(node, 0, str1);
					}
					Tree_Node *id2 = node3->child;
					char str2[20];
					if (strcmp(id2->type, "ID") == 0 && id2->sibling == NULL)
						strcpy(str2, id2->value);
					else {
						int tempvar2 = gettempvar();
						sprintf(str2, "v%d", tempvar2);
						translateExp(node3, 0, str2);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s < %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s <= %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s > %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s >= %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s != %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s == %s GOTO label%d\n", str1, str2, falselabel);				
				}
			}
		}
		else if (strcmp(node->type, "ID") == 0) {
			Tree_Node *node2 = node->sibling;
			if (node2 == NULL)
				fprintf(outfile, "IF %s == #0 GOTO label%d\n", node->value, falselabel);
			else {
				node2 = node2->sibling;
				int tempvar1 = gettempvar();
				if (strcmp(node2->type, "Args") == 0) {
					void analysisArgs(Tree_Node *args) {
						args = args->child;
						if (args->sibling != NULL)
							analysisArgs(args->sibling->sibling);
						expans ans = analysisExp(args);
						if (ans.flag == 1)
							fprintf(outfile, "ARG #%d\n", ans.ans);
						else {
							Tree_Node *id = args->child;
							char str[20];
							if (ans.flag == 2)
								strcpy(str, id->value);
							else {
								int tempvar2 = gettempvar();
								sprintf(str, "v%d", tempvar2);
								translateExp(args, 0, str);
							}
							fprintf(outfile, "ARG %s\n", str);
						}
					}
					analysisArgs(node2);
					fprintf(outfile, "v%d := CALL %s\n", tempvar1, node->value);
				}
				else {
					if (strcmp(node->value, "read") == 0)
						fprintf(outfile, "READ v%d\n", tempvar1);
					else
						fprintf(outfile, "v%d := CALL %s\n", tempvar1, node->value);
				}
				fprintf(outfile, "IF v%d == #0 GOTO label%d\n", tempvar1, falselabel);
			}
		}
		else if (strcmp(node->type, "LP") == 0) {
			Tree_Node *node2 = node->sibling;
			expans ans = analysisExp(node2);
			if (ans.flag == 1) {
				if (ans.ans == 0)
					fprintf(outfile, "GOTO label%d\n", falselabel);
			}
			else
				translateCon(node2, truelabel, falselabel);
		}
		else if (strcmp(node->type, "NOT") == 0) {
			expans ans = analysisExp(node->sibling);
			if (ans.flag == 1) {
				if (ans.ans != 0)
					fprintf(outfile, "GOTO label%d\n", falselabel);
			}
			else
				translateCon(node->sibling, falselabel, truelabel);
		}
		else if (strcmp(node->type, "INT") == 0) {
			int num = atoi(node->value);
			if (num == 0)
				fprintf(outfile, "GOTO label%d\n", falselabel);
		}
	}
	else if (falselabel == 0 && truelabel != 0) {	
		if (strcmp(node->type, "Exp") == 0) {
			Tree_Node *node2 = node->sibling;
			Tree_Node *node3 = node2->sibling;
			if (strcmp(node2->type, "LB") == 0) {
				int tempvar1 = gettempvar(), 
					tempvar4 = gettempvar();
				expans ans = analysisExp(node3);
				char dest1[20];
				sprintf(dest1, "v%d", tempvar1);
				translateExp(node, 1, dest1);
				if (ans.flag == 0) {
					int tempvar2 = gettempvar(),
						tempvar3 = gettempvar();
					char dest2[20];
					sprintf(dest2, "v%d", tempvar2);
					translateExp(node3, 0, dest2);
					fprintf(outfile, "v%d := v%d * #4\n", tempvar3, tempvar2);
					fprintf(outfile, "v%d := v%d + v%d\n", tempvar4, tempvar1, tempvar3);
				}
				else
					fprintf(outfile, "v%d := v%d + #%d\n", tempvar4, tempvar1, 4 * ans.ans);
				fprintf(outfile, "IF *v%d != #0 GOTO label%d\n", tempvar4, truelabel);
			}
			else if (strcmp(node2->type, "DOT") == 0) {
				int tempvar = gettempvar(),
					tempvar2 = gettempvar();
				char dest1[20];
				sprintf(dest1, "v%d", tempvar);
				symbol_type *type = translateExp(node, 1, dest1);
				int size = 0, i = 0;
				for (i = 0; i < type->struct_field.table_length; i++) {
					if (strcmp(type->struct_field.field_table[i].name, node3->value) == 0) {
						fprintf(outfile, "v%d := v%d + #%d\n", tempvar2, tempvar, size);
						break;
					}
					size += type->struct_field.field_table[i].type->size;
				}
				fprintf(outfile, "IF *v%d != #0 GOTO label%d\n", tempvar2, falselabel);
			}
			else if (strcmp(node2->type, "OR") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag != 1 && ans2.flag != 1){
					translateCon(node, truelabel, 0);
					translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans != 0)
						fprintf(outfile, "GOTO label%d\n", truelabel);
					else
						translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans != 0)
						fprintf(outfile, "GOTO label%d\n", truelabel);
					else
						translateCon(node, truelabel, falselabel);
				}
				else {
					if (ans1.ans != 0 || ans2.ans != 0)	
						fprintf(outfile, "GOTO label%d\n", truelabel);
				}
			}
			else if (strcmp(node2->type, "AND") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag != 1 && ans2.flag != 1){
					int newlabel = getlabel();
					translateCon(node, 0, newlabel);
					translateCon(node3, truelabel, falselabel);
					fprintf(outfile, "LABEL label%d :\n", newlabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans != 0)
						translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans != 0)
						translateCon(node, truelabel, falselabel);
				}
				else {
					if (ans1.ans != 0 && ans2.ans == 0)	
						fprintf(outfile, "GOTO label%d\n", falselabel);
				}

			}
			else if (strcmp(node2->type, "RELOP") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag == 1 && ans2.flag == 1) {
					if ((strcmp(node2->value, ">=") == 0 && ans1.ans >= ans2.ans)
					|| (strcmp(node2->value, ">") == 0 && ans1.ans > ans2.ans)
					|| (strcmp(node2->value, "<=") == 0 && ans1.ans <= ans2.ans)
					|| (strcmp(node2->value, "<") == 0 && ans1.ans < ans2.ans)
					|| (strcmp(node2->value, "==") == 0 && ans1.ans == ans2.ans)
					|| (strcmp(node2->value, "!=") == 0 && ans1.ans != ans2.ans))
						fprintf(outfile, "GOTO label%d\n", truelabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					Tree_Node *id = node->child;
					char str[20];
					if (ans1.flag == 2)
						strcpy(str, id->value);
					else {
						int tempvar = gettempvar();
						sprintf(str, "v%d", tempvar);
						translateExp(node, 0, str);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s >= #%d GOTO label%d\n", str, ans2.ans, truelabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s > #%d GOTO label%d\n", str, ans2.ans, truelabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s <= #%d GOTO label%d\n", str, ans2.ans, truelabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s < #%d GOTO label%d\n", str, ans2.ans, truelabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s == #%d GOTO label%d\n", str, ans2.ans, truelabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s != #%d GOTO label%d\n", str, ans2.ans, truelabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					Tree_Node *id = node3->child;
					char str[20];
					if (ans2.flag == 2)
						strcpy(str, id->value);
					else {
						int tempvar = gettempvar();
						sprintf(str, "v%d", tempvar);
						translateExp(node, 0, str);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s <= #%d GOTO label%d\n", str, ans1.ans, truelabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s < #%d GOTO label%d\n", str, ans1.ans, truelabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s >= #%d GOTO label%d\n", str, ans1.ans, truelabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s > #%d GOTO label%d\n", str, ans1.ans, truelabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s == #%d GOTO label%d\n", str, ans1.ans, truelabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s != #%d GOTO label%d\n", str, ans1.ans, truelabel);
				}
				else {
					Tree_Node *id1 = node->child;
					char str1[20];
					if (ans1.flag == 2)
						strcpy(str1, id1->value);
					else {
						int tempvar1 = gettempvar();
						sprintf(str1, "v%d", tempvar1);
						translateExp(node, 0, str1);
					}
					Tree_Node *id2 = node3->child;
					char str2[20];
					if (ans2.flag == 2)
						strcpy(str2, id2->value);
					else {
						int tempvar2 = gettempvar();
						sprintf(str2, "v%d", tempvar2);
						translateExp(node3, 0, str2);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s >= %s GOTO label%d\n", str1, str2, truelabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s > %s GOTO label%d\n", str1, str2, truelabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s <= %s GOTO label%d\n", str1, str2, truelabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s < %s GOTO label%d\n", str1, str2, truelabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s == %s GOTO label%d\n", str1, str2, truelabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s != %s GOTO label%d\n", str1, str2, truelabel);				
				}
			}
		}
		else if (strcmp(node->type, "ID") == 0) {
			Tree_Node *node2 = node->sibling;
			if (node2 == NULL)
				fprintf(outfile, "IF %s != #0 GOTO label%d\n", node->value, truelabel);
			else{
				node2 = node2->sibling;
				int tempvar1 = gettempvar();
				if (strcmp(node2->type, "Args") == 0) {
					void analysisArgs(Tree_Node *args) {
						args = args->child;
						if (args->sibling != NULL)
							analysisArgs(args->sibling->sibling);
						expans ans = analysisExp(args);
						if (ans.flag == 1)
							fprintf(outfile, "ARG #%d\n", ans.ans);
						else {
							Tree_Node *id = args->child;
							char str[20];
							if (ans.flag == 2)
								strcpy(str, id->value);
							else {
								int tempvar2 = gettempvar();
								sprintf(str, "v%d", tempvar2);
								translateExp(args, 0, str);
							}
							fprintf(outfile, "ARG %s\n", str);
						}
					}
					analysisArgs(node2);
					fprintf(outfile, "v%d := CALL %s\n", tempvar1, node->value);
				}
				else {
					if (strcmp(node->value, "read") == 0)
						fprintf(outfile, "READ v%d\n", tempvar1);
					else
						fprintf(outfile, "v%d := CALL %s\n", tempvar1, node->value);
				}
				fprintf(outfile, "IF v%d != #0 GOTO label%d\n", tempvar1, truelabel);
			}
		}
		else if (strcmp(node->type, "LP") == 0) {
			Tree_Node *node2 = node->sibling;
			expans ans = analysisExp(node2);
			if (ans.flag == 1) {
				if (ans.ans != 0)
					fprintf(outfile, "GOTO label%d\n", truelabel);
			}
			else
				translateCon(node2, truelabel, falselabel);
		}
		else if (strcmp(node->type, "NOT") == 0) {
			expans ans = analysisExp(node->sibling);
			if (ans.flag == 1) {
				if (ans.ans == 0)
					fprintf(outfile, "GOTO label%d\n", truelabel);
			}
			else
				translateCon(node->sibling, falselabel, truelabel);
		}
		else if (strcmp(node->type, "INT") == 0) {
			int num = atoi(node->value);
			if (num != 0)
				fprintf(outfile, "GOTO label%d\n", truelabel);
		}
	}
	else if (truelabel != 0 && falselabel!= 0) {	
		if (strcmp(node->type, "Exp") == 0) {
			Tree_Node *node2 = node->sibling;
			Tree_Node *node3 = node2->sibling;
			if (strcmp(node2->type, "LB") == 0) {
				int tempvar1 = gettempvar(),
					tempvar4 = gettempvar();
				expans ans = analysisExp(node3);
				char dest1[20];
				sprintf(dest1, "v%d", tempvar1);

				translateExp(node, 1, dest1);
				if (ans.flag == 0) {
					int tempvar2 = gettempvar(), 
						tempvar3 = gettempvar();
					char dest2[20];
					sprintf(dest2, "v%d", tempvar2);
					translateExp(node3, 0, dest2);
					fprintf(outfile, "v%d := v%d * #4\n", tempvar3, tempvar2);
					fprintf(outfile, "v%d := v%d + v%d\n", tempvar4, tempvar1, tempvar3);
				}
				else
					fprintf(outfile, "v%d := v%d + #%d\n", tempvar4, tempvar1, 4 * ans.ans);
				fprintf(outfile, "IF *v%d == #0 GOTO label%d\n", tempvar4, falselabel);
				fprintf(outfile, "GOTO label%d\n", truelabel);
			}
			else if (strcmp(node2->type, "DOT") == 0) {
				int tempvar = gettempvar(),
					tempvar2 = gettempvar();
				char dest1[20];
				sprintf(dest1, "v%d", tempvar);
				symbol_type *type = translateExp(node, 1, dest1);
				int size = 0, i = 0;
				for (i = 0; i < type->struct_field.table_length; i++) {
					if (strcmp(type->struct_field.field_table[i].name, node3->value) == 0) {
						fprintf(outfile, "v%d := v%d + #%d\n", tempvar2, tempvar, size);
						break;
					}
					size += type->struct_field.field_table[i].type->size;
				}
				fprintf(outfile, "IF *v%d == #0 GOTO label%d\n", tempvar2, falselabel);
				fprintf(outfile, "GOTO label%d\n", truelabel);
			}
			else if (strcmp(node2->type, "AND") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag != 1 && ans2.flag != 1){
					translateCon(node, 0, falselabel);
					translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans == 0)
						fprintf(outfile, "GOTO label%d\n", falselabel);
					else
						translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans == 0)
						fprintf(outfile, "GOTO label%d\n", falselabel);
					else
						translateCon(node, truelabel, falselabel);
				}
				else {
					if (ans1.ans == 0 || ans2.ans == 0)	
						fprintf(outfile, "GOTO label%d\n", falselabel);
					else
						fprintf(outfile, "GOTO label%d\n", truelabel);
				}
			}
			else if (strcmp(node2->type, "OR") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag != 1 && ans2.flag != 1){
					int newlabel = getlabel();
					translateCon(node, truelabel, 0);
					translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans == 0)
						translateCon(node3, truelabel, falselabel);
					else
						fprintf(outfile, "GOTO label%d\n", truelabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans == 0)
						translateCon(node, truelabel, falselabel);
					else
						fprintf(outfile, "GOTO label%d\n", truelabel);
				}
				else {
					if (ans1.ans == 0 && ans2.ans == 0)	
						fprintf(outfile, "GOTO label%d\n", falselabel);
					else
						fprintf(outfile, "GOTO label%d\n", truelabel);
				}

			}
			else if (strcmp(node2->type, "RELOP") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
				if (ans1.flag == 1 && ans2.flag == 1) {
					if ((strcmp(node2->value, ">=") == 0 && ans1.ans < ans2.ans)
					|| (strcmp(node2->value, ">") == 0 && ans1.ans <= ans2.ans)
					|| (strcmp(node2->value, "<=") == 0 && ans1.ans > ans2.ans)
					|| (strcmp(node2->value, "<") == 0 && ans1.ans >= ans2.ans)
					|| (strcmp(node2->value, "==") == 0 && ans1.ans != ans2.ans)
					|| (strcmp(node2->value, "!=") == 0 && ans1.ans == ans2.ans))
						fprintf(outfile, "GOTO label%d\n", falselabel);
					else
						fprintf(outfile, "GOTO label%d\n", truelabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					Tree_Node *id = node->child;
					char str[20];
					if (ans1.flag == 2)
						strcpy(str, id->value);
					else {
						int tempvar = gettempvar();
						sprintf(str, "v%d", tempvar);
						translateExp(node, 0, str);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s < #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s <= #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s > #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s >= #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s != #%d GOTO label%d\n", str, ans2.ans, falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s == #%d GOTO label%d\n", str, ans2.ans, falselabel);
					fprintf(outfile, "GOTO label%d\n", truelabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					Tree_Node *id = node3->child;
					char str[20];
					if (ans2.flag == 2)
						strcpy(str, id->value);
					else {
						int tempvar = gettempvar();
						sprintf(str, "v%d", tempvar);
						translateExp(node, 0, str);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s > #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s >= #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s < #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s <= #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s != #%d GOTO label%d\n", str, ans1.ans, falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s == #%d GOTO label%d\n", str, ans1.ans, falselabel);
					fprintf(outfile, "GOTO label%d\n", truelabel);
				}
				else {
					Tree_Node *id1 = node->child;
					char str1[20];
					if (ans1.flag == 2)
						strcpy(str1, id1->value);
					else {
						int tempvar1 = gettempvar();
						sprintf(str1, "v%d", tempvar1);
						translateExp(node, 0, str1);
					}
					Tree_Node *id2 = node3->child;
					char str2[20];
					if (ans2.flag == 2)
						strcpy(str2, id2->value);
					else {
						int tempvar2 = gettempvar();
						sprintf(str2, "v%d", tempvar2);
						translateExp(node3, 0, str2);
					}
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "IF %s < %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "IF %s <= %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "IF %s > %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "IF %s >= %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "IF %s != %s GOTO label%d\n", str1, str2, falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "IF %s == %s GOTO label%d\n", str1, str2, falselabel);				
					fprintf(outfile, "GOTO label%d\n", truelabel);
				}
			}
		}
		else if (strcmp(node->type, "ID") == 0) {
			Tree_Node *node2 = node->sibling;
			if (node2 == NULL)
				fprintf(outfile, "IF %s == #0 GOTO label%d\n", node->value, falselabel);
			else{
				node2 = node2->sibling;
				int tempvar1 = gettempvar();
				if (strcmp(node2->type, "Args") == 0) {
					void analysisArgs(Tree_Node *args) {
						args = args->child;
						if (args->sibling != NULL)
							analysisArgs(args->sibling->sibling);
						expans ans = analysisExp(args);
						if (ans.flag == 1)
							fprintf(outfile, "ARG #%d\n", ans.ans);
						else {
							Tree_Node *id = args->child;
							char str[20];
							if (ans.flag == 2)
								strcpy(str, id->value);
							else {
								int tempvar2 = gettempvar();
								sprintf(str, "v%d", tempvar2);
								translateExp(args, 0, str);
							}
							fprintf(outfile, "ARG %s\n", str);
						}
					}
					analysisArgs(node2);
					fprintf(outfile, "v%d := CALL %s\n", tempvar1, node->value);
				}
				else {
					if (strcmp(node->value, "read") == 0)
						fprintf(outfile, "READ v%d\n", tempvar1);
					else
						fprintf(outfile, "v%d := CALL %s\n", tempvar1, node->value);
				}
				fprintf(outfile, "IF v%d != #0 GOTO label%d\n", tempvar1, falselabel);
			}
			fprintf(outfile, "GOTO label%d\n", truelabel);
		}
		else if (strcmp(node->type, "LP") == 0) {
			Tree_Node *node2 = node->sibling;
			expans ans = analysisExp(node2);
			if (ans.flag == 1) {
				if (ans.ans == 0)
					fprintf(outfile, "GOTO label%d\n", falselabel);
				else
					fprintf(outfile, "GOTO label%d\n", truelabel);
			}
			else
				translateCon(node2, truelabel, falselabel);
		}
		else if (strcmp(node->type, "NOT") == 0) {
			expans ans = analysisExp(node->sibling);
			if (ans.flag == 1) {
				if (ans.ans == 0)
					fprintf(outfile, "GOTO label%d\n", falselabel);
				else
					fprintf(outfile, "GOTO label%d\n", truelabel);

			}
			else
				translateCon(node->sibling, falselabel, truelabel);
		}
		else if (strcmp(node->type, "INT") == 0) {
			int num = atoi(node->value);
			if (num != 0)
				fprintf(outfile, "GOTO label%d\n", falselabel);
			else		
				fprintf(outfile, "GOTO label%d\n", truelabel);
		}
	}

}

symbol_type *translateExp(Tree_Node *exp, int flag, char *dest) {
	//printf("exp\n");
	// flag = 0 : value, flag = 1 : address
	symbol_type *int_type = (symbol_type *)malloc(sizeof(symbol_type));
	int_type->type = TYPE_INT;

	Tree_Node *node = exp->child;
	
	if (strcmp(node->type, "Exp") == 0) {
		Tree_Node *node2 = node->sibling;
		Tree_Node *node3 = node2->sibling;
		if (strcmp(node2->type, "ASSIGNOP") == 0) {
			Tree_Node *id = node->child;
			if (strcmp(id->type, "ID") == 0 && id->sibling == NULL)
				translateExp(node3, 0, id->value);
			else {
				int tempvar1 = gettempvar();
				char dest1[20], dest2[20];
				sprintf(dest1, "v%d", tempvar1);
				translateExp(node, 1, dest1);
				expans ans = analysisExp(node3);
				if (ans.flag == 1)
					sprintf(dest2, "#%d", ans.ans);
				else {
					if (ans.flag == 2)
						strcpy(dest2, node3->child->value);
					else {
						int tempvar2 = gettempvar();
						sprintf(dest2, "v%d", tempvar2);
						translateExp(node3, 0, dest2);
					}
				}
				fprintf(outfile, "*v%d := %s\n", tempvar1, dest2);
			}
			return int_type;
		}
		else if (strcmp(node2->type, "LB") == 0 && dest != NULL) {
			int tempvar1 = gettempvar();
			expans ans = analysisExp(node3);
			char dest1[20];
			sprintf(dest1, "v%d", tempvar1);
			symbol_type *type = translateExp(node, 1, dest1);
			if (ans.flag != 1) {
				int tempvar3 = gettempvar();
				char dest2[20];
				if (ans.flag == 2)
					strcpy(dest2, node3->child->value);
				else {
					int	tempvar2 = gettempvar();
					sprintf(dest2, "v%d", tempvar2);
					translateExp(node3, 0, dest2);
				}
				fprintf(outfile, "v%d := %s * #%d\n", tempvar3, dest2, type->array_field.subtype->size);
				if (flag == 0 && type->array_field.subtype->type == TYPE_INT) {
					int tempvar4 = gettempvar();
					fprintf(outfile, "v%d := v%d + v%d\n", tempvar4, tempvar1, tempvar3);
					fprintf(outfile, "%s := *v%d\n", dest, tempvar4);
				}
				else
					fprintf(outfile, "%s := v%d + v%d\n", dest, tempvar1, tempvar3);
			}
			else {
				if (flag == 0 && type->array_field.subtype->type == TYPE_INT) {
					int tempvar2 = gettempvar();
					fprintf(outfile, "v%d := v%d + #%d\n", tempvar2, tempvar1, 4 * ans.ans);
					fprintf(outfile, "%s := *v%d\n", dest, tempvar2);
				}
				else
					fprintf(outfile, "%s := v%d + #%d\n", dest, tempvar1, type->array_field.subtype->size * ans.ans);
			}
			return type->array_field.subtype;
		}
		else if (strcmp(node2->type, "DOT") == 0 && dest != NULL) {	
			int tempvar1 = gettempvar();
			char dest1[20];
			sprintf(dest1, "v%d", tempvar1);
			symbol_type *type = translateExp(node, 1, dest1);
			symbol_type *fieldtype;			
			int size = 0, i = 0;
			for (i = 0; i < type->struct_field.table_length; i++) {
				if (strcmp(type->struct_field.field_table[i].name, node3->value) == 0) {
					fieldtype = type->struct_field.field_table[i].type;
					if (flag == 0 && fieldtype->type == TYPE_INT) {
						int tempvar2 = gettempvar();
						fprintf(outfile, "v%d := v%d + #%d\n", tempvar2, tempvar1, size);
						fprintf(outfile, "%s := *v%d\n", dest, tempvar2);
					}
					else
						fprintf(outfile, "%s := v%d + #%d\n", dest, tempvar1, size);
					break;
				}
				size += type->struct_field.field_table[i].type->size;
			}
		}
		else if (dest != NULL) {
			char dest1[20], dest2[20];
			expans ans1 = analysisExp(node),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1)
				sprintf(dest1, "#%d", ans1.ans);
			else {
				if (node->child->sibling == NULL && strcmp(node->child->type, "ID") == 0)
					strcpy(dest1, node->child->value);
				else {
					int tempvar1 = gettempvar();
					sprintf(dest1, "v%d", tempvar1);
					translateExp(node, 0, dest1);
				}
			}

			if (ans2.flag == 1)
				sprintf(dest2, "#%d", ans2.ans);
			else {
				if (node3->child->sibling == NULL && strcmp(node3->child->type, "ID") == 0)
					strcpy(dest2, node3->child->value);
				else {
					int tempvar2 = gettempvar();
					sprintf(dest2, "v%d", tempvar2);
					translateExp(node3, 0, dest2);
				}
			}
			fprintf(outfile, "%s := %s %s %s\n", dest, dest1, node2->value, dest2);
			return int_type;
		}
	}
	else if (strcmp(node->type, "ID") == 0) {
		Tree_Node *node2 = node->sibling;
		symbol_table *table = NULL;
		symbol_type *type1 = NULL;
		int i = 0;

		if (node2 == NULL){
			for (i = variable_symbol_table.top - 1; i >=0; i--){
				symbol_table_entry *table = variable_symbol_table.table[i];
				if (variable_symbol_table.existence[i] >= 0 && strcmp(table->name, node->value) == 0) {
					if (table->type->type == TYPE_INT){
						if (flag == 0)
							fprintf(outfile, "%s := %s\n", dest, node->value);
						else
							fprintf(outfile, "%s := &%s\n", dest, node->value);
					}
					else {
						if (variable_symbol_table.existence[i] == 1)
							fprintf(outfile, "%s := &%s\n", dest, node->value);
						else
							fprintf(outfile, "%s := %s\n", dest, node->value);
					}
					return variable_symbol_table.table[i]->type;
				}
			}
		}
		else {
			node2 = node2->sibling;
			if (strcmp(node2->type, "Args") == 0) {
				if (strcmp(node->value, "write") == 0){
					node2 = node2->child;
					expans ans = analysisExp(node2);
					if (ans.flag == 1)
						fprintf(outfile, "WRITE #%d\n", ans.ans);
					else {
						Tree_Node *id = node2->child;
						if (ans.flag == 2)
							fprintf(outfile, "WRITE %s\n", id->value);
						else {
							int tempvar1 = gettempvar();
							char dest1[20];
							sprintf(dest1, "v%d", tempvar1);
							translateExp(node2, 0, dest1);
							fprintf(outfile, "WRITE %s\n", dest1);
						}
					}
					return NULL;
				}
				else {
					void analysisArgs(Tree_Node *args) {
						args = args->child;
						if (args->sibling != NULL)
							analysisArgs(args->sibling->sibling);
						expans ans = analysisExp(args);
						if (ans.flag == 1)
							fprintf(outfile, "ARG #%d\n", ans.ans);
						else {
							Tree_Node *id = args->child;
							char str[20];
							if (ans.flag == 2){
								int i = 0;
								for (i = 0; i < variable_symbol_table.top; i++)
									if (variable_symbol_table.existence[i] > 0 && strcmp(id->value, variable_symbol_table.table[i]->name) == 0) {
										if (variable_symbol_table.existence[i] == 1 && variable_symbol_table.table[i]->type->type != TYPE_INT)
											sprintf(str, "&%s", id->value);
										else
											strcpy(str, id->value);
								}

							}
							else {
								int tempvar2 = gettempvar();
								sprintf(str, "v%d", tempvar2);
								translateExp(args, 0, str);
							}
							fprintf(outfile, "ARG %s\n", str);
						}
					}
					analysisArgs(node2);
					if (dest != NULL)
						fprintf(outfile, "%s := CALL %s\n", dest, node->value);
					else {
						int tempvarn = gettempvar();
						fprintf(outfile, "v%d := CALL %s\n", tempvarn, node->value);
					}
					return int_type;
				}
			}
			else {
				if (strcmp(node->value, "read") == 0)
					fprintf(outfile, "READ %s\n", dest);
				else {
					if (dest != NULL)
						fprintf(outfile, "%s := CALL %s\n", dest, node->value);
					else {
						int tempvarn = gettempvar();
						fprintf(outfile, "v%d := CALL %s\n", tempvarn, node->value);
					}
				}
			}
		}
	}
	else if (strcmp(node->type, "LP") == 0 && dest != NULL) {
		Tree_Node *exp = node->sibling;
		expans ans = analysisExp(exp);
		if (ans.flag == 1)
			fprintf(outfile, "%s := #%d\n", dest, ans.ans);
		else {
			Tree_Node *id = exp->child;
			if (ans.flag == 2)
				fprintf(outfile, "%s := %s\n", dest, id->value);
			else
				translateExp(exp, flag, dest);
		}
	}
	else if (strcmp(node->type, "MINUS") == 0) {	
		Tree_Node *exp = node->sibling;
		expans ans = analysisExp(exp);
		if (ans.flag == 1)
			fprintf(outfile, "%s := #-%d\n", dest, ans.ans);
		else {
			Tree_Node *id = exp->child;
			if (ans.flag == 2)
				fprintf(outfile, "%s := -%s\n", dest, id->value);
			else {
				int tempvar1 = gettempvar();
				char dest1[20];
				sprintf(dest1, "v%d", tempvar1);
				translateExp(exp, flag, dest1);
				fprintf(outfile, "%s := -%s\n", dest, dest1);
			}
		}
		return int_type;
	}
	else if (strcmp(node->type, "INT") == 0) {
		fprintf(outfile, "%s := #%s\n", dest, node->value);
		return int_type;
	}
}
