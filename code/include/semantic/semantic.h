#ifndef __SEMANTIC_SEMANTIC__
#define __SEMANTIC_SEMANTIC__

#include <stdio.h>
#include "parser/tree.h"
#include "common.h"

typedef enum {false, true} bool;

void semantic_error(int error_type, int lineno, char *msg);

typedef enum {TYPE_INT, TYPE_FLOAT, TYPE_STRUCT, TYPE_ARRAY, TYPE_FUNCTION} symbol_type_class;

typedef enum {STRUCT_TABLE, FUNCTION_TABLE, VARIABLE_TABLE} table_type_class;

typedef struct symbol_type_struct {
	symbol_type_class type;
	union {
		struct {
			int length;
			struct symbol_type_struct *subtype;
		} array_field;
		struct {
			int table_length;
			struct symbol_table_entry_struct *field_table;
		} struct_field;
		struct {
			int variable_length;
			struct symbol_type_struct *return_type;
			struct symbol_table_entry_struct *variable;
		} function_field;
	};
} symbol_type;

typedef struct symbol_table_entry_struct {
	char name[30];
	int lineno;
	symbol_type* type;
} symbol_table_entry;

bool check_type_equal(symbol_type *type1, symbol_type *type2);

typedef struct {
	int top, local_bottom;
	symbol_table_entry *table[100];
	int existence[100];
	table_type_class type;
} symbol_table;

symbol_table variable_symbol_table, struct_symbol_table, function_symbol_table;

bool add_symbol_entry(char *name, symbol_type *type, int lineno, table_type_class table_type, int exist);

void creat_new_scope();
void delete_local_scope();

symbol_type *get_type(Tree_Node *specifier);

void get_symbol_entries(symbol_table_entry *table, Tree_Node *deflist, int *length, table_type_class type);

symbol_type *exp_analysis(Tree_Node *exp);

void stmtlist_analysis(Tree_Node *stmtlist);

symbol_type *exp_analysis(Tree_Node *exp);

int semantic_analysis(Tree_Node *head);

extern int global_symbol_number;

symbol_type *now_return_type;

#endif
