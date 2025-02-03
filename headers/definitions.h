#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define USE_DOUBLES_INTERNALLY
#ifdef USE_DOUBLES_INTERNALLY  
    typedef double number_t;
    typedef long double long_number_t;
    #define PRINT_NUMBER_T(value) \
      printf("%g", value);

    #define NUMBER_MAX DBL_MAX
    #define NUMBER_MIN -DBL_MAX
#else
    typedef int number_t ;
    typedef long long int long_number_t;
    #define PRINT_NUMBER_T(value) \
      printf("%d", value);

    #define NUMBER_MAX INT_MAX
    #define NUMBER_MIN INT_MIN
#endif 

enum ComparisonResult {
  COMPARISON_RESULT__GREATER, 
  COMPARISON_RESULT__LESSER,
  COMPARISON_RESULT__EQUAL
};

typedef enum   ComparisonResult ComparisonResult;

ComparisonResult convertToComparisonResult(int value);


#define PROGRAMMING_ERROR(message, ...) \
  printf("==PROGRAMMING ERROR==\n"); \
  printf(message, ##__VA_ARGS__); \
  exit(EXIT_FAILURE);

#define UNRECOVERABLE_ERROR(message, ...) \
  printf("==UNRECOVERABLE ERROR==\n"); \
  printf(message, ##__VA_ARGS__); \
  exit(EXIT_FAILURE);

#define OPERAND_CENTINEL ((number_t) -1);

#define WITH_SUBSTRING(string, endIndex, code) \
  char previousEndCharacter = string[endIndex]; \
  string[endIndex] = '\0'; \
  code \
  string[endIndex] = previousEndCharacter;

#define COUNT_ARGUMENTS_UP_TO_10(...) \
  __COUNT_ARGUMENTS_UP_TO_10__IMPLEMENTATION(, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define __COUNT_ARGUMENTS_UP_TO_10__IMPLEMENTATION(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, count, ...) count

#define RESIZE_MEMORY(dataType, memory, success, newSize, onFail) \
  dataType* __nextChunkOfMemory = realloc(memory, (newSize) * sizeof(dataType)); \
  if (__nextChunkOfMemory == NULL) {\
    success = false;\
    onFail\
  }\
  memory = __nextChunkOfMemory;

typedef struct {
  uint32_t length;
  void* data[];
} CustomArray;

#define PRINT_LINE() printf("%s: %i\n", __FILE__, __LINE__);
#define DEBUG(format, ...) \
  printf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);\
  printf(format, ## __VA_ARGS__); \
  printf("\n\n");
#define ASSERT(condition, message) \
  if (condition) {\
    printf("Test passed: '%s'\n", message);\
  } else {\
    PRINT_LINE();\
    printf("Test FAILED: '%s'\n", message);\
  }

#endif // !__DEFINITIONS_H__

