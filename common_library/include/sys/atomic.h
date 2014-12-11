#ifndef __ATOMIC_H__
#define __ATOMIC_H__
#include <stdint.h>

#if __GNUC__ < 4
#include "sys/atomic_asm.h"
#else
#include "sys/atomic_gcc.h"
#if __WORDSIZE==64
#include "sys/atomic_gcc8.h"
#endif
#endif
#include "sys/compiler.h"
#endif /* __ATOMIC_H__ */
