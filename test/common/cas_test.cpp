
#include "common/file.h"
#include "common/logger.h"
#include "gtest/gtest.h"

using namespace lizlib;

TEST(CASTest, t1) {
  long* value = (long*)malloc(sizeof(long));
  *value = 42;
  long expected = 42, new_val = -100;

  if (__atomic_compare_exchange_n(value, &expected, new_val, true,
                                  __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
    printf("CAS operation successful. New value: %ld\n", *value);
  } else {
    printf("CAS operation failed. Current value: %ld\n", *value);
  }
  free(value);
}
