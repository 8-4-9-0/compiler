/**
* @file parse.c 
* @brief LL(1)構文解析を行う
* @author Kyogo Yamashita
*/

#include "parse.h"

char *token_str[] = {
	"",
	"", //TNAME
	"program",
	"var",
	"array",
	"of",
	"begin",
	"end",
	"if",
	"then",
	"else",
	"procedure",
	"return",
	"call",
	"while",
	"do",
	"not",
	"or",
	"div",
	"and",
	"char",
	"integer",
	"boolean",
	"readln",
	"writeln",
	"true",
	"false",
	"", // TNUMBER
	"", // TSTRING
	"+",
	"-",
	"*",
	"=",
	"<>",
	"<",
	"<=",
	">",
	">=",
	"(",
	")",
	"[",
	"]",
	":=",
	".",
	",",
	":",
	";",
	"read",
	"write",
	"break"
};

int token;
int nest = 0;
char tab[] = "    ";
int break_next = 0;
int isVariable = 0;

void print_token(void) {
    token = scan();
    return; // 課題3はプリティプリントを行わない

    char *str;

    switch(token) {
        case TNAME:
        case TSTRING:
        case TNUMBER:
            str = string_attr;
            break;
        default:
            str = token_str[token];
            break;
    }
    if(token == TSTRING) {
        printf("'%s'", str);
    } else {
        printf("%s", str);
    }
    
    if(token != TEND) {
        token = scan();
        //if(token != TEND && token != TSEMI && token != TDOT && token != TELSE && break_next != 1)  // 行中の字句と字句の間は1つの空白だけがある｜";","."の直前には空白を入れない
    } else {
        token = scan();
    }
    break_next = 0;
}
/*
void print_tab(void) {
    for(int i=0; i<nest; i++) {
        printf(tab); // 段付の1段は空白4文字とする
    }
}
*/

int parse_program(int t) {
    token = t;

    if(token != TPROGRAM) return error("Keyword 'program' is not found");
    print_token();
    if(token != TNAME) return error("Program name is not found");
    gen_program_name(string_attr, 1);  // %%name   START   L0001
    print_token();
    if(token != TSEMI) return error("Symbol ';' is not found");
    print_token();
    if(!parse_block()) return ERROR;
    if(token != TDOT) return error("Symbol '.' is not found at the end of program");
    print_token();

    gen_ret();
    outlib();
    gen_end();
    return NORMAL;
}
int parse_block() {
    add_symbol(SBLOCK);
    while(1) {
        if(token == TVAR) {
            if(!parse_variable_declaration()) return ERROR;
        } else if(token == TPROCEDURE) {
            if(!parse_subprogram_declaration()) return ERROR;
        } else {
            break;
        }
    }
    gen_label(1);
    gen_code_3("LAD", "GR0", "0", NULL);
    if(!parse_compound_statement()) return ERROR;
    remove_symbol_tail();
    return NORMAL;
}
int parse_variable_declaration() {
    char *np, *procnp;
    struct TYPE *tp;
    int canDefine;

    add_symbol(SVARDECL);
    if(token != TVAR) return error("Keyword 'var' is not found");
    print_token();
    if(!parse_variable_names()) return ERROR;
    if(token != TCOLON) return error("Symbol ':' is not found");
    print_token();
    if(!parse_type()) return ERROR;
    if(token != TSEMI) return error("Symbol ';' is not found");
    print_token();
    while(token == TNAME) {
        if(!parse_variable_names()) return ERROR;
        if(token != TCOLON) return error("Symbol ':' is not found");
        print_token();
        if(!parse_type()) return ERROR;
        if(token != TSEMI) return error("Symbol ';' is not found");
        print_token();
    }
    while(1) {
        canDefine = iterate_undefined_variable(&np, &procnp, &tp);
        // 変数コード生成
        if(canDefine) {
            gen_variable_decl(np, procnp, tp, 0);
        } else {
            break;
        }
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_variable_names() {
    add_symbol(SVARNAMES);
    if(!parse_variable_name()) return ERROR;
    while(token == TCOMMA) {
        print_token();
        if(!parse_variable_name()) return ERROR;
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_variable_name() {
    add_symbol(SVARNAME);
    if(token != TNAME) return error("Variable name is not found");
    if(find_symbol(SVARDECL) || find_symbol(SFORMALPARAMS)) {
        if(!register_identifier_name()) return ERROR;
    } else if(find_symbol(SCOMPSTMT)) {
        if(!(typeNoRef = add_refline())) return ERROR;
    }
    print_token();

    remove_symbol_tail();
    return NORMAL;
}
int parse_type() {
    add_symbol(STYPE);
    if(token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        if(!parse_standard_type()) return ERROR;
    } else if(token == TARRAY) {
        if(!parse_array_type()) return ERROR;
    } else {
        return error("Variable type is invalid or missing"); 
    }
    if(find_symbol(SVARDECL) || find_symbol(SFORMALPARAMS)) {
        if(!register_identifier_type()) return ERROR;
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_standard_type() {
    add_symbol(SSTDTYPE);
    switch(token) {
        case TINTEGER:
            if(find_symbol(SARRTYPE)) {
                arrayElemTypeNoRef = TPINT;
            } else {
                typeNoRef = TPINT;
            }
            break;
        case TBOOLEAN:
            if(find_symbol(SARRTYPE)) {
                arrayElemTypeNoRef = TPBOOL;
            } else {
                typeNoRef = TPBOOL;
            }
            break;
        case TCHAR:
            if(find_symbol(SARRTYPE)) {
                arrayElemTypeNoRef = TPCHAR;
            } else {
                typeNoRef = TPCHAR;
            }
            break;
        default:
            return error("Variable type is invalid or missing");
    }
    print_token();

    remove_symbol_tail();    
    return NORMAL;
}
int parse_array_type() {
    add_symbol(SARRTYPE);
    if(token != TARRAY) return error("Keyword 'array' is not found");
    typeNoRef = TPARRAY;
    print_token();
    if(token != TLSQPAREN) return error("Symbol '[' is not found");
    print_token();
    if(token != TNUMBER) return error("Array size is missing");
    if((arraySizeRef = num_attr) < 1) return error("Array size is 1 or more");
    print_token();
    if(token != TRSQPAREN) return error("Symbol ']' is not found");
    print_token();
    if(token != TOF) return error("Keyword 'of' is not found");
    print_token();
    if(!parse_standard_type()) return ERROR;

    remove_symbol_tail();
    return NORMAL;
}
int parse_subprogram_declaration() {
    char *np, *procnp;
    struct type *tp;
    add_symbol(SSUBPROGDECL);
    if(token != TPROCEDURE) return error("Keyword 'procedure' is not found");
    print_token();
    if(!parse_procedure_name()) return ERROR;
    if(token == TLPAREN) if(!parse_formal_parameters()) return ERROR;
    if(token != TSEMI) return error("Symbol ';' is not found");
    print_token();
    if(token == TVAR) if(!parse_variable_declaration()) return ERROR;
    gen_subprogram_name(procNameRef);
    if(iterate_parameter_rev(&np, &procnp)) {
        gen_code_1("POP", "GR2");
        do {
            gen_code_1("POP", "GR1");
            gen_code_3_variable("ST", "GR1", np, procnp, 1, NULL);
        } while(iterate_parameter_rev(&np, &procnp));
        gen_code_3("PUSH", "0", "GR2", NULL);
    }
    
    if(!parse_compound_statement()) return ERROR;
    if(token != TSEMI) return error("Symbol ';' is not found"); 
    print_token();
    gen_ret();

    if(!switch_root()) return ERROR;
    remove_symbol_tail();
    return NORMAL;
}
int parse_procedure_name() {
    add_symbol(SPROCNAME);
    if(token != TNAME) return error("Procedure name is not found");
    if(find_symbol(SCALLSTMT)) {
        if(!add_refline()) return ERROR;
    } else {
        if(!register_identifier_name()) return ERROR;
    }
    print_token();
    
    remove_symbol_tail();
    return NORMAL;
}
int parse_formal_parameters() {
    char *np, *procnp;
    struct TYPE *tp;
    int canDefine;

    add_symbol(SFORMALPARAMS);
    if(token != TLPAREN) return error("Symbol '(' is not found");
    print_token();
    if(!parse_variable_names()) return ERROR;
    if(token != TCOLON) return error("Symbol ':' is not found");
    print_token();
    if(!parse_type()) return ERROR;
    while(token == TSEMI) {
        print_token();
        if(!parse_variable_names()) return ERROR;
        if(token != TCOLON) return error("Symbol ':' is not found");
        print_token();
        if(!parse_type()) return ERROR;
    }
    if(token != TRPAREN) return error("Symbol ')' is not found; or is symbol ';' missing?");
    print_token();
    while(1) {
        canDefine = iterate_undefined_variable(&np, &procnp, &tp);
        // 変数コード生成
        if(canDefine) {
            gen_variable_decl(np, procnp, tp, 1);
        } else {
            break;
        }
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_compound_statement() {
    add_symbol(SCOMPSTMT);
    if(token != TBEGIN) return error("Keyword 'begin' is not found");
    print_token();
    if(!parse_statement()) return ERROR;
    while(token == TSEMI) {
        print_token();
        if(token == TEND) break;
        if(!parse_statement()) return ERROR;
    }
    if(token != TEND) return error("Keyword 'end' is not found; or is symbol ';' missing?");
    print_token();

    remove_symbol_tail();
    return NORMAL;
}
int parse_statement() {
    add_symbol(SSTMT);
    if(token == TNAME) {
        if(!parse_assignment_statement()) return ERROR;
    } else if(token == TIF) {
        if(!parse_condition_statement()) return ERROR;
    } else if(token == TWHILE) {
        if(!parse_iteration_statement()) return ERROR;
    } else if(token == TBREAK) {
        if(!parse_exit_statement()) return ERROR;
    } else if(token == TCALL) {
        if(!parse_call_statement()) return ERROR;
    } else if(token == TRETURN) {
        if(!parse_return_statement()) return ERROR;
    } else if(token == TREAD || token == TREADLN) {
        if(!parse_input_statement()) return ERROR;
    } else if(token == TWRITE || token == TWRITELN) {
        if(!parse_output_statement()) return ERROR;
    } else if(token == TBEGIN) {
        if(!parse_compound_statement()) return ERROR;
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_condition_statement() {
    int label1, label2;
    add_symbol(SCONDSTMT);
    if(token != TIF) return error("Keyword 'if' is not found");
    print_token();
    if(!parse_expression()) return ERROR;
    // (式の型はboolean 型でなくてはならない)
    if(typeNoRef != TPBOOL) return error("The expression in a if statement must be a boolean");
    label1 = get_label_num();
    gen_code_3("CPA", "GR1", "GR0", NULL);
    gen_jump_label("JZE", label1, NULL);  // JZEが真ならばfalse(ZF=1 => false)
    if(token != TTHEN) return error("Keyword 'then' is not found");
    print_token();
    if(!parse_statement()) return ERROR;
    if(token == TELSE) {
        label2 = get_label_num();
        gen_jump_label("JUMP", label2, NULL);
        gen_label(label1);
        print_token();
        if(!parse_statement()) return ERROR;
        gen_label(label2);
        /*
        if(token != TIF) {
            if(!parse_statement()) return ERROR;
        } else {
            if(!parse_condition_statement()) return ERROR;
        }
        */
    } else {
        gen_label(label1);
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_iteration_statement() {
    int label1, label2;
    add_symbol(SITERSTMT);
    if(token != TWHILE) return error("Keyword 'while' is not found");
    print_token();
    label1 = get_label_num();
    gen_label(label1);
    if(!parse_expression()) return ERROR;
    // (式の型はboolean 型でなくてはならない)
    if(typeNoRef != TPBOOL) return error("The expression in a while statement must be a boolean");
    gen_code_3("CPA", "GR1", "GR0", NULL);
    label2 = get_label_num();
    add_label_end(label2);
    gen_jump_label("JZE", label2, NULL);
    if(token != TDO) return error("Keyword 'do' is not found");
    print_token();
    if(!parse_statement()) return ERROR;
    gen_jump_label("JUMP", label1, NULL);
    gen_label(label2);
    remove_label_end_tail();

    remove_symbol_tail();
    return NORMAL;
}
int parse_exit_statement() {
    add_symbol(SEXITSTMT);
    if(token != TBREAK) return error("Keyword 'break' is not found");
    if(labelIterateEnd != NULL) gen_jump_label("JUMP", get_label_end_tail(), NULL);
    print_token();

    remove_symbol_tail();
    return NORMAL;
}
int parse_call_statement() {
    char procNameTemp[MAXSTRLEN+1];
    add_symbol(SCALLSTMT);
    if(token != TCALL) return error("Keyword 'call' is not found");
    print_token();
    strcpy(procNameTemp, string_attr);
    if(!parse_procedure_name()) return ERROR;

    if(token == TLPAREN) {
        print_token();
        if(paramTypeRef == NULL) return error("The procedure has no parameters");
        if(!parse_expressions()) return ERROR;
        // if(typeNoRef != TPPROC) return error("Specified identifier is not procedure");
        if(token != TRPAREN) return error("Symbol ')' is not found");
        print_token();
    }

    gen_call(procNameTemp);
    remove_symbol_tail();
    return NORMAL;
}
int parse_expressions() {
    int label1, label2;
    /*
    式の並び がない場合や，仮引数部 がないという場合はそれぞれの数は 0とする
    */
    add_symbol(SEXPS);
    if(!parse_expression()) return ERROR;
    if(isVariable) {
        gen_code_3("PUSH", "0", "GR1", NULL);
    } else {
        label1 = get_label_num();
        label2 = get_label_num();
        gen_jump_label("JUMP", label2, NULL);
        gen_label(label1);
        gen_code_1("DC", "0");
        gen_label(label2);
        gen_code_3_label("ST", "GR1", label1, NULL);
        gen_code_1_label("PUSH", label1);
    }
    isVariable = 0;
    // 式と仮引数部の変数は順序で対応し，対応する式と変数は同じ標準型でなくてはならない
    // if(typeNoRef != localIdRootRef->type->typeNo) return error("Argument type mismatch");
    if(typeNoRef != paramTypeRef->typeNo) return error("Argument type mismatch");
    while(token == TCOMMA) {
        // 式の数はその手続き名の宣言の仮引数部の変数の数と一致していなくてはならない
        /*
        if(localIdRootRef->nextp == NULL || localIdRootRef->nextp->isParam != 1) return error("Too many arguments");
        localIdRootRef = localIdRootRef->nextp;
        */
        if(paramTypeRef->paramTypep == NULL) return error("Too many arguments");
        paramTypeRef = paramTypeRef->paramTypep;
        print_token();
        if(!parse_expression()) return ERROR;
        if(isVariable) {
            gen_code_3("PUSH", "0", "GR1", NULL);
        } else {
            label1 = get_label_num();
            label2 = get_label_num();
            gen_jump_label("JUMP", label2, NULL);
            gen_label(label1);
            gen_code_1("DC", "0");
            gen_label(label2);
            gen_code_3_label("ST", "GR1", label1, NULL);
            gen_code_1_label("PUSH", label1);
        }
        // 式と仮引数部の変数は順序で対応し，対応する式と変数は同じ標準型でなくてはならない
        if(typeNoRef != paramTypeRef->typeNo) return error("Argument type mismatch");
    }

    // 式の数はその手続き名の宣言の仮引数部の変数の数と一致していなくてはならない
    if(paramTypeRef->paramTypep != NULL) return error("Too few arguments");
    remove_symbol_tail();
    return NORMAL;
}
int parse_return_statement() {
    add_symbol(SRETSTMT);
    if(token != TRETURN) return error("Keyword 'return' is not found");
    gen_ret();
    print_token();

    remove_symbol_tail();
    return NORMAL;
}
int parse_assignment_statement() {
    int typeNoRefTemp;
    add_symbol(SASGSTMT);
    if(!parse_left_part()) return ERROR;
    gen_code_3("PUSH", "0", "GR1", NULL);
    typeNoRefTemp = typeNoRef;
    if(token != TASSIGN) return error("Symbol ':=' is not found");
    print_token();
    if(!parse_expression()) return ERROR;
    // 左辺部と式の型は同じ型で標準型でなくてはならない
    if(typeNoRefTemp != typeNoRef) return error("The left side and the expression must have the same standard type");
    gen_code_1("POP", "GR2");
    gen_code_3("ST", "GR1", "0", "GR2");

    remove_symbol_tail();
    return NORMAL;
}
int parse_left_part() {
    add_symbol(SLEFTPART);
    if(!parse_variable()) return ERROR;

    remove_symbol_tail();
    return NORMAL;
}
int parse_variable() {
    struct ID *variableTemp;
    int arraySizeRefTemp;
    int arrayElemTypeNoRefTemp;
    char numBuf[6];

    if(find_symbol(SVAR)) arraySizeRef = 0;
    add_symbol(SVAR);
    if(!parse_variable_name()) return ERROR;
    arraySizeRefTemp = arraySizeRef;
    if((variableTemp = search_identifier(varNameRef, procNameRef)) == NULL) variableTemp = search_identifier(varNameRef, NULL);
    // 式がついていないときの変数の型は変数名の型である
    if(token == TLSQPAREN) {
        add_symbol(SARRAYIDX);
        // 式がついているときの変数名の型はarray型
        if(typeNoRef != TPARRAY) return error("The type of the variable must be array");
        arrayElemTypeNoRefTemp = arrayElemTypeNoRef;
        print_token();
        if(!parse_expression()) return ERROR;
        // 式の型はinteger型でなくてはならない
        if(typeNoRef != TPINT) return error("Array index must be an integer");
        if(token != TRSQPAREN) return error("Symbol ']' is not found");
        print_token();
        remove_symbol_tail();
        // そのときの変数の型はarray 型の要素型である．
        typeNoRef = arrayElemTypeNoRefTemp;
        gen_code_3("CPA", "GR1", "GR0", NULL);
        gen_jump("JMI", "EROV", NULL);
        snprintf(numBuf, 6, "%d", arraySizeRefTemp-1);
        gen_code_3("LAD", "GR2", numBuf, NULL);
        gen_code_3("CPA", "GR1", "GR2", NULL);
        gen_jump("JPL", "EROV", NULL);
        if((find_symbol(SLEFTPART) || find_symbol(SCALLSTMT) || find_symbol(SINSTMT)) && !find_symbol(SARRAYIDX)) {
            gen_code_3_variable("LAD", "GR1", variableTemp->name, variableTemp->procName, 0, "GR1");
        } else {
            gen_code_3_variable("LD", "GR1", variableTemp->name, variableTemp->procName, 0, "GR1");
        }
    } else {
        if(variableTemp->isParam) {
            gen_code_3_variable("LD", "GR1", variableTemp->name, variableTemp->procName, 1, NULL);
            if(!(find_symbol(SLEFTPART) || find_symbol(SCALLSTMT) || find_symbol(SINSTMT))) gen_code_3("LD", "GR1", "0", "GR1");
        } else {
            if((find_symbol(SLEFTPART) || find_symbol(SCALLSTMT) || find_symbol(SINSTMT)) && !find_symbol(SARRAYIDX)) {
                gen_code_3_variable("LAD", "GR1", variableTemp->name, variableTemp->procName, 0, NULL);
                if(token == TPLUS || token == TMINUS || token == TSTAR || token == TDIV || token == TAND || token == TOR) {
                    gen_code_3("LD", "GR1", "0", "GR1");
                }
            } else {
                gen_code_3_variable("LD", "GR1", variableTemp->name, variableTemp->procName, 0, NULL);
            }
        }
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_expression() {
    int typeNoRefTemp;
    int operatorTokenTemp;
    int label1, label2;
    add_symbol(SEXP);
    if(!parse_simple_expression()) return ERROR;
    // 一つの単純式だけの式の型はその単純式の型である
    while(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ) {
        operatorTokenTemp = token;
        gen_code_3("PUSH", "0", "GR1", NULL);
        if(!(typeNoRef == TPINT || typeNoRef == TPBOOL || typeNoRef == TPCHAR)) return error("The expression type must be one of standard types");
        typeNoRefTemp = typeNoRef;
        if(!parse_relational_operator()) return ERROR;
        if(!parse_simple_expression()) return ERROR;
        // 関係演算子：関係演算子の被演算子の型は同じ標準型でなくてはならない
        if(typeNoRefTemp != typeNoRef) return error("Operands of a relational operator must have the same standard type");
        // 関係演算子：結果の型はboolean型である
        typeNoRef = TPBOOL;
        gen_code_1("POP", "GR2");
        gen_code_3("CPA", "GR2", "GR1", NULL);
        label1 = get_label_num();
        switch(operatorTokenTemp) {
            case TEQUAL:
                gen_jump_label("JZE", label1, NULL);
                break;
            case TNOTEQ:
                gen_jump_label("JNZ", label1, NULL);
                break;
            case TLE:
                gen_jump_label("JMI", label1, NULL);
                break;
            case TLEEQ:
                gen_jump_label("JMI", label1, NULL);
                gen_jump_label("JZE", label1, NULL);
                break;
            case TGR:
                gen_jump_label("JPL", label1, NULL);
                break;
            case TGREQ:
                gen_jump_label("JPL", label1, NULL);
                gen_jump_label("JZE", label1, NULL);
                break;
        }
        
        gen_code_3("LAD", "GR1", "0", NULL);
        label2 = get_label_num();
        gen_jump_label("JUMP", label2, NULL);
        gen_label(label1);
        gen_code_3("LAD", "GR1", "1", NULL);
        gen_label(label2);
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_simple_expression() {
    int operatorNoRefTemp;
    add_symbol(SSIMPLEEXP);
    if(token == TPLUS || token == TMINUS) {
        if(token == TMINUS && find_symbol(SVAR) && typeNoRef == TPARRAY) {
            return error("Array index must not be negative");
        }
        operatorNoRefTemp = token == TPLUS ? OPLUS : OMINUS;
        print_token();
    }
    if(!parse_term()) return ERROR;
    // "+"か"-"があるとき左の項の型はinteger型でなくてはならない
    // 加法演算子："+"と"-"の被演算子の型はinteger 型でなくてはならない．
    if((operatorNoRefTemp == OPLUS || operatorNoRefTemp == OMINUS) && typeNoRef != TPINT) return error("The type of the operand must be integer");
    if(operatorNoRefTemp == OMINUS) {
        gen_code_3("LAD", "GR2", "65535", NULL);
        gen_code_3("XOR", "GR1", "GR2", NULL);
        gen_code_3("LAD", "GR2", "1", NULL);
        gen_code_3("ADDA", "GR1", "GR2", NULL);
    }
    // 加法演算子：結果の型はinteger型である
    // 一つの項だけの単純式の型はその項の型である
    while(token == TPLUS || token == TMINUS || token == TOR) {
        gen_code_3("PUSH", "0", "GR1", NULL);
        if(!parse_additive_operator()) return ERROR;
        operatorNoRefTemp = operatorNoRef;
        if(!parse_term()) return ERROR;
        gen_code_1("POP", "GR2");
        if(operatorNoRefTemp == OPLUS || operatorNoRefTemp == OMINUS) {
            // 加法演算子："+"と"-"の被演算子の型はinteger 型でなくてはならない．
            if(typeNoRef != TPINT) return error("The type of the operand must be integer");
            if(operatorNoRefTemp == OPLUS) {
                gen_code_3("ADDA", "GR1", "GR2", NULL);
            } else {
                gen_code_3("SUBA", "GR2", "GR1", NULL);
            }
            gen_jump("JOV", "EOVF", NULL);
            if(operatorNoRefTemp == OMINUS) gen_code_3("LD", "GR1", "GR2", NULL);
            // 加法演算子：結果の型はinteger型である
        } else {
            // 加法演算子："or"の被演算子の型はboolean 型でなくてはならない．
            if(typeNoRef != TPBOOL) return error("The type of the operand must be boolean");
            gen_code_3("OR", "GR1", "GR2", NULL);
            // 加法演算子：結果の型はboolean型である
        }
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_term() {
    int operatorNoRefTemp;
    int label;
    add_symbol(STERM);
    if(!parse_factor()) return ERROR;
    // 一つの因子だけの項の型はその因子の型である
    while(token == TSTAR || token == TDIV || token == TAND) {
        gen_code_3("PUSH", "0", "GR1", NULL);
        if(!parse_multiplicative_operator()) return ERROR;
        operatorNoRefTemp = operatorNoRef;
        if(!parse_factor()) return ERROR;
        gen_code_1("POP", "GR2");
        if(operatorNoRefTemp == OSTAR || operatorNoRefTemp == ODIV) {
            // 乗法演算子："*"と"div"の被演算子の型はinteger 型でなくてはならない
            if(typeNoRef != TPINT) return error("The type of the right operand must be integer");
            if(operatorNoRefTemp == OSTAR) {
                gen_code_3("MULA", "GR1", "GR2", NULL);
                gen_jump("JOV", "EOVF", NULL);
            } else {
                gen_code_3("DIVA", "GR2", "GR1", NULL);
                // オーバーフロー: OF=1
                // ゼロ除算: OF=ZF=1
                // その他: OF=0 SF, ZFは結果による
                label = get_label_num();
                gen_jump_label("JZE", label, NULL);
                gen_jump("JOV", "EROV", NULL);
                gen_label(label);
                gen_jump("JOV", "E0DIV", NULL);
                gen_code_3("LD", "GR1", "GR2", NULL);
            }
            // 乗法演算子：結果の型はinteger型である
        } else {
            // 乗法演算子："and"の被演算子の型はboolean 型でなくてはならない
            if(typeNoRef != TPBOOL) return error("The type of the right operand must be boolean");
            gen_code_3("AND", "GR1", "GR2", NULL);
            // 乗法演算子：結果の型はboolean型である
        }
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_factor() {
    int typeNoRefTemp;
    int label1, label2;
    add_symbol(SFACTOR);
    isVariable = 0;
    if(token == TNAME) {
        if(!parse_variable()) return ERROR;
        if(find_symbol(SCALLSTMT)) isVariable = 1;
        // 変数や定数のとき，結果の因子の型はそれぞれの型である
    } else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING) {
        if(!parse_constant()) return ERROR;
        // 変数や定数のとき，結果の因子の型はそれぞれの型である
    } else if(token == TLPAREN) {
        print_token();
        if(!parse_expression()) return ERROR;
        // "(" 式 ")"のとき，結果の因子の型は式の型である
        if(token != TRPAREN) return error("Symbol ')' is not found");
        print_token();
    } else if(token == TNOT) {
        print_token();
        if(!parse_factor()) return ERROR;
        // "not"の被演算子の型はboolean型でなくてはならない
        if(typeNoRef != TPBOOL) return error("The type of the operand must be boolean");
        // 結果の因子の型はboolean 型である
        gen_code_3("LAD", "GR2", "1", NULL);
        gen_code_3("XOR", "GR1", "GR2", NULL);
    } else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        if(!parse_standard_type()) return ERROR;
        typeNoRefTemp = typeNoRef;
        if(token != TLPAREN) return error("Symbol '(' is not found");
        print_token();
        if(!parse_expression()) return ERROR;
        // 標準型 "(" 式 ")"の式の型は標準型でなくてはならない
        if(!(typeNoRef == TPINT || typeNoRef == TPBOOL || typeNoRef == TPCHAR)) return error("The type of the operand must be one of standard types");
        if(token != TRPAREN) return error("Symbol ')' is not found");
        print_token();
        // 標準型 "(" 式 ")"のとき，結果の因子の型はその標準型である．
        if(typeNoRef == TPINT) {
            if(typeNoRefTemp == TPBOOL) {
                gen_code_3("CPA", "GR1", "GR0", NULL);
                label1 = get_label_num();
                gen_jump_label("JZE", label1, NULL);
                gen_code_3("LAD", "GR1", "1", NULL);
                label2 = get_label_num();
                gen_jump_label("JUMP", label2, NULL);
                gen_label(label1);
                gen_code_3("LAD", "GR1", "0", NULL);
                gen_label(label2);
            } else if(typeNoRefTemp == TPCHAR) {
                gen_code_3("LAD", "GR2", "255", NULL);
                gen_code_3("AND", "GR1", "GR2", NULL);
            }
        } else if(typeNoRef == TPBOOL) {
            if(typeNoRefTemp == TPCHAR) {
                gen_code_3("CPA", "GR1", "GR0", NULL);
                label1 = get_label_num();
                gen_jump_label("JZE", label1, NULL);
                gen_code_3("LAD", "GR1", "1", NULL);
                label2 = get_label_num();
                gen_jump_label("JUMP", label2, NULL);
                gen_label(label1);
                gen_code_3("LAD", "GR1", "0", NULL);
                gen_label(label2);
            }
        } else if(typeNoRef == TPCHAR) {
            if(typeNoRefTemp == TPBOOL) {
                gen_code_3("CPA", "GR1", "GR0", NULL);
                label1 = get_label_num();
                gen_jump_label("JZE", label1, NULL);
                gen_code_3("LAD", "GR1", "1", NULL);
                label2 = get_label_num();
                gen_jump_label("JUMP", label2, NULL);
                gen_label(label1);
                gen_code_3("LAD", "GR1", "0", NULL);
                gen_label(label2);
            }
        }
        typeNoRef = typeNoRefTemp;
    } else {
        return error("Factor is not found");
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_constant() {
    char numBuf[6];
    add_symbol(SCONST);
    switch(token) {
        case TNUMBER:
            // "符号なし整数"のとき、定数の型はinteger型
            snprintf(numBuf, 6, "%d", num_attr);
            gen_code_3("LAD", "GR1", numBuf, NULL);
            typeNoRef = TPINT;
            break;
        case TFALSE:
            // "false"と"true"のときはboolean 型である
            gen_code_3("LAD", "GR1", "0", NULL);
            typeNoRef = TPBOOL;
            break;
        case TTRUE:
            // "false"と"true"のときはboolean 型である
            gen_code_3("LAD", "GR1", "1", NULL);
            typeNoRef = TPBOOL;
            break;
        case TSTRING:
            // 文字列は長さが 1でなくてはならず
            if(slen != 1) return error("The length of string must be 1");
            // そのときの定数の型はchar 型である
            gen_code_3_string("LD", "GR1", string_attr, NULL);
            typeNoRef = TPCHAR;
            break;
    }
    print_token();

    remove_symbol_tail();
    return NORMAL;
}
int parse_multiplicative_operator() {
    add_symbol(SMLTOPR);
    switch(token) {
        case TSTAR:
        case TDIV:
            // "*"と"div"の被演算子の型はinteger 型でなくてはならない
            if(typeNoRef != TPINT) return error("The type of the operand must be integer");
            operatorNoRef = token == TSTAR ? OSTAR : ODIV;
            break;
        case TAND:
            // "and"の被演算子の型はboolean 型でなくてはならない
            if(typeNoRef != TPBOOL) return error("The type of the operand must be boolean");
            operatorNoRef = OAND;
            break;
    }
    print_token();
    
    remove_symbol_tail();
    return NORMAL;
}
int parse_additive_operator() {
    add_symbol(SADDOPR);
    switch(token) {
        case TPLUS:
        case TMINUS:
            // "+"と"-"の被演算子の型はinteger 型でなくてはならない
            if(typeNoRef != TPINT) return error("The type of the operand must be integer");
            operatorNoRef = token == TPLUS ? OPLUS : OMINUS;
            break;
        case TOR:
            // "or"の被演算子の型はboolean 型でなくてはならない
            if(typeNoRef != TPBOOL) return error("The type of the operand must be boolean");
            operatorNoRef = OOR;
            break;
    }
    print_token();

    remove_symbol_tail();
    return NORMAL;
}
int parse_relational_operator() {
    add_symbol(SRELOPR);
    if(!(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ)) return error("Operator is not found");
    print_token();

    remove_symbol_tail();
    return NORMAL;
}
int parse_input_statement() {
    int isLn = 0;
    add_symbol(SINSTMT);
    if(!(token == TREAD || token == TREADLN)) return error("Keyword 'read' or 'readln' is not found");
    isLn = token == TREADLN ? 1 : 0;
    print_token();
    if(token == TLPAREN) {
        do {
            print_token();
            if(!parse_variable()) return ERROR;
            if(typeNoRef == TPINT) {
                gen_jump("CALL", "READINT", NULL);
            } else if(typeNoRef == TPCHAR) {
                gen_jump("CALL", "READCHAR", NULL);
            } else {
                return error("Argument types must be integer or character");
            }
        } while(token == TCOMMA);
        if(token != TRPAREN) return error("Symbol ')' is not found");
        print_token();
        if(isLn) gen_jump("CALL", "READLINE", NULL);
    }

    remove_symbol_tail();
    return NORMAL;
}
int parse_output_statement() {
    int isLn = 0;
    add_symbol(SOUTSTMT);
    if(!(token == TWRITE || token == TWRITELN)) return error("Keyword 'write' or 'writeln' is not found");
    isLn = token == TWRITELN ? 1 : 0;
    print_token();
    if(token == TLPAREN) {
        print_token();
        if(!parse_output_format()) return ERROR;
        while(token == TCOMMA) {
            print_token();
            if(!parse_output_format()) return ERROR;
        }
        if(token != TRPAREN) return error("Symbol ')' is not found");
        print_token();
    }
    if(isLn) gen_jump("CALL", "WRITELINE", NULL);

    remove_symbol_tail();
    return NORMAL;
}
int parse_output_format() {
    char numBuf[6];
    add_symbol(SOUTFORMAT);
    if(token == TPLUS || token == TMINUS || token == TNAME || token == TNUMBER || token == TFALSE || token == TTRUE || (token == TSTRING && slen == 1) || token == TLPAREN || token == TNOT || token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        if(!parse_expression()) return ERROR;
        // 式の型は標準型でなくてはならない
        if(!(typeNoRef == TPINT || typeNoRef == TPBOOL || typeNoRef == TPCHAR)) {
            return error("The expression type must be one of standard types");
        }
        if(token == TCOLON) {
            print_token();
            if(token != TNUMBER) return error("Number is not found");
            snprintf(numBuf, 6, "%d", num_attr);
            gen_code_3("LAD", "GR2", numBuf, NULL);
            print_token();
        } else {
            gen_code_3("LAD", "GR2", "0", NULL);
        }
        switch(typeNoRef) {
            case TPINT:
                gen_jump("CALL", "WRITEINT", NULL);
                break;
            case TPBOOL:
                gen_jump("CALL", "WRITEBOOL", NULL);
                break;
            case TPCHAR:
                gen_jump("CALL", "WRITECHAR", NULL);
                break;
        }
    } else if(token != TSTRING) {
        return error("String is not found");
    } else {
        gen_code_3_string("LAD", "GR1", string_attr, NULL);
        gen_code_3("LAD", "GR2", "0", NULL);
        gen_jump("CALL", "WRITESTR", NULL);
        print_token();
    }

    remove_symbol_tail();
    return NORMAL;
}