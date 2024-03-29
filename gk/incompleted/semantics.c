/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "error.h"

extern SymTab *symtab;
extern Token *currentToken;

Object *lookupObject(char *name)
{
  Scope *scope = symtab->currentScope;
  Object *obj;

  while (scope != NULL)
  {
    obj = findObject(scope->objList, name);
    if (obj != NULL)
      return obj;
    scope = scope->outer;
  }
  obj = findObject(symtab->globalObjectList, name);
  if (obj != NULL)
    return obj;
  return NULL;
}

void checkFreshIdent(char *name)
{
  if (findObject(symtab->currentScope->objList, name) != NULL)
    error(ERR_DUPLICATE_IDENT, currentToken->lineNo, currentToken->colNo);
}

Object *checkDeclaredIdent(char *name)
{
  Object *obj = lookupObject(name);
  if (obj == NULL)
  {
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

Object *checkDeclaredConstant(char *name)
{
  Object *obj = lookupObject(name);
  if (obj == NULL)
    error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_CONSTANT)
    error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);

  return obj;
}

Object *checkDeclaredType(char *name)
{
  Object *obj = lookupObject(name);
  if (obj == NULL)
    error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_TYPE)
    error(ERR_INVALID_TYPE, currentToken->lineNo, currentToken->colNo);

  return obj;
}

Object *checkDeclaredVariable(char *name)
{
  Object *obj = lookupObject(name);
  if (obj == NULL)
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_VARIABLE)
    error(ERR_INVALID_VARIABLE, currentToken->lineNo, currentToken->colNo);

  return obj;
}

Object *checkDeclaredFunction(char *name)
{
  Object *obj = lookupObject(name);
  if (obj == NULL)
    error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_FUNCTION)
    error(ERR_INVALID_FUNCTION, currentToken->lineNo, currentToken->colNo);

  return obj;
}

Object *checkDeclaredProcedure(char *name)
{
  Object *obj = lookupObject(name);
  if (obj == NULL)
    error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_PROCEDURE)
    error(ERR_INVALID_PROCEDURE, currentToken->lineNo, currentToken->colNo);

  return obj;
}

Object *checkDeclaredLValueIdent(char *name)
{
  Object *obj = lookupObject(name);
  if (obj == NULL)
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);

  switch (obj->kind)
  {
  case OBJ_VARIABLE:
  case OBJ_PARAMETER:
    break;
  case OBJ_FUNCTION:
    if (obj != symtab->currentScope->owner)
      error(ERR_INVALID_IDENT, currentToken->lineNo, currentToken->colNo);
    break;
  case OBJ_CONSTANT:
    error(ERR_CHANGE_OF_CONSTANT, currentToken->lineNo, currentToken->colNo); //khong the thay doi const
    break;
  default:
    error(ERR_INVALID_IDENT, currentToken->lineNo, currentToken->colNo);
  }

  return obj;
}

void checkIntType(Type *type)
{
  // TODO
  printf("Vao trong nay1");
  if (type->typeClass == TP_INT && type != NULL)
  {
    return;
  }
  else
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
}

void checkDoubleType(Type *type)
{
  printf("Vao trong nay2");
  if ((type != NULL) && (type->typeClass == TP_DOUBLE))
  {
    return;
  }
  else
  {
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
  }
}

void checkNumberType(Type *type)
{
  printf("Vao trong nay3");
  if ((type != NULL) && (type->typeClass == TP_INT || type->typeClass == TP_DOUBLE))
  {
    return;
  }
  else
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
}

// Them --- String ---
void checkStringType(Type *type)
{
  if ((type != NULL) && (type->typeClass == TP_STRING))
  {
    return;
  }
  else
  {
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
  }
}

void checkCharType(Type *type)
{
  // printf("Vao trong nay");
  if ((type != NULL) && (type->typeClass == TP_CHAR))
  {
    return;
  }
  else
  {
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
  }
}

void checkBasicType(Type *type)
{
  // TODO
  // printf("Vao trong nay");

  if ((type != NULL) && ((type->typeClass == TP_INT) || (type->typeClass == TP_CHAR) || (type->typeClass == TP_DOUBLE) || (type->typeClass == TP_STRING)))
  {
    return;
  }
  else
  {
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
  }
}

void checkArrayType(Type *type)
{
  // TODO
  if (type != NULL && type->typeClass == TP_ARRAY)
  {
    return;
  }
  else
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
}

// void checkTypeEquality(Type *type1, Type *type2)
// {
//   // TODO
//   // printf("Vao trong nay");

//   if (type1->typeClass != type2->typeClass)
//     error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
//   else if (type1->typeClass == TP_ARRAY)
//   {
//     checkTypeEquality(type1->elementType, type2->elementType);
//     if (type1->arraySize != type2->arraySize)
//       error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
//   }
// }

// **************** START UPDATE ****************
void checkTypeEquality(Type *type1, Type *type2)
{

  if (type1 == NULL || type2 == NULL)
  {
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
  }
  // Cho ép kiểu từ int -> double
  if (type1->typeClass == TP_DOUBLE && type2->typeClass == TP_INT)
  {
    return;
  }
  if (compareType(type1, type2) == 0)
  {
    error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
  }
}

Type *upcasting(Type *type1, Type *type2)
{

  checkNumberType(type1);
  checkNumberType(type2);

  if (type1->typeClass == TP_DOUBLE)
  {
    return type1;
  }
  else
  {
    return type2;
  }
}

Type *Operator_check(Type *type1, Type *type2)
{
  checkNumberType(type1);
  checkNumberType(type2);
  if (type1->typeClass == TP_DOUBLE)
    return type1;
  else if(type2->typeClass == TP_DOUBLE)
    return type2;
  else
    return type1;
  
}

// **************** END UPDATE ****************
