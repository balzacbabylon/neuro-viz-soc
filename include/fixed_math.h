#ifndef FIXED_MATH_H
#define FIXED_MATH_H

#include "common.h"
#include <stdint.h>

#define FIXED_ONE (1 << 16)
#define INT_TO_FIXED(x) ((x) << 16)
#define FIXED_TO_INT(x) ((x) >> 16)
#define FLOAT_TO_FIXED(f) ((fixed)((f) * 65536.0f))
#define FIXED_TO_FLOAT(fi) (((float)fi) / 65536.0f)

// TODO: There is no overflow/underflow checking...need to add that
static inline fixed mul_fixed(fixed a, fixed b) {
    return (fixed)(((int64_t)a * b) >> 16);
}

static inline fixed div_fixed(fixed a, fixed b) {
    return (fixed)(((int64_t)a << 16) / (int64_t)b);
}


// --- Fast Integer Math Helpers ---

// Integer Square Root (for bounding sphere)
// Standard "bit-shift" implementation, much faster than soft-float sqrt
static inline unsigned int isqrt(uint64_t n) {
    unsigned int c = 0x8000;
    unsigned int g = 0x8000;

    for(;;) {
        // g*g can exceed 32 bits, so cast to 64-bit for comparison
        if((uint64_t)g * g > n) g ^= c;
        c >>= 1;
        if(c == 0) return g;
        g |= c;
    }
}

// Sine Lookup Table (0 to 90 degrees in fixed point)
// 256 entries for 0-90 degrees is usually plenty for Nios II
static const int SIN_LUT_90[91] = {
    0, 1143, 2287, 3429, 4571, 5711, 6850, 7986, 9120, 10252, 
    11380, 12504, 13625, 14742, 15854, 16961, 18064, 19160, 20251, 21336, 
    22414, 23486, 24550, 25606, 26655, 27696, 28729, 29752, 30767, 31772, 
    32768, 33753, 34728, 35693, 36647, 37590, 38521, 39440, 40347, 41243, 
    42125, 42995, 43852, 44695, 45525, 46340, 47142, 47929, 48702, 49460, 
    50203, 50931, 51643, 52339, 53019, 53683, 54331, 54963, 55577, 56175, 
    56755, 57319, 57864, 58393, 58903, 59395, 59870, 60326, 60763, 61183, 
    61583, 61965, 62328, 62672, 62997, 63302, 63589, 63856, 64103, 64331, 
    64540, 64729, 64898, 65047, 65176, 65286, 65376, 65446, 65496, 65526, 
    65536 // 1.0 in Fixed Point (16.16)
};

// Fixed Point Sine
static inline fixed fixed_sin(int degrees) {
    degrees = degrees % 360;
    if (degrees < 0) degrees += 360;

    if (degrees <= 90) return SIN_LUT_90[degrees];
    if (degrees <= 180) return SIN_LUT_90[180 - degrees];
    if (degrees <= 270) return -SIN_LUT_90[degrees - 180];
    return -SIN_LUT_90[360 - degrees];
}

// Fixed Point Cosine (just sin(angle + 90))
static inline fixed fixed_cos(int degrees) {
    return fixed_sin(degrees + 90);
}

#endif // FIXED_MATH_H