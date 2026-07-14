#ifndef INC_FIXED_MATH_H_
#define INC_FIXED_MATH_H_

#include "stdint.h"

/* Q30: 1.0 = 2^30. Angles are stored as radians in Q30. */
#define Q30              1073741824L
#define Q30_PI           3373259426L
#define Q30_HALF_PI      1686629713L
#define Q30_CENTI_DEG    5730L   /* (180/pi)*100, for rad Q30 -> 0.01 deg */

int32_t q30_mul(int32_t a, int32_t b);
int32_t q30_asin(int32_t x);
int32_t q30_atan2(int32_t y, int32_t x);

/* Convert Q30 radians to centi-degrees (0.01 deg units). */
static inline int32_t q30_to_centi_deg(int32_t rad_q30)
{
    return (int32_t)(((int64_t)rad_q30 * Q30_CENTI_DEG) >> 30);
}

#endif /* INC_FIXED_MATH_H_ */
