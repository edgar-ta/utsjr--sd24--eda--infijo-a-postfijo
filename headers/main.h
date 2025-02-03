#ifndef __MAIN_H__

#include "./struct-declarations.h"
#include "./definitions.h"
#include "./token.h"
#include "./error.h"

#include <stdint.h>

ComparisonResult convertToComparisonResult(int value);

bool isOperationValid(
  OperationInstance* currentOperation, 
  OperationInstance* nextOperation, 
  int nestingLevel
);

bool isWhitespace(char character);
bool isDigit(char character);
bool isLetter(char character);
bool isLetterOrDigit(char character);
bool isSymbol(char character);
bool isParenthesis(char character);

void skipWhitespace(char* input, uint32_t* cursor);
void skipLetters(char* input, uint32_t* cursor);
void skipAll(char* input, uint32_t* cursor, bool (*identityFunction)(char));

int parseDigit(bool* success, char character);
Token* parseNumber(bool* success, char* input, uint32_t* cursor, ErrorInstance** error);
Token* parseOperation(bool* success, char* input, uint32_t* cursor, ErrorInstance** error);
void parseExpression(bool* success, char* input, uint32_t* cursor, TokenStack* tokenStack, ErrorInstance** error);
number_t evaluateExpression(TokenStack* tokenStack, uint32_t lastIndex, ErrorInstance** error);

#endif // !__MAIN_H__

