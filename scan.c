/**
* @file scan.c
* @brief MPPL構文の解析を行う
* @author Kyogo Yamashita
*/

#include "scan.h"
#include "id-list.h"

//! 字句解析された数値
int num_attr;
//! 字句解析された文字列
char string_attr[MAXSTRSIZE+1];

//! 先読みバッファ
int cbuf;
//! 読み込んだ行数
int linenum = 1;
//! 読み込んだ行数（実際に返す値）
int linenum_return = 0;
//! 読み込んだ文字列の長さ
int slen = 0;
//! 記号および対応するトークン番号のリスト配列
int symbols[14][2] = {{'+', TPLUS}, {'-', TMINUS}, {'*', TSTAR}, {'=', TEQUAL}, {'<', TLE}, {'>', TGR}, 
    {'(', TLPAREN}, {')', TRPAREN}, {'[', TLSQPAREN}, {']', TRSQPAREN}, {':', TCOLON}, {'.', TDOT}, {',', TCOMMA}, {';', TSEMI}};
//! 記号の種類。具体的には記号に対応するトークン番号
int symboltype;
int isScanningComment = 0;
//! ファイルポインタ
FILE *f = NULL;

/**
* @brief 解析対象ファイルの読み込み
* @param[in] char* filename 入力ファイル名 
* @return 正常=0, 異常=-1
* @details ファイルを開き、最初の一文字をcbufに先読みしておく
*/
int init_scan(char *filename) {
    if((f = fopen(filename, "r")) == NULL) {
        return ERROR;
    }
    read_next();

    return NORMAL;
}

/**
* @brief 字句解析を行う
* @return 読み込んだ字句に対応するトークン番号。読み込み中にファイルの末尾に到達した場合は-1を返す
* @details cbufに格納されている文字の種類によって処理を分岐させる
*/
int scan(void) {
    memset(string_attr, '\0', MAXSTRSIZE+1);
    /*! カウンタ */
    int i = 0;
    /*! cbufの一時保存用変数 */
    int ctemp = cbuf;
    slen = 0;

    while(cbuf != EOF) {
        linenum_return = linenum;
        switch(get_type(cbuf)) {
            case 1:
                do {
                    if(slen++ >= MAXSTRLEN) {
                        error("The string in the input is too long.");
                        exit(-1);
                    }
                    string_attr[i++] = cbuf;
                    read_next();
                } while(get_type(cbuf) == 1 || get_type(cbuf) == 2);
                for(int j=0; j<KEYWORDSIZE; j++) {
                    if(strcmp(string_attr, key[j].keyword) == 0) {
                        return key[j].keytoken;
                    }
                }
                id_countup(string_attr);
                return TNAME;
            case 2:
                do {
                    if(slen++ >= MAXSTRLEN) {
                        error("The string in the input is too long.");
                        exit(-1);
                    }
                    string_attr[i++] = cbuf;
                    read_next();
                } while(get_type(cbuf) == 2);
                if((num_attr = (int)strtol(string_attr, NULL, 10)) > MAXNUM) {
                    error("The number in the input exceeds the maximum value.");
                    exit(-1);
                } 
                return TNUMBER;
            case 3:
                read_next();
                while(1) {
                    if(cbuf == '\n' || cbuf == '\r' || cbuf == EOF) {
                        error("invalid Syntax");
                        exit(-1);
                    }
                    if(cbuf == '\'') {
                        read_next();
                        if(cbuf == '\'') {
                            string_attr[i++] = cbuf;
                        } else {
                            break;
                        }
                    }
                    if(slen++ >= MAXSTRLEN) {
                        error("The string in the input is too long.");
                        exit(-1);
                    }
                    string_attr[i++] = cbuf;
                    read_next();
                }
                return TSTRING;
            case 4:
                do {
                    read_next();
                } while(cbuf != '}' && cbuf != EOF);
                break;
            case 5:
                read_next();
                if(cbuf == '*') {
                    isScanningComment = 1;
                    read_next();
                    while(cbuf != EOF) {
                        if(cbuf == '*') {
                            read_next();
                            if(cbuf == '/') {
                                isScanningComment = 0;
                                break;
                            } else {
                                continue;
                            }
                        }
                        read_next();
                    }
                } else {
                    error("invalid Syntax");
                    exit(-1);
                }
                break;
            case 6:
                ctemp = cbuf;
                read_next();
                switch(ctemp) {
                    case '<':
                        if(cbuf == '>') {
                            read_next();
                            return TNOTEQ;
                        } else if(cbuf == '=') {
                            read_next();
                            return TLEEQ;
                        }
                        break;
                    case '>':
                    case ':':
                        if(cbuf == '=') {
                            read_next();
                            return ctemp == '>' ? TGREQ : TASSIGN;
                        }
                        break;
                }
                return symboltype;
            case 7:
                error("invalid Syntax");
                exit(-1);
            default:
                break;
        }

        read_next();
    }

    return -1;
}

/**
* @brief これまでに読み込んだ行数を返す
* @return 読み込んだ行数。まだ一度もscan()が実行されていない場合は0を返す
* @details 直近のscan()で返されたトークンが存在した行の番号を返す
*/
int get_linenum(void) {
    return linenum_return;
}   

/**
* @brief 解析対象ファイルのクローズ
* @details init_scan(filename)でオープンしたファイルをクローズする
*/
void end_scan(void) {
    if(f != NULL) fclose(f);

    return;
}

/**
* @brief 先読みを行う
* @details ファイルから1文字読み、cbufに格納する。改行コードが読み込まれた場合は行数を増やす。読み込んだ文字が表示文字（に加えて改行コード、タブ、EOF）でない場合はエラーを返す
*/
void read_next(void) {
    cbuf = fgetc(f);
    if(!((cbuf >= 0x20 && cbuf <= 0x7E) || cbuf == '\n' || cbuf == '\r' || cbuf == '\t' || cbuf == EOF) && !isScanningComment) {
        error("invalid Syntax");
        end_scan();
        exit(-1);
    }
    if(cbuf == '\r') {
        if((cbuf = fgetc(f)) != '\n') {
            ungetc(cbuf, f);
            cbuf = '\r';
        }
        linenum++;
    } else if(cbuf == '\n') {
        if((cbuf = fgetc(f)) != '\r') {
            ungetc(cbuf, f);
            cbuf = '\n';
        }
        linenum++;
    }

    return;
}

/// @brief 文字の種類の判別
/// @param[in] int c 文字コード 
/// @return 分離子=0, 英字=1, 数字=2, 文字列=3, 注釈({...})=4, 注釈(/*...*/)=5, 記号=6, 無効な文字=7
/// @details 読み込んだ文字コードの値によって処理を分岐させる
int get_type(int c) {
    if(c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        return 0;
    } else if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        return 1;
    } else if(c >= '0' && c <= '9') {
        return 2;
    } else if(c == '\'') {
        return 3;
    } else if(c == '{') {
        return 4;
    } else if(c == '/') {
        return 5;
    } else {
        for(int i=0; i<14; i++) {
            if(c == symbols[i][0]) {
                symboltype = symbols[i][1];
                return 6;
            }
        }

        return 7;
    }
}