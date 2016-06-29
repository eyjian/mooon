#ifndef __ATOMIC_H__
#define __ATOMIC_H__
#include <stdint.h>

#if __GNUC__ < 4
#include "mooon/sys/atomic_asm.h"
#else
#include "mooon/sys/atomic_gcc.h"
#if __WORDSIZE==64
#include "mooon/sys/atomic_gcc8.h"
#endif
#endif
#include "mooon/sys/compiler.h"
#endif /* __ATOMIC_H__ */
