#include "../headers/testing.h"
#include <stdarg.h>

void __test(
  char* input, 
  char* testCaseName, 
  testing_predicate_t testingCallback,
  Token* (*parsingFunction)(bool*, char*, uint32_t*, ErrorInstance**)
) {
  bool success = true;
  uint32_t cursor = 0;
  ErrorInstance* error = NULL;
  Token* token = NULL;

  token = parsingFunction(&success, input, &cursor, &error);
  ASSERT(testingCallback(success, cursor, error, token), testCaseName);

  free(token);
  free(error);
}

void __runTests(
  char* testUnit, 
  Token* (*parsingFunction)(bool*, char*, uint32_t*, ErrorInstance**),
  int numberOfTests, 
  ...
) {
  va_list testCases;
  va_start(testCases, numberOfTests);
  printf("Running %d tests for '%s'\n", numberOfTests, testUnit);

  for (int i = 0; i < numberOfTests; i++) {
    TestCase* testCase = va_arg(testCases, TestCase*);
    __test(testCase->input, testCase->name, testCase->predicate, parsingFunction);
  }
  printf("\n");
  
  va_end(testCases);
}

