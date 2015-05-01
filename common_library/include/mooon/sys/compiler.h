#ifndef __H_COMPILER_H_
#define __H_COMPILER_H_

#ifndef MIN
#define MIN(a,b) ((a<b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a>b) ? (a) : (b))
#endif

#if __x86_64__
#define INT2PTR(x)	((void *)(long)(x))
#define PTR2INT(x)	((int)(long)(x))
#define PTR2UINT(x)	((unsigned int)(long)(x))
#else
#define INT2PTR(x)	((void *)(x))
#define PTR2INT(x)	((int)(x))
#define PTR2UINT(x)	((unsigned int)(x))
#endif


#if __x86_64__
#define F64	"l"
#else
#define F64	"ll"
#endif

#if __GNUC__ < 3
#error You need GCC 3.0 or above to compile.
#endif

#undef __attr_cdecl__
#if __x86_64__
#define __attr_cdecl__ /* */
#else
#define __attr_cdecl__ __attribute__((__cdecl__))
#endif

#undef __attr_regparm__
#define __attr_regparm__(x) __attribute__((__regparm__(x)))

#undef __attr_const__
#define __attr_const__ __attribute__((__const__))

#undef __attr_pure__
#define __attr_pure__	__attribute__((__pure__))

#undef __attr_nonnull__
#define __attr_nonnull__ __attribute__((__nonnull__))

#undef __attr_malloc__
#define __attr_malloc__	__attribute__((__malloc__))

#undef __attr_printf__
#define __attr_printf__(x,y) __attribute__((__format__(printf,x,y)))

#undef __align1__
#undef __align2__
#undef __align4__
#undef __align8__
#define __align1__  __attribute__((__aligned__(1)))
#define __align2__  __attribute__((__aligned__(2)))
#define __align4__  __attribute__((__aligned__(4)))
#define __align8__  __attribute__((__aligned__(8)))
#if __WORDSIZE==64
#define __align__  __attribute__((__aligned__(8)))
#else
#define __align__  __attribute__((__aligned__(4)))
#endif

#if USE_LINKSCRIPT
#define __init__ __attribute__((section(".hdata")))
#else
#define __init__ /* */
#endif

#ifndef likely
#define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#if __WORDSIZE==64
struct stat;
typedef struct stat stat64_t;
#else
struct stat64;
typedef struct stat64 stat64_t;
#endif

#define BADADDR(x) ((unsigned long )x > (unsigned long)-4096)

#if __GNUC__ < 4
static inline void barrier(void) { __asm__ volatile("":::"memory"); }
#else
static inline void barrier(void) { __sync_synchronize (); }
#endif

#endif
