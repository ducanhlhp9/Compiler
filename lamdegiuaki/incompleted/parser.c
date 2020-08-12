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
//TODO:GK3
#include "symtab.h"

Token *currentToken;        // Token hien tai dang xu ly
Token *lookAhead;            // Token tiep theo se xu ly

extern Type* intType;
extern Type* charType;
// TODO::GK1
extern Type* stringType;    // GK1: extends từ file semantics.c
extern Type* doubleType;    // GK1: extends từ file semantics.c

extern SymTab* symtab;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  Object* program;

  eat(KW_PROGRAM);
  eat(TK_IDENT);

  program = createProgramObject(currentToken->string);
  enterBlock(program->progAttrs->scope);

  eat(SB_SEMICOLON);

  compileBlock();
  eat(SB_PERIOD);

  exitBlock();
}

void compileBlock(void) {
  Object* constObj;
  ConstantValue* constValue;

  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);

    do {
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
  else compileBlock2();
}

void compileBlock2(void) {
  Object* typeObj;
  Type* actualType;

  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);

    do {
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
  else compileBlock3();
}

void compileBlock3(void) {
  Object* varObj;
  Type* varType;

  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);

    do {
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
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void) {
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else compileProcDecl();
  }
}

void compileFuncDecl(void) {
  Object* funcObj;
  Type* returnType;

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

void compileProcDecl(void) {
  Object* procObj;

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

// TODO:GK1
ConstantValue* compileUnsignedConstant(void) {    // Xu ly them 2 kieu moi
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
    case TK_NUMBER:
      eat(TK_NUMBER);
      constValue = makeIntConstant(currentToken->value);
      if (DEBUG == 1) printf("Line %d: Debug compileUnsignedConstant: type => %d // val => %d\n",currentToken->lineNo, constValue->type, constValue->intValue);
      break;
    case TK_DOUBLE:
      eat(TK_DOUBLE);
      constValue = makeDoubleConstant(currentToken->dvalue);
      if (DEBUG == 1) printf("Line %d: Debug compileUnsignedConstant: type => %d // val => %f\n",currentToken->lineNo, constValue->type, constValue->doubleValue);
      break;
    case TK_IDENT:
      eat(TK_IDENT);
      obj = checkDeclaredConstant(currentToken->string);
      constValue = duplicateConstantValue(obj->constAttrs->value);
      break;
    case TK_CHAR:
      eat(TK_CHAR);
      constValue = makeCharConstant(currentToken->string[0]);
      if (DEBUG == 1) printf("Line %d: Debug compileUnsignedConstant: type => %d // val => %c\n",currentToken->lineNo, constValue->type, constValue->charValue);
      break;
    case TK_STRING:
      eat(TK_STRING);
      constValue = makeStringConstant(currentToken->string);
      if (DEBUG == 1) printf("Line %d: Debug compileUnsignedConstant: type => %d // val => %s\n",currentToken->lineNo, constValue->type, constValue->stringValue);
      break;
    default:
      error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
  return constValue;
}
// TODO:GK1
ConstantValue* compileConstant(void) {
  ConstantValue* constValue;

  switch (lookAhead->tokenType) {
    case SB_PLUS:
      eat(SB_PLUS);
      constValue = compileConstant2();
      break;
    case SB_MINUS:
      eat(SB_MINUS);
      constValue = compileConstant2();
      if( constValue->type == TP_INT){    // GK1: Check them kieu double
        constValue->intValue = - constValue->intValue;
        if (DEBUG == 1) printf("Line %d: Debug compileConstant: type => %d // val => %d\n",currentToken->lineNo, constValue->type, constValue->intValue);
      } else {
        constValue->doubleValue = - constValue->doubleValue;
        if (DEBUG == 1) printf("Line %d: Debug compileConstant: type => %d // val => %f\n",currentToken->lineNo, constValue->type, constValue->doubleValue);
      }
      break;
    case TK_CHAR:
      eat(TK_CHAR);
      constValue = makeCharConstant(currentToken->string[0]);
      if (DEBUG == 1) printf("Line %d: Debug compileConstant: type => %d // val => %c\n",currentToken->lineNo, constValue->type, constValue->charValue);
      break;
    case TK_STRING:
      eat(TK_STRING);
      constValue = makeStringConstant(currentToken->string); 
      if (DEBUG == 1) printf("Line %d: Debug compileConstant: type => %d // val => %s\n",currentToken->lineNo, constValue->type, constValue->stringValue);   
      break;
    default:
      constValue = compileConstant2();
      break;
  }
  return constValue;
}
// TODO:GK1
ConstantValue* compileConstant2(void) {
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_DOUBLE:     // Them keyword moi
    eat(TK_DOUBLE);
    constValue = makeDoubleConstant(currentToken->dvalue);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredConstant(currentToken->string);
    if (obj->constAttrs->value->type == TP_INT || obj->constAttrs->value->type == TP_DOUBLE)
      constValue = duplicateConstantValue(obj->constAttrs->value);
    else
      error(ERR_UNDECLARED_INT_CONSTANT,currentToken->lineNo, currentToken->colNo);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

// TODO:GK1
Type* compileType(void) {
  Type* type;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
    type =  makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break;
  case KW_STRING:         // GK1: Them 2 keyword moi
    eat(KW_STRING); 
    type = makeStringType();
    break;
  case KW_DOUBLE:        // GK1: Them 2 keyword moi
    eat(KW_DOUBLE); 
    type = makeDoubleType();
    break;
  //
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

// TODO:GK1
Type* compileBasicType(void) {
  Type* type;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER); 
    type = makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break; 
  case KW_STRING:         // GK1: make them StringType 
    eat(KW_STRING); 
    type = makeStringType();
    break;
  case KW_DOUBLE:         // GK1: make them DoubleType 
    eat(KW_DOUBLE); 
    type = makeDoubleType();
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void) {
  Object* param;
  Type* type;
  enum ParamKind paramKind;

  switch (lookAhead->tokenType) {
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

void compileStatements(void) {
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

// TODO:GK2
void compileStatement(void) {
  switch (lookAhead->tokenType) {
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
  case KW_DO:
    compileDoWhileSt();
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

Type* compileLValue(void) {
  Object* var;
  Type* varType;

  eat(TK_IDENT);
  if(DEBUG == 3) printf("Line %d: Debug compileLValue: ident => %s\n",currentToken->lineNo, currentToken->string);
  var = checkDeclaredLValueIdent(currentToken->string);
  if (var->kind == OBJ_VARIABLE){
    if(var->varAttrs->type->typeClass == TP_ARRAY){
      varType = compileIndexes(var->varAttrs->type);
    } else {
      varType = var->varAttrs->type;
    }
  } else if(var->kind == OBJ_FUNCTION) {
    varType = var->funcAttrs->returnType;
  } else if(var->kind == OBJ_PARAMETER){
    varType = var->paramAttrs->type;
  }
  
  return varType;
}

// void compileAssignSt(void) {
//   Type* type = compileLValue();
//   eat(SB_ASSIGN);
//   Type* typeExp = compileExpression();
//   checkTypeEquality(type, typeExp);
// }

// TODO:GK3
// Ham modify: x, y, … , z  := <Expression>, <Expression>, … , <Expression> 
// AssignST ::= VariableFunctions SB_ASSIGN Expressions
// VariableFunction ::= Variable
// VariableFunction ::= FunctionIdent
// VariableFunctions ::= VariableFunction SB_COMMA VariableFunctions
// VariableFunctions ::= VariableFunction
// Expressions ::= Expression SB_COMMA Expressions
// Expressions ::= Expression
// TODO:GK3 => sua lai ham
void compileAssignSt(void) {    // BNF 56m
  TypeNode* listTypeVars;
  listTypeVars = compileListLVars();
  eat(SB_ASSIGN);
  TypeNode* listTypeExps;
  listTypeExps = compileListRExps();
  checkMultiTypeAssignment(listTypeVars, listTypeExps);     // So sanh kieu du lieu 2 ben co ep kieu
  // freeTypeNode(listTypeVars);
  // freeTypeNode(listTypeExps);
}
// TODO:GK3
TypeNode* compileListLVars() {
  if(DEBUG == 3) printf("Line %d: Debug compileListLVars!\n", currentToken->lineNo);
  TypeNode* types = NULL;
  Type* type;
  
  do {
    if (lookAhead->tokenType == SB_COMMA) eat(SB_COMMA);
    type  = compileLValue();
    addType(&types, type);
    if (DEBUG == 3) printf("Line %d: Debug left variable in assignment: type => %d\n", currentToken->lineNo, type->typeClass);
  } while (lookAhead->tokenType == SB_COMMA);

  return types;
} 
// TODO:GK3
TypeNode* compileListRExps() {
  if(DEBUG == 3) printf("Line %d: Debug compileListRExps!\n", currentToken->lineNo);
  TypeNode* types = NULL;
  Type* type;
  do {
    if (lookAhead->tokenType == SB_COMMA) eat(SB_COMMA);
    type  = compileExpression();
    addType(&types, type);
    if (DEBUG == 3) printf("Line %d: Debug right value in assignment: type => %d\n", currentToken->lineNo, type->typeClass);
  } while (lookAhead->tokenType == SB_COMMA);

  return types;
}




void compileCallSt(void) {
  Object* proc;

  eat(KW_CALL);
  eat(TK_IDENT);

  proc = checkDeclaredProcedure(currentToken->string);

  compileArguments(proc->procAttrs->paramList);
}

void compileGroupSt(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void) {
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

void compileForSt(void) {
  eat(KW_FOR);

  // check if the identifier is a variable
  Type *varType = compileLValue();

  eat(SB_ASSIGN);
  Type* type1 = compileExpression();
  checkTypeEquality(varType, type1);    // Check type trong phep gan 1 for => so sanh bang

  eat(KW_TO);
  Type* type2 = compileExpression();
  checkBasicType(type2);
  checkTypeEquality(varType, type2);    // Check type trong phep gan 2 to  => so sanh bang

  eat(KW_DO);
  compileStatement();
}

// TODO:GK2
void compileDoWhileSt(void) {
  if(DEBUG == 2) printf("Line %d: Debug start parse do-while statement!\n", currentToken->lineNo);
  eat(KW_DO);
  if(DEBUG == 2) printf("Line %d: Ate KW_DO\n", currentToken->lineNo);
  compileStatement();
  eat(KW_WHILE);
  if(DEBUG == 2) printf("Line %d: Ate KW_WHILE\n", currentToken->lineNo);
  compileCondition();
  if(DEBUG == 2) printf("Line %d: Debug end parse do-while statement!\n", currentToken->lineNo);
}

void compileArgument(Object* param) {
  Type* type;
  if(param->paramAttrs->kind == PARAM_VALUE){
    type = compileExpression();
    checkTypeEquality(type, param->paramAttrs->type);   // Check Type du lieu vao cua function, procedure => so sanh bang
  } else {
    type = compileLValue();
    checkTypeEquality(type, param->paramAttrs->type);   // Check Type du lieu vao cua function, procedure => so sanh bang
  }
}

void compileArguments(ObjectNode* paramList) {
  ObjectNode* node = paramList;
  switch (lookAhead->tokenType) {
    case SB_LPAR:
      eat(SB_LPAR);
      if(node == NULL) {
        error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->colNo, currentToken->lineNo);
      } 
      compileArgument(node->object);
      while (lookAhead->tokenType == SB_COMMA) {
        eat(SB_COMMA);
        node = node->next;
        if(node == NULL) {
          error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->colNo, currentToken->lineNo);
        } 
        compileArgument(node->object);
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
}


// TODO:GK1
void compileCondition(void) {     // Kiem tra kieu cua 1 phep gan du lieu
  Type* lhs = compileExpression();
  checkBasicType(lhs);

  switch (lookAhead->tokenType) {
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

  Type* rhs = compileExpression();
  checkBasicType(rhs);
  checkTypeEquality(lhs, rhs);      // Check type cua 2 bieu thuc so sanh => So sanh bang
}

// TODO:GK1
Type* compileExpression(void) {
  Type* type;
  
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    type = compileExpression2();
    checkNumberType(type);        // GK1: Check truong hop phep + co phai la kieu so
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type = compileExpression2();
    checkNumberType(type);      // GK1: Check truong hop phep - co phai la kieu so
    break;
  default:
    type = compileExpression2();
  }
  return type;
}

// TODO:GK3 => Sua de check cong dc cac kieu bien
Type* compileExpression2(void) {  // GK1: Kiem tra type cua 1 phep +-*/
  Type* type1;
  Type* type2;

  type1 = compileTerm();
  type2 = compileExpression3();
  if (type2 == NULL) return type1;
  else {
    checkTypeInExpression(type1, type2);      // Check type cua phep +-*/ => So sanh ko bang
    return type1;
  }
}

// TODO:GK1
// TODO:GK2
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
        checkNumberType(type2);
      }
      return type1;
    case SB_MINUS:
      eat(SB_MINUS);
      type1 = compileTerm();
      checkNumberType(type1);
    
      type2 = compileExpression3();
      if (type2 != NULL) {
        checkNumberType(type2);
      }
      return type1;
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
    case KW_WHILE:        // GK2
      return NULL;
    default:
      error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
      return NULL;
  }
}

// TODO:GK1   
Type* compileTerm(void) {     // GK1: Check kieu du lieu cua phan truoc ve */
  Type* type;

  type = compileFactor();
  checkBasicType(type);
  
  compileTerm2();

  return type;
}

// TODO:GK1
// TODO:GK2
void compileTerm2(void) {     // GK1: Check kieu du lieu 
  Type* type;

  switch (lookAhead->tokenType) {
    case SB_TIMES:
      eat(SB_TIMES);
      type = compileFactor();
      checkNumberType(type);

      compileTerm2();
      break;
    case SB_SLASH:
      eat(SB_SLASH);
      type = compileFactor();
      checkNumberType(type);

      compileTerm2();
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
    case KW_WHILE:      // GK2
      break;
    default:
      error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
}

// TODO:GK1
Type* compileFactor(void) {

  Object* obj;
  Type* type;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    type = intType;
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    type = charType;
    break;
  
  case TK_STRING:            // GK1: Check token string
    eat(TK_STRING);
    type = stringType;
    break;

  case TK_DOUBLE:           // GK1: Check token double
    eat(TK_DOUBLE);
    type = doubleType;
    break;

  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredIdent(currentToken->string);

    switch (obj->kind) {
    case OBJ_CONSTANT:
      if(obj->constAttrs->value->type == TP_INT){
        type = intType;
      } else if(obj->constAttrs->value->type == TP_CHAR){
        type = charType;
      } else if(obj->constAttrs->value->type == TP_DOUBLE){   //GK1
        type = doubleType;
      } else if(obj->constAttrs->value->type == TP_STRING){   // GK1
      }
      break;
    case OBJ_VARIABLE:
      if(obj->varAttrs->type->typeClass == TP_ARRAY){
        type = compileIndexes(obj->varAttrs->type);
      } else {
        type = obj->varAttrs->type;
      }
      break;
    case OBJ_PARAMETER:
      type = obj->paramAttrs->type;
      break;
    case OBJ_FUNCTION:
      compileArguments(obj->funcAttrs->paramList);
      type = obj->funcAttrs->returnType;
      break;
    default: 
      error(ERR_INVALID_FACTOR,currentToken->lineNo, currentToken->colNo);
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
  return type;
}

Type* compileIndexes(Type* arrayType) {
  Type* type;
  while (lookAhead->tokenType == SB_LSEL) {
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

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();

  printObject(symtab->program,0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
