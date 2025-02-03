#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "./definitions.h"
#include "./struct-declarations.h"
#include <stdint.h>

enum TokenKind {
  TOKEN_KIND__OPERAND,
  TOKEN_KIND__OPERATION
};

typedef enum   TokenKind TokenKind;

struct Token {
  enum TokenKind kind;
  uint32_t start;
  uint32_t end;
  union {
    struct OperationInstance* operation;
    number_t number;
  };
};

struct TokenNode {
  struct Token* token;
  struct TokenNode* next;
};

struct TokenStack {
  struct TokenNode* head;
  uint32_t numberOfOperations;
};

Token* Token_newOperand(uint32_t startIndex, uint32_t endIndex, number_t value);
Token* Token_newOperation(uint32_t startIndex, uint32_t endIndex, struct OperationInstance* operationInstance);
void Token_print(Token* self);
void Token_destroy(Token* self);

TokenNode* TokenNode_new(Token* token, TokenNode* next);
void TokenNode_findLargestInputRange(TokenNode* token, uint32_t* start, uint32_t* end);
void TokenNode_deleteChain(TokenNode* root);

// Fatal function that gets the nth `number_t` counting
// from the root
long_number_t TokenNode_nthArgument(TokenNode* root, int n);

TokenStack* TokenStack_new();
void TokenStack_pushOperation(TokenStack* self, Token* token);
void TokenStack_pushOperand(TokenStack* self, Token* operand, Token* operation);
Token* TokenStack_pop(TokenStack* self);
Token* TokenStack_findNextUnfulfilledOperation(TokenStack* self);
void TokenStack_dropToken(TokenStack* self, Token* token);
void TokenStack_print(TokenStack* self);
TokenNode*** TokenStack_getOrderedOperations(TokenStack* self);

#endif // !__TOKEN_H__

