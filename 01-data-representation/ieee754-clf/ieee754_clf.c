#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "ieee754_clf.h"


float_class_t classify(double x) {
    uint64_t bits;
    memcpy(&bits, &x, sizeof(double));
    bool sign = (bits >> 63) & 1;
    uint64_t exponent = 0;
    uint64_t mantissa = 0;

    for (int i = 0; i < 52; ++i) {
        mantissa <<= 1;
        mantissa |= (bits >> i) & 1;
    }
    for (int i = 52; i < 63; ++i) {
        exponent <<= 1;
        exponent |= (bits >> i) & 1;
    }

    if (exponent == 0 && mantissa == 0) {
        return sign ? MinusZero : Zero;
    } else if (exponent == 0b11111111111 && mantissa == 0) {
        return sign ? MinusInf : Inf;
    } else if (exponent == 0b11111111111) {
        return NaN;
    } else if(exponent == 0) {
        return sign ? MinusDenormal : Denormal;
    } else {
        return sign ? MinusRegular : Regular;
    }
}
