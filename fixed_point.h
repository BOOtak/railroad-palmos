//
// Created by kirill on 27.12.2020.
//

#ifndef RAILROAD_FIXED_POINT_H
#define RAILROAD_FIXED_POINT_H

#include <PalmTypes.h>

// 1 bit - sign; 19 bits - integer part; 12 bits - fraction part
#define FRAC_BITS 12
#define SIGN_MASK 0xFFF00000
#define FRAC_MASK 0x00000FFF
#define MANT_MASK 0xFFFFF000

#define FIX_MUL(X, Y) ((Fixed)(((Int32)((X) * (Y))) >> FRAC_BITS))
#define FIX_DIV(X, Y) ((Fixed)(((Int32)(X) << FRAC_BITS) / Y))
#define TO_INT(X) ((X) >> FRAC_BITS)
#define FLOAT_TO_FIX(X) ((Fixed)((X) * (1 << FRAC_BITS)))
#define INT_TO_FIX(X) ((Fixed)((X) << FRAC_BITS))

typedef Int32 Fixed;

float to_float(Fixed fp);

#endif  // RAILROAD_FIXED_POINT_H
