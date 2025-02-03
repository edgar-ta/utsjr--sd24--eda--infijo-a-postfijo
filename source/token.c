#include "../headers/token.h"
#include "../headers/definitions.h"
#include "../headers/operation.h"

#include <stdlib.h>

Token* Token_newOperand(uint32_t startIndex, uint32_t endIndex, number_t value) {
  Token* token = (Token*) malloc(sizeof(Token));
  token->start = startIndex;
  token->end = endIndex;
  token->number = value;
  token->kind = TOKEN_KIND__OPERAND;
  return token;
}

Token* Token_newOperation(uint32_t startIndex, uint32_t endIndex, OperationInstance* operationInstance) {
  Token* token = malloc(sizeof(Token));
  token->start = startIndex;
  token->end = endIndex;
  token->operation = operationInstance;
  token->kind = TOKEN_KIND__OPERATION;
  return token;
}

void Token_destroy(Token* self) {
  if (self == NULL) return;
  if (self->kind == TOKEN_KIND__OPERATION) {
    free(self->operation);
  }
  free(self);
}

void Token_print(Token* self) {
  switch (self->kind) {
    case  TOKEN_KIND__OPERATION:
      printf("'%s'", self->operation->class_->representations[0].data);
    break;
    
    case TOKEN_KIND__OPERAND:
      #ifdef USE_DOUBLES_INTERNALLY
    printf("%g", self->number);
      #else
    printf("%i", self->number);
      #endif /* ifdef USE_DOUBLES_INTERNALLY */
    break;

    default:
      PROGRAMMING_ERROR("Token of unknown type");
    break;
  }
}

TokenNode* TokenNode_new(Token* token, TokenNode* next) {
  TokenNode* tokenNode = (TokenNode*) malloc(sizeof(TokenNode));
  tokenNode->token = token;
  tokenNode->next = next;
  return tokenNode;
}

TokenStack* TokenStack_new() {
  TokenStack* tokenStack = (TokenStack*) malloc(sizeof(TokenStack));
  tokenStack->head = NULL;
  tokenStack->numberOfOperations = 0;
  return tokenStack;
}

void TokenStack_pushOperation(TokenStack* self, Token* token) {
  if (token->kind != TOKEN_KIND__OPERATION) {
    PROGRAMMING_ERROR("The passed token is not of kind OPERATION");
  }
  TokenNode** pushReference = &self->head;
  OperationInstance* operationToInsert = token->operation;
  while ((*pushReference) != NULL) {
    if ((*pushReference)->token->kind == TOKEN_KIND__OPERATION) {
      OperationInstance* currentOperation = (*pushReference)->token->operation;
      if (OperationInstance_compare(operationToInsert, currentOperation) == COMPARISON_RESULT__GREATER) {
        break;
      }
    }
    pushReference = &(*pushReference)->next;
  }
  TokenNode* nextTokenNode = *pushReference;
  *pushReference = TokenNode_new(token, nextTokenNode);

  self->numberOfOperations++;
}

void TokenStack_pushOperand(TokenStack* self, Token* operand, Token* operation) {
  if (self == NULL) return;

  TokenNode** operationTokenReference = &self->head;
  while ((*operationTokenReference) != NULL) {
    if ((*operationTokenReference)->token == operation) {
      break;
    }
    operationTokenReference = &(*operationTokenReference)->next;
  }

  *operationTokenReference = TokenNode_new(operand, *operationTokenReference);
  if (operand->kind == TOKEN_KIND__OPERATION) {
    self->numberOfOperations++;
  }
}

Token* TokenStack_pop(TokenStack* self) {
  TokenNode* previousNode = self->head;
  if (previousNode == NULL) {
    return NULL;
  }

  TokenNode* nextNode = previousNode->next;
  previousNode->next = NULL;
  self->head = nextNode;

  Token* token = previousNode->token;
  free(previousNode);
  return token;
}

Token* TokenStack_findNextUnfulfilledOperation(TokenStack* self) {
  TokenNode* currentNode = self->head;
  while (currentNode != NULL) {
    if (currentNode->token->kind == TOKEN_KIND__OPERATION) {
      if (!OperationInstance_isFulfilled(currentNode->token->operation)) {
        break;
      }
    }
    currentNode = currentNode->next;
  }

  if (currentNode == NULL) {
    return NULL;
  }
  return currentNode->token;
}

void TokenStack_dropToken(TokenStack *self, Token *token) {
  TokenNode** nodeReference = &self->head;
  while (*nodeReference != NULL) {
    if ((*nodeReference)->token == token) {
      break;
    }
    nodeReference = &(*nodeReference)->next;
  }
  if (*nodeReference == NULL) {
    return;
  }
  TokenNode* nextNode = (*nodeReference)->next;
  TokenNode* currentNode = *nodeReference;

  currentNode->next = NULL;
  *nodeReference = nextNode;

  currentNode->token = NULL;
  free(currentNode);
}

void TokenStack_print(TokenStack* self) {
  TokenNode* slipknot = self->head;
  bool isFirst = true;
  while (slipknot != NULL) {
    if (!isFirst) {
      printf(", ");
    }
    Token_print(slipknot->token);
    isFirst = false;
    slipknot = slipknot->next;
  }
  printf("\n");
}

void TokenNode_findLargestInputRange(TokenNode *token, uint32_t *start, uint32_t *end) {
  if (token == NULL) {
    *start = 0;
    *end = 0;
    return;
  }

  *start = token->token->start;
  *end = token->token->end;

  while (true) {
    token = token->next;
    if (token == NULL) return;

    if (token->token->start < *start) {
      *start = token->token->start;
    }

    if (token->token->end > *end) {
      *end = token->token->end;
    }
  }
}

void TokenNode_deleteChain(TokenNode *root) {
  while (root != NULL) {
    TokenNode* previousValue = root;
    root = root->next;

    Token_destroy(previousValue->token);
    free(previousValue);
  }
}

TokenNode*** TokenStack_getOrderedOperations(TokenStack* self) {
  if (self->numberOfOperations == 0) return NULL;
  TokenNode*** orderedOperations = malloc(self->numberOfOperations * sizeof(TokenNode**));

  TokenNode* previousNode = NULL;
  TokenNode* currentNode = self->head;
  int operationsFound = 0;

  while (currentNode != NULL) {
    TokenNode* nextNode = currentNode->next;
    if (previousNode != NULL && previousNode->token->kind == TOKEN_KIND__OPERATION) {
      orderedOperations[operationsFound] = &(currentNode->next);
      operationsFound++;
    }

    currentNode->next = previousNode;
    previousNode = currentNode;
    currentNode = nextNode;
  }
  self->head = previousNode;
  if (self->head->token->kind == TOKEN_KIND__OPERATION) {
    orderedOperations[self->numberOfOperations - 1] = &(self->head);
  }

  return orderedOperations;
}

long_number_t TokenNode_nthArgument(TokenNode *root, int n) {
  for (int i = 0; i < n; i++) {
    if (root == NULL) {
      UNRECOVERABLE_ERROR("Null argument passed");
    }
    root = root->next;
  }
  if (root->token->kind != TOKEN_KIND__OPERAND) {
    UNRECOVERABLE_ERROR("Wrong kind of token passed");
  }
  return ((long_number_t) root->token->number);
}

