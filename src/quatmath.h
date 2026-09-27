#ifndef QUATMATH_H_INCLUDED
#define QUATMATH_H_INCLUDED

#include "zouna/utils.h"

void ToQuat(float* in, float* out);
void ToEuler(float* in, float* out);
Vec3f GetAxisAngle(float* radians, Quat* q);

#endif // QUATMATH_H_INCLUDED
