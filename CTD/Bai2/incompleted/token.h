/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __TOKEN_H__
#define __TOKEN_H__

#define MAX_IDENT_LEN 15
#define KEYWORDS_COUNT 20

typedef enum {
  TK_NONE, TK_IDENT, TK_NUMBER, TK_CHAR, TK_EOF, 
  // TK_NONE      Dai dien cho 1 loi
  // TK_IDENT     Dinh danh
  // TK_NUMBER    So
  // TK_CHAR      Hang ki tu
  // TK_EOF       Ket thuc chuong trinh
  KW_PROGRAM, KW_CONST, KW_TYPE, KW_VAR,
  KW_INTEGER, KW_CHAR, KW_ARRAY, KW_OF, 
  KW_FUNCTION, KW_PROCEDURE,
  KW_BEGIN, KW_END, KW_CALL,
  KW_IF, KW_THEN, KW_ELSE,
  KW_WHILE, KW_DO, KW_FOR, KW_TO,
  // Cac tu khoa PROGRAM,CONST,TYPE,VAR,INTEGER,CHAR,ARRAY,OF,FUNCTION,PROCEDURE,BEGIN,END,CALL,IF,THEN,ELSE,WHILE,DO,FOR,TO
  SB_SEMICOLON, SB_COLON, SB_PERIOD, SB_COMMA,
  SB_ASSIGN, SB_EQ, SB_NEQ, SB_LT, SB_LE, SB_GT, SB_GE,
  SB_PLUS, SB_MINUS, SB_TIMES, SB_SLASH,
  SB_LPAR, SB_RPAR, SB_LSEL, SB_RSEL
  // Cac ki hieu dac biet ; : . , @ = !=  < <= > >= + - * / ( ) (. .)
} TokenType; 

typedef struct {
  char string[MAX_IDENT_LEN + 1];
  int lineNo, colNo;
  TokenType tokenType;
  int value;
} Token;

TokenType checkKeyword(char *string);
Token* makeToken(TokenType tokenType, int lineNo, int colNo);


#endif
