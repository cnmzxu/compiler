#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "intercode/intercode.h"
#include "parser/tree.h"
#include "semantic/semantic.h"

FILE *outfile;
int labelnum, tempvarnum;
int nowoffset;

int getlabel() {
	labelnum++;
	return labelnum;
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
symbol_type *translateExp(Tree_Node *node, int flag, int destreg);// flag = 0 : value, flag = 1 : address
int translateCon(Tree_Node *node, int truelabel, int falselabel2);


int translate(Tree_Node *node) {
	outfile = fopen("./out.s", "w+");

	fprintf(outfile, ".data\n");
	fprintf(outfile, "_promopt: .asciiz \"Enter an integer:\"\n");
	fprintf(outfile, "_ret: .asciiz \"\\n\"\n");
	fprintf(outfile, ".globl main\n");
	fprintf(outfile, ".text\n");
	fprintf(outfile, "read:\n");
	fprintf(outfile, "li $v0, 4\n");
	fprintf(outfile, "la $a0, _promopt\n");
	fprintf(outfile, "syscall\n");
	fprintf(outfile, "li $v0, 5\n");
	fprintf(outfile, "syscall\n");
	fprintf(outfile, "jr $ra\n");
	fprintf(outfile, "\n");
	fprintf(outfile, "write:\n");
	fprintf(outfile, "li $v0, 1\n");
	fprintf(outfile, "syscall\n");
	fprintf(outfile, "li $v0, 4\n");
	fprintf(outfile, "la $a0, _ret\n");
	fprintf(outfile, "syscall\n");
	fprintf(outfile, "move $v0, $0\n");
	fprintf(outfile, "jr $ra\n");
	fprintf(outfile, "\n");

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
	
	if (strcmp(extdef->type, "FunDec") == 0) {
		symbol_type *functype = (symbol_type *)malloc(sizeof(symbol_type));
		functype->function_field.return_type = type;
		functype->function_field.variable_length = 0;
		symbol_table_entry *variable = (symbol_table_entry *)malloc(sizeof(symbol_table_entry) * 20);

		void translateVarDec(Tree_Node *vardec, symbol_type *ttype) {
			int tlength = 0;
			
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
			tlength = functype->function_field.variable_length;
			strcpy(variable[tlength].name, vardec->value);
			variable[tlength].type = ttype;
			tlength++;
			functype->function_field.variable_length = tlength;
			
			add_variable_symbol_entry(vardec->value, ttype, vardec->lineno, 2, -4 * (tlength + 3));
		}

		functype->function_field.variable = variable;
		Tree_Node *fundec = extdef->child;
		fprintf(outfile, "%s:\n", fundec->value);
		if (strcmp(fundec->value, "main") == 0)
			fprintf(outfile, "move $fp, $sp\n");
		add_function_symbol_entry(fundec->value, functype, fundec->lineno, 1);
		fundec = fundec->sibling->sibling;
		Tree_Node *temp;
		creat_new_scope();
		nowoffset = 4;
		
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
		fprintf(outfile, "\n");
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

		add_variable_symbol_entry(id->value, ttype, id->lineno, 1, nowoffset);
		fprintf(outfile, "addi $sp, $sp, %d\n", -ttype->size);
		if (vardec->sibling != NULL) {
			expans ans = analysisExp(vardec->sibling->sibling);
			if (ans.flag == 1) {
				fprintf(outfile, "li $8, %d\n", ans.ans);
				fprintf(outfile, "sw $8, %d($fp)\n", -nowoffset);
			}
				//fprintf(outfile, "%s := #%d\n", id->value, ans.ans);
			else {
				translateExp(vardec->sibling->sibling, 0, nowoffset);
			}
				//translateExp(vardec->sibling->sibling, 0, id->value);
		}
		nowoffset += ttype->size;

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
				fprintf(outfile, "j label%d\n", label);
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
		translateExp(stmt, 0, -100);
		if (label != 0)
			fprintf(outfile, "j label%d\n", label);
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
			fprintf(outfile, "li $v0, %d\n", ans.ans);	
		else {
			translateExp(stmt, 0, -2);
		}
		fprintf(outfile, "jr $ra\n");
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
						fprintf(outfile, "j label%d\n", label);
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
					fprintf(outfile, "label%d:\n", newlabel);
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
				fprintf(outfile, "label%d:\n", newlabel1);
				translateStmt(stmt2, label);
				if (label == 0)
					fprintf(outfile, "label%d:\n", newlabel2);
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
		fprintf(outfile, "label%d:\n", newlabel1);
		translateCon(exp, 0, newlabel2);
		translateStmt(stmt, newlabel1);
		fprintf(outfile, "label%d:\n", newlabel2);
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
	if (truelabel == falselabel && truelabel != 0) {
		fprintf(outfile, "j label%d\n", truelabel);
		return 0;
	}

	if (strcmp(node->type, "Exp") == 0) {
		Tree_Node *node2 = node->sibling;
		Tree_Node *node3 = node2->sibling;
		if (strcmp(node2->type, "LB") == 0) {
			int i, varaddr = 0;
			for (i = 0; i < variable_symbol_table.top; i++)
				if (variable_symbol_table.existence[i] > 0 && strcmp(variable_symbol_table.table[i]->name, node->child->value) == 0) {
					varaddr = variable_symbol_table.table[i]->offset;
					break;
				}
			expans ans = analysisExp(node3);
			if (ans.flag != 1) {
				translateExp(node3, 0, -8);
				fprintf(outfile, "sll $8, $8, 2\n");
				fprintf(outfile, "neg $8, $8\n");
				fprintf(outfile, "la %d($8)\n", -varaddr);
				fprintf(outfile, "add $8, $8, $fp\n");
				fprintf(outfile, "lw $8, 0($8)\n");
			}
			else {
				fprintf(outfile, "lw $8, %d($fp)\n", -(4 * ans.ans + varaddr));
			}
			if (truelabel == 0 && falselabel != 0)
				fprintf(outfile, "beqz $8, label%d\n", falselabel);
			else if (truelabel != 0 && falselabel == 0)
				fprintf(outfile, "bnez $8, label%d\n", truelabel);
			else if (truelabel != 0 && falselabel != 0) {
				fprintf(outfile, "bnez $8, label%d\n", truelabel);
				fprintf(outfile, "j label%d\n", falselabel);
			}
					
		}
		else if (strcmp(node2->type, "AND") == 0) {
			expans ans1 = analysisExp(node),
				   ans2 = analysisExp(node3);
			if (truelabel == 0 && falselabel != 0) {
				if (ans1.flag != 1 && ans2.flag != 1){
					translateCon(node, 0, falselabel);
					translateCon(node3, 0, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans == 0)
						fprintf(outfile, "j label%d\n", falselabel);
					else
						translateCon(node3, 0, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans == 0)
						fprintf(outfile, "j label%d\n", falselabel);
					else
						translateCon(node, 0, falselabel);
				}
				else {
					if (ans1.ans == 0 || ans2.ans == 0)	
						fprintf(outfile, "j label%d\n", falselabel);
				}
			}
			else if (truelabel != 0 && falselabel == 0) {
				if (ans1.flag != 1 && ans2.flag != 1){
					int newlabel = getlabel();
					translateCon(node, 0, newlabel);
					translateCon(node3, truelabel, 0);
					fprintf(outfile, "label%d:\n", newlabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans != 0)
						translateCon(node3, truelabel, 0);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans != 0)
						translateCon(node, truelabel, 0);
				}
				else {
					if (ans1.ans != 0 && ans2.ans != 0)	
						fprintf(outfile, "j label%d\n", truelabel);
				}

			}
			else if (truelabel != 0 && falselabel != 0) {
				if (ans1.flag != 1 && ans2.flag != 1){
					translateCon(node, 0, falselabel);
					translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans == 0)
						fprintf(outfile, "j label%d\n", falselabel);
					else
						translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans == 0)
						fprintf(outfile, "j label%d\n", falselabel);
					else
						translateCon(node, truelabel, falselabel);
				}
				else {
					if (ans1.ans == 0 || ans2.ans == 0)	
						fprintf(outfile, "j label%d\n", falselabel);
					else
						fprintf(outfile, "j label%d\n", truelabel);
				}

			}	
		}
		else if (strcmp(node2->type, "OR") == 0) {
				expans ans1 = analysisExp(node),
					   ans2 = analysisExp(node3);
			if (truelabel == 0 && falselabel != 0) {	
				if (ans1.flag != 1 && ans2.flag != 1){
					int newlabel = getlabel();
					translateCon(node, newlabel, 0);
					translateCon(node3, truelabel, falselabel);
					fprintf(outfile, "label%d:\n", newlabel);
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
						fprintf(outfile, "j label%d\n", falselabel);
				}

			}
			if (truelabel != 0 && falselabel == 0) {	
				if (ans1.flag != 1 && ans2.flag != 1){
					translateCon(node, truelabel, 0);
					translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans != 0)
						fprintf(outfile, "j label%d\n", truelabel);
					else
						translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans != 0)
						fprintf(outfile, "j label%d\n", truelabel);
					else
						translateCon(node, truelabel, falselabel);
				}
				else {
					if (ans1.ans != 0 || ans2.ans != 0)	
						fprintf(outfile, "j label%d\n", truelabel);
				}

			}
			if (truelabel != 0 && falselabel != 0) {	
				if (ans1.flag != 1 && ans2.flag != 1){
					int newlabel = getlabel();
					translateCon(node, truelabel, 0);
					translateCon(node3, truelabel, falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					if (ans1.ans == 0)
						translateCon(node3, truelabel, falselabel);
					else
						fprintf(outfile, "j label%d\n", truelabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					if (ans2.ans == 0)
						translateCon(node, truelabel, falselabel);
					else
						fprintf(outfile, "j label%d\n", truelabel);
				}
				else {
					if (ans1.ans == 0 && ans2.ans == 0)	
						fprintf(outfile, "j label%d\n", falselabel);
					else
						fprintf(outfile, "j label%d\n", truelabel);
				}

			}
		}
		else if (strcmp(node2->type, "RELOP") == 0) {
			expans ans1 = analysisExp(node),
				   ans2 = analysisExp(node3);
			if (truelabel == 0 && falselabel != 0) {
				if (ans1.flag == 1 && ans2.flag == 1) {
					if ((strcmp(node2->value, ">=") == 0 && ans1.ans < ans2.ans)
					|| (strcmp(node2->value, ">") == 0 && ans1.ans <= ans2.ans)
					|| (strcmp(node2->value, "<=") == 0 && ans1.ans > ans2.ans)
					|| (strcmp(node2->value, "<") == 0 && ans1.ans >= ans2.ans)
					|| (strcmp(node2->value, "==") == 0 && ans1.ans != ans2.ans)
					|| (strcmp(node2->value, "!=") == 0 && ans1.ans == ans2.ans))
						fprintf(outfile, "j label%d\n", falselabel);
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					translateExp(node, 0, -8);
					fprintf(outfile, "li $9, %d\n", ans2.ans);
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "blt $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "ble $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "bgt $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "bge $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "bne $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "beq $8, $9, label%d\n", falselabel);
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					translateExp(node3, 0, -8);
					fprintf(outfile, "li $9, %d\n", ans1.ans);
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "bgt $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "bge $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "blt $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "ble $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "bne $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "beq $8, $9, label%d\n", falselabel);
				}
				else {
					translateExp(node, 0, -8);
					translateExp(node3, 0, -9);
					
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "blt $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "ble $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "bgt $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "bge $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "bne $8, $9, label%d\n", falselabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "beq $8, $9, label%d\n", falselabel);
		
				}
			}
			if (truelabel != 0) {
				if (ans1.flag == 1 && ans2.flag == 1) {
					if ((strcmp(node2->value, ">=") == 0 && ans1.ans >= ans2.ans)
					|| (strcmp(node2->value, ">") == 0 && ans1.ans > ans2.ans)
					|| (strcmp(node2->value, "<=") == 0 && ans1.ans <= ans2.ans)
					|| (strcmp(node2->value, "<") == 0 && ans1.ans < ans2.ans)
					|| (strcmp(node2->value, "==") == 0 && ans1.ans == ans2.ans)
					|| (strcmp(node2->value, "!=") == 0 && ans1.ans != ans2.ans))
						fprintf(outfile, "j label%d\n", truelabel);
					else if (falselabel != 0)
						fprintf(outfile, "j label%d\n", falselabel);	
				}
				else if (ans1.flag != 1 && ans2.flag == 1) {
					translateExp(node, 0, -8);
					fprintf(outfile, "li $9, %d\n", ans2.ans);
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "bge $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "bgt $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "ble $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "blt $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "beq $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "bne $8, $9, label%d\n", truelabel);
					if (falselabel != 0)
						fprintf(outfile, "j label%d\n", falselabel);		
				}
				else if (ans1.flag == 1 && ans2.flag != 1) {
					translateExp(node3, 0, -8);
					fprintf(outfile, "li $9, %d\n", ans1.ans);
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "ble $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "blt $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "bge $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "bgt $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "beq $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "bne $8, $9, label%d\n", truelabel);
					if (falselabel != 0)
						fprintf(outfile, "j label%d\n", falselabel);		
				}
				else {
					translateExp(node, 0, -8);
					translateExp(node3, 0, -9);
					if (strcmp(node2->value, ">=") == 0)
						fprintf(outfile, "bge $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, ">") == 0)
						fprintf(outfile, "bgt $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "<=") == 0)
						fprintf(outfile, "ble $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "<") == 0)
						fprintf(outfile, "blt $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "==") == 0)
						fprintf(outfile, "beq $8, $9, label%d\n", truelabel);
					else if (strcmp(node2->value, "!=") == 0)
						fprintf(outfile, "bne $8, $9, label%d\n", truelabel);
					if (falselabel != 0)
						fprintf(outfile, "j label%d\n", falselabel);		
				}
			}
		}	
	}
	else if (strcmp(node->type, "ID") == 0) {
		Tree_Node *node2 = node->sibling;
		if (node2 == NULL) {
			int i = 0;
			for (i = 0; i < variable_symbol_table.top; i++)
				if (variable_symbol_table.existence[i] > 0 && strcmp(variable_symbol_table.table[i]->name, node->value) == 0) {
					fprintf(outfile, "lw $8, %d($fp)\n", -variable_symbol_table.table[i]->offset);
					break;
				}

			if (truelabel == 0 && falselabel != 0)
				fprintf(outfile, "beqz $8, label%d\n", falselabel);
			else if (truelabel != 0 && falselabel == 0)
				fprintf(outfile, "bnez $8, label%d\n", truelabel);
			else if (truelabel != 0 && falselabel != 0){
				fprintf(outfile, "beqz $8, label%d\n", falselabel);
				fprintf(outfile, "j label%d\n", truelabel);
			}
		}
		else {
			node2 = node2->sibling;
			int varsize = 0;
			if (strcmp(node2->type, "Args") == 0) {
				void analysisArgs(Tree_Node *args) {
					args = args->child;
					if (args->sibling != NULL)
						analysisArgs(args->sibling->sibling);
					expans ans = analysisExp(args);
					if (ans.flag == 1) {
						fprintf(outfile, "li $4, %d\n", ans.ans);
						fprintf(outfile, "sw $4, 0($sp)\n");
						varsize += 4;
						fprintf(outfile, "addi $sp, $sp, -4\n");
					}
					else {
						translateExp(args, 0, -4);
						fprintf(outfile, "sw $4, 0($sp)\n");
						varsize += 4;
						fprintf(outfile, "addi $sp, $sp, -4\n");
					}
				}
				analysisArgs(node2);
			}
			
			fprintf(outfile, "sw $v0, 0($sp)\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");
			fprintf(outfile, "sw $ra, 0($sp)\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");

			fprintf(outfile, "sw $fp, 0($sp)\n");
			fprintf(outfile, "move $fp, $sp\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");

			fprintf(outfile, "jal %s\n", node->value);
			
			fprintf(outfile, "move $sp, $fp\n");
			fprintf(outfile, "lw $fp, 0($sp)\n");
			
			fprintf(outfile, "addi $sp, $sp, 4\n");
			fprintf(outfile, "lw $ra, 0($sp)\n");

			
			if (truelabel == 0 && falselabel != 0)
				fprintf(outfile, "beqz $v0 label%d\n", falselabel);
			if (truelabel != 0 && falselabel == 0)
				fprintf(outfile, "bnez $v0 label%d\n", truelabel);
			if (truelabel != 0 && falselabel != 0) {
				fprintf(outfile, "bnez $v0 label%d\n", truelabel);
				fprintf(outfile, "j label%d\n", falselabel);	
			}
			
			fprintf(outfile, "addi $sp, $sp, 4\n");
			fprintf(outfile, "lw $v0, 0($sp)\n");

			if (varsize > 0)
				fprintf(outfile, "addi $sp, $sp, %d\n", varsize);
		}
	}
	else if (strcmp(node->type, "LP") == 0) {
		Tree_Node *node2 = node->sibling;
		expans ans = analysisExp(node2);
		if (ans.flag == 1) {
			if (truelabel == 0 && falselabel != 0 && ans.ans == 0)
				fprintf(outfile, "j label%d\n", falselabel);
			if (truelabel != 0 && falselabel == 0 && ans.ans != 0)
				fprintf(outfile, "j label%d\n", truelabel);
			if (truelabel != 0 && falselabel != 0) {
				if (ans.ans == 0)
					fprintf(outfile, "j label%d\n", falselabel);
				else
					fprintf(outfile, "j label%d\n", truelabel);
			}
		}
		else
			translateCon(node2, truelabel, falselabel);
	}
	else if (strcmp(node->type, "NOT") == 0) {
		expans ans = analysisExp(node->sibling);
		if (ans.flag == 1) {
			if (truelabel == 0 && falselabel != 0 && ans.ans != 0)
				fprintf(outfile, "j label%d\n", falselabel);
			if (truelabel != 0 && falselabel == 0 && ans.ans == 0)
				fprintf(outfile, "j label%d\n", truelabel);
			if (truelabel != 0 && falselabel != 0) {
				if (ans.ans != 0)
					fprintf(outfile, "j label%d\n", falselabel);
				else
					fprintf(outfile, "j label%d\n", truelabel);
			}
		}
		else
			translateCon(node->sibling, falselabel, truelabel);
	}
	else if (strcmp(node->type, "INT") == 0) {
		int num = atoi(node->value);
		
		if (truelabel == 0 && falselabel != 0 && num == 0)
			fprintf(outfile, "j label%d\n", falselabel);
		if (truelabel != 0 && falselabel == 0 && num != 0)
			fprintf(outfile, "j label%d\n", truelabel);
		if (truelabel != 0 && falselabel != 0) {
			if (num == 0)
				fprintf(outfile, "j label%d\n", falselabel);
			else
				fprintf(outfile, "j label%d\n", truelabel);
		}
	}
}

symbol_type *translateExp(Tree_Node *exp, int flag, int dest) {
	//printf("exp\n");
	// flag = 0 : value, flag = 1 : address
	// dest < 0 : reg, dest >= 0 : offset
	symbol_type *int_type = (symbol_type *)malloc(sizeof(symbol_type));
	int_type->type = TYPE_INT;

	Tree_Node *node = exp->child;
	
	if (strcmp(node->type, "Exp") == 0) {
		Tree_Node *node2 = node->sibling;
		Tree_Node *node3 = node2->sibling;
		if (strcmp(node2->type, "ASSIGNOP") == 0) {
			Tree_Node *id = node->child;
			if (strcmp(id->type, "ID") == 0) {
				int i = 0;
				for (i = 0; i < variable_symbol_table.top; i++)
					if (variable_symbol_table.existence[i] > 0 && strcmp(variable_symbol_table.table[i]->name, id->value) == 0) {
						translateExp(node3, 0, variable_symbol_table.table[i]->offset);
						break;
					}
			}
			else {
				int i = 0, varaddr = 0;
				for (i = 0; i < variable_symbol_table.top; i++)
					if (variable_symbol_table.existence[i] > 0 && strcmp(variable_symbol_table.table[i]->name, id->child->value) == 0) {
						varaddr = variable_symbol_table.table[i]->offset;
						break;
					}
				
				expans ans = analysisExp(id->sibling->sibling),
					   ans2 = analysisExp(node3);
				if (ans.flag == 1) {
					varaddr += 4 * ans.ans;
					if (ans2.flag == 1) {
						fprintf(outfile, "li $8, %d\n", ans2.ans);
						fprintf(outfile, "sw $8, %d($fp)\n", -varaddr);
					}
					else
						translateExp(node3, 0, varaddr);
				}
				else {
					translateExp(id->sibling->sibling, 0, -8);
					fprintf(outfile, "sll $8, $8, 2\n");
					fprintf(outfile, "neg $8, $8\n");
					fprintf(outfile, "la $8, %d($8)\n", -varaddr);
					fprintf(outfile, "add, $8, $8, $fp\n");
					if (ans2.flag == 2) {
						fprintf(outfile, "li, $9, %d\n", ans2.ans);
						fprintf(outfile, "sw $9, 0($8)\n");
					}
					else {
						translateExp(node3, 0, -9);
						fprintf(outfile, "sw $9, 0($8)\n");
					}
				}
			}
			return NULL;
		}
		else if (strcmp(node2->type, "LB") == 0 && dest > -32) {
			Tree_Node *id = node->child;
				int i = 0, varaddr = 0;
				for (i = 0; i < variable_symbol_table.top; i++)
					if (variable_symbol_table.existence[i] > 0 && strcmp(variable_symbol_table.table[i]->name, id->value) == 0) {
						varaddr = variable_symbol_table.table[i]->offset;
						break;
					}
				
				expans ans = analysisExp(node3);
				if (ans.flag == 1) {
					varaddr += 4 * ans.ans;
					if (dest >= 0) {
						fprintf(outfile, "sw $8, 0($sp)\n");
						fprintf(outfile, "addi $sp, $sp, -4\n");
						
						fprintf(outfile, "lw $8, %d($fp)\n", -varaddr);
						fprintf(outfile, "sw $8, %d($fp)\n", -dest);
						
						fprintf(outfile, "addi $sp, $sp, 4\n");
						fprintf(outfile, "lw $8, 0($sp)\n");
					}
					else if (dest < 0 && dest > -32) {
						fprintf(outfile, "lw $%d, %d($fp)\n", -dest, -varaddr);
					}
				}
				else {
					int tdest;
					if (dest >= 0) {
						tdest = 8;
						fprintf(outfile, "sw $8, 0($sp)\n");
						fprintf(outfile, "addi $sp, $sp, -4\n");
					}
					else {
						tdest = -dest;
					}

					translateExp(node3, 0, -tdest);
					fprintf(outfile, "sll $%d, $%d, 2\n", tdest, tdest);
					fprintf(outfile, "neg $%d, $%d\n", tdest, tdest);
					fprintf(outfile, "la $%d, %d($%d)\n", tdest, -varaddr, tdest);
					fprintf(outfile, "add $%d, $%d, $fp\n", tdest, tdest);


					if (dest >= 0) {
						fprintf(outfile, "lw $8, 0($8)\n");
						fprintf(outfile, "sw $8, %d($fp)\n", -dest);
						fprintf(outfile, "addi $sp, $sp, 4\n");
						fprintf(outfile, "lw $8, 0($sp)\n");
					}
					else
						fprintf(outfile, "lw $%d, 0($%d)\n", -dest, -dest);
				}
		}
		else if (dest > -32) {
			if (dest != -8) {
				fprintf(outfile, "sw $8, 0($sp)\n");
				fprintf(outfile, "addi $sp, $sp, -4\n");
			}
			if (dest != -9) {
				fprintf(outfile, "sw $9, 0($sp)\n");
				fprintf(outfile, "addi $sp, $sp, -4\n");
			}

			expans ans1 = analysisExp(node),
				   ans2 = analysisExp(node3);
			if (ans1.flag == 1)
				fprintf(outfile, "li $8, %d\n", ans1.ans);
			else 
				translateExp(node, 0, -8);

			if (ans2.flag == 1)
				fprintf(outfile, "li $9, %d\n", ans2.ans);	
			else 
				translateExp(node3, 0, -9);
			
			int tdest;
			if (dest >= 0)
				tdest = 8;
			else
				tdest = -dest;

			if (strcmp(node2->type, "AND") == 0)
				fprintf(outfile, "and $%d, $8, $9\n", tdest);
			else if (strcmp(node2->type, "OR") == 0)
				fprintf(outfile, "or $%d, $8, $9\n", tdest);
			else if (strcmp(node2->type, "PLUS") == 0)
				fprintf(outfile, "add $%d, $8, $9\n", tdest);
			else if (strcmp(node2->type, "MINUS") == 0) {	
				fprintf(outfile, "neg $9, $9\n");
				fprintf(outfile, "add $%d, $8, $9\n", tdest);
			}
			else if (strcmp(node2->type, "STAR") == 0)
				fprintf(outfile, "mul $%d, $8, $9\n", tdest);
			else if (strcmp(node2->type, "DIV") == 0)
				fprintf(outfile, "div $%d, $8, $9\n", tdest);
			
			if (dest >= 0)
				fprintf(outfile, "sw $8, %d($fp)\n", -dest);
			if (dest != -9) {
				fprintf(outfile, "addi $sp, $sp, 4\n");
				fprintf(outfile, "lw $9, 0($sp)\n");
			}
			if (dest != -8) {
				fprintf(outfile, "addi $sp, $sp, 4\n");
				fprintf(outfile, "lw $8, 0($sp)\n");
			}
			return int_type;
		}
	}
	else if (strcmp(node->type, "ID") == 0) {
		Tree_Node *node2 = node->sibling;
		if (node2 == NULL) {
			int i = 0, tdest;
			if (dest >= 0) {
				tdest = 8;
				fprintf(outfile, "sw $8, 0($sp)\n");
				fprintf(outfile, "addi $sp, $sp, -4\n");
			}
			else {
				tdest = -dest;
			}
			for (i = 0; i < variable_symbol_table.top; i++)
				if (variable_symbol_table.existence[i] > 0 && strcmp(variable_symbol_table.table[i]->name, node->value) == 0) {
					fprintf(outfile, "lw $%d, %d($fp)\n", tdest, -variable_symbol_table.table[i]->offset);
					break;
				}

			if (dest >= 0) {
				fprintf(outfile, "sw $8, %d($fp)\n", -dest);

				fprintf(outfile, "addi $sp, $sp, 4\n");
				fprintf(outfile, "lw $8, 0($sp)\n");
			}
		}
		else {
			node2 = node2->sibling;
			int varsize = 0;
			if (strcmp(node2->type, "Args") == 0) {
				void analysisArgs(Tree_Node *args) {
					args = args->child;
					if (args->sibling != NULL)
						analysisArgs(args->sibling->sibling);
					expans ans = analysisExp(args);
					if (ans.flag == 1) {
						fprintf(outfile, "li $4, %d\n", ans.ans);
						fprintf(outfile, "sw $4, 0($sp)\n");
						varsize += 4;
						fprintf(outfile, "addi $sp, $sp, -4\n");
					}
					else {
						translateExp(args, 0, -4);
						fprintf(outfile, "sw $4, 0($sp)\n");
						varsize += 4;
						fprintf(outfile, "addi $sp, $sp, -4\n");
					}
				}
				analysisArgs(node2);
			}

			fprintf(outfile, "sw $ra, 0($sp)\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");

			fprintf(outfile, "sw $8, 0($sp)\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");
			fprintf(outfile, "sw $9, 0($sp)\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");


			fprintf(outfile, "sw $fp, 0($sp)\n");
			fprintf(outfile, "move $fp, $sp\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");

			fprintf(outfile, "jal %s\n", node->value);

			fprintf(outfile, "move $sp, $fp\n");
			fprintf(outfile, "lw $fp, 0($sp)\n");
			
			fprintf(outfile, "addi $sp, $sp, 4\n");
			fprintf(outfile, "lw $9, 0($sp)\n");
			fprintf(outfile, "addi $sp, $sp, 4\n");
			fprintf(outfile, "lw $8, 0($sp)\n");
			
			fprintf(outfile, "addi $sp, $sp, 4\n");
			fprintf(outfile, "lw $ra, 0($sp)\n");
			
			if (dest >= 0) {
				fprintf(outfile, "sw $v0, %d($fp)\n", -dest);
			}
			else if (dest > -32) 
				fprintf(outfile, "move $%d, $v0\n", -dest);
			
			if (varsize > 0)
				fprintf(outfile, "addi $sp, $sp, %d\n", varsize);
		
		}
	}
	else if (strcmp(node->type, "LP") == 0 && dest > -32) {
		Tree_Node *exp = node->sibling;
		expans ans = analysisExp(exp);
		if (ans.flag == 1) {
			if (dest >= 0) {
				fprintf(outfile, "sw $8, 0($sp)\n");
				fprintf(outfile, "addi $sp, $sp, -4\n");

				fprintf(outfile, "li $8, %d\n", ans.ans);
				fprintf(outfile, "sw $8, %d($fp)", -dest);

				fprintf(outfile, "addi $sp, $sp, 4\n");
				fprintf(outfile, "lw $8, 0($sp)\n");	
			}
			else
				fprintf(outfile, "li $%d, %d\n", -dest, ans.ans);
		}
		else {
				translateExp(exp, flag, dest);
		}
	}
	else if (strcmp(node->type, "MINUS") == 0) {	
		Tree_Node *exp = node->sibling;
		expans ans = analysisExp(exp);
		if (ans.flag == 1) {
			if (dest >= 0) {
				fprintf(outfile, "sw $8, 0($sp)\n");
				fprintf(outfile, "addi $sp, $sp, -4\n");

				fprintf(outfile, "li $8, %d\n", ans.ans);
				fprintf(outfile, "sw $8, %d($fp)", -dest);

				fprintf(outfile, "addi $sp, $sp, 4\n");
				fprintf(outfile, "lw $8, 0($sp)\n");	
			}
			else
				fprintf(outfile, "li $%d, %d\n", -dest, ans.ans);
		}
		else {
			if (dest >= 0) {
				fprintf(outfile, "sw $8, 0($sp)\n");
				fprintf(outfile, "addi $sp, $sp, -4\n");
	
				translateExp(exp, flag, -8);
				fprintf(outfile, "sw $8, %d($fp)\n", -dest);

				fprintf(outfile, "addi $sp, $sp, 4\n");
				fprintf(outfile, "lw $8, 0($sp)\n");
			}
			else {
				translateExp(exp, flag, dest);
				fprintf(outfile, "neg $%d, $%d\n", -dest, -dest);
			}
		}
		return int_type;
	}
	else if (strcmp(node->type, "INT") == 0) {
		int num = atoi(node->value);
		if (dest >= 0) {
			fprintf(outfile, "sw $8, 0($sp)\n");
			fprintf(outfile, "addi $sp, $sp, -4\n");
	
			fprintf(outfile, "li $8, %d\n", num);
			fprintf(outfile, "sw $8, %d($fp)\n", -dest);

			fprintf(outfile, "addi $sp, $sp, 4\n");
			fprintf(outfile, "lw $8, 0($sp)\n");
		}
		else {
			fprintf(outfile, "li $%d, %d\n", -dest, num);
		}
		return int_type;
	}
}
