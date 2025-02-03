#include "../headers/custom-string.h"

#include <string.h>
#include <stdarg.h>

void CustomString_build(CustomString* self, char* string) {
  unsigned long long length = strlen(string);
  char* data = malloc(sizeof(char) * (length + 1));
  memcpy(data, string, length + 1);

  self->length = length;
  self->data = data;
}

CustomString* __CustomString_new(char* string) {
  CustomString* customString = malloc(sizeof(CustomString));
  CustomString_build(customString, string);
  return customString;
}

CustomString* __CustomString_newArray(uint32_t numberOfStrings, va_list strings) {
  CustomString* customStrings = malloc(sizeof(CustomString) * numberOfStrings);

  for (uint32_t i = 0; i < numberOfStrings; i++) {
    char* currentString = va_arg(strings, char*);
    CustomString_build(customStrings + i, currentString);
  }

  return customStrings;
}

