/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __TOKEN_H__
#define __TOKEN_H__

#define MAX_IDENT_LEN 15
#define MAX_STRING_LEN 1000
#define KEYWORDS_COUNT 26

typedef enum
{
  TK_NONE,
  TK_IDENT,
  TK_NUMBER,
  TK_CHAR,
  TK_EOF,
  //TK_EQUAL,
  // Thêm token String
  TK_STRING,

  KW_PROGRAM,
  KW_CONST,
  KW_TYPE,
  KW_VAR,
  KW_INTEGER,
  KW_CHAR,
  KW_ARRAY,
  KW_OF,
  KW_FUNCTION,
  KW_PROCEDURE,
  KW_BEGIN,
  KW_END,
  KW_CALL,
  KW_IF,
  KW_THEN,
  KW_ELSE,
  KW_WHILE,
  KW_DO,
  KW_FOR,
  KW_TO,

  // Thêm keyword KW_DOUBLE vào trong token.h
  KW_DOUBLE,

  // Thêm keyword STRING
  KW_STRING,

  SB_SEMICOLON,
  SB_COLON,
  SB_PERIOD,
  SB_COMMA,
  SB_ASSIGN,
  SB_EQ,
  SB_NEQ,
  SB_LT,
  SB_LE,
  SB_GT,
  SB_GE,
  SB_PLUS,
  SB_MINUS,
  SB_TIMES,
  SB_SLASH,
  SB_LPAR,
  SB_RPAR,
  SB_LSEL,
  SB_RSEL
} TokenType;

typedef struct
{
  char string[MAX_IDENT_LEN + 1];
  int lineNo, colNo;
  TokenType tokenType;
             // --- Int value ---
  int flagNumber;       // --- flagNumber = 0 -> int value, flagNumber = 1 -> doubleType ---
  double fValue;  
  int value;        // --- Double value ---
  char stringNode[MAX_STRING_LEN]; // --- Thêm String ---
} Token;

TokenType checkKeyword(char *string);
Token *makeToken(TokenType tokenType, int lineNo, int colNo);
char *tokenToString(TokenType tokenType);

#endif
