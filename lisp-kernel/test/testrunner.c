
#include <stdlib.h>
#include "unity.h"
#include "../lisptypes.h"

#ifdef X8664
void flush_cache_lines(void *, natural, natural);
#endif

natural current_stack_pointer(void);
Boolean touch_page(void *);
unsigned count_leading_zeros(natural w);

void setUp(void) {
    // This function is called before each test
}

void tearDown(void) {
    // This function is called after each test
}

void test_FlushCacheLines(void) {
    //flush_cache_lines();
}

void test_CurrentStackPointer(void) {
    // current_stack_pointer is a function, so it's stack pointer will be further down the stack, so
    // adjust to compare.
    void *builtin_version = __builtin_frame_address(0) - sizeof(void *); 
    natural *utils_version = (natural *)current_stack_pointer();
    TEST_ASSERT_NOT_NULL(utils_version);
    TEST_ASSERT_EQUAL_PTR(builtin_version, utils_version);
}

void test_TouchPage(void) {
    void *mem = malloc(0x4000);
    TEST_ASSERT_TRUE(touch_page(mem));
    free(mem);
}

void test_CountLeadingZeros(void) {
    natural test_value = 0x00000000;
    unsigned result = count_leading_zeros(test_value);
    TEST_ASSERT_EQUAL(WORD_SIZE, result); // All bits are zero, so all WORD_SIZE bits are leading zeros

    test_value = 0x80000000; 
    result = count_leading_zeros(test_value);
    TEST_ASSERT_EQUAL(WORD_SIZE - 32, result); // No leading zeros or 32

    test_value = 0x00000001; // Only the lowest bit is set
    result = count_leading_zeros(test_value);
    TEST_ASSERT_EQUAL(WORD_SIZE - 1, result); // 31 leading zeros or 63
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_FlushCacheLines);
    RUN_TEST(test_CurrentStackPointer);
    RUN_TEST(test_TouchPage);
    RUN_TEST(test_CountLeadingZeros);

    return UNITY_END();
}