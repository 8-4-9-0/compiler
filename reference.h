#ifndef REF_H
#define REF_H

#include <stdlib.h>
#include "scan.h"

#define TPINT 1
#define TPCHAR 2
#define TPBOOL 3
#define TPARRAY 4
#define TPPROC 5

#define SPROGRAM 0
#define SBLOCK 1
#define SVARDECL 2
#define SVARNAMES 3
#define SVARNAME 4
#define STYPE 5
#define SSTDTYPE 6
#define SARRTYPE 7
#define SSUBPROGDECL 8
#define SPROCNAME 9
#define SFORMALPARAMS 10
#define SCOMPSTMT 11
#define SSTMT 12
#define SCONDSTMT 13
#define SITERSTMT 14
#define SEXITSTMT 15
#define SCALLSTMT 16
#define SEXPS 17
#define SRETSTMT 18
#define SASGSTMT 19
#define SLEFTPART 20
#define SVAR 21
#define SEXP 22
#define SSIMPLEEXP 23
#define STERM 24
#define SFACTOR 25
#define SCONST 26
#define SMLTOPR 27
#define SADDOPR 28
#define SRELOPR 29
#define SINSTMT 30
#define SOUTSTMT 31
#define SOUTFORMAT 32
#define SEMPSTMT 33
#define SARRAYIDX 34

#define OPLUS 0
#define OMINUS 1
#define OOR 2
#define OSTAR 3
#define ODIV 4
#define OAND 5

struct SYMBOL {
    int symbolNo;
    struct SYMBOL *nextp;
}; 
extern struct SYMBOL *symbolRoot;

struct ID {
	char *name;
	char *procName;
	struct TYPE *type;
	int isParam;
	int isDefined;
	int defLineNo;
	struct LINE *refLinep;
    struct ID *nextp;
}; 
extern struct ID *globalIdRoot, *localIdRoot, *localIdRootRef;

struct TYPE {
	int typeNo;
	int arraySize;
	int arrayElemTypeNo;
	struct TYPE *paramTypep;
}; 

struct LINE {
	int lineNo;
	struct LINE *nextp;
};

struct IDROOT {
    struct ID *root;
    struct IDROOT *nextp;
}; 
extern struct IDROOT *idRootRoot;

extern int typeNoRef;
extern int operatorNoRef;
extern int arraySizeRef;
extern int arrayElemTypeNoRef;
extern char varNameRef[MAXSTRLEN+1];
extern char procNameRef[MAXSTRLEN+1];
extern struct TYPE *paramTypeRef;

void add_symbol(int sn);
void remove_symbol_tail();
int find_symbol(int sn);
int register_identifier_name();
int register_identifier_type();
struct ID *search_identifier(char *np, char *procnp);
int add_refline();
int iterate_undefined_variable(char **np, char **procnp, struct TYPE **tp);
int iterate_parameter_rev(char **np, char **procnp);
int switch_root();
extern void print_reference_table();
struct ID *sort_name_list_asc(struct ID *irp);

#endif