#include "quatmath.h"

#define _USE_MATH_DEFINES
#include <math.h>

void ToQuat(float* in, float* out)
{
	float pitch = in[0];
	float yaw = in[1];
	float roll = in[2];

	float rollOver2 = roll * 0.5f;
	float cosRollOver2 = cosf(rollOver2);
	float sinRollOver2 = sinf(rollOver2);
	float pitchOver2 = pitch * 0.5f;
	float cosPitchOver2 = cosf(pitchOver2);
	float sinPitchOver2 = sinf(pitchOver2);
	float yawOver2 = yaw * 0.5f;
	float cosYawOver2 = cosf(yawOver2);
	float sinYawOver2 = sinf(yawOver2);

	out[0] = cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2; // x
	out[1] = sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2; // y
	out[2] = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2; // z
	out[3] = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2; // w
}

void ToEuler(float* in, float* out)
{
	float sqw = in[3] * in[3];
	float sqx = in[0] * in[0];
	float sqy = in[1] * in[1];
	float sqz = in[2] * in[2];
	float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
	float test = in[0] * in[3] - in[1] * in[2];

	if (test>0.4995f*unit) // singularity at north pole
	{
		out[1] = 2.f * atan2f(in[1], in[0]);
		out[0] = M_PI / 2;
		out[2] = 0;
		return;
	}
	if (test<-0.4995f*unit) // singularity at south pole
	{
		out[1] = -2.f * atan2f(in[1], in[0]);
		out[0] = -M_PI / 2;
		out[2] = 0;
		return;
	}

	out[0] = asinf(2.f * (in[3] * in[0] - in[1] * in[2]));                             // Pitch
	out[1] = atan2f(2.f * in[3] * in[1] + 2.f * in[2] * in[0], 1 - 2.f * (in[0] * in[0] + in[1] * in[1]));     // Yaw
	out[2] = atan2f(2.f * in[3] * in[2] + 2.f * in[0] * in[1], 1 - 2.f * (in[2] * in[2] + in[0] * in[0]));      // Roll

	for (int i=0; i<3; i++)
		while (out[i] < 0) out[i] += M_PI;
}
