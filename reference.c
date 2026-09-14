#include "reference.h"

struct SYMBOL *symbolRoot = NULL;
struct TYPE *paramTypeRef = NULL;
struct ID *globalIdRoot = NULL, *localIdRoot = NULL;
struct IDROOT *idRootRoot = NULL;

int typeNoRef = -1;
int operatorNoRef = -1;
int arraySizeRef = -1;
int arrayElemTypeNoRef = -1;
char varNameRef[MAXSTRLEN+1];
char procNameRef[MAXSTRLEN+1];

char *type_str[] = {
    "",
    "integer",
    "char",
    "boolean",
    "array",
    "procedure",
};

int get_digit(int n){
	int digit = 0;
	while(n != 0){
		n /= 10;
		digit++;
	}
	return digit;
}

void add_symbol(int sn) {
    struct SYMBOL *p, *tail;

    if((p = (struct SYMBOL *)malloc(sizeof(struct SYMBOL))) == NULL) {
        error("Cannot allocate memory");
        return;
    }

    p->symbolNo = sn;
    p->nextp = NULL;

    if(symbolRoot == NULL) {
        symbolRoot = p;
    } else {
        tail = symbolRoot;
        for(; tail->nextp!=NULL; tail=tail->nextp);
        tail->nextp = p;
    }
}

void remove_symbol_tail() {
    struct SYMBOL *p = symbolRoot;
    struct SYMBOL *rmp;

    for(p; p->nextp!=NULL&&p->nextp->nextp!=NULL; p=p->nextp);
    rmp = p->nextp;
    p->nextp = NULL;
    free(rmp);
}

int find_symbol(int sn) {
    struct SYMBOL *p = symbolRoot;

    for(; p!=NULL; p=p->nextp) {
        if(p->symbolNo == sn) {
            
            return 1;
        }
    }
    
    return 0;
}

int register_identifier_name() {
    struct ID *root, *tail;
    struct ID *idp, *p;
    struct TYPE *typep;
    char *np;

    if((idp = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
        return error("Cannot allocate memory");
    }
    if((np = (char *)malloc(strlen(string_attr) + 1)) == NULL) {
        return error("Cannot allocate memory");
    }

    memset(np, '\0', sizeof(np));
    strcpy(np, string_attr);
    idp->name = np;
    if(find_symbol(SSUBPROGDECL) && !find_symbol(SPROCNAME)) {
        p = globalIdRoot;
        while(p != NULL && p->nextp != NULL) {
            p = p->nextp;
        }
        idp->procName = p->name;
    } else {
        idp->procName = NULL;
    }
    // 追加エラー 1. 名前の二重定義
    // 同じスコープを持つ同じ名前が複数回宣言されていてはならない
    if(search_identifier(idp->name, idp->procName) != NULL) {
        if(find_symbol(SFORMALPARAMS)) {
            return error("Multiple definition of parameter");
        } else {
            return error("Multiple definition of variable");
        }
        
    }
    if(find_symbol(SPROCNAME) && !find_symbol(SCALLSTMT)) {
        if((typep = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
            return error("Cannot allocate memory");
        }
        strcpy(procNameRef, idp->name);
        typep->typeNo = TPPROC;
        idp->isDefined = 1;
    } else {
        typep = NULL;
        idp->isDefined = 0;
    }
    idp->type = typep;
    idp->isParam = find_symbol(SFORMALPARAMS) ? 1 : 0;
    idp->defLineNo = get_linenum();
    idp->refLinep = NULL;
    idp->nextp = NULL;

    root = find_symbol(SSUBPROGDECL) && !find_symbol(SPROCNAME) ? localIdRoot : globalIdRoot;
    if(root == globalIdRoot && globalIdRoot == NULL) {
        globalIdRoot = idp;
    } else if(root == localIdRoot && localIdRoot == NULL) {
        localIdRoot = idp;
    } else {
        for(tail = root; tail->nextp!=NULL; tail=tail->nextp);
        tail->nextp = idp;
    }

    return NORMAL;
}

int register_identifier_type() {
    // 仮引数部：仮引数部に現れる型は標準型でなくてはならない
    if(find_symbol(SFORMALPARAMS) && typeNoRef == TPARRAY) {
        return error("Formal parameter types must be standard types");
    }

    struct TYPE *typep, *tail;
    struct ID *idp = find_symbol(SSUBPROGDECL) ? localIdRoot : globalIdRoot;

    // 配列の場合...大域変数で詳細を登録
    // 副プログラムの場合...仮引数登録後に型登録。ローカル変数用の記号表から型情報を取得
    // 型が空の変数に型を割り当てる実装にすれば変数が複数でも対応できる？とりあえずこの設計で行く
    do {
        typep = NULL;
        if((typep = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
            return error("Cannot allocate memory");
        }
        typep->typeNo = typeNoRef;
        for(; idp->type!=NULL&&idp->nextp!=NULL; idp=idp->nextp);
        idp->type = typep;
        if(typeNoRef == TPARRAY) {  // 配列の場合
            typep->arraySize = arraySizeRef;
            typep->arrayElemTypeNo = arrayElemTypeNoRef;
        } else {
            typep->arraySize = -1;
            typep->arrayElemTypeNo = -1;
        }
    } while(idp->nextp != NULL);
    if(find_symbol(SFORMALPARAMS)) {  // 副プログラムの場合
        for(idp=globalIdRoot; idp->nextp!=NULL; idp=idp->nextp);
        tail = idp->type;
        for(struct ID *p=localIdRoot; p!=NULL; p=p->nextp) {
            typep = NULL;
            if((typep = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
                return error("Cannot allocate memory");
            }
            if(tail->paramTypep == NULL) {
                typep->typeNo = typeNoRef;
                typep->paramTypep = NULL;
                tail->paramTypep = typep;
            }
            tail = tail->paramTypep;
        }
    }

    return NORMAL;
}

struct ID *search_identifier(char *np, char *procnp) {
    struct ID *p = procnp == NULL ? globalIdRoot : localIdRoot;
    struct LINE *lp;

    if(p == NULL) return NULL;
    for(; p!=NULL; p=p->nextp) {
        if(strcmp(np, p->name) == 0) {
            if(p->type != NULL && p->type->typeNo == TPARRAY) arrayElemTypeNoRef = p->type->arrayElemTypeNo;
            
            return p;
        }
    }
    
    return NULL;
}

int add_refline() {
    struct ID *idp;
    char *procnp = NULL;
    struct LINE *lp, *tail;
    
    // 追加エラー 2. 名前の未定義
    // ブロックの複合文中で使われる変数名はブロックの変数宣言部で宣言されていなければならない
    // ブロックの複合文中で使われる手続き名は副プログラム宣言で宣言されていなければならない
    // すべての名前は，プログラムテキスト中で使用される前に宣言されていなければならない
    if(find_symbol(SSUBPROGDECL)) procnp = procNameRef;
    //if(find_symbol(SCALLSTMT)) strcpy(procNameRef, string_attr);
    
    idp = search_identifier(string_attr, procnp);
    if(procnp != NULL && idp == NULL) {
        // 局所変数として見つからなかった場合、再帰呼び出しされていないかチェック
        if((idp = search_identifier(string_attr, NULL)) != NULL && strcmp(string_attr, procnp) == 0) {
            // 副プログラム宣言：この手続き名をこの複合文内で使用することはできない．再帰呼び出しはできない
            return error("Recursive calls are not allowed");
        }  
        // 最後に大域変数にないかチェック
        if((idp = search_identifier(string_attr, NULL)) == NULL) {
            return error("Specified identifier is not declared");
        }
    } else if(procnp == NULL && idp == NULL) {
        return error("Specified identifier is not declared");
    }
    strcpy(varNameRef, string_attr);
    if(idp != NULL && idp->type->typeNo == TPARRAY) {
        arraySizeRef = idp->type->arraySize;
        arrayElemTypeNoRef = idp->type->arrayElemTypeNo;
    }
    if(find_symbol(SPROCNAME)) paramTypeRef = search_identifier(string_attr, NULL) != NULL ? search_identifier(string_attr, NULL)->type->paramTypep : NULL;
    if((lp = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) {
        return error("Cannot allocate memory");
    }
    lp->lineNo = get_linenum();
    lp->nextp = NULL;
    if(idp->refLinep == NULL) {
        idp->refLinep = lp;
    } else {
        tail = idp->refLinep;
        for(; tail->nextp!=NULL; tail=tail->nextp);
        tail->nextp = lp;
    }

    return idp->type->typeNo;
}

int iterate_undefined_variable(char **np, char **procnp, struct TYPE **tp) {
    struct ID *root = find_symbol(SSUBPROGDECL) && !find_symbol(SPROCNAME) ? localIdRoot : globalIdRoot;
    static struct ID *p;
    static int isInitialized = 0;
    int val = 0;
    
    if(!isInitialized) {
        p = root;
        isInitialized = 1;
    }
    for(; p!=NULL&&p->isDefined; p=p->nextp);
    if(p != NULL) {
        *np = p->name;
        *tp = p->type;
        *procnp = p->procName;
        p->isDefined = 1;
        val = 1;
    } else {
        val = 0;
        isInitialized = 0;
    }

    return val;
}

int iterate_parameter_rev(char **np, char **procnp) {
    struct ID *root = localIdRoot;
    struct ID *p = root;
    static int count = 0;
    static int isInitialized = 0;

    if(root == NULL) return 0;
    if(!isInitialized) {
        for(; p->nextp!=NULL&&p->nextp->isParam; p=p->nextp) {
            count++;
        }
        isInitialized = 1;
    } else {
        for(int i=0; i<count; i++) {
            p = p->nextp;
        }
    }
    if(count >= 0) {
        *np = p->name;
        *procnp = p->procName;
        count--;
        return 1;
    } else {
        count = 0;
        isInitialized = 0;
        return 0;
    }
}

int switch_root() {
    if(localIdRoot == NULL) return NORMAL;

    struct IDROOT *p;
    if ((p = malloc(sizeof *p)) == NULL) {
        return error("Cannot allocate memory");
    }

    p->root = localIdRoot;
    p->nextp = idRootRoot;
    idRootRoot = p;

    localIdRoot = NULL;
    memset(procNameRef, '\0', sizeof(procNameRef));
    return NORMAL;
}

void print_reference_table() {
    struct IDROOT *rrp = idRootRoot;
    struct ID *rp = globalIdRoot, *lrp, *idp;
    char *np = NULL;
    int maxNameLen = 4;
    int maxTypeNameLen = 4;
    int lenTemp = 0;

    if(rp == NULL) {
        printf("Name | Type | Define | References\n");
        return;  // 出力させるものがない
    }

    while(rp->nextp != NULL) {
        rp = rp->nextp;
    }
    
    // これまで記録したローカル変数用の記号表をglobalIdRootの末尾に連結
    while(rrp != NULL) {
        rp->nextp = rrp->root;
        while(rp->nextp != NULL) {
            rp = rp->nextp;
            if((np = (char *)malloc(strlen(rp->name) + strlen(":") + strlen(rp->procName) + 1)) == NULL) {
                error("Cannot allocate memory");
                return;
            }
            memset(np, 0, sizeof(np));
            strcat(np, rp->name);
            strcat(np, ":");
            strcat(np, rp->procName);
            
            rp->name = np;
            np = NULL;
        }
        rrp = rrp->nextp;
    }

    globalIdRoot = sort_name_list_asc(globalIdRoot);

    for(struct ID *p=globalIdRoot; p!=NULL; p=p->nextp) {
        maxNameLen = maxNameLen < strlen(p->name) ? strlen(p->name) : maxNameLen;
    }
    printf("Name");
    for(int i=0; i<maxNameLen-strlen("Name"); i++) printf(" ");
    printf(" | ");

    for(struct ID *p=globalIdRoot; p!=NULL; p=p->nextp) {
        lenTemp = 0;
        if(p->type->typeNo == TPPROC) {
            lenTemp = 9;
            for(struct TYPE *tp=p->type->paramTypep; tp!=NULL; tp=tp->paramTypep) {
                lenTemp += strlen(type_str[tp->typeNo]);
                lenTemp++;
            }
            lenTemp++;
        } else if(p->type->typeNo == TPARRAY) {
            lenTemp = 11 + get_digit(p->type->arraySize);
            lenTemp += strlen(type_str[p->type->arrayElemTypeNo]);
        } else if(p->type->typeNo == TPINT || p->type->typeNo == TPBOOL) {
            lenTemp = 7;
        } else {
            lenTemp = 4;
        }

        maxTypeNameLen = lenTemp > maxTypeNameLen ? lenTemp : maxTypeNameLen;
    }
    printf("Type");
    for(int i=0; i<maxTypeNameLen-strlen("Type"); i++) printf(" ");
    printf(" | ");

    printf("Define");
    printf(" | ");

    printf("References");

    printf("\n");

    for(rp=globalIdRoot; rp!=NULL; rp=rp->nextp) {
        // Name
        printf("%s", rp->name);
        lenTemp = strlen(rp->name);
        for(int i=0; i<maxNameLen-lenTemp; i++) printf(" ");
        printf(" | ");
        // Type
        switch(rp->type->typeNo) {
            case TPARRAY:
                printf("array[%d] of %s", rp->type->arraySize, type_str[rp->type->arrayElemTypeNo]);
                lenTemp = 11 + strlen(type_str[rp->type->arrayElemTypeNo]) + get_digit(rp->type->arraySize);
                break;
            case TPPROC:
                printf("procedure");
                lenTemp = 9;
                if((idp = search_identifier(rp->name, NULL)) != NULL && idp->type->paramTypep != NULL) {
                    printf("(");
                    for(struct TYPE *p=idp->type->paramTypep; p!=NULL; p=p->paramTypep) {
                        printf("%s", type_str[p->typeNo]);
                        if(p->paramTypep != NULL) printf(",");
                        lenTemp += strlen(type_str[p->typeNo]);
                        lenTemp++; 
                    }
                    lenTemp++;
                    printf(")");
                }
                break;
            default:
                printf("%s", type_str[rp->type->typeNo]);
                lenTemp = strlen(type_str[rp->type->typeNo]);
                break;
        }
        for(int i=0; i<maxTypeNameLen-lenTemp; i++) printf(" ");
        printf(" | ");
        // Define
        printf("%d", rp->defLineNo);
        for(int i=0; i<6-(get_digit(rp->defLineNo)); i++) printf(" ");
        printf(" | ");
        // References
        for(struct LINE *p=rp->refLinep; p!=NULL; p=p->nextp) {
            printf("%d", p->lineNo);
            if(p->nextp != NULL) printf(",");
        }
        printf("\n");
    }
}

struct ID *sort_name_list_asc(struct ID *irp) {
    struct ID *sorted = NULL;

    while (irp) {
        struct ID *cur = irp;
        irp = irp->nextp;

        if (!sorted || strcmp(cur->name, sorted->name) < 0) {
            cur->nextp = sorted;
            sorted = cur;
        } else {
            struct ID *p = sorted;
            while (p->nextp && strcmp(p->nextp->name, cur->name) <= 0) {
                p = p->nextp;
            }
            cur->nextp = p->nextp;
            p->nextp = cur;
        }
    }

    return sorted;
}