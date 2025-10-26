#define WORD_SIZE 64
#define PLATFORM_OS PLATFORM_OS_DARWIN
#define PLATFORM_CPU PLATFORM_CPU_ARM
#define PLATFORM_WORD_SIZE PLATFORM_WORD_SIZE_64

#define _DARWIN_C_SOURCE

#include <sys/signal.h>
#include <sys/ucontext.h>
#include <stdint.h>

/* ucontext/mcontext stuff; saner if OS >= 10.5 */
typedef mcontext_t MCONTEXT_T;
typedef ucontext_t ExceptionInformation;
#define UC_MCONTEXT(UC) UC->uc_mcontext

#define MAXIMUM_MAPPABLE_MEMORY (512L<<30L)
#define IMAGE_BASE_ADDRESS 0x300000000000L

#include "lisptypes.h"
#include "arm64-constants.h"

/* xp accessors, sigreturn stuff */
#define DARWIN_USE_PSEUDO_SIGRETURN 1

extern void darwin_sigreturn(ExceptionInformation *, unsigned);
extern natural os_major_version;

#define DarwinSigReturn(context)        \
  do                                    \
  {                                     \
    darwin_sigreturn(context, 0x1e);    \
    Bug(context, "sigreturn returned"); \
  } while (0)

// xp accessors for Darwin ARM64 (Apple Silicon)
#define xpGPRvector(x) ((natural *)&((x)->uc_mcontext->__ss.__x[0]))
#define xpGPR(x, gprno) (xpGPRvector(x))[gprno]
#define xpPC(x) (*((pc*)(&((x)->uc_mcontext->__ss.__pc))))
#define xpLR(x) ((x)->uc_mcontext->__ss.__lr)
#define xpFP(x) ((x)->uc_mcontext->__ss.__fp)
#define xpSP(x) ((x)->uc_mcontext->__ss.__sp)
#define xpPSR(x) ((x)->uc_mcontext->__ss.__cpsr)
#define xpFaultAddress(x) ((x)->uc_mcontext->__es.__far)
#define xpFaultStatus(x) ((x)->uc_mcontext->__es.__esr)
#define SIGRETURN(context) DarwinSigReturn(context)

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/machine/thread_state.h>
#include <mach/machine/thread_status.h>

#include "os-darwin.h"
