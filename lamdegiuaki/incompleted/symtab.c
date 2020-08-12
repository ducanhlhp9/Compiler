/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "error.h"

void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode *objList);
void freeReferenceList(ObjectNode *objList);

SymTab* symtab;
Type* intType;
Type* charType;
// TODO:GK1
Type* stringType;   // GK1: Thêm khởi tạo stringType sử dụng trong parse.c
Type* doubleType;   // GK1: Thêm khởi tạo doubleType sử dụng trong parse.c

/******************* Type utilities ******************************/

Type* makeIntType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_INT;
  return type;
}

Type* makeCharType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_CHAR;
  return type;
}
// TODO:GK1
Type* makeStringType(void) {        // GK1: Tạo hàm makeStringType
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_STRING;
  return type;
}
Type* makeDoubleType(void) {        // GK1: Tạo hàm makeDoubleType
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_DOUBLE;
  return type;
}


Type* makeArrayType(int arraySize, Type* elementType) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_ARRAY;
  type->arraySize = arraySize;
  type->elementType = elementType;
  return type;
}

Type* duplicateType(Type* type) {
  Type* resultType = (Type*) malloc(sizeof(Type));
  resultType->typeClass = type->typeClass;
  if (type->typeClass == TP_ARRAY) {
    resultType->arraySize = type->arraySize;
    resultType->elementType = duplicateType(type->elementType);
  }
  return resultType;
}


int compareType(Type* type1, Type* type2) {
  if(DEBUG == 1) printf("Debug compareType: type1 => %d // type2 => %d\n", type1->typeClass, type2->typeClass);
  if (type1->typeClass == type2->typeClass) {
    if (type1->typeClass == TP_ARRAY) {
      if (type1->arraySize == type2->arraySize)
	      return compareType(type1->elementType, type2->elementType);
      else return 0;
    } else return 1;
  } else return 0; 
}

// TODO:GK3 
int compareTypeInAssignment(Type* type1, Type* type2) {
  if(DEBUG == 3) printf("Debug compareTypeInAssignment: type1 => %d // type2 => %d\n", type1->typeClass, type2->typeClass);
  if (type1->typeClass == type2->typeClass) {
    if (type1->typeClass == TP_ARRAY) {
      if (type1->arraySize == type2->arraySize)
	      return compareType(type1->elementType, type2->elementType);
      else return 0;
    } else return 1;
  } else {
    if((type1->typeClass == TP_DOUBLE && type2->typeClass == TP_INT)) {
      return 1;
    } else return 0;
  }
  
}
// TODO:GK3
int compareTypeInExpression(Type* type1, Type* type2) {
  if(DEBUG == 3) printf("Debug compareTypeInExpression: type1 => %d // type2 => %d\n", type1->typeClass, type2->typeClass);
  if (type1->typeClass == type2->typeClass) {
    if (type1->typeClass == TP_ARRAY) {
      if (type1->arraySize == type2->arraySize)
	      return compareType(type1->elementType, type2->elementType);
      else return 0;
    } else return 1;
  } else {
    if((type1->typeClass == TP_DOUBLE && type2->typeClass == TP_INT) || (type1->typeClass == TP_INT && type2->typeClass == TP_DOUBLE)) {
      return 1;
    } else return 0;
  }
  
}


// TODO:GK1
void freeType(Type* type) {
  switch (type->typeClass) {
    case TP_INT:
    case TP_CHAR:
    case TP_STRING:         // GK1: Free thêm 2 kiểu dữ liệu mới
    case TP_DOUBLE:         // GK1: Free thêm 2 kiểu dữ liệu mới
      free(type);
      break;
    case TP_ARRAY:
      freeType(type->elementType);
      freeType(type);
      break;
    }
}

/******************* Constant utility ******************************/

ConstantValue* makeIntConstant(int i) {
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  value->type = TP_INT;
  value->intValue = i;
  return value;
}

ConstantValue* makeCharConstant(char ch) {
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  value->type = TP_CHAR;
  value->charValue = ch;
  return value;
}

// TODO:GK1
ConstantValue* makeStringConstant(char str[MAX_LENGTH]) {    // GK1: MakeStringConstant
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  value->type = TP_STRING;
  strcpy(value->stringValue, str);
  if(DEBUG == 1) printf("Debug makeStringConstant: param => %s // constantValue => %s\n", str, value->stringValue);
  return value;
}
// TODO:GK1
ConstantValue* makeDoubleConstant(double d) {     // GK1: MakeDoubleConstant
  
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  value->type = TP_DOUBLE;
  value->doubleValue = d;
  if(DEBUG == 1) printf("Debug makeDoubleConstant: param => %f // constantValue => %f\n", d, value->doubleValue);
  return value;
}


// TODO:GK1
ConstantValue* duplicateConstantValue(ConstantValue* v) {   // GK1: Xu ly duplicate 2 kieu du lieu moi
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  value->type = v->type;
  if (v->type == TP_INT) {
    value->intValue = v->intValue;
    if(DEBUG == 1) printf("Debug duplicateConstantValue: param => %d // dup => %d\n", v->intValue, value->intValue);
  }
  else if (v->type == TP_CHAR) {
    value->charValue = v->charValue;
    if(DEBUG == 1) printf("Debug duplicateConstantValue: param => %c // dup => %c\n", v->charValue, value->charValue);
  }
  else if (v->type == TP_STRING) {
    strcpy(value->stringValue, v->stringValue);
    if(DEBUG == 1) printf("Debug duplicateConstantValue: param => %s // dup => %s\n", v->stringValue, value->stringValue);
  }
  else {
    value->doubleValue = v->doubleValue;
    if(DEBUG == 1) printf("Debug duplicateConstantValue: param => %f // dup => %f\n", v->doubleValue, value->doubleValue);
  }
    
  return value;
}

/******************* Object utilities ******************************/

Scope* createScope(Object* owner, Scope* outer) {
  Scope* scope = (Scope*) malloc(sizeof(Scope));
  scope->objList = NULL;
  scope->owner = owner;
  scope->outer = outer;
  return scope;
}

Object* createProgramObject(char *programName) {
  Object* program = (Object*) malloc(sizeof(Object));
  strcpy(program->name, programName);
  program->kind = OBJ_PROGRAM;
  program->progAttrs = (ProgramAttributes*) malloc(sizeof(ProgramAttributes));
  program->progAttrs->scope = createScope(program,NULL);
  symtab->program = program;

  return program;
}

Object* createConstantObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_CONSTANT;
  obj->constAttrs = (ConstantAttributes*) malloc(sizeof(ConstantAttributes));
  return obj;
}

Object* createTypeObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_TYPE;
  obj->typeAttrs = (TypeAttributes*) malloc(sizeof(TypeAttributes));
  return obj;
}

Object* createVariableObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_VARIABLE;
  obj->varAttrs = (VariableAttributes*) malloc(sizeof(VariableAttributes));
  obj->varAttrs->scope = symtab->currentScope;
  return obj;
}

Object* createFunctionObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_FUNCTION;
  obj->funcAttrs = (FunctionAttributes*) malloc(sizeof(FunctionAttributes));
  obj->funcAttrs->paramList = NULL;
  obj->funcAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createProcedureObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_PROCEDURE;
  obj->procAttrs = (ProcedureAttributes*) malloc(sizeof(ProcedureAttributes));
  obj->procAttrs->paramList = NULL;
  obj->procAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createParameterObject(char *name, enum ParamKind kind, Object* owner) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_PARAMETER;
  obj->paramAttrs = (ParameterAttributes*) malloc(sizeof(ParameterAttributes));
  obj->paramAttrs->kind = kind;
  obj->paramAttrs->function = owner;
  return obj;
}

void freeObject(Object* obj) {
  switch (obj->kind) {
  case OBJ_CONSTANT:
    free(obj->constAttrs->value);
    free(obj->constAttrs);
    break;
  case OBJ_TYPE:
    free(obj->typeAttrs->actualType);
    free(obj->typeAttrs);
    break;
  case OBJ_VARIABLE:
    free(obj->varAttrs->type);
    free(obj->varAttrs);
    break;
  case OBJ_FUNCTION:
    freeReferenceList(obj->funcAttrs->paramList);
    freeType(obj->funcAttrs->returnType);
    freeScope(obj->funcAttrs->scope);
    free(obj->funcAttrs);
    break;
  case OBJ_PROCEDURE:
    freeReferenceList(obj->procAttrs->paramList);
    freeScope(obj->procAttrs->scope);
    free(obj->procAttrs);
    break;
  case OBJ_PROGRAM:
    freeScope(obj->progAttrs->scope);
    free(obj->progAttrs);
    break;
  case OBJ_PARAMETER:
    freeType(obj->paramAttrs->type);
    free(obj->paramAttrs);
  }
  free(obj);
}

void freeScope(Scope* scope) {
  freeObjectList(scope->objList);
  free(scope);
}

void freeObjectList(ObjectNode *objList) {
  ObjectNode* list = objList;

  while (list != NULL) {
    ObjectNode* node = list;
    list = list->next;
    freeObject(node->object);
    free(node);
  }
}

void freeReferenceList(ObjectNode *objList) {
  ObjectNode* list = objList;

  while (list != NULL) {
    ObjectNode* node = list;
    list = list->next;
    free(node);
  }
}

void addObject(ObjectNode **objList, Object* obj) {
  ObjectNode* node = (ObjectNode*) malloc(sizeof(ObjectNode));
  node->object = obj;
  node->next = NULL;
  if ((*objList) == NULL) 
    *objList = node;
  else {
    ObjectNode *n = *objList;
    while (n->next != NULL) 
      n = n->next;
    n->next = node;
  }
}

// TODO:GK3
void addType(TypeNode** typeList, Type* type) {
  TypeNode* node = (TypeNode*) malloc(sizeof(TypeNode));
  node->type = type;
  node->next = NULL;
  if ((*typeList) == NULL) {
    *typeList = node;
  } else {
    TypeNode *l = *typeList;
    while (l->next != NULL) 
      l = l->next;
    l->next = node;
  }
}
// TODO:GK3
void freeTypeNode(TypeNode *typeList) {
  TypeNode* list = typeList;

  while (list != NULL) {
    TypeNode* node = list;
    list = list->next;
    freeType(node->type);
    free(node);
  }
}

// TODO:GK3
int countTypeNode(TypeNode *typeList) {
  TypeNode* list = typeList;
  int count = 0;
  while (list != NULL) {
    count++;
    list = list->next;
  }

  return count;
}


Object* findObject(ObjectNode *objList, char *name) {
  while (objList != NULL) {
    if (strcmp(objList->object->name, name) == 0) 
      return objList->object;
    else objList = objList->next;
  }
  return NULL;
}

/******************* others ******************************/

void initSymTab(void) {
  Object* obj;
  Object* param;

  symtab = (SymTab*) malloc(sizeof(SymTab));
  symtab->globalObjectList = NULL;
  
  obj = createFunctionObject("READC");
  obj->funcAttrs->returnType = makeCharType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createFunctionObject("READI");
  obj->funcAttrs->returnType = makeIntType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEI");
  param = createParameterObject("i", PARAM_VALUE, obj);
  param->paramAttrs->type = makeIntType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEC");
  param = createParameterObject("ch", PARAM_VALUE, obj);
  param->paramAttrs->type = makeCharType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITELN");
  addObject(&(symtab->globalObjectList), obj);


  intType = makeIntType();
  charType = makeCharType();
// TODO:GK1
  stringType = makeStringType();      // GK1: khoi tạo stringType
  doubleType = makeDoubleType();      // GK1: khoi tao doubleType
}

// TODO:GK1
void cleanSymTab(void) {
  freeObject(symtab->program);
  freeObjectList(symtab->globalObjectList);
  free(symtab);
  freeType(intType);
  freeType(charType);
  freeType(stringType);         // GK1: Free stringType
  freeType(doubleType);         // GK1: Free doubleType
}

void enterBlock(Scope* scope) {
  symtab->currentScope = scope;
}

void exitBlock(void) {
  symtab->currentScope = symtab->currentScope->outer;
}

void declareObject(Object* obj) {
  if (obj->kind == OBJ_PARAMETER) {
    Object* owner = symtab->currentScope->owner;
    switch (owner->kind) {
    case OBJ_FUNCTION:
      addObject(&(owner->funcAttrs->paramList), obj);
      break;
    case OBJ_PROCEDURE:
      addObject(&(owner->procAttrs->paramList), obj);
      break;
    default:
      break;
    }
  }
 
  addObject(&(symtab->currentScope->objList), obj);
}


