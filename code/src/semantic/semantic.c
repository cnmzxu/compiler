#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic/semantic.h"
#include "parser/tree.h"
#include "common.h"
int global_symbol_number = 0;
void semantic_error(int error_type, int lineno, char *msg){
	printf("Error Type %d at Line %d: %s\n",error_type, lineno, msg);
}

bool check_type_equal(symbol_type *type1, symbol_type *type2) {
	printf("check_type_equal\n");
	if (type1 == type2)
		return true;
	if (type1 == NULL)
		return false;
	if (type2 == NULL)
		return false;
	int i = 0;

	if (type1->type != type2->type)
		return false;
	else if (type1->type == TYPE_ARRAY) {
		if (type1->array_field.length != type2->array_field.length)
			return false;
		return (check_type_equal(type1->array_field.subtype, type2->array_field.subtype));
	}
	else if(type1->type == TYPE_STRUCT) {
		if (type1->struct_field.table_length != type2->struct_field.table_length)
			return false;
		for (i = 0; i < type1->struct_field.table_length; i++)
			if (!check_type_equal(type1->struct_field.field_table[i].type, type2->struct_field.field_table[i].type))
				return false;
	}
	else if (type1->type == TYPE_FUNCTION) {
		if (type1->function_field.variable_length != type2->function_field.variable_length
		|| !check_type_equal(type1->function_field.return_type, type2->function_field.return_type))
			return false;
		for (i = 0; i < type1->function_field.variable_length; i++)
			if (!check_type_equal(type1->function_field.variable[i].type, type2->function_field.variable[i].type))
				return false;
	}
	return true;
}

bool add_symbol_entry(char* name, symbol_type *type, int lineno, table_type_class table_type, int exist) {
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
		table->top = table->top + 1;
	}

	switch (table_type) {
		case STRUCT_TABLE:
			if (check_existence(struct_symbol_table) == -1
			 && check_existence(function_symbol_table) == -1
			 && check_existence(variable_symbol_table) == -1)
				add_entry(&struct_symbol_table);
			else {
				semantic_error(16, lineno, "Redefined Struct Name.");
				return false;
			}
			break;
		case FUNCTION_TABLE:
			if (check_existence(struct_symbol_table) == -1 && check_existence(variable_symbol_table) == -1) {
				int nn = check_existence(function_symbol_table);
				if (nn == -1)
					add_entry(&function_symbol_table);
				else {
					int equal = check_type_equal(function_symbol_table.table[nn]->type, type);
					if (exist == 0){
						if (!equal){
							semantic_error(19, lineno, "Inconsistent Declaration of Function.");
							return false;
						}
					}
					else {
						if (function_symbol_table.existence[nn] == 0) {
							if (equal)
								function_symbol_table.existence[nn] = 1;
							else{
								semantic_error(19, lineno, "Inconsistent Declaration Function.");
								return false;
							}

						}
						else{
							semantic_error(4, lineno, "Redefined Function Name.");
							return false;
						}
					}
				}
			}
			else {
				semantic_error(4, lineno, "Redefined Function Name.");
				return false;
			}
			break;
		case VARIABLE_TABLE:
			if (check_existence(struct_symbol_table) == -1
			 && check_existence(function_symbol_table) == -1
			 && check_existence(variable_symbol_table) == -1)
				add_entry(&variable_symbol_table);
			else{
				semantic_error(3, lineno, "Redefined Variable Name.");
				return false;
			}
			break;
	}
	return true;
}

void creat_new_scope() {
	void push_stack(symbol_table *table) {
		table->table[table->top] = (symbol_table_entry *)table->local_bottom;
		table->existence[table->top] = -1;
		table->top = table->top + 1;
		table->local_bottom = table->top;
	}
	push_stack(&function_symbol_table);
	push_stack(&variable_symbol_table);
	push_stack(&struct_symbol_table);
}

void delete_local_scope() {
	void pop_stack(symbol_table *table) {
		table->top = table->local_bottom - 1;
		table->local_bottom = (int)table->table[table->top];
	}
	pop_stack(&function_symbol_table);
	pop_stack(&variable_symbol_table);
	pop_stack(&struct_symbol_table);

}

symbol_type *get_type(Tree_Node *specifier) {
	printf("get_type\n");
	symbol_type *type = (symbol_type *)malloc(sizeof(symbol_type));
	Tree_Node *child = specifier->child;
	if (strcmp(child->type, "TYPE") == 0){
		if (strcmp(child->value, "int") == 0)
			type->type = TYPE_INT;
		else if (strcmp(child->value, "float") == 0)
			type->type = TYPE_FLOAT;
		type->size = 4;
	}
	else if (strcmp(child->type, "StructSpecifier") == 0) {
		child = child->child->sibling;
		if (strcmp(child->type, "Tag") == 0) {
			char *id = child->child->value;
			int i = 0;
			free(type);
			type = NULL;
			for (i = 0; i < struct_symbol_table.top; i++)
				if (struct_symbol_table.existence[i] < 2 && strcmp(id, struct_symbol_table.table[i]->name) == 0){
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
			
			int i = 0;
			symbol_table_entry *table = type->struct_field.field_table;
			type->size = 0;
			for (i = 0; i < type->struct_field.table_length; i++)
				type->size = type->size + table[i].type->size;
			add_symbol_entry(child->child->value, type, child->child->lineno, STRUCT_TABLE, 1);
		}
	}
	return type;
}

void get_symbol_entries(symbol_table_entry *table, Tree_Node *deflist, int *length) {
	printf("get_symbol_entries\n");
	int tag = 0;
	symbol_type *analysis_vardec(symbol_type *type, Tree_Node *vardec) {
		while(1) {
			vardec = vardec->child;
			if (strcmp(vardec->type, "ID") == 0)
				break;
			symbol_type *new_type = (symbol_type *)malloc(sizeof(symbol_type));
			new_type->type = TYPE_ARRAY;
			new_type->array_field.subtype = type;
			new_type->array_field.length = atoi(vardec->sibling->sibling->value);
			new_type->size = type->size * new_type->array_field.length;
			type = new_type;
		}	
		strcpy(table[tag].name, vardec->value);
		table[tag].lineno = vardec->lineno;
		table[tag].type = type;
		tag++;
		*length = *length + 1;
		return type;
	}

	Tree_Node *specifier, *declist, *vardec;
	symbol_type *spetype = (symbol_type *)malloc(sizeof(symbol_type));
	while (1) {
		if (strcmp(deflist->type, "empty") == 0)
			break;

		specifier = deflist->child->child;
		declist = specifier->sibling;
		spetype = get_type(specifier);
		
		while (1) {
			Tree_Node *dec = declist->child;
			vardec = dec->child;
			analysis_vardec(spetype, vardec);				
			declist = dec->sibling;
			if (declist == NULL)
				break;
			declist = declist->sibling;
		}
			deflist = deflist->child->sibling;
	}
	return ;
}



