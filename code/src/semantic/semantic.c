#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic/semantic.h"
#include "parser/tree.h"
#include "common.h"

void semantic_error(int error_type, int lineno, char *msg){
	printf("Error Type %d at Line %d: %s\n",error_type, lineno, msg);
}

int check_type_equal(symbol_type *type1, symbol_type *type2) {
	if (type1 == type2)
		return 1;
	if (type1 == NULL)
		return 0;
	if (type2 == NULL)
		return 0;
	int i = 0;
	if (type1->type != type2->type)
		return 0;
	else if (type1->type == TYPE_ARRAY)
		return (check_type_equal(type1->array_field.subtype, type2->array_field.subtype));
	else if(type1->type == TYPE_STRUCT) {
		if (type1->struct_field.table_length != type2->struct_field.table_length)
			return 0;
		for (i = 0; i < type1->struct_field.table_length; i++)
			if (!check_type_equal(type1->struct_field.field_table[i].type, type2->struct_field.field_table[i].type))
				return 0;
	}
	else if (type1->type == TYPE_FUNCTION) {
		if (type1->function_field.variable_length != type2->function_field.variable_length
		|| !check_type_equal(type1->function_field.return_type, type2->function_field.return_type))
			return 0;
		for (i = 0; i < type1->function_field.variable_length; i++)
			if (!check_type_equal(type1->function_field.variable[i].type, type2->function_field.variable[i].type))
				return 0;
	}

	return 1;
}

void add_symbol_entry(char* name, symbol_type *type, int lineno, table_type_class table_type, int exist) {
	symbol_table_entry *entry = (symbol_table_entry *)malloc(sizeof(symbol_table_entry));
	
	if(entry == NULL)
		error("malloc error\n");

	int check_existence(symbol_table table) {
		int i = 0;
		for (i = table.local_bottom; i < table.top; i++)
			if (strcmp(name, table.table[i]->name) == 0)
				return i;
		return -1;
	}

	void add_entry(symbol_table *table) {
		strcpy(entry->name, name);
		entry->lineno = lineno;
		entry->type = type;
		table->table[table->top] = entry;
		table->existence[table->top] = exist;
		table->top++;
	}

	switch (table_type) {
		case STRUCT_TABLE:
			if (check_existence(struct_symbol_table) == -1
			 && check_existence(function_symbol_table) == -1
			 && check_existence(variable_symbol_table) == -1)
				add_entry(&struct_symbol_table);
			else
				semantic_error(16, lineno, "Redefined Struct Name.");
			break;
		case FUNCTION_TABLE:
			if (check_existence(struct_symbol_table) == -1 && check_existence(variable_symbol_table) == -1) {
				int nn = check_existence(function_symbol_table);
				if (nn == -1)
					add_entry(&function_symbol_table);
				else {
					int equal = check_type_equal(function_symbol_table.table[nn]->type, type);
					if (exist == 0)
						if (!equal)
							semantic_error(19, lineno, "Inconsistent Declaration of Function.");
					else {
						if (function_symbol_table.existence[nn] == 0) {
							if (equal)
								function_symbol_table.existence[nn] = 1;
							else
								semantic_error(19, lineno, "Inconsistent Declaration Function.");

						}
						else
							semantic_error(4, lineno, "Redefined Function Name.");
					}
				}
			}
			else
				semantic_error(4, lineno, "Redefined Function Name.");
			break;
		case VARIABLE_TABLE:
			if (check_existence(struct_symbol_table) == -1
			 && check_existence(function_symbol_table) == -1
			 && check_existence(variable_symbol_table) == -1)
				add_entry(&variable_symbol_table);
			else
				semantic_error(3, lineno, "Redefined Variable Name.");
			break;
	}
}

void creat_new_scope() {
	void push_stack(symbol_table table) {
		table.table[table.top] = (symbol_table_entry *)table.local_bottom;
		table.existence[table.top++] = 2;
		table.local_bottom = table.top;
	}
	push_stack(function_symbol_table);
	push_stack(variable_symbol_table);
	push_stack(struct_symbol_table);
}

void delete_local_scope() {
	void pop_stack(symbol_table table) {
		table.top = table.local_bottom - 1;
		table.local_bottom = (int)table.table[table.top];
	}
	pop_stack(function_symbol_table);
	pop_stack(variable_symbol_table);
	pop_stack(struct_symbol_table);
}

symbol_type *get_type(Tree_Node *specifier) {
	symbol_type *type = (symbol_type *)malloc(sizeof(symbol_type));
	Tree_Node *child = specifier->child;
	if (strcmp(child->type, "Type") == 0){
		if (strcmp(child->value, "int") == 0)
			type->type = TYPE_INT;
		else if (strcmp(child->value, "float") == 0)
			type->type = TYPE_FLOAT;
	}
	else if (strcmp(child->type, "StructSpecifier") == 0) {
		child = child->child->sibling;
		if (strcmp(child->type, "Tag") == 0) {
			char *id = child->child->value;
			int i = 0;
			free(type);
			type = NULL;
			for (i = 0; i < struct_symbol_table.top; i++)
				if (strcmp(id, struct_symbol_table.table[i]->name) == 0){
					type = struct_symbol_table.table[i]->type;
					break;
				}
			if (type == NULL)
				semantic_error(17, child->child->lineno, "Undefined Struct Name");
		}
		else {
			creat_new_scope();
			type->type = TYPE_STRUCT;
			type->struct_field.table_length = 0;
			type->struct_field.field_table = (symbol_table_entry *)malloc(sizeof(symbol_table_entry) * 20);
			get_symbol_entries(type->struct_field.field_table, child->sibling->sibling, &(type->struct_field.table_length));
			delete_local_scope();
			if (strcmp(child->type, "OptTag") == 0)
				add_symbol_entry(child->child->value, type, child->child->lineno, STRUCT_TABLE, 1);
		}
	}
	return type;
}



void get_symbol_entries(symbol_table_entry *table, Tree_Node *deflist, int *length) {
	
	void analysis_vardec(symbol_type *type, Tree_Node *vardec) {
		while(1) {
			vardec = vardec->child;
			if (strcmp(vardec->type, "ID") == 0)
				break;
			symbol_type *new_type = (symbol_type *)malloc(sizeof(symbol_type));
			new_type->type = TYPE_ARRAY;
			new_type->array_field.subtype = type;
			new_type->array_field.length = atoi(vardec->sibling->sibling->value);
			type = new_type;
		}
		add_symbol_entry(vardec->value, type, vardec->lineno, VARIABLE_TABLE, 1);
		if (table != NULL) {
			strcpy(table->name, vardec->value);
			table->lineno = vardec->lineno;
			table->type = type;
			table++;
			*length = *length + 1;
		}
	}

	
	Tree_Node *specifier, *declist, *vardec;
	symbol_type *spetype = (symbol_type *)malloc(sizeof(symbol_type));
	while (1) {
		if (strcmp(deflist->type, "empty") == 0)
			break;

		specifier = deflist->child->child;
		declist = specifier->sibling;
		spetype = get_type(specifier);
		if (strcmp(declist->type, "ExtDecList") == 0) {
			while (1) {
				vardec = declist->child;
				analysis_vardec(spetype, vardec);
				declist = vardec->sibling;
				if (deflist == NULL)
					break;
				declist = declist->sibling;
			}
			deflist = deflist->child->sibling;
		}
		else if (strcmp(declist->type, "FunDec") == 0) {			
			Tree_Node *CompstList = declist->sibling;
			Tree_Node *id = declist->child;
			Tree_Node *varlist = id->sibling->sibling;

			symbol_type *functype = (symbol_type *)malloc(sizeof(symbol_type));
			functype->function_field.return_type = spetype;
			
			creat_new_scope();
			if (strcmp(varlist->type, "VarList") == 0) {
				functype->function_field.variable = (symbol_table_entry *)malloc(sizeof(symbol_table_entry) * 20);
				get_symbol_entries(functype->function_field.variable, declist->child->sibling->sibling, &(functype->function_field.variable_length));
			}
			else
				functype->function_field.variable = NULL;
			if (strcmp(CompstList->type, "Compst") == 0){
				CompstList = CompstList->child->sibling;
				get_symbol_entries(NULL, CompstList, &global_symbol_number);
				stmtlist_analysis(CompstList->sibling);
			}
			delete_local_scope();
			if (strcmp(CompstList->type, "Compst") == 0)
				add_symbol_entry(id->value, functype, declist->lineno, FUNCTION_TABLE, 1);
			else
				add_symbol_entry(id->value, functype, declist->lineno, FUNCTION_TABLE, 0);

		}
		else if	(strcmp(declist->type, "DecList") == 0) {
			while (1) {
				vardec = declist->child;
				analysis_vardec(spetype, vardec);
				declist = vardec->sibling;
				if (declist == NULL)
					break;
				declist = declist->sibling;
			}
			deflist = deflist->child->sibling;
		}
		else if (strcmp(deflist->type, "VarList") == 0){
			analysis_vardec(spetype, declist);
			deflist = deflist->child->sibling->sibling;
		}
	}
	return ;
}

symbol_type *exp_analysis(Tree_Node *exp) {
	Tree_Node *node = node->child;
	symbol_type *int_type = (symbol_type *)malloc(sizeof(symbol_type));
	int_type->type = TYPE_INT;
	symbol_type *float_type = (symbol_type *)malloc(sizeof(symbol_type));
	float_type->type = TYPE_FLOAT;

	if (strcmp(node->type, "Exp") == 0) {
		Tree_Node *node2 = node->sibling;
		Tree_Node *node3 = node2->sibling;
		if (strcmp(node2->type, "ASSIGNOP") == 0) {
			if (strcmp(node->child->type, "RExp") == 0)
				semantic_error(6, node->lineno, "Exp Has no Left Value.");
			else {
				symbol_type *type1 = exp_analysis(node),
							*type2 = exp_analysis(node3);
				if (type1 == NULL || type2 == NULL)
					return NULL;
				if (!check_type_equal(type1, type2))
					semantic_error(5, node->lineno, "Mismatched Type.");
			}
			return NULL;
		}
		else if (strcmp(node2->type, "LB") == 0) {
			symbol_type *type1 = exp_analysis(node),
						*type2 = exp_analysis(node3);
			if (type1 == NULL || type2 == NULL)
				return NULL;
			if (type1->type != TYPE_ARRAY) {
				semantic_error(10, node->lineno, "Not a array vaiable.");
				return NULL;
			}
			if (type2->type != TYPE_INT) {
				semantic_error(12, node->lineno, "Should use INT to visit array.");
				return NULL;
			}
			return type1->array_field.subtype;
		}
		else if (strcmp(node2->type, "DOT") == 0) {
			symbol_type* type1 = exp_analysis(node);
			if (type1 == NULL)
				return NULL;
			if (type1->type != TYPE_STRUCT){
				semantic_error(13, node->lineno, "Not a struct variable.");
				return NULL;
			}
			symbol_table_entry *field_table = type1->struct_field.field_table;
			int i = 0;
			for (i = 0; i < type1->struct_field.table_length; i++)
				if (strcmp(node3->value, field_table[i].name) == 0)
					return field_table[i].type;
			semantic_error(14, node->lineno, "No such a field.");
			return NULL;			
		}
		else if (strcmp(node2->type, "AND") == 0 || strcmp(node2->type, "OR") == 0) {
			symbol_type *type1 = exp_analysis(node),
						*type2 = exp_analysis(node3);
			if (type1 == NULL || type2 == NULL)
				return NULL;
			if (type1->type == TYPE_INT && type2->type == TYPE_INT)
				return int_type;
			semantic_error(7, node->lineno, "Mismatched Type.");
			return NULL;
		}
		else {
			symbol_type *type1 = exp_analysis(node),
						*type2 = exp_analysis(node3);
			if (type1 == NULL || type2 == NULL)
				return NULL;
			if (type1->type == type2->type)
				return type1;
			semantic_error(7, node->lineno, "Mismatched Type.");
			return NULL;
		}
	}
	else if (strcmp(node->type, "ID") == 0) {
		Tree_Node *node2 = node->sibling;
		symbol_table *table = NULL;
		symbol_type *type1;
		int i = 0;
		if (node2 == NULL){
			table = &variable_symbol_table;
			for (i = table->top - 1; i >= 0; i--)
				if (table->existence[i] < 2 && strcmp(table->table[i]->name, node->value) == 0)
					return table->table[i]->type;
			semantic_error(1, node->lineno, "No such a variable.");
		}
		else {
			Tree_Node *node3 = node2->sibling;
			table = &function_symbol_table;
			for (i = table->top - 1; i >= 0; i--)
				if (table->existence[i] < 2 && strcmp(table->table[i]->name, node->value) == 0)
					if (table->existence[i] == 1)
						type1 = table->table[i]->type;
					else 
						semantic_error(18, node->lineno, "Declared but not Defined.");
			semantic_error(2, node->lineno, "Undefined Function.");
		}
	}
	else if (strcmp(node->type, "LP") == 0) {
		return exp_analysis(node->sibling);
	}
	else if (strcmp(node->type, "MINUS") == 0) {
		return exp_analysis(node->sibling);
	}
	else if (strcmp(node->type, "NOT") == 0) {
		return exp_analysis(node->sibling);
	}
	else if (strcmp(node->type, "INT") == 0) {
		return int_type;
	}
	else if (strcmp(node->type, "FLOAT") == 0) {
		return float_type;
	}
	else if (strcmp(node->type, "RExp") == 0) {
		return exp_analysis(node);
	}
	else if (strcmp(node->type, "LExp") == 0) {
		return exp_analysis(node);
	}
}

void stmtlist_analysis(Tree_Node *stmtlist) {
	Tree_Node *stmt;
	
	void stmt_analysis(Tree_Node *stmt) {
		Tree_Node *node = stmt->child;
		symbol_type int_type;
		int_type.type = TYPE_INT;

		if (strcmp(node->type, "IF") == 0) {
			node = node->sibling->sibling;
			if (!check_type_equal(exp_analysis(node), &int_type))
				semantic_error(7, node->lineno, "Conditional Statement Should be INT.");

			node = node->sibling->sibling;
			stmt_analysis(node);
			if (node->sibling != NULL) {
				node = node->sibling->sibling;
				stmt_analysis(node);
			}
		}
		else if (strcmp(node->type, "WHILE") == 0) {
			node = node->sibling->sibling;
			if (!check_type_equal(exp_analysis(node), &int_type))
				semantic_error(7, node->lineno, "Conditional Statement Should be INT.");

			node = node->sibling->sibling;
			stmt_analysis(node);
			
		}
		else if (strcmp(node->type, "RETURN") == 0) {
			node = node->sibling;
			exp_analysis(node);
		}
		else if (strcmp(node->type, "Exp") == 0) {
			exp_analysis(node);
		}
		else if (strcmp(node->type, "CompSt") == 0) {
			node = node->child->sibling;
			creat_new_scope();
			get_symbol_entries(NULL, node, &global_symbol_number);
			stmtlist_analysis(node->sibling);
			delete_local_scope();
		}
		
	}
	
	while (1) {
		if (strcmp(stmtlist->type, "empty") == 0)
			break;
		stmt = stmtlist->child;
		stmtlist = stmt->sibling;
		stmt_analysis(stmt);
	}
	return;
}


int semantic_analysis(Tree_Node *node) {

}
