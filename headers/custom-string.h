#ifndef __CUSTOM_STRING_H__

#include "./definitions.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


// Data structure that represents a 
// length-aware string; it holds an integer 
// that indicates how many characters there 
// are in the string, not including the 
// null terminating character that the latter
// has
typedef struct {
  uint32_t length;
  char* data;
} CustomString;

#define CUSTOM_STRING_LITERAL(string) {\
  .length = sizeof(string) - 1,\
  .data = string\
}

void CustomString_build(CustomString* self, char* string);
CustomString* __CustomString_new(char* string);
CustomString* __CustomString_newArray(uint32_t numberOfStrings, va_list strings);

#endif // !__CUSTOM_STRING_H__

