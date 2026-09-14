/**
 * @file code.c
 * @author Kyogo Yamashita
 * @brief  ファイル生成及びアセンブリ生成を行う
 *
 */
#include "code.h"

FILE *fp;
struct LABEL_END *labelIterateEnd;

int open_file(char *name) {
    int pos = 0;
    char *fileName;

    if((fileName = (char *)malloc(strlen(name) + 4 + 1)) == NULL) {
        return ERROR;
    }
    strcpy(fileName, name);
    for(; fileName[pos]!='\0'; pos++);
    if(strncmp(&fileName[pos-4], ".mpl", 4) == 0) {
        fileName[pos-3] = 'c';
        fileName[pos-2] = 's';
    } else {
        snprintf(&fileName[pos], 5, "%s", ".csl");
    }

    if((fp = fopen(fileName, "w")) == NULL) {
        return ERROR;
    }

    return NORMAL;
}

void close_file() {
    fclose(fp);
}

int get_label_num(void) {
    static int labelCounter = 2;

    return labelCounter++;
}

void add_label_end(int label) {
    struct LABEL_END *p, *tail;

    if((p = (struct LABEL_END *)malloc(sizeof(struct LABEL_END))) == NULL) {
        error("Cannot allocate memory");
        return;
    }

    p->labelNum = label;
    p->nextp = NULL;

    if(labelIterateEnd == NULL) {
        labelIterateEnd = p;
    } else {
        tail = labelIterateEnd;
        for(; tail->nextp!=NULL; tail=tail->nextp);
        tail->nextp = p;
    }
}

int get_label_end_tail() {
    struct LABEL_END *p = labelIterateEnd;
    int label;

    if(p == NULL) return 0;

    for(; p->nextp!=NULL; p=p->nextp);

    return p->labelNum;
}

void remove_label_end_tail() {
    struct LABEL_END *p = labelIterateEnd;
    struct LABEL_END *rmp;
    
    if(p == NULL) return;

    for(p; p->nextp!=NULL&&p->nextp->nextp!=NULL; p=p->nextp);
    rmp = p->nextp;
    p->nextp = NULL;
    free(rmp);
}

void gen_label(int label) {
    fprintf(fp, "L%04d\n", label);
}

void gen_program_name(char *name, int label) {
    fprintf(fp, "%%%%%s\tSTART\tL%04d\n", name, label);
}

void gen_subprogram_name(char *name) {
    fprintf(fp, "$%s\n", name);
}

void gen_variable(char *varName, char *procName, int isParam) {
    fprintf(fp, "$");
    if(isParam) fprintf(fp, "$");
    fprintf(fp, "%s", varName);
    if(procName != NULL && procName[0] != '\0') fprintf(fp, "%%%s", procName);
}

void gen_variable_decl(char *varName, char *procName, struct TYPE *type, int isParam) {
    gen_variable(varName, procName, isParam);
    fprintf(fp, "\n\t");
    if(type->typeNo == TPARRAY) {
        fprintf(fp, "DS\t%d\n", type->arraySize);
    } else {
        fprintf(fp, "DC\t0\n");
    }
}

void gen_code_1(char *opc, char *opr) {
    fprintf(fp, "\t%s\t%s\n", opc, opr);
}

void gen_code_1_label(char *opc, int label) {
    fprintf(fp, "\t%s\tL%04d\n", opc, label);
}

void gen_code_3(char *opc, char *opr1, char *opr2, char *idxr) {
    fprintf(fp, "\t%s\t%s,%s", opc, opr1, opr2);
    if(idxr != NULL) fprintf(fp, ",%s", idxr);
    fprintf(fp, "\n");
}

void gen_code_3_variable(char *opc, char *opr, char *varName, char *procName, int isParam, char *idxr) {
    fprintf(fp, "\t%s\t%s,", opc, opr);
    gen_variable(varName, procName, isParam);
    if(idxr != NULL) fprintf(fp, ",%s", idxr);
    fprintf(fp, "\n");
}

void gen_code_3_string(char *opc, char *opr, char *str, char *idxr) {
    fprintf(fp, "\t%s\t%s,='%s'", opc, opr, str);
    if(idxr != NULL) fprintf(fp, ",%s", idxr);
    fprintf(fp, "\n");
}

void gen_code_3_label(char *opc, char *opr, int label, char *idxr) {
    fprintf(fp, "\t%s\t%s,L%04d", opc, opr, label);
    if(idxr != NULL) fprintf(fp, ",%s", idxr);
    fprintf(fp, "\n");
}

void gen_jump(char *opc, char *dest, char *idxr) {
    fprintf(fp, "\t%s\t%s\n", opc, dest);
    if(idxr != NULL) fprintf(fp, ",%s", idxr);
}

void gen_jump_label(char *opc, int label, char *idxr) {
    fprintf(fp, "\t%s\tL%04d\n", opc, label);
    if(idxr != NULL) fprintf(fp, ",%s", idxr);
}

void gen_call(char *dest) {
    fprintf(fp, "\tCALL\t$%s\n", dest);
}

void gen_ret() {
    fprintf(fp, "\tRET\n");
}

void gen_nop() {
    fprintf(fp, "\tNOP\n");
}

void gen_end() {
    fprintf(fp, "\tEND\n");
}