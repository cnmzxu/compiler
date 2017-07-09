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

int add_variable_symbol_entry(char* name, symbol_type *type, int lineno, int exist, int offset) {
	symbol_table_entry *entry = (symbol_table_entry *)malloc(sizeof(symbol_table_entry));
	
	if(entry == NULL)
		error("malloc error\n");

	symbol_table *table = &variable_symbol_table;

	strcpy(entry->name, name);
	entry->lineno = lineno;
	entry->type = type;
	entry->offset = offset;

	table->table[table->top] = entry;
	table->existence[table->top] = exist;
	table->top = table->top + 1;

	return table->top - 1;
}

bool add_function_symbol_entry(char* name, symbol_type *type, int lineno, int exist) {
	symbol_table_entry *entry = (symbol_table_entry *)malloc(sizeof(symbol_table_entry));
	
	if(entry == NULL)
		error("malloc error\n");

	symbol_table *table = &function_symbol_table;
	
	strcpy(entry->name, name);
	entry->lineno = lineno;
	entry->type = type;
	table->table[table->top] = entry;
	table->existence[table->top] = exist;
	table->top = table->top + 1;

	return true;
}

bool add_struct_symbol_entry(char* name, symbol_type *type, int lineno, int exist) {
	symbol_table_entry *entry = (symbol_table_entry *)malloc(sizeof(symbol_table_entry));
	
	if(entry == NULL)
		error("malloc error\n");
	
	symbol_table *table = &struct_symbol_table;

	strcpy(entry->name, name);
	entry->lineno = lineno;
	entry->type = type;
	
	table->table[table->top] = entry;
	table->existence[table->top] = exist;
	table->top = table->top + 1;

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
	symbol_type *type = (symbol_type *)malloc(sizeof(symbol_type));
	Tree_Node *child = specifier->child;
	if (strcmp(child->type, "TYPE") == 0){
		if (strcmp(child->value, "int") == 0)
			type->type = TYPE_INT;
		else if (strcmp(child->value, "float") == 0)
			type->type = TYPE_FLOAT;
		type->size = 4;
	}
	return type;
}

void get_symbol_entries(symbol_table_entry *table, Tree_Node *deflist, int *length) {
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



