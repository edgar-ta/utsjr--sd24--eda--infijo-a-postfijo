#ifndef __TESTING_H__

#include <stdint.h>

#include "./error.h"
#include "./token.h"
#include "./definitions.h"

typedef bool (*testing_predicate_t)(bool, uint32_t, ErrorInstance*, Token*);
typedef struct {
  char* input;
  char* name;
  testing_predicate_t predicate;
} TestCase;

void testParseOperation();
void testParseNumber();
void __test(
  char* input, 
  char* testCaseName, 
  testing_predicate_t predicate,
  Token* (*parsingFunction)(bool*, char*, uint32_t*, ErrorInstance**)
);
void __runTests(
  char* testUnit,
  Token* (*parsingFunction)(bool*, char*, uint32_t*, ErrorInstance**),
  int numberOfTests, 
  ...
);

#define TEST_CASE_LITERAL(testUnit, testName, testMessage, input_, predicate_) \
bool __##testUnit##__##testName##__predicate(\
  bool success, \
  uint32_t cursor, \
  ErrorInstance* error, \
  Token* token\
) {\
  return (predicate_);\
}\
TestCase testUnit##__##testName = { \
  .input = input_,\
  .name = testMessage,\
  .predicate = & __##testUnit##__##testName##__predicate \
};

#define RUN_TESTS(testUnit, parsingFunction, ...) \
__runTests(\
    testUnit, \
    parsingFunction, \
    COUNT_ARGUMENTS_UP_TO_10(__VA_ARGS__),\
    __VA_ARGS__\
)

#endif // !__TESTING_H__


