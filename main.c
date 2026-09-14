/**
 * @file main.c
 * @brief 字句解析器メインファイル
 * @author omzn (@omzn.aquatan.net)
 */
#include "scan.h"
#include "parse.h"
#include "reference.h"
#include "code.h"
#include "outlib.h"
#include "id-list.h"

//! キーワードリスト配列
struct KEY key[KEYWORDSIZE] = {
  {"and", TAND},         {"array", TARRAY},     {"begin", TBEGIN},
  {"boolean", TBOOLEAN}, {"break", TBREAK},     {"call", TCALL},
  {"char", TCHAR},       {"div", TDIV},         {"do", TDO},
  {"else", TELSE},       {"end", TEND},         {"false", TFALSE},
  {"if", TIF},           {"integer", TINTEGER}, {"not", TNOT},
  {"of", TOF},           {"or", TOR},           {"procedure", TPROCEDURE},
  {"program", TPROGRAM}, {"read", TREAD},       {"readln", TREADLN},
  {"return", TRETURN},   {"then", TTHEN},       {"true", TTRUE},
  {"var", TVAR},         {"while", TWHILE},     {"write", TWRITE},
  {"writeln", TWRITELN}};

//! トークンカウンター
int numtoken[NUMOFTOKEN + 1];

//! 各トークンに対応する文字列
char *tokenstr[NUMOFTOKEN + 1] = {
  "",        "NAME",    "program", "var",     "array",     "of",     
  "begin",   "end",     "if",      "then",    "else",      "procedure",
  "return",  "call",    "while",   "do",      "not",       "or",
  "div",     "and",     "char",    "integer", "boolean",   "readln",
  "writeln", "true",    "false",   "NUMBER",  "STRING",    "+",
  "-",       "*",       "=",       "<>",      "<",         "<=",
  ">",       ">=",      "(",       ")",       "[",         "]",
  ":=",      ".",       ",",       ":",       ";",         "read",   
  "write",   "break"};

/**
 * @brief メイン関数
 * 
 * @param argc コマンドライン引数の数
 * @param argv 引数が格納された配列
 * @return int 正常:0 異常:非0
 */
int main(int argc, char *argv[]) {
  //! トークン, i
  int token, i;

  if (argc < 2) {
    error("File name is not given.");
    return 0;
  }
  if (init_scan(argv[1]) == 0) {
    error("Cannot open input file.");
    return 0;
  }

  if(!open_file(argv[1])) return error("Failed to write the output file");
  if(!parse_program(token = scan())) return -1;
  //print_reference_table();

  close_file();
  end_scan();
  
  /*
  memset(numtoken, 0, NUMOFTOKEN+1);
  init_idtab();

  while ((token = scan()) >= 0) {
    numtoken[token]++;
  }
  end_scan();

  for(int i=0; i<=NUMOFTOKEN; i++) {
    if(numtoken[i] > 0) printf("\"%s\"\t%d\n", tokenstr[i], numtoken[i]);
    if(i == TNAME) {
      print_idtab();
    }
  }
  */

  return 0;
}

/**
 * @brief エラーメッセージを表示する関数
 * 
 * @param mes メッセージ
 * @return int ERROR (固定)
 */
int error(char *mes) { 
	fprintf(stderr, "Line: %4d ERROR: %s\n", get_linenum(), mes); 
  //fflush(stdout);
  return ERROR;
}
