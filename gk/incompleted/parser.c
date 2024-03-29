/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "semantics.h"
#include "error.h"
#include "debug.h"
#include "symtab.h"

Token *currentToken;
Token *lookAhead;

extern Type *intType;

// Them --double---
extern Type *doubleType;

// Them ---String---
extern Type *stringType;

extern Type *charType;
extern SymTab *symtab;

void scan(void)
{
  Token *tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType)
{
  if (lookAhead->tokenType == tokenType)
  {
    scan();
  }
  else
    missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void)
{
  Object *program;

  eat(KW_PROGRAM);
  eat(TK_IDENT);

  program = createProgramObject(currentToken->string);
  enterBlock(program->progAttrs->scope);

  eat(SB_SEMICOLON);

  compileBlock();
  eat(SB_PERIOD);

  exitBlock();
}

void compileBlock(void)
{
  Object *constObj;
  ConstantValue *constValue;

  if (lookAhead->tokenType == KW_CONST)
  {
    eat(KW_CONST);

    do
    {
      eat(TK_IDENT);

      checkFreshIdent(currentToken->string);
      constObj = createConstantObject(currentToken->string);

      eat(SB_EQ);
      constValue = compileConstant();

      constObj->constAttrs->value = constValue;
      declareObject(constObj);

      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock2();
  }
  else
    compileBlock2();
}

void compileBlock2(void)
{
  Object *typeObj;
  Type *actualType;

  if (lookAhead->tokenType == KW_TYPE)
  {
    eat(KW_TYPE);

    do
    {
      eat(TK_IDENT);

      checkFreshIdent(currentToken->string);
      typeObj = createTypeObject(currentToken->string);

      eat(SB_EQ);
      actualType = compileType();

      typeObj->typeAttrs->actualType = actualType;
      declareObject(typeObj);

      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock3();
  }
  else
    compileBlock3();
}

void compileBlock3(void)
{
  Object *varObj;
  Type *varType;

  if (lookAhead->tokenType == KW_VAR)
  {
    eat(KW_VAR);

    do
    {
      eat(TK_IDENT);

      checkFreshIdent(currentToken->string);
      varObj = createVariableObject(currentToken->string);

      eat(SB_COLON);
      varType = compileType();

      varObj->varAttrs->type = varType;
      declareObject(varObj);

      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock4();
  }
  else
    compileBlock4();
}

void compileBlock4(void)
{
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void)
{
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void)
{
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE))
  {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else
      compileProcDecl();
  }
}

void compileFuncDecl(void)
{
  Object *funcObj;
  Type *returnType;

  eat(KW_FUNCTION);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  funcObj = createFunctionObject(currentToken->string);
  declareObject(funcObj);

  enterBlock(funcObj->funcAttrs->scope);

  compileParams();

  eat(SB_COLON);
  returnType = compileBasicType();
  funcObj->funcAttrs->returnType = returnType;

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

void compileProcDecl(void)
{
  Object *procObj;

  eat(KW_PROCEDURE);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  procObj = createProcedureObject(currentToken->string);
  declareObject(procObj);

  enterBlock(procObj->procAttrs->scope);

  compileParams();

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

ConstantValue *compileUnsignedConstant(void)
{
  ConstantValue *constValue = NULL;
  Object *obj;

  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    eat(TK_NUMBER);

    // Tạo dựng cho Double
    if (currentToken->flagNumber == 0)
      constValue = makeIntConstant(currentToken->value);
    else
      constValue = makeDoubleConstant(currentToken->fValue);
    break;

  case TK_IDENT:
    eat(TK_IDENT);

    obj = checkDeclaredConstant(currentToken->string);
    constValue = duplicateConstantValue(obj->constAttrs->value);

    break;
  case TK_CHAR:
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

ConstantValue *compileConstant(void)
{
  ConstantValue *constValue = NULL;

  switch (lookAhead->tokenType)
  {
  case SB_PLUS:
    eat(SB_PLUS);
    constValue = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    constValue = compileConstant2();
    constValue->intValue = -constValue->intValue;
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  case TK_STRING:
    eat(TK_STRING);
    constValue = makeStringConstant(currentToken->stringNode); //phần này phải để string node để tạo token in được string 
    break;
  default:
    constValue = compileConstant2();
    break;
  }
  return constValue;
}

ConstantValue *compileConstant2(void)
{
  ConstantValue *constValue = NULL;
  Object *obj;

  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    eat(TK_NUMBER);
    // Tạo thêm cho double
    if (currentToken->flagNumber == 0)
      constValue = makeIntConstant(currentToken->value);
    else
      constValue = makeDoubleConstant(currentToken->fValue);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredConstant(currentToken->string);
    // if (obj->constAttrs->value->type == TP_INT)
    // printf("loi o day\n");
    if (obj->constAttrs->value->type == TP_INT || obj->constAttrs->value->type == TP_DOUBLE
        || obj->constAttrs->value->type == TP_CHAR || obj->constAttrs->value->type == TP_STRING) //add various type
    {
      // printf("hay loi o day\n");
      constValue = duplicateConstantValue(obj->constAttrs->value);
      // printf("stop this pain\n");
      }
    else
      error(ERR_UNDECLARED_INT_CONSTANT, currentToken->lineNo, currentToken->colNo);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

Type *compileType(void)
{
  Type *type = NULL;
  Type *elementType;
  int arraySize;
  Object *obj;

  switch (lookAhead->tokenType)
  {
  case KW_INTEGER:
    eat(KW_INTEGER);
    type = makeIntType();
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    type = makeCharType();
    break;

  // Them --- double ---
  case KW_DOUBLE:
    eat(KW_DOUBLE);
    type = makeDoubleType();
    break;

    // Them --- string ---
  case KW_STRING:
    eat(KW_STRING);
    type = makeStringType();
    break;

  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);

    arraySize = currentToken->value;

    eat(SB_RSEL);
    eat(KW_OF);
    elementType = compileType();
    type = makeArrayType(arraySize, elementType);
    break;

  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredType(currentToken->string);
    type = duplicateType(obj->typeAttrs->actualType);
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

Type *compileBasicType(void)
{
  Type *type = NULL;

  switch (lookAhead->tokenType)
  {
  case KW_INTEGER:
    eat(KW_INTEGER);
    type = makeIntType();
    break;

    // -- Them double ---
  case KW_DOUBLE:
    eat(KW_DOUBLE);
    type = makeDoubleType();
    break;

  // --- Them string
  case KW_STRING:
    eat(KW_STRING);
    type = makeStringType();
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    type = makeCharType();
    break;

  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void)
{
  if (lookAhead->tokenType == SB_LPAR)
  {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON)
    {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void)
{
  Object *param;
  Type *type;
  enum ParamKind paramKind = PARAM_VALUE; //default

  switch (lookAhead->tokenType)
  {
  case TK_IDENT:
    paramKind = PARAM_VALUE;
    break;
  case KW_VAR:
    eat(KW_VAR);
    paramKind = PARAM_REFERENCE;
    break;
  default:
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
    break;
  }

  eat(TK_IDENT);
  checkFreshIdent(currentToken->string);
  param = createParameterObject(currentToken->string, paramKind, symtab->currentScope->owner);
  eat(SB_COLON);
  type = compileBasicType();
  param->paramAttrs->type = type;
  declareObject(param);
}

void compileStatements(void)
{
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON)
  {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void)
{
  switch (lookAhead->tokenType)
  {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;

  // ************ START UPDATE ************
  // Thêm trường hợp DO - WHILE
  case KW_DO:
    compileDoWhileSt();
    break;
  // ************ END UPDATE ************

  // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

Type *compileLValue(void)
{
  // TODO: parse a lvalue (a variable, an array element, a parameter, the current function identifier)
  Object *var;
  Type *varType = NULL;

  eat(TK_IDENT);
  // check if the identifier is a function identifier, or a variable identifier, or a parameter
  var = checkDeclaredLValueIdent(currentToken->string);
  switch (var->kind)
  {
  case OBJ_VARIABLE:
    if (var->varAttrs->type->typeClass == TP_ARRAY)
      varType = compileIndexes(var->varAttrs->type);
    else
      // varType = var->varAttrs->type;
      varType = duplicateType(var->varAttrs->type);
    break;
  case OBJ_PARAMETER:
    // varType = var->paramAttrs->type;
    varType = duplicateType(var->paramAttrs->type);
    break;
  case OBJ_FUNCTION:
    // varType = var->funcAttrs->returnType;
    varType = duplicateType(var->funcAttrs->returnType);
    break;
  default:
    error(ERR_INVALID_LVALUE, currentToken->lineNo, currentToken->colNo);
  }

  return varType;
}

// void compileAssignSt(void)
// {
//   // TODO: parse the assignment and check type consistency
//   Type *varType;
//   Type *expressType;

//   varType = compileLValue();

//   eat(SB_ASSIGN);

//   expressType = compileExpression();
//   checkTypeEquality(varType, expressType);
// }

// ************* START UPDATE *************
// x, y, … , z  := <Expression>, <Expression>, … , <Expression>

void compileAssignSt(void)
{
  Type *varType[100];
  Type *expressType[100];
  int i = 0;
  int j = 0;
  while (1)
  {
    varType[i++] = compileLValue();
    if (lookAhead->tokenType == SB_ASSIGN)
      break;
    if (lookAhead->tokenType == SB_COMMA)
      eat(SB_COMMA);
  }


  eat(SB_ASSIGN);
  while (1)
  {
    expressType[j++] = compileExpression();
    if (lookAhead->tokenType == SB_COMMA)
      eat(SB_COMMA);
    else
      break;
  }

  if (i != j)
  {
    error(ERR_NUMBER_OF_ELEMENTS, currentToken->lineNo, currentToken->colNo);
  }
  else
  {
    for (int k = 0; k < i; k++)
    {
      checkTypeEquality(varType[k], expressType[k]);
    }
  }
}

// ************* END UPDATE *************

void compileCallSt(void)
{
  Object *proc;

  eat(KW_CALL);
  eat(TK_IDENT);

  proc = checkDeclaredProcedure(currentToken->string);

  compileArguments(proc->procAttrs->paramList);
}

void compileGroupSt(void)
{
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void)
{
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE)
    compileElseSt();
}

void compileElseSt(void)
{
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void)
{
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

void compileForSt(void)
{
  // TODO: Check type consistency of FOR's variable
  Type *varType;
  Type *type;

  eat(KW_FOR);

  // // check if the identifier is a variable
  // checkDeclaredVariable(currentToken->string);
  varType = compileLValue();

  eat(SB_ASSIGN);
  type = compileExpression();
  checkTypeEquality(varType, type);

  eat(KW_TO);
  type = compileExpression();
  checkTypeEquality(varType, type);

  eat(KW_DO);
  compileStatement();
}

// ************* START UPDATE *************
// Thêm Do - while
void compileDoWhileSt(void)
{
  eat(KW_DO);
  compileStatement();
  eat(KW_WHILE);
  compileCondition();
}
// ************* END UPDATE *************

void compileArgument(Object *param)
{
  // TODO: parse an argument, and check type consistency
  //       If the corresponding parameter is a reference, the argument must be a lvalue
  Type *type;
  if (param->paramAttrs->kind == PARAM_VALUE)
  {
    type = compileExpression();
    checkTypeEquality(type, param->paramAttrs->type);
  }
  else
  {
    type = compileLValue();
    checkTypeEquality(type, param->paramAttrs->type);
  }
}
/*
void compileArguments(ObjectNode *paramList)
{
  //TODO: parse a list of arguments, check the consistency of the arguments and the given parameters
  ObjectNode *node = paramList;
  switch (lookAhead->tokenType)
  {
  case SB_LPAR:
    eat(SB_LPAR);
    if (node == NULL)
      error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    compileArgument(node->object);
    node = node->next;
    while (lookAhead->tokenType == SB_COMMA)
    {
      eat(SB_COMMA);
      if (node == NULL)
        error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
      compileArgument(node->object);
      node = node->next;
    }
    eat(SB_RPAR);
    break;
    // Check FOLLOW set
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}*/

void compileArguments(ObjectNode* paramList) {
  //parse a list of arguments, check the consistency of the arguments and the given parameters
  Object *param;
  ObjectNode *root = paramList;
  if (root == NULL) {
    param = NULL;
  } else {
    param = root->object;
    root = root->next;
  }

  switch (lookAhead->tokenType) {
  case SB_LPAR:
    eat(SB_LPAR);
    compileArgument(param);

    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      if (root != NULL) {
        param = root->object;
        root = root->next;
      } else {
        param = NULL;
      }
      compileArgument(param);
    }
    
    eat(SB_RPAR);
    break;
    // Check FOLLOW set 
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
  case KW_WHILE:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void)
{
  // TODO: check the type consistency of LHS and RSH, check the basic type
  Type *type1;
  Type *type2;

  type1 = compileExpression();
  checkBasicType(type1);

  switch (lookAhead->tokenType)
  {
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  default:
    error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }

  type2 = compileExpression();
  checkTypeEquality(type1, type2);
}

Type *compileExpression(void)
{
  Type *type;

  switch (lookAhead->tokenType)
  {
  case SB_PLUS:
    eat(SB_PLUS);
    type = compileExpression2();
    checkNumberType(type);
    // checkIntType(type);
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type = compileExpression2();
    checkNumberType(type);
    // checkIntType(type);
    break;
  default:
    type = compileExpression2();
  }
  return type;
}

Type *compileExpression2(void)
{
  Type *type1;
  Type *type2;

  type1 = compileTerm();
  type2 = compileExpression3();
  if (type2 == NULL)
    return type1;
  else
  {
    // 
    //return type1;
    // or 
    return Operator_check(type1, type2);
  }
}

Type* compileExpression3(void) {
  Type* type1;
  Type* type2;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    type1 = compileTerm();
    checkNumberType(type1);

    type2 = compileExpression3();
    if (type2 != NULL) {
      return Operator_check(type1, type2);
    }
    return type1;
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type1 = compileTerm();
    checkNumberType(type1);
    type2 = compileExpression3();
    if (type2 != NULL) {
      return Operator_check(type1, type2);
    }
    return type1;
    break;
    // check the FOLLOW set
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
  case KW_WHILE:
    return NULL;
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
  return NULL;
}

Type* compileTerm(void) {
  Type* type1;
  Type* type2;

  type1 = compileFactor();
  type2 = compileTerm2();

  if (type2 != NULL) {
    return Operator_check(type1, type2);
  }

  return type1;
}

Type* compileTerm2(void) {
  Type* type1;
  Type* type2;

  switch (lookAhead->tokenType) {
  case SB_TIMES:
    eat(SB_TIMES);
    type1 = compileFactor();
    checkNumberType(type1);

    type2 = compileTerm2();
    if (type2 != NULL) {
      return Operator_check(type1, type2);
    }
    return type1;
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    type1 = compileFactor();
    checkNumberType(type1);

    type2 = compileTerm2();
    if (type2 != NULL) {
      return Operator_check(type1, type2);
    }
    return type1;
    break;
    // check the FOLLOW set
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
  case KW_WHILE:

    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
  return NULL;
}


Type *compileFactor(void)
{
  // TODO: parse a factor and return the factor's type

  Object *obj;
  Type *type = NULL;

  switch (lookAhead->tokenType)
  {
  case TK_NUMBER:
    // Thêm cho double
    eat(TK_NUMBER);
    if (currentToken->flagNumber == 0)
      type = makeIntType();
    else
      type = makeDoubleType();
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    type = makeCharType();
    break;

  // Them string
  case TK_STRING:
    eat(TK_STRING);
    type = makeStringType();
    break;

  case TK_IDENT:
    eat(TK_IDENT);
    // check if the identifier is declared
    obj = checkDeclaredIdent(currentToken->string);

    switch (obj->kind)
    {
    case OBJ_CONSTANT:
      if (obj->constAttrs->value->type == TP_INT)
      {
        type = makeIntType();
      }
      else if (obj->constAttrs->value->type == TP_DOUBLE)
      {
        type = makeDoubleType();
      }
      else if (obj->constAttrs->value->type == TP_CHAR)
      {
        type = makeCharType();
      }
      else if (obj->constAttrs->value->type == TP_STRING)
      {
        type = makeStringType();
      }
      else
      {
        error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
      }
      break;
    case OBJ_VARIABLE:
      if (obj->varAttrs->type->typeClass == TP_ARRAY)
      {
        type = compileIndexes(obj->varAttrs->type);
      }
      else
      {
        type = duplicateType(obj->varAttrs->type);
      }
      break;
    case OBJ_PARAMETER:
      type = duplicateType(obj->paramAttrs->type);
      break;
    case OBJ_FUNCTION:
      compileArguments(obj->funcAttrs->paramList);
      type = duplicateType(obj->funcAttrs->returnType);
      break;
    default:
      error(ERR_INVALID_FACTOR, currentToken->lineNo, currentToken->colNo);
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
  return type;
}

Type *compileIndexes(Type *arrayType)
{
  // TODO: parse a sequence of indexes, check the consistency to the arrayType, and return the element type
  Type *type;
  while (lookAhead->tokenType == SB_LSEL)
  {
    eat(SB_LSEL);
    type = compileExpression();
    checkIntType(type);

    checkArrayType(arrayType);

    arrayType = arrayType->elementType;

    eat(SB_RSEL);
  }
  checkBasicType(arrayType);
  return arrayType;
}

int compile(char *fileName)
{
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();

  printObject(symtab->program, 0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;
}
