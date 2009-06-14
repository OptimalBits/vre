
#ifndef VRE_MATH_H
#define VRE_MATH_H

#include "vre_defs.h"

#define vre_pi  (3.14159265 * 65536)
#define VRE_PI_4_28 ((vre_int32)( 3.14159265 * ( 1 << 28 ) ))

#define VRE_IS_POWER_OF_2(x) !(x & (x - 1)) && x

#define VRE_ABS(x)  (((x) < 0) ? -(x) : (x))
#define VRE_MAX(x, y)  (((x) > (y)) ? (x) : (y))
#define VRE_MIN(x, y)  (((x) < (y)) ? (x) : (y))


void vre_cordic_rotate(vre_fix16 *px, vre_fix16 *py, vre_fix16 theta) ;

//
// (c) Martin Jacobson
// Portable macros to perform 64 bit multiplication by 4 32 bit multiplications
// The macros are a little complicated but expands into very optimizable code.

// MUL64<HI, LO> returns the high or low 32 bits of a * b
// MUL64SH returns a * b rightshifted rsh bits and clamped to 32 bits
// FPMUL<HI, LO, SH> are versions for signed numbers

// These macros were originally written by Martin Jacobsson for use in the
// First Lady Checkers program. They are now public domain. Derivations
// may be copyrighted by derivating party.
// Fixed point numbers of different sizes
//

#define M64LO16(x) ((vre_uint32)(x) % (1 << 16))
#define M64HI16(x) ((vre_uint32)(x) / (1 << 16))
#define M64EX16(x)  ((vre_uint32)(x) * (1 << 16))

#define M64M1(a, b) (M64LO16(a) * M64LO16(b))
#define M64M2(a, b) (M64LO16(a) * M64HI16(b))
#define M64M3(a, b) (M64HI16(a) * M64LO16(b))
#define M64M4(a, b) (M64HI16(a) * M64HI16(b))
#define M64LP(a, b) (M64HI16(M64M1((a), (b))) + M64LO16(M64M2((a), (b))) + M64LO16(M64M3((a), (b))))

#define MUL64HI(a, b) (M64HI16(M64M2((a), (b))) + M64HI16(M64M3((a),(b))) + M64M4((a), (b)) + M64HI16(M64LP((a), (b))))
#define MUL64LO(a, b) (M64EX16(M64LP((a), (b))) | M64LO16(M64M1((a), (b))))
#define MUL64SH(a, b, rsh) ((vre_uint32)MUL64LO((a), (b)) >> rsh |(MUL64HI((a), (b)) << (32 - rsh)))

#define MUL64SHR(a, b, rsh) ((((vre_uint32)MUL64LO((a), (b)) + (CPOW(2,rsh)/2)) >> (rsh)) | ((MUL64HI((a), (b)) << (32 - (rsh)))))

#define MS64NEG(a) ((a) < 0 || (vre_uint32)(a) >> 31)
#define MS64CAR(a) ((vre_uint32)(a) + 1 == 0 ? 1 : 0)
#define MS64HI5(a, b) (~MUL64HI(-(a), (b)) + MS64CAR(~MUL64LO(-(a), (b))))
#define MS64HI4(a, b) (~MUL64HI((a),-(b)) + MS64CAR(~MUL64LO((a), -(b))))
#define MS64HI2(a, b) (MS64NEG(b) ? MUL64HI(-(a), -(b)) : MS64HI5((a), (b)))
#define MS64HI1(a, b) (MS64NEG(b) ? MS64HI4((a), (b)) : MUL64HI(a,b))

#define FPMULHI(a, b) (MS64NEG(a) ? MS64HI2((a), (b)) : MS64HI1((a), (b)))

#define MS64LO5(a, b) ((vre_uint32) ~MUL64LO(-(a), (b)) + 1)
#define MS64LO4(a, b) ((vre_uint32) ~MUL64LO((a), -(b)) + 1)
#define MS64LO2(a, b) (MS64NEG(b) ? MUL64LO(-(a), -(b)) : MS64LO5((a), (b)))
#define MS64LO1(a, b) (MS64NEG(b) ? MS64LO4((a), (b)) : MUL64LO((a), (b)))

#define FPMULLO(a, b) (MS64NEG(a) ? MS64LO2((a), (b)) : MS64LO1((a), (b)))

#define FPMULSH(a, b, rsh) ((vre_uint32)FPMULLO((a), (b)) >> rsh | (FPMULHI((a), (b)) << (32 - rsh)))


vre_ufix16 vre_sqrt (vre_ufix16);

vre_fix16 vre_pow ( vre_fix16 a, vre_fix16 b );

vre_fix16 vre_sin (vre_fix16 a);

vre_fix16 vre_cos (vre_fix16 a);

vre_fix16 vre_arccos (vre_fix16 a);


vre_uint32 vre_most_significant_bit ( vre_uint32 x );




#endif
