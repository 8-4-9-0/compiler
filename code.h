#ifndef CODE_H
#define CODE_H

#include "reference.h"

struct LABEL_END {
	int labelNum;
	struct LABEL_END *nextp;
};

extern FILE *fp;
extern struct LABEL_END *labelIterateEnd;
extern int open_file(char *name);
extern void close_file();
extern void add_label_end(int label);
extern void remove_label_end_tail();
extern int get_label_end_tail();
extern int get_label_num(void);
extern void gen_label(int label);
extern void gen_program_name(char *name, int label);
extern void gen_subprogram_name(char *name);
extern void gen_variable(char *varName, char *procName, int isParam);
extern void gen_code_1(char *opc, char *r);
extern void gen_code_1_label(char *opc, int label);
extern void gen_code_3(char *opc, char *r1, char *r2, char *idxr);
extern void gen_code_3_variable(char *opc, char *r, char *varName, char *procName, int isParam, char *idxr);
extern void gen_code_3_string(char *opc, char *r, char *str, char *idxr);
extern void gen_code_3_label(char *opc, char *r, int label, char *idxr);
extern void gen_call(char *dest);
extern void gen_ret();
extern void gen_nop();
extern void gen_end();
extern void gen_jump(char *opc, char *dest, char *idxr);
extern void gen_jump_label(char *opc, int label, char *idxr);
extern void gen_variable_decl(char *varName, char *procName, struct TYPE *type, int isParam);
#endif