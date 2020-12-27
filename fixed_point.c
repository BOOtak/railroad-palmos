//
// Created by kirill on 27.12.2020.
//

#include "fixed_point.h"

float to_float(Fixed fp) {
  float res;
  Int16 fix_pow = fp & FRAC_MASK;
  Int16 mant = (fp & MANT_MASK) >> FRAC_BITS;
  if (fp & 0x8000) {
    // keep sign if negative
    mant |= SIGN_MASK;
  }

  res = (float) (mant + fix_pow) * (1.0f / (1 << FRAC_BITS));
  return res;
}
