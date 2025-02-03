#include "../headers/operation.h"
#include "../headers/error.h"
#include "../headers/token.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

OperationClass* ALL_OPERATION_CLASSES = NULL;
uint32_t NUMBER_OF_OPERATION_CLASSES = 0;

ComparisonResult OperationInstance_compare(OperationInstance* first, OperationInstance* second) {
  if (first->nestingLevel != second->nestingLevel) {
    return convertToComparisonResult(first->nestingLevel - second->nestingLevel);
  }

  if (first->class_->positionType != second->class_->positionType) {
    if (first->class_->positionType == OPERATION_POSITION_TYPE__PREFIX) return COMPARISON_RESULT__GREATER;
    if (second->class_->positionType == OPERATION_POSITION_TYPE__PREFIX) return COMPARISON_RESULT__LESSER;
  }

  if (first->class_->positionType == OPERATION_POSITION_TYPE__PREFIX) {
    return COMPARISON_RESULT__EQUAL;
  }

  return convertToComparisonResult(first->class_->precedence - second->class_->precedence);
}

OperationInstance* OperationInstance_new(OperationClass* class, uint32_t nestingLevel, uint32_t fulfilledArity) {
  OperationInstance* operationInstance = (OperationInstance*) malloc(sizeof(OperationInstance));
  operationInstance->class_ = class;
  operationInstance->nestingLevel = nestingLevel;
  operationInstance->fulfilledArity = fulfilledArity;
  return operationInstance;
}

bool OperationInstance_isFulfilled(OperationInstance *self) {
  return self->fulfilledArity == self->class_->arity;
}

// !TODO Finish this function
number_t __OPERATION_CLASS__ADDITION__callback(
  TokenNode* node,
  ErrorInstance** error,
  uint32_t start,
  uint32_t end
) {
  long_number_t left = TokenNode_nthArgument(node, 2);
  long_number_t right = TokenNode_nthArgument(node, 1);
  long_number_t result = left + right;

  return Operation_catchFlows(result, error, start, end);
}

number_t __OPERATION_CLASS__SUBTRACTION__callback(
  TokenNode* node,
  ErrorInstance** error,
  uint32_t start,
  uint32_t end
) {
  long_number_t left = TokenNode_nthArgument(node, 2);
  long_number_t right = TokenNode_nthArgument(node, 1);
  long_number_t result = left - right;

  return Operation_catchFlows(result, error, start, end);
}

number_t __OPERATION_CLASS__MULTIPLICATION__callback(
  TokenNode* node,
  ErrorInstance** error,
  uint32_t start,
  uint32_t end
) {
  long_number_t left = TokenNode_nthArgument(node, 2);
  long_number_t right = TokenNode_nthArgument(node, 1);
  long_number_t result = left * right;

  return Operation_catchFlows(result, error, start, end);
}

number_t __OPERATION_CLASS__DIVISION__callback(
  TokenNode* node,
  ErrorInstance** error,
  uint32_t start,
  uint32_t end
) {
  long_number_t left = TokenNode_nthArgument(node, 2);
  long_number_t right = TokenNode_nthArgument(node, 1);

  if (right == 0) {
    *error = Error_rawMessageError(ERROR_CLASS__DIVISION_BY_ZERO, start, end);
    return OPERAND_CENTINEL;
  }

  return ((number_t) (left / right));
}

number_t __OPERATION_CLASS__EXPONENTIATION__callback(
  TokenNode* node,
  ErrorInstance** error,
  uint32_t start,
  uint32_t end
) {
  long_number_t left = TokenNode_nthArgument(node, 2);
  long_number_t right = TokenNode_nthArgument(node, 1);
  long_number_t result = (long_number_t) pow(left, right);

  return Operation_catchFlows(result, error, start, end);
}

#ifdef USE_DOUBLES_INTERNALLY

number_t __OPERATION_CLASS__SQUARE_ROOT__callback(
  TokenNode* node,
  ErrorInstance** error,
  uint32_t start,
  uint32_t end
) {
  long_number_t argument = TokenNode_nthArgument(node, 1);
  return (number_t) sqrt(argument);
}

#endif /* ifdef USE_DOUBLES_INTERNALLY */

uint32_t __OperationClass_getNumberOfOperationClasses() {
  return NUMBER_OF_OPERATION_CLASSES;
}

OperationClass* __OperationClass_getAllOperationClasses() {
  return ALL_OPERATION_CLASSES;
}

__OperationRepresentationEntry* __OperationClass_getAllOperationRepresentationEntries(
  bool* success, 
  int* entriesCount
) {
  if (!(*success)) return NULL;

  int numberOfEntries = 0;
  __OperationRepresentationEntry* representationEntries = NULL;

  for (int i = 0; i < NUMBER_OF_OPERATION_CLASSES; i++) {
    OperationClass* currentOperation = ALL_OPERATION_CLASSES + i;
    RESIZE_MEMORY(
      __OperationRepresentationEntry, 
      representationEntries, 
      success, 
      (numberOfEntries + currentOperation->numberOfRepresentations), 
      return NULL;
    );

    for (int j = 0; j < currentOperation->numberOfRepresentations; j++) {
      representationEntries[numberOfEntries + j].representation = currentOperation->representations + j;
      representationEntries[numberOfEntries + j].operationClass = currentOperation;
    }

    numberOfEntries += currentOperation->numberOfRepresentations;
  }

  *success = true;
  *entriesCount = numberOfEntries;
  return representationEntries;
}

OperationClass* __OperationClass_getOperationByName(char* name) {
  OperationClass* classes = __OperationClass_getAllOperationClasses();
  int classCount = __OperationClass_getNumberOfOperationClasses();

  for (int i = 0; i < classCount; i++) {
    OperationClass* class_ = classes + i;
    for (int j = 0; j < class_->numberOfRepresentations; j++) {
      CustomString* representation = class_->representations + j;
      if (strcmp(representation->data, name) == 0) {
        return class_;
      }
    }
  }

  return NULL;
}

void OperationClass_initialize(
  OperationClass *self, 
  uint32_t arity, 
  uint32_t precedence, 
  operation_callback_t callback, 
  OperationPositionType positionType, 
  uint32_t numberOfRepresentations, 
  CustomString *representations
) {
  self->arity = arity;
  self->precedence = precedence;
  self->callback = callback;
  self->positionType = positionType;
  self->numberOfRepresentations = numberOfRepresentations;
  self->representations = representations;
}

void __registerGlobalOperation(
  bool* success,
  uint32_t arity,
  uint32_t precedence,
  operation_callback_t callback,
  OperationPositionType positionType,
  uint32_t numberOfRepresentations,
  ...
) {
  // !TODO probably, there should be some sort of validation 
  // with the representations of the new operation to be 
  // registered, but for now I'm going to leave the inputs 
  // of this function unchecked because it's not really meant 
  // to be used by others yet
  
  if (!(*success)) return;

  *success = false;
  va_list representations;
  va_start(representations, numberOfRepresentations);

  RESIZE_MEMORY(
    OperationClass, 
    ALL_OPERATION_CLASSES, 
    *success, 
    NUMBER_OF_OPERATION_CLASSES + 1, 
    return;
  );

  OperationClass* newOperationClass = ALL_OPERATION_CLASSES + NUMBER_OF_OPERATION_CLASSES;

  OperationClass_initialize(
    newOperationClass, 
    arity, 
    precedence, 
    callback, 
    positionType, 
    numberOfRepresentations, 
    __CustomString_newArray(numberOfRepresentations, representations)
  );

  va_end(representations);

  NUMBER_OF_OPERATION_CLASSES++;
  *success = true;
}

void loadDefaultOperations(bool* success) {
  // ADDITION AND SUBTRACTION
  REGISTER_GLOBAL_OPERATION(
    success, 2, 1, &__OPERATION_CLASS__ADDITION__callback, 
    OPERATION_POSITION_TYPE__INFIX, 
    "+",
    "+=="
  );
  REGISTER_GLOBAL_OPERATION(
    success, 2, 1, &__OPERATION_CLASS__ADDITION__callback,
    OPERATION_POSITION_TYPE__PREFIX,
    "add",
    "addition",
    "sum",
    "suma"
  );

  REGISTER_GLOBAL_OPERATION(
    success, 2, 1, &__OPERATION_CLASS__SUBTRACTION__callback, 
    OPERATION_POSITION_TYPE__INFIX, 
    "-"
  );
  REGISTER_GLOBAL_OPERATION(
    success, 2, 1, &__OPERATION_CLASS__SUBTRACTION__callback,
    OPERATION_POSITION_TYPE__PREFIX,
    "sub",
    "subtraction",
    "resta"
  );

  // MULTIPLICATION AND DIVISION
  REGISTER_GLOBAL_OPERATION(
    success, 2, 2, &__OPERATION_CLASS__MULTIPLICATION__callback, 
    OPERATION_POSITION_TYPE__INFIX, 
    "*"
  );
  REGISTER_GLOBAL_OPERATION(
    success, 2, 2, &__OPERATION_CLASS__MULTIPLICATION__callback,
    OPERATION_POSITION_TYPE__PREFIX,
    "mul",
    "multiplication",
    "multiplicacion"
  );

  REGISTER_GLOBAL_OPERATION(
    success, 2, 2, &__OPERATION_CLASS__DIVISION__callback, 
    OPERATION_POSITION_TYPE__INFIX, 
    "/"
  );
  REGISTER_GLOBAL_OPERATION(
    success, 2, 2, &__OPERATION_CLASS__DIVISION__callback,
    OPERATION_POSITION_TYPE__PREFIX,
    "div",
    "division"
  );

  // EXPONENTIATION AND SQUARE ROOT
  REGISTER_GLOBAL_OPERATION(
    success, 2, 3, &__OPERATION_CLASS__EXPONENTIATION__callback, 
    OPERATION_POSITION_TYPE__INFIX, 
    "**"
  );
  REGISTER_GLOBAL_OPERATION(
    success, 2, 3, &__OPERATION_CLASS__EXPONENTIATION__callback,
    OPERATION_POSITION_TYPE__PREFIX,
    "exp",
    "exponentiation",
    "exponenciacion"
  );

  #ifdef USE_DOUBLES_INTERNALLY
    REGISTER_GLOBAL_OPERATION(
      success, 1, 3, &__OPERATION_CLASS__SQUARE_ROOT__callback,
      OPERATION_POSITION_TYPE__PREFIX,
      "sqrt",
      "squareRoot",
      "raizCuadrada"
    );
  #endif /* ifdef USE_DOUBLES_INTERNALLY */
}

number_t Operation_catchFlows(number_t value, ErrorInstance **error, uint32_t start, uint32_t end) {
  bool isError = false; 
  ErrorClassIdentifier errorClassIdentifier; 
  if (value > NUMBER_MAX) { 
    isError = true; 
    errorClassIdentifier = ERROR_CLASS__INPUT_INTEGER_TOO_LARGE; 
  } else if (value < NUMBER_MIN) { 
    isError = true; 
    errorClassIdentifier = ERROR_CLASS__INPUT_INTEGER_TOO_SMALL; 
  } 

  if (isError) { 
    *error = Error_inputExcerptError( errorClassIdentifier, start, end ); 
    return OPERAND_CENTINEL; 
  } 

  return value;
}

