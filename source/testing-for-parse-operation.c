#include "../headers/testing.h"

#include "../headers/error.h"
#include "../headers/token.h"
#include "../headers/main.h"
#include "../headers/operation.h"

TEST_CASE_LITERAL(
  parseOperation, 
  failWhenParsingEmptyString, 
  "Fail when parsing empty string", 
  "",

  success == false &&
  cursor == 0 && 
  error == NULL && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseOperation, 
  failWhenParsingWhitespace, 
  "Fail when parsing whitespace", 
  " ",

  success == false &&
  cursor == 0 && 
  error == NULL && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseOperation, 
  errorWhenParsingIdentifierTooShortLetters, 
  "Error when parsing identifier too short letters", 
  "ad",

  success == false &&
  cursor == 2 && 
  error->class_->identifier == ERROR_CLASS__IDENTIFIER_TOO_SHORT && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseOperation, 
  errorWhenParsingIdentifierTooShortSymbols, 
  "Error when parsing identifier too short symbols", 
  "+=",

  success == false &&
  cursor == 2 && 
  error->class_->identifier == ERROR_CLASS__IDENTIFIER_TOO_SHORT && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseOperation, 
  errorWhenParsingWrongTextIdentifierWithSuggestions, 
  "Error when parsing wrong text identifier with suggestions", 
  "adx",

  success == false &&
  cursor == 3 && 
  error->class_->identifier == ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseOperation, 
  errorWhenParsingWrongSymbolsIdentifierWithSuggestions, 
  "Error when parsing wrong symbols identifier with suggestions", 
  "+=+",

  success == false &&
  cursor == 3 && 
  error->class_->identifier == ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseOperation, 
  errorWhenParsingIdentifierThatDoesNotMatchAnyOperation, 
  "Error when parsing identifier that does not match any operation", 
  "?",

  success == false &&
  cursor == 1 && 
  error->class_->identifier == ERROR_CLASS__NOT_AN_OPERATION && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseOperation, 
  successWithExistingFunctionLetters, 
  "Success with existing function letters", 
  "add",

  success == true &&
  cursor == 3 && 
  error == NULL && 
  token->operation->class_ == __OperationClass_getOperationByName("add")
);

TEST_CASE_LITERAL(
  parseOperation, 
  successWithExistingFunctionSymbols, 
  "Success with existing function symbols", 
  "+",

  success == true &&
  cursor == 1 && 
  error == NULL && 
  token->operation->class_ == __OperationClass_getOperationByName("+")
);

// Function to run test cases
void testParseOperation() {
  RUN_TESTS(
    "parseOperation", 
    &parseOperation,
    &parseOperation__failWhenParsingEmptyString,
    &parseOperation__failWhenParsingWhitespace,
    &parseOperation__errorWhenParsingIdentifierTooShortLetters,
    &parseOperation__errorWhenParsingIdentifierTooShortSymbols,
    &parseOperation__errorWhenParsingWrongTextIdentifierWithSuggestions,
    &parseOperation__errorWhenParsingWrongSymbolsIdentifierWithSuggestions,
    &parseOperation__errorWhenParsingIdentifierThatDoesNotMatchAnyOperation,
    &parseOperation__successWithExistingFunctionLetters,
    &parseOperation__successWithExistingFunctionSymbols
  );
}
