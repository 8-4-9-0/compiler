#ifndef PARSE_H
#define PARSE_H

#include <stdlib.h>
#include <stdarg.h>
#include "scan.h"
#include "reference.h"
#include "code.h"
#include "outlib.h"

void print_token(void);
void print_tab(void);
extern int parse_program(int t);
int parse_block();
int parse_variable_declaration();
int parse_variable_names();
int parse_variable_name();
int parse_type();
int parse_standard_type();
int parse_array_type();
int parse_subprogram_declaration();
int parse_procedure_name();
int parse_formal_parameters();
int parse_compound_statement();
int parse_statement();
int parse_condition_statement();
int parse_iteration_statement();
int parse_exit_statement();
int parse_call_statement();
int parse_expressions();
int parse_return_statement();
int parse_assignment_statement();
int parse_left_part();
int parse_variable();
int parse_expression();
int parse_simple_expression();
int parse_term();
int parse_factor();
int parse_constant();
int parse_multiplicative_operator();
int parse_additive_operator();
int parse_relational_operator();
int parse_input_statement();
int parse_output_statement();
int parse_output_format();

#endif