#ifndef __arm_exceptions64_h__
#define __arm_exceptions64_h__

#include "arm-constants64.h"

#ifdef DARWIN

typedef arm_thread_state64_t native_thread_state_t;
#define NATIVE_THREAD_STATE_COUNT ARM_THREAD_STATE64_COUNT
#define NATIVE_THREAD_STATE_FLAVOR ARM_THREAD_STATE64
typedef arm_vfp_state_t native_float_state_t;
#define NATIVE_FLOAT_STATE_COUNT ARM_THREAD_STATE64_COUNT
#define NATIVE_FLOAT_STATE_FLAVOR ARM_FLOAT_STATE64
typedef arm_exception_state64_t native_exception_state_t;
#define NATIVE_EXCEPTION_STATE_COUNT ARM_EXCEPTION_STATE64_COUNT
#define NATIVE_EXCEPTION_STATE_FLAVOR ARM_EXCEPTION_STATE64

void enable_fp_exceptions(void);
void disable_fp_exceptions(void);
void associate_tcr_with_exception_port(mach_port_t, TCR *);

#endif /* DARWIN */

#endif /* __arm_exceptions64_h__ */