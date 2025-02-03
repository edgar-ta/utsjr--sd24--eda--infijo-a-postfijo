#ifndef __OPERATION_H__
#define __OPERATION_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "./definitions.h"
#include "./struct-declarations.h"
#include "./custom-string.h"

enum OperationPositionType {
  OPERATION_POSITION_TYPE__INFIX,
  OPERATION_POSITION_TYPE__PREFIX
};

typedef enum OperationPositionType OperationPositionType;
typedef number_t (*operation_callback_t)(
  TokenNode* node,
  ErrorInstance** error,
  uint32_t start,
  uint32_t end
);

struct OperationClass {
  uint32_t arity;
  uint32_t precedence;

  uint32_t numberOfRepresentations;
  CustomString* representations; 

  operation_callback_t callback;
  enum OperationPositionType positionType;
};

struct OperationInstance {
  struct OperationClass* class_;

  uint32_t fulfilledArity;
  uint32_t nestingLevel;
};

// This object is used in the functions that 
// handle the logic to parse operations

struct __OperationRepresentationEntry {
  OperationClass* operationClass;
  CustomString* representation;
};

void OperationClass_initialize(
  OperationClass* self,
  uint32_t arity,
  uint32_t precedence,
  operation_callback_t callback,
  OperationPositionType positionType,
  uint32_t numberOfRepresentations,
  CustomString* representations
);

ComparisonResult OperationInstance_compare(OperationInstance* first, OperationInstance* second);
OperationInstance* OperationInstance_new(OperationClass* class_, uint32_t nestingLevel, uint32_t fulfilledArity);
bool OperationInstance_isFulfilled(OperationInstance* self);

uint32_t __OperationClass_getNumberOfOperationClasses();
OperationClass* __OperationClass_getAllOperationClasses();
__OperationRepresentationEntry* __OperationClass_getAllOperationRepresentationEntries(bool* success, int* entriesCount);
OperationClass* __OperationClass_getOperationByName(char* name);

number_t Operation_catchFlows(number_t value, ErrorInstance** error, uint32_t start, uint32_t end);

number_t __OPERATION_CLASS__ADDITION__callback(TokenNode* node, ErrorInstance** error, uint32_t start, uint32_t end);
number_t __OPERATION_CLASS__SUBTRACTION__callback(TokenNode* node, ErrorInstance** error, uint32_t start, uint32_t end);
number_t __OPERATION_CLASS__MULTIPLICATION__callback(TokenNode* node, ErrorInstance** error, uint32_t start, uint32_t end);
number_t __OPERATION_CLASS__DIVISION__callback(TokenNode* node, ErrorInstance** error, uint32_t start, uint32_t end);
number_t __OPERATION_CLASS__EXPONENTIATION__callback(TokenNode* node, ErrorInstance** error, uint32_t start, uint32_t end);

#ifdef USE_DOUBLES_INTERNALLY
  number_t __OPERATION_CLASS__SQUARE_ROOT__callback(TokenNode* node, ErrorInstance** error, uint32_t start, uint32_t end);
#endif // DEBUG

// Function that loads a new operation class to 
// make available for use by the whole program; 
// it takes a variable number of representations 
// (i. e., `char*`)
void __registerGlobalOperation(
  bool* success,
  uint32_t arity,
  uint32_t precedence,
  operation_callback_t callback,
  OperationPositionType positionType,
  uint32_t numberOfRepresentations,
  ...
);

#define REGISTER_GLOBAL_OPERATION(success, arity, precedence, callback, positionType, ...) __registerGlobalOperation(\
  success,\
  arity,\
  precedence,\
  callback,\
  positionType,\
  COUNT_ARGUMENTS_UP_TO_10(__VA_ARGS__),\
  __VA_ARGS__\
)

void loadDefaultOperations(bool* success);

#endif // !__OPERATION_H__

