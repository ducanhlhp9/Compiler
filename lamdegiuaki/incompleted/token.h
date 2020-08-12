/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __TOKEN_H__
#define __TOKEN_H__

#define MAX_IDENT_LEN 15
// TODO:GK1
#define KEYWORDS_COUNT 22     		// GK1: Doi lai keyword_count vi them 2 keyword moi
#define DEBUG 0  					// GK1: Bổ sung để debug  1: debug bai 1 // 2: debug bai 2 // 0: ko debug

// TODO:GK1
typedef enum {
  TK_NONE, TK_IDENT, TK_NUMBER, TK_CHAR, TK_EOF,
  TK_STRING, TK_DOUBLE,     		// GK1: Bổ sung để check kí tự mình đang đọc
  
  KW_PROGRAM, KW_CONST, KW_TYPE, KW_VAR,
  KW_STRING, KW_DOUBLE,     		// GK1: Bổ sung để tạo token cho 2 kiểu dữ liệu mới.
  KW_INTEGER, KW_CHAR, KW_ARRAY, KW_OF, 
  KW_FUNCTION, KW_PROCEDURE,
  KW_BEGIN, KW_END, KW_CALL,
  KW_IF, KW_THEN, KW_ELSE,
  KW_WHILE, KW_DO, KW_FOR, KW_TO,

  SB_SEMICOLON, SB_COLON, SB_PERIOD, SB_COMMA,
  SB_ASSIGN, SB_EQ, SB_NEQ, SB_LT, SB_LE, SB_GT, SB_GE,
  SB_PLUS, SB_MINUS, SB_TIMES, SB_SLASH,
  SB_LPAR, SB_RPAR, SB_LSEL, SB_RSEL
} TokenType; 


// TODO:GK1
typedef struct {
  char string[MAX_IDENT_LEN + 1];
  int lineNo, colNo;
  TokenType tokenType;
  union {
    int value;
    double dvalue;      // GK1: Bổ sung để lưu lại giá trị kiểu double
  };
} Token;

TokenType checkKeyword(char *string);
Token* makeToken(TokenType tokenType, int lineNo, int colNo);
char *tokenToString(TokenType tokenType);


#endif
