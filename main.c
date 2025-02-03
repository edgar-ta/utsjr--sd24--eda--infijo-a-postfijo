#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "./headers/definitions.h"
#include "./headers/token.h"
#include "./headers/operation.h"
#include "./headers/error.h"
#include "./headers/struct-declarations.h"
#include "./headers/main.h"

ComparisonResult convertToComparisonResult(int value) {
  if (value < 0) return COMPARISON_RESULT__LESSER;
  if (value > 0) return COMPARISON_RESULT__GREATER;
  return COMPARISON_RESULT__EQUAL;
}

bool isOperationValid(OperationInstance *currentOperation, OperationInstance *nextOperation, int nestingLevel) {
  bool isUnique = currentOperation == NULL || currentOperation->nestingLevel != nestingLevel;
  if (isUnique) return true;

  if (nextOperation->class_->positionType == OPERATION_POSITION_TYPE__PREFIX) {
    return !OperationInstance_isFulfilled(currentOperation);
  } else if (nextOperation->class_->positionType == OPERATION_POSITION_TYPE__INFIX) {
    return OperationInstance_isFulfilled(currentOperation);
  }

  return true;
}

bool isWhitespace(char character) {
  return 
    character == ' ' || 
    character == '\n' || 
    character == '\t' || 
    character == '\v'
  ;
}

bool isDigit(char character) {
  return isdigit(character);
}

bool isLetter(char character) {
  return isalpha(character);
}

bool isLetterOrDigit(char character) {
  return isalnum(character);
}

bool isSymbol(char character) {
  return ispunct(character);
}

bool isParenthesis(char character) {
  return character == '(' || character == ')';
}

void skipWhitespace(char* input, uint32_t* cursor) {
  while (isWhitespace(input[*cursor])) {
    (*cursor)++;
  }
}

void skipLetters(char* input, uint32_t* cursor) {
  while (isLetter(input[*cursor])) {
    (*cursor)++;
  }
}

void skipAll(char* input, uint32_t* cursor, bool (*identityFunction)(char)) {
  while (identityFunction(input[*cursor])) {
    (*cursor)++;
  }
}

int parseDigit(bool* success, char character) {
  *success = true;
  switch (character) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    default: 
      *success = false;
      return -1;
  }
}

Token* parseNumber(bool* success, char* input, uint32_t* cursor, ErrorInstance** error) {
  if (!(*success)) return NULL;

  int initialCursor = *cursor;
  bool isPositiveNumber = true;

  if (input[*cursor] == '-') {
    isPositiveNumber = false;
    (*cursor)++;
  }

  long_number_t value = parseDigit(success, input[*cursor]);
  if (!(*success)) {
    if (!isPositiveNumber) {
      *error = Error_rawMessageError(
        ERROR_CLASS__NO_DIGITS_AFTER_SIGN, 
        initialCursor, 
        *cursor
      );
    }
    return NULL;
  }
  (*cursor)++;

  bool isError = false;
  ErrorClassIdentifier errorClassIdentifier;

  while (true) {
    char currentCharacter = input[*cursor];
    if (currentCharacter == '\0') break;

    int currentCharacterValue = parseDigit(success, currentCharacter);
    if (!(*success)) break;

    value = value * 10;
    if (isPositiveNumber) 
         value += currentCharacterValue;
    else value -= currentCharacterValue;

    number_t castedValue = (number_t) value;

    if (isPositiveNumber) {
      if (((long_number_t) castedValue) < value) {
        isError = true;
        errorClassIdentifier = ERROR_CLASS__INPUT_INTEGER_TOO_LARGE;
      }
    } else {
      if (((long_number_t) castedValue) > value) {
        isError = true;
        errorClassIdentifier = ERROR_CLASS__INPUT_INTEGER_TOO_SMALL;
      }
    }

    if (isError) {
      skipAll(input, cursor, &isDigit);
      *success = false;
      *error = Error_inputExcerptError(
        errorClassIdentifier, 
        initialCursor,
        *cursor
      );
      return NULL;
    }

    (*cursor)++;
  }

  *success = true;
  return Token_newOperand(initialCursor, *cursor, value);
}

Token* __parseOperation(
  bool* success,
  char* input,
  uint32_t* cursor, 
  ErrorInstance** error,
  int numberOfRepresentations,
  __OperationRepresentationEntry** possibleRepresentations
) {
  if (!(*success)) return NULL;
  *success = false;

  uint32_t initialCursor = *cursor;
  int previouslyFoundResults = numberOfRepresentations;
  int charactersFound = 0;
  bool (*identityFunction)(char input);

  char currentCharacter = input[*cursor];

  if (isLetter(currentCharacter)) {
    identityFunction = &isLetterOrDigit;
  } else if (isSymbol(currentCharacter)) {
    identityFunction = &isSymbol;
  } else {
    // This case is not an error necessarily; the conditions to reach 
    // this if statement imply that the first character of the next 
    // section of the input is not a letter, which means that the 
    // user could have typed something other than an operation (i. e.,
    // the input is not necessarily wrong because it isn't necessarily
    // meant to represent an operation)
    return NULL;
  }

  do {
    int resultsFound = 0;

    for (int i = 0; i < previouslyFoundResults; i++) {
      __OperationRepresentationEntry* currentRepresentation = (*possibleRepresentations) + i;

      if (charactersFound >= currentRepresentation->representation->length) {
        continue;
      }

      char characterInRepresentation = currentRepresentation->representation->data[charactersFound];
      if (characterInRepresentation == currentCharacter) {
        (*possibleRepresentations)[resultsFound] = *currentRepresentation;
        resultsFound++;
      }
    }

    if (resultsFound == 0) {
      skipAll(input, cursor, identityFunction);

      if (charactersFound == 0) {
        *error = Error_inputExcerptError(
          ERROR_CLASS__NOT_AN_OPERATION,
          initialCursor, 
          *cursor
        );
      } else {
        *error = Error_notAnOperationWithSuggestions(
          initialCursor,
          *cursor, 
          previouslyFoundResults, 
          *possibleRepresentations
        );
      }
      return NULL;
    }

    previouslyFoundResults = resultsFound;
    charactersFound++;

    RESIZE_MEMORY(
      __OperationRepresentationEntry, 
      *possibleRepresentations, 
      *success, 
      resultsFound, 
      UNRECOVERABLE_ERROR("No memory available to parse the given operation");
    );

    currentCharacter = input[*cursor + charactersFound];

  } while (identityFunction(currentCharacter));

  // It is guaranteed that by reaching this part of the code, there exists at least 
  // one valid representation entry that corresponds to what the user could have typed 
  __OperationRepresentationEntry* finalEntry = (*possibleRepresentations) + 0;

  // In case there is more than one possible entry at the end, select the shortest one
  if (previouslyFoundResults > 1) {
    for (int i = 1; i < previouslyFoundResults; i++) {
      __OperationRepresentationEntry* entry = (*possibleRepresentations) + i;
      if (entry->representation->length < finalEntry->representation->length) {
        finalEntry = entry;
      }
    }
  }

  *cursor += charactersFound;

  if (finalEntry->representation->length != charactersFound) {
    *error = Error_identifierTooShort(
      initialCursor,
      *cursor,
      finalEntry->representation->data
    );
    return NULL;
  }

  Token* operationToken = Token_newOperation(
    initialCursor,
    *cursor, 
    OperationInstance_new(finalEntry->operationClass, 0, 0)
  );
  *success = true;

  return operationToken;
}

Token* parseOperation(
  bool* success, 
  char* input, 
  uint32_t* cursor, 
  ErrorInstance** error
) {
  int numberOfRepresentations = 0;
  __OperationRepresentationEntry* representations = __OperationClass_getAllOperationRepresentationEntries(
    success, 
    &numberOfRepresentations
  );

  Token* token = __parseOperation(
    success, 
    input, 
    cursor, 
    error, 
    numberOfRepresentations,
    &representations
  );
  free(representations);
  return token;
}

void __onArgumentBonusOverflow(ErrorInstance** error) {
  // !TODO Throw an actual error
  *error = NULL;
}

void __transferArgumentBonus(
  Token** currentOperation, 
  Token** previousOperation, 
  TokenStack* tokenStack,
  int nestingLevel,
  int* argumentBonus,
  Token* argument,
  ErrorInstance** error
) {
  if (*currentOperation != NULL) {
    if ((*currentOperation)->operation->nestingLevel == nestingLevel && (*argumentBonus) > 0) {
      (*currentOperation)->operation->fulfilledArity += *argumentBonus;
      argumentBonus = 0;
    }
  }

  if (argument != NULL) {
    TokenStack_pushOperand(tokenStack, argument, *currentOperation);
  }

  if (*currentOperation != NULL) {
    if (OperationInstance_isFulfilled((*currentOperation)->operation)) {
      (*argumentBonus)++;
      if ((*argumentBonus) >= 2) {
        __onArgumentBonusOverflow(error);
        return;
      }
      *previousOperation = *currentOperation;
      *currentOperation = TokenStack_findNextUnfulfilledOperation(tokenStack);
    }
  }
}

void parseExpression(
  bool* success, 
  char* input, 
  uint32_t* cursor, 
  TokenStack* tokenStack,
  ErrorInstance** error
) {
  if (!(*success)) return;

  Token* previousOperation = NULL;
  Token* currentOperation = NULL;
  int argumentBonus = 0;
  int nestingLevel = 0;
  uint32_t startOfParentheses = 0;

  Token* currentToken = NULL;

  while (true) {
    skipWhitespace(input, cursor);

    if (input[*cursor] == '\0') {
      break;
    }

    if (isParenthesis(input[*cursor])) {
      if (input[*cursor] == '(') { 
        if (nestingLevel == 0) {
          startOfParentheses = *cursor;
        }
        (*cursor)++;
        nestingLevel++; 
        continue;
      } else { 
        (*cursor)++;
        if (nestingLevel == 0) {
          *error = Error_rawMessageError(ERROR_CLASS__TOO_MANY_CLOSING_PARENTHESES, startOfParentheses, *cursor);
          return;
        }
        nestingLevel--; 
        goto __argument_bonus_check;
      }
    }

    int previousCursor = *cursor;
    currentToken = parseNumber(success, input, cursor, error);

    if (*error != NULL) return; 
    if (*success) {
      argumentBonus++;
      if (argumentBonus >= 2) {
        if (previousOperation != NULL) {
          previousOperation->operation->fulfilledArity += argumentBonus - 1;
          *error = Error_otherOperandsThanNeededError(
            previousOperation->start, 
            *cursor, 
            previousOperation
          );
        } else {
          *error = Error_rawMessageError(ERROR_CLASS__TOO_MANY_FREE_ARGUMENTS, 0, *cursor);
        }
        return;
      }
      TokenStack_pushOperand(tokenStack, currentToken, currentOperation);
      goto __argument_bonus_check;
    }

    *success = true;
    *cursor = previousCursor;

    currentToken = parseOperation(success, input, cursor, error);
    if (*error != NULL) return; 
    if (*success) {
      currentToken->operation->nestingLevel = nestingLevel;
      bool isOperationValid_ = isOperationValid(
        currentOperation? currentOperation->operation: NULL, 
        currentToken? currentToken->operation: NULL, 
        nestingLevel
      );
      if (isOperationValid_) {
        if (currentToken->operation->class_->positionType == OPERATION_POSITION_TYPE__PREFIX) {
          TokenStack_pushOperand(tokenStack, currentToken, currentOperation);
        } else if (currentToken->operation->class_->positionType == OPERATION_POSITION_TYPE__INFIX) {
          TokenStack_pushOperation(tokenStack, currentToken);
        }
      } else {
        uint32_t startIndex = 0;
        if (previousOperation != NULL) {
          startIndex = previousOperation->start;
        }

        *error = Error_operationNotAdmitted(
          startIndex, 
          currentToken->end, 
          currentToken->start, 
          currentToken->end, 
          currentToken->operation->class_->positionType
        );
        return;
      }

      previousOperation = currentOperation;
      currentOperation = currentToken;

      goto __argument_bonus_check;
    }

    *error = Error_rawMessageError(
      ERROR_CLASS__EXPRESSION_NOT_RECOGNIZED, 
      previousCursor, 
      *cursor
    );
    return;

__argument_bonus_check:
    if (currentOperation != NULL) {
      int missingArity = currentOperation->operation->class_->arity - currentOperation->operation->fulfilledArity;
      if (currentOperation->operation->nestingLevel == nestingLevel && argumentBonus > missingArity) {
        UNRECOVERABLE_ERROR("Too many arguments for operation");
      }
      if (currentOperation->operation->nestingLevel == nestingLevel && argumentBonus > 0) {
        currentOperation->operation->fulfilledArity += argumentBonus;
        argumentBonus = 0;
      }
      if (OperationInstance_isFulfilled(currentOperation->operation)) {
        argumentBonus++; // i. e., argumentBonus = 1
        previousOperation = currentOperation;
        currentOperation = TokenStack_findNextUnfulfilledOperation(tokenStack);
      }
    }
  }

  if (tokenStack->head == NULL) {
    *error = Error_rawMessageError(ERROR_CLASS__EMPTY_EXPRESSION, 0, *cursor);
    return;
  }

  if (nestingLevel > 0) {
    *error = Error_tooFewClosingParentheses(startOfParentheses, *cursor, nestingLevel);
    return;
  }

  if (currentOperation != NULL && !OperationInstance_isFulfilled(currentOperation->operation)) {
    *error = Error_otherOperandsThanNeededError(
      currentOperation->start, 
      *cursor, 
      currentOperation
    );
    return;
  }
}

number_t __evaluateExpression(
  TokenStack* tokenStack, 
  uint32_t lastIndex, 
  TokenNode*** orderedOperations, 
  ErrorInstance** error
)  {
  for (int i = 0; i < tokenStack->numberOfOperations; i++) {
    TokenNode** lastOperationReference = orderedOperations[i];

    TokenNode* slipknot = *lastOperationReference;
    TokenNode* tail = NULL;

    int i = 0;
    uint32_t intendedArity = (*lastOperationReference)->token->operation->class_->arity;
    while (i < intendedArity) {
      if (slipknot->next == NULL) break;
      slipknot = slipknot->next; 
      i++;
    }

    tail = slipknot->next;
    slipknot->next = NULL;

    uint32_t start = 0;
    uint32_t end = 0;
    TokenNode_findLargestInputRange(*lastOperationReference, &start, &end);

    if (i != intendedArity) {
      *error = Error_otherOperandsThanNeededError(start, end, (*lastOperationReference)->token);
      slipknot->next = tail;
      return OPERAND_CENTINEL;
    }

    number_t result = (*lastOperationReference)->token->operation->class_->callback(
      *lastOperationReference, 
      error,
      start,
      end
    );

    if (*error) {
      return OPERAND_CENTINEL;
    }

    TokenNode* resultNode = TokenNode_new(Token_newOperand(start, end, result), tail);
    TokenNode_deleteChain(*lastOperationReference);
    *lastOperationReference = resultNode;

  }
  return tokenStack->head->token->number;
}

number_t evaluateExpression(TokenStack *tokenStack, uint32_t lastIndex, ErrorInstance** error) {
  if (tokenStack->head == NULL) {
    *error = Error_rawMessageError(ERROR_CLASS__EMPTY_EXPRESSION, 0, lastIndex);
    return OPERAND_CENTINEL;
  }

  TokenNode*** orderedOperations = TokenStack_getOrderedOperations(tokenStack);
  number_t result = __evaluateExpression(tokenStack, lastIndex, orderedOperations, error);
  free(orderedOperations);
  return result;
}

int mainExecution(bool* success, uint32_t* cursor, char* input, ErrorInstance** error);

int main(int argumentCount, char** argumentVector) {
  bool success = true;
  uint32_t cursor = 0;
  ErrorInstance* error = NULL;
  char* input = argumentVector[1];

  if (argumentCount < 2) {
    UNRECOVERABLE_ERROR("Wrong usage; supply some input to the program");
  }

  loadDefaultOperations(&success);
  if (!success) {
    UNRECOVERABLE_ERROR("Couldn't load the default operations of the program; exiting immediately");
  }

  int result = mainExecution(&success, &cursor, input, &error);

  if (error != NULL) {
    ErrorInstance_printMessage(error, input);
  } else if (!success) {
    printf("Something went wrong with the program");
  }

  return result;
}

int mainExecution(bool* success, uint32_t* cursor, char* input, ErrorInstance** error) {
  TokenStack* tokenStack = TokenStack_new();
  parseExpression(success, input, cursor, tokenStack, error);
  if (*error != NULL) {
    return EXIT_FAILURE;
  }

  printf("Despues del analisis gramatical, la estructura de la operacion es la siguiente: \n");
  TokenStack_print(tokenStack);
  printf("\n");

  number_t result = evaluateExpression(tokenStack, *cursor, error);
  if (*error != NULL) {
    return EXIT_FAILURE;
  }

  printf("Despues de la evaluacion interna, el resultado de la expresion es el siguiente: \n");
  PRINT_NUMBER_T(result);
  printf("\n");

  return EXIT_SUCCESS;
}

