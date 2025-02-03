#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/error.h"
#include "../headers/operation.h"
#include "../headers/token.h"

ErrorClass ALL_ERROR_CLASSES[] = {
  { 
    .formatString = "Se esperaban digitos despues del signo negativo\n",
    .style = ERROR_STYLE__RAW_MESSAGE,
    .identifier = ERROR_CLASS__NO_DIGITS_AFTER_SIGN
  },
  {
    .formatString = "El entero '%s', es demasiado grande para ser representado internamente\n",
    .style = ERROR_STYLE__INPUT_EXCERPT_MESSAGE,
    .identifier = ERROR_CLASS__INPUT_INTEGER_TOO_LARGE
  },
  {
    .formatString = "El entero '%s', es demasiado pequeno para ser representado internamente\n",
    .style = ERROR_STYLE__INPUT_EXCERPT_MESSAGE,
    .identifier = ERROR_CLASS__INPUT_INTEGER_TOO_SMALL,
  },
  {
    .formatString = "La expresion ingresada no es ni un operando ni un operador; no se reconoce la entrada\n",
    .style = ERROR_STYLE__RAW_MESSAGE,
    .identifier = ERROR_CLASS__EXPRESSION_NOT_RECOGNIZED,
  },
  {
    .formatString = "La expresion ingresada no se puede evaluar porque esta vacia\n",
    .style = ERROR_STYLE__RAW_MESSAGE,
    .identifier = ERROR_CLASS__EMPTY_EXPRESSION,
  },
  {
    .formatString = "Se esperaba un operando en lugar del operador '%s'\n",
    .style = ERROR_STYLE__INPUT_EXCERPT_MESSAGE,
    .identifier = ERROR_CLASS__OPERATION_WHEN_OPERAND_EXPECTED,
  },
  {
    .printingFunction = &__ERROR_CLASS__OTHER_OPERANDS_THAN_NEEDED__printingFunction,
    .style = ERROR_STYLE__CUSTOM_FUNCTION,
    .identifier = ERROR_CLASS__OTHER_OPERANDS_THAN_NEEDED,
  },
  {
    .formatString = "La cadena de caracteres '%s' no es una operacion valida\n",
    .style = ERROR_STYLE__INPUT_EXCERPT_MESSAGE,
    .identifier = ERROR_CLASS__NOT_AN_OPERATION,
  },
  {
    .printingFunction = &__ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS__printingFunction,
    .style = ERROR_STYLE__CUSTOM_FUNCTION,
    .identifier = ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS,
  },
  {
    .printingFunction = &__ERROR_CLASS__IDENTIFIER_TOO_SHORT__printingFunction,
    .style = ERROR_STYLE__CUSTOM_FUNCTION,
    .identifier = ERROR_CLASS__IDENTIFIER_TOO_SHORT
  },
  {
    .formatString = "Demasiados parentesis de cierre",
    .style = ERROR_STYLE__RAW_MESSAGE,
    .identifier = ERROR_CLASS__TOO_MANY_CLOSING_PARENTHESES
  },
  {
    .printingFunction = &__ERROR_CLASS__OPERATION_NOT_ADMITTED__printingFunction,
    .style = ERROR_STYLE__CUSTOM_FUNCTION,
    .identifier = ERROR_CLASS__OPERATION_NOT_ADMITTED
  },
  {
    .printingFunction = &__ERROR_CLASS__TOO_FEW_CLOSING_PARENTHESES__printingFunction,
    .style = ERROR_STYLE__CUSTOM_FUNCTION,
    .identifier = ERROR_CLASS__TOO_FEW_CLOSING_PARENTHESES
  },
  {
    .formatString = "Demasiados argumentos sin ninguna operacion",
    .style = ERROR_STYLE__RAW_MESSAGE,
    .identifier = ERROR_CLASS__TOO_MANY_FREE_ARGUMENTS
  },
  {
    .formatString = "Se intento dividir por cero",
    .style = ERROR_STYLE__RAW_MESSAGE,
    .identifier = ERROR_CLASS__DIVISION_BY_ZERO
  }
};

const uint32_t numberOfErrorClasses = sizeof(ALL_ERROR_CLASSES) / sizeof(ALL_ERROR_CLASSES[0]);

ErrorInstance* __Error_new(
  uint32_t startIndex, 
  uint32_t endIndex,
  ErrorClass* errorClass
) {
  ErrorInstance* instance = (ErrorInstance*) malloc(sizeof(ErrorInstance));

  if (startIndex > endIndex) {
    uint32_t temporary = endIndex;
    endIndex = startIndex;
    startIndex = temporary;
  }

  instance->start = startIndex;
  instance->end = endIndex;
  instance->class_ = errorClass;
  instance->payload = NULL;
  return instance;
}

ErrorInstance* Error_rawMessageError(
  ErrorClassIdentifier classIdentifier, 
  uint32_t startIndex, 
  uint32_t endIndex
) {
  ErrorClass* errorClass = __ErrorClass_getClassByIdentifier(classIdentifier);
  if (errorClass->style != ERROR_STYLE__RAW_MESSAGE) {
    PROGRAMMING_ERROR("The given class does not define a raw message");
  }
  return __Error_new(startIndex, endIndex, errorClass);
}

ErrorInstance* Error_inputExcerptError(
  ErrorClassIdentifier classIdentifier, 
  uint32_t startIndex, 
  uint32_t endIndex
) {
  ErrorClass* errorClass = __ErrorClass_getClassByIdentifier(classIdentifier);
  if (errorClass->style != ERROR_STYLE__INPUT_EXCERPT_MESSAGE) {
    PROGRAMMING_ERROR("The given class does not define a message taking an input excerpt");
  }
  return __Error_new(startIndex, endIndex, errorClass);
}

ErrorInstance* Error_otherOperandsThanNeededError(uint32_t startIndex, uint32_t endIndex, Token* operation) {
  ErrorInstance* error = __Error_new(
    startIndex, 
    endIndex, 
    __ErrorClass_getClassByIdentifier(ERROR_CLASS__OTHER_OPERANDS_THAN_NEEDED)
  );

  __Error_OtherOperandsThanNeededPayload* payload = malloc(sizeof(__Error_OtherOperandsThanNeededPayload));
  payload->start = operation->start;
  payload->end = operation->end;
  payload->expectedArity = operation->operation->class_->arity;
  payload->providedArity = operation->operation->fulfilledArity;

  error->payload = payload;

  return error;
}
void __ERROR_CLASS__OTHER_OPERANDS_THAN_NEEDED__printingFunction(ErrorInstance* self, char* input) {
  __Error_OtherOperandsThanNeededPayload* payload = self->payload;
  WITH_SUBSTRING(
    input, 
    payload->end, 
    printf(
      "La operacion '%s' esperaba %u operandos, pero %u fueron dados", 
      input + payload->start, 
      payload->expectedArity,
      payload->providedArity
    );
  );
}

ErrorInstance* Error_notAnOperationWithSuggestions(
  uint32_t startIndex,
  uint32_t endIndex,
  int numberOfRepresentations,
  __OperationRepresentationEntry* representations
) {
  ErrorInstance* error = __Error_new(
    startIndex, 
    endIndex, 
    __ErrorClass_getClassByIdentifier(ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS)
  );

  CustomArray* array = malloc(
    sizeof(CustomArray) + numberOfRepresentations * sizeof(char*)
  );
  array->length = numberOfRepresentations;
  for (int i = 0; i < numberOfRepresentations; i++) {
    array->data[i] = representations[i].representation->data;
  }

  error->payload = array;
  return error;
}
void __ERROR_CLASS__NOT_AN_OPERATION_WITH_SUGGESTIONS__printingFunction(ErrorInstance *self, char *input) {
  WITH_SUBSTRING(
    input, 
    self->end, 
    printf("No se reconoce ninguna operacion con el nombre...\n\n'%s'\n\n", input + self->start);
  );
  printf("..sin embargo, existen algunas opciones similares que podrias probar: ");

  CustomArray* suggestions = self->payload;
  for (int i = 0; i < suggestions->length; i++) {
    char* suggestion = suggestions->data[i];
    if (i != 0) {
      putchar(',');
      putchar(' ');
    }
    printf("'%s'", suggestion);
  }
  putchar('\n');
}

#define ANSI_COLOR_RED "\x1b[31m" 
#define ANSI_COLOR_RESET "\x1b[0m"

void ErrorInstance_printMessage(ErrorInstance* self, char* input) {
  {
    WITH_SUBSTRING(
      input,
      self->start,
      printf("%s", input);
    );
  }
  {
    printf(ANSI_COLOR_RED);
    WITH_SUBSTRING(
      input, 
      self->end, 
      printf("%s", input + self->start);
    );
    printf(ANSI_COLOR_RESET);
  }
  printf("%s", input + self->end);
  printf("\n");

  uint32_t width = self->end - self->start;
  if (width < 4) width = 4;
  for (uint32_t _i = 0; _i < self->start; _i++) {
    putchar(' ');
  }
  printf(ANSI_COLOR_RED);
  for (uint32_t _i = 0; _i < width; _i++) {
    putchar('^');
  }
  printf(ANSI_COLOR_RESET);
  printf("\n");

  switch (self->class_->style) {
  case ERROR_STYLE__RAW_MESSAGE:
    __ERROR_CLASS__printMessageWithRawMessage(self);
    break;

  case ERROR_STYLE__INPUT_EXCERPT_MESSAGE:
    __ERROR_CLASS__printMessageWithInputExcerpt(self, input);
    break;

  case ERROR_STYLE__CUSTOM_FUNCTION:
    self->class_->printingFunction(self, input);
    break;
  }
}

void ErrorInstance_destroy(ErrorInstance *self) {
  if (self->class_->identifier != ERROR_CLASS__IDENTIFIER_TOO_SHORT) {
    free(self->payload);
  }
  free(self);
}

void __ERROR_CLASS__printMessageWithRawMessage(ErrorInstance* self) {
  printf("%s", self->class_->formatString);
}

void __ERROR_CLASS__printMessageWithInputExcerpt(ErrorInstance* self, char* input) {
  WITH_SUBSTRING(
    input,
    self->end,
    printf(self->class_->formatString, input + self->start);
  );
}

ErrorClass* __ErrorClass_getClassByIdentifier(ErrorClassIdentifier identifier) {
  for (uint32_t i = 0; i < numberOfErrorClasses; i++) {
    ErrorClass* currentClass = &(ALL_ERROR_CLASSES[i]);
    if (currentClass->identifier == identifier) {
      return currentClass;
    }
  }
  return NULL;
}

ErrorInstance* Error_identifierTooShort(
  uint32_t startIndex,
  uint32_t endIndex,
  char* suggestion
) {
  ErrorInstance* error = __Error_new(
    startIndex, 
    endIndex, 
    __ErrorClass_getClassByIdentifier(ERROR_CLASS__IDENTIFIER_TOO_SHORT)
  );
  error->payload = suggestion;
  return error;
}

void __ERROR_CLASS__IDENTIFIER_TOO_SHORT__printingFunction(ErrorInstance* self, char* input) {
  char* suggestion = self->payload;
  WITH_SUBSTRING(
    input,
    self->end,
    printf(
      "La cadena de caracteres '%s' no es una operacion valida; quisiste decir '%s'?\n",
      input + self->start,
      suggestion
    );
  );
}

ErrorInstance* Error_operationNotAdmitted(
  uint32_t startIndex,
  uint32_t endIndex,
  uint32_t startNameOfOperation,
  uint32_t endNameOfOperation,
  OperationPositionType operationPositionType
) {
  ErrorInstance* error = __Error_new(
    startIndex, 
    endIndex, 
    __ErrorClass_getClassByIdentifier(ERROR_CLASS__OPERATION_NOT_ADMITTED)
  );

  __Error_OperationNotAdmittedPayload* payload = malloc(sizeof(__Error_OperationNotAdmittedPayload));
  payload->start = startNameOfOperation;
  payload->end = endNameOfOperation;
  payload->position = operationPositionType;
  error->payload = payload;

  return error; 
}
void __ERROR_CLASS__OPERATION_NOT_ADMITTED__printingFunction(ErrorInstance* self, char* input) {
  __Error_OperationNotAdmittedPayload* payload = self->payload;
  if (payload->position == OPERATION_POSITION_TYPE__INFIX) {
    WITH_SUBSTRING(
      input, 
      payload->end, 
      printf(
        "No se admiten funciones de tipo infijo en el contexto actual; tal vez te ayudaria un poco poner parentesis alrededor del operador '%s'",
        input + payload->start
      );
   );
  } else if (payload->position == OPERATION_POSITION_TYPE__PREFIX) {
    WITH_SUBSTRING(
      input, 
      payload->end, 
      printf(
        "No se admiten funciones de tipo prefijo en el contexto actual; tal vez deberias poner un operador antes de llamar a la funcion '%s'",
        input + payload->start
      );
   );
  }
}

ErrorInstance* Error_tooFewClosingParentheses(
  uint32_t startIndex,
  uint32_t endIndex,
  int missingAmount
) {
  ErrorInstance* error = __Error_new(
    startIndex, 
    endIndex, 
    __ErrorClass_getClassByIdentifier(ERROR_CLASS__TOO_MANY_CLOSING_PARENTHESES)
  );

  int* payload = malloc(sizeof(int));
  *payload = missingAmount;
  error->payload = payload;
  
  return error;
}

void __ERROR_CLASS__TOO_FEW_CLOSING_PARENTHESES__printingFunction(ErrorInstance* self, char* input) {
  int* payload = self->payload;
  if (*payload > 1) {
    printf("No se encontraron los suficientes parentesis de cierre; se esperaban otros %d todavia", *payload);
  } else {
    printf("No se encontraron los suficintes parentesis de cierre; se esperaba otro todavia");
  }
}

