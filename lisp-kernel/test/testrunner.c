
#include <stdlib.h>
#include "unity.h"
#include "../bits.h"
#include "../lisptypes.h"

#ifdef X86 
int cpuid(natural, natural*, natural*, natural*);
#endif

#ifdef X8664
void flush_cache_lines(void *, natural, natural);
natural get_mxcsr();
void set_mxcsr(natural);
#endif

#ifdef ARM64
void flush_cache_lines(void *, natural);
#endif



natural current_stack_pointer(void);
Boolean touch_page(void *);
unsigned count_leading_zeros(natural w);
void noop(void);
natural store_conditional(natural*, natural, natural);
signed_natural atomic_swap(signed_natural*, signed_natural);
natural atomic_ior(natural*, natural);
natural atomic_and(natural*, natural);

void setUp(void) {
    // This function is called before each test
}

void tearDown(void) {
    // This function is called after each test
}

void test_FlushCacheLines(void) {

    void *mem = malloc(0x4000);
    #if defined(X8664)
    flush_cache_lines(mem, 1, 8);
    #elif defined(ARM64)
    flush_cache_lines(mem, 0x4000);
    #endif
    free(mem);
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
    natural test_value;
    test_value = 0x80000000; 
    int result = count_leading_zeros(test_value);
    TEST_ASSERT_EQUAL(WORD_SIZE - 32, result); // No leading zeros or 32

    test_value = 0x00000001; // Only the lowest bit is set
    result = count_leading_zeros(test_value);
    TEST_ASSERT_EQUAL(WORD_SIZE - 1, result); // 31 leading zeros or 63

    #if WORD_SIZE == 64
    test_value = 0xf000000000000000; // Only the lowest bit
    result = count_leading_zeros(test_value);
    TEST_ASSERT_EQUAL(0, result);
    #endif
}

void test_Noop(void) {
    noop();
}

// X86-64 only
void test_Mxcsr(void) {
    #ifdef X8664
    natural save_mxcsr = get_mxcsr();
    set_mxcsr(0x1f80);
    natural post_mxcsr = get_mxcsr();
    TEST_ASSERT_EQUAL(0x1f80, post_mxcsr);
    #endif
}

void test_StoreConditional(void) {
    natural value = 0;
    natural result = store_conditional(&value, 0, 42);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(42, value);

    // Test conditional failed
    natural value2 = 0; 
    result = store_conditional(&value2, 10, 42);
    TEST_ASSERT_EQUAL(42, result);
    TEST_ASSERT_EQUAL(0, value2);   
}

void test_AtomicSwap(void) {
    signed_natural value = 0;
    signed_natural result = atomic_swap(&value, 42);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(42, value);

    // Test atomic swap with a different value
    signed_natural value2 = 10;
    result = atomic_swap(&value2, 20);
    TEST_ASSERT_EQUAL(10, result);
    TEST_ASSERT_EQUAL(20, value2);
}

void test_AtomicIor(void) {
    natural bits = 0;

    natural bitmask = 0x10; // Set the 5th bit
    natural result = atomic_ior(&bits, bitmask);
    TEST_ASSERT_EQUAL(0, result); // should return 0 since none of the bits in that bitmask were not set.
    TEST_ASSERT_EQUAL(0x10, bits); // The 5th bit should now be set

    result = atomic_ior(&bits, bitmask);
    TEST_ASSERT_GREATER_THAN(0, result); // Should return non-zero as it was already set now
    TEST_ASSERT_EQUAL(0x10, bits); // The 5th bit should now be set
}

void test_AtomicAnd(void) {
    natural bits = 0xFF; // All bits set

    natural bitmask = 0xF0; 
    natural result = atomic_and(&bits, bitmask);
    TEST_ASSERT_EQUAL(0xF0, result); // other bits should be cleared, only the 4th and 5th bits should remain set
    TEST_ASSERT_EQUAL(0xF0, bits);

    bitmask = 0x00;
    result = atomic_and(&bits, bitmask);
    TEST_ASSERT_EQUAL(0, result); 
    TEST_ASSERT_EQUAL(0, bits); 
}

void test_CPUID(void) {
    #ifdef X86
    natural eax, ebx, ecx, edx;
    eax = cpuid(0, &ebx, &ecx, &edx);
    TEST_ASSERT_NOT_EQUAL(0, eax);
    TEST_ASSERT_NOT_EQUAL(0, ebx);
    TEST_ASSERT_NOT_EQUAL(0, ecx);
    TEST_ASSERT_NOT_EQUAL(0, edx);
    #endif
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_FlushCacheLines);
    RUN_TEST(test_CurrentStackPointer);
    RUN_TEST(test_TouchPage);
    RUN_TEST(test_CountLeadingZeros);
    RUN_TEST(test_Noop);
    RUN_TEST(test_Mxcsr);
    RUN_TEST(test_StoreConditional);
    RUN_TEST(test_AtomicSwap);
    RUN_TEST(test_AtomicIor);
    RUN_TEST(test_AtomicAnd);
    RUN_TEST(test_CPUID);

    return UNITY_END();
}