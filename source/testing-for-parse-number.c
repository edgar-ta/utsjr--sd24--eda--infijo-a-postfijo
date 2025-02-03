#include "../headers/testing.h"

#include "../headers/main.h"

TEST_CASE_LITERAL(
  parseNumber, 
  failWhenParsingEmptyString, 
  "Fail when parsing empty string", 
  "", 

  success == false &&
  cursor == 0 && 
  error == NULL && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseNumber, 
  failWhenParsingNonDigits, 
  "Fail when parsing non-digits", 
  "+*&", 

  success == false &&
  cursor == 0 && 
  error == NULL && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseNumber, 
  errorWhenParsingIntegerTooLarge, 
  "Error when parsing integer too large", 
  "9876543210", 

  success == false &&
  cursor == 10 && 
  error->class_->identifier == ERROR_CLASS__INPUT_INTEGER_TOO_LARGE && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseNumber, 
  errorWhenParsingIntegerTooSmall, 
  "Error when parsing integer too small", 
  "-9876543210", 

  success == false &&
  cursor == 11 && 
  error->class_->identifier == ERROR_CLASS__INPUT_INTEGER_TOO_SMALL && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseNumber, 
  errorWhenNoDigitsAfterSign, 
  "Error when no digits after sign", 
  "-", 

  success == false &&
  cursor == 1 && 
  error->class_->identifier == ERROR_CLASS__NO_DIGITS_AFTER_SIGN && 
  token == NULL
);

TEST_CASE_LITERAL(
  parseNumber, 
  successWithPositiveNumber, 
  "Success with positive number", 
  "54321", 

  success == true &&
  cursor == 5 && 
  error == NULL &&
  token->number == 54321
);

TEST_CASE_LITERAL(
  parseNumber, 
  successWithNegativeNumber, 
  "Success with negative number", 
  "-54321", 

  success == true &&
  cursor == 6 && 
  error == NULL &&
  token->number == -54321
);


void testParseNumber() {
  RUN_TESTS(
    "parseNumber", 
    &parseNumber, 
    &parseNumber__failWhenParsingNonDigits,
    &parseNumber__failWhenParsingEmptyString,
    &parseNumber__errorWhenNoDigitsAfterSign,
    &parseNumber__errorWhenParsingIntegerTooLarge,
    &parseNumber__errorWhenParsingIntegerTooSmall,
    &parseNumber__successWithNegativeNumber,
    &parseNumber__successWithPositiveNumber
  );
}

