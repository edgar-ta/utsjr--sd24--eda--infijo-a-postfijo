#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdint.h>
#include <stdbool.h>
#include "./definitions.h"
#include "./struct-declarations.h"
#include "operation.h"

enum ErrorPrintingStyle {
  ERROR_STYLE__RAW_MESSAGE,
  ERROR_STYLE__INPUT_EXCERPT_MESSAGE,
  ERROR_STYLE__CUSTOM_FUNCTION
};

enum ErrorClassIdentifier {
  ERROR_CLASS__NO_DIGITS_AFTER_SIGN,
  ERROR_CLASS__INPUT_INTEGER_TOO_LARGE,
  ERROR_CLASS__INPUT_INTEGER_TOO_SMALL,
  ERROR_CLASS__EXPRESSION_NOT_RECOGNIZED,
  ERROR_CLASS__EMPTY_EXPRESSION,
  ERROR_CLASS__OPERATION_WHEN_OPERAND_EXPECTED,
  ERROR_CLASS__OTHER_OPERANDS_THAN_NEEDED,
  ERROR_CLASS__NOT_AN_OPERATION,
  ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS,
  ERROR_CLASS__IDENTIFIER_TOO_SHORT,
  ERROR_CLASS__TOO_MANY_CLOSING_PARENTHESES,
  ERROR_CLASS__OPERATION_NOT_ADMITTED,
  ERROR_CLASS__TOO_FEW_CLOSING_PARENTHESES,
  ERROR_CLASS__TOO_MANY_FREE_ARGUMENTS,
  ERROR_CLASS__DIVISION_BY_ZERO
};

typedef enum ErrorPrintingStyle ErrorPrintingStyle;
typedef enum ErrorClassIdentifier ErrorClassIdentifier;

struct ErrorClass {
  union {
    char* formatString;
    void (*printingFunction)(struct ErrorInstance* self, char* input);
  };

  ErrorClassIdentifier identifier;
  ErrorPrintingStyle style;
};

// Data structure that represents an error instance of the 
// associated class; it owns the token it references
struct ErrorInstance {
  struct ErrorClass* class_;
  uint32_t start;
  uint32_t end;
  void* payload;
};

// Data structure that's used by the `OperationNotAdmitted` 
// error class
struct __Error_OperationNotAdmittedPayload {
  uint32_t start;
  uint32_t end;
  OperationPositionType position;
};

struct __Error_OtherOperandsThanNeededPayload {
  uint32_t start;
  uint32_t end;
  uint32_t expectedArity;
  uint32_t providedArity;
};

void __ERROR_CLASS__printMessageWithRawMessage(ErrorInstance* self);
void __ERROR_CLASS__printMessageWithInputExcerpt(ErrorInstance* self, char* input);


ErrorClass* __ErrorClass_getClassByIdentifier(ErrorClassIdentifier identifier);

ErrorInstance* __Error_new(uint32_t startIndex, uint32_t endIndex, ErrorClass* errorClass);

ErrorInstance* Error_rawMessageError(
  ErrorClassIdentifier classIdentifier, 
  uint32_t startIndex, 
  uint32_t endIndex
);
ErrorInstance* Error_inputExcerptError(
  // Pointer to static object
  ErrorClassIdentifier classIdentifier, 
  uint32_t startIndex, 
  uint32_t endIndex
);

// Function that creates a new error of type 'OtherOperandsThanNeeded';
// it takes ownership of the token that it receives
ErrorInstance* Error_otherOperandsThanNeededError(
  uint32_t startIndex, 
  uint32_t endIndex, 
  Token* operation
);
void __ERROR_CLASS__OTHER_OPERANDS_THAN_NEEDED__printingFunction(ErrorInstance* self, char* input);

ErrorInstance* Error_notAnOperationWithSuggestions(
  uint32_t startIndex,
  uint32_t endIndex,
  int numberOfRepresentations,
  __OperationRepresentationEntry* representations
);
void __ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS__printingFunction(ErrorInstance* self, char* input);

ErrorInstance* Error_identifierTooShort(
  uint32_t startIndex,
  uint32_t endIndex,
  char* suggestion
);
void __ERROR_CLASS__IDENTIFIER_TOO_SHORT__printingFunction(ErrorInstance* self, char* input);

ErrorInstance* Error_operationNotAdmitted(
  uint32_t startIndex,
  uint32_t endIndex,
  uint32_t startNameOfOperation,
  uint32_t endNameOfOperation,
  OperationPositionType operationPositionType
);
void __ERROR_CLASS__OPERATION_NOT_ADMITTED__printingFunction(ErrorInstance* self, char* input);

ErrorInstance* Error_tooFewClosingParentheses(
  uint32_t startIndex,
  uint32_t endIndex,
  int missingAmount
);
void __ERROR_CLASS__TOO_FEW_CLOSING_PARENTHESES__printingFunction(ErrorInstance* self, char* input);

void ErrorInstance_printMessage(ErrorInstance* self, char* input);
void ErrorInstance_destroy(ErrorInstance* self);

#endif // !__ERROR_H__
