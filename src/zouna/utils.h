#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <cstring>
#include <cstdint>
#include <cfloat>
#include <vector>
#include "../mod.h"
#include "libsm64/libsm64.h"

struct Vec2f
{
    float x;
    float y;
};

struct Vec3f
{
    float x;
    float y;
    float z;
};

struct Vec4f {
    float x, y, z, w;

    Vec4f() { }

    Vec4f(const Vec3f& i_Vec) {
        x = i_Vec.x;
        y = i_Vec.y;
        z = i_Vec.z;
        w = 1.f;
    }

    Vec4f(float i_x, float i_y, float i_z, float _w) {
        x = i_x;
        y = i_y;
        z = i_z;
        w = _w;
    }

    Vec4f operator*(float i_Factor) const { return Vec4f(x * i_Factor, y * i_Factor, z * i_Factor, w * i_Factor); }

    Vec4f& operator*=(float i_Factor);

    float operator*(const Vec4f& i_Vec) const { return x * i_Vec.x + y * i_Vec.y + z * i_Vec.z; }
};

struct Quat {
    Vec3f v;
    float w;
};

struct Mat3x3 {
    float m[3][4];
};

struct Mat4x4 {
    float m[4][4];

    Mat4x4() {}

    Mat4x4(float* values) {
		memcpy(m, values, sizeof(m));
    }

    Mat4x4(
        float i_00, float i_01, float i_02, float i_03,
        float i_10, float i_11, float i_12, float i_13,
        float i_20, float i_21, float i_22, float i_23,
        float i_30, float i_31, float i_32, float i_33
    ) {
        m[0][0] = i_00;
        m[0][1] = i_01;
        m[0][2] = i_02;
        m[0][3] = i_03;
        m[1][0] = i_10;
        m[1][1] = i_11;
        m[1][2] = i_12;
        m[1][3] = i_13;
        m[2][0] = i_20;
        m[2][1] = i_21;
        m[2][2] = i_22;
        m[2][3] = i_23;
        m[3][0] = i_30;
        m[3][1] = i_31;
        m[3][2] = i_32;
        m[3][3] = i_33;
    }

    Vec4f operator*(const Vec4f& i_Vec) const {
		Vec4f l_Vec;
		float l_Z = i_Vec.z;
		float l_X = i_Vec.x;
		l_Vec.x = m[0][0] * l_X + m[1][0] * i_Vec.y + m[2][0] * l_Z + m[3][0];
		l_Vec.y = m[0][1] * l_X + m[1][1] * i_Vec.y + m[2][1] * l_Z + m[3][1];
		l_Vec.z = m[0][2] * l_X + m[1][2] * i_Vec.y + m[2][2] * l_Z + m[3][2];
		l_Vec.w = 1.0f;
		return l_Vec;
	}

	Mat4x4 operator*(const Mat4x4& i_m) const;

	void Transp(Mat4x4& o_Matrix) const {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				o_Matrix.m[i][j] = m[j][i];
			}
		}
	}

	void Inverse(Mat4x4& Out) const {
		float Det = 0.f;
		Det += m[0][0] * m[1][1] * m[2][2];
		Det += m[1][0] * m[2][1] * m[0][2];
		Det += m[2][0] * m[0][1] * m[1][2];
		Det -= m[2][0] * m[1][1] * m[0][2];
		Det -= m[1][0] * m[0][1] * m[2][2];
		Det -= m[0][0] * m[2][1] * m[1][2];

		float Det1 = 1.f / Det;

		Out.m[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * Det1;
		Out.m[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * Det1;
		Out.m[0][2] = (m[0][1] * m[1][2] - m[1][1] * m[0][2]) * Det1;
		Out.m[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * Det1;
		Out.m[1][1] = (m[0][0] * m[2][2] - m[2][0] * m[0][2]) * Det1;
		Out.m[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * Det1;
		Out.m[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * Det1;
		Out.m[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * Det1;
		Out.m[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * Det1;

		Out.m[3][0] = -(Out.m[0][0] * m[3][0] + Out.m[1][0] * m[3][1] + Out.m[2][0] * m[3][2]);
		Out.m[3][1] = -(Out.m[0][1] * m[3][0] + Out.m[1][1] * m[3][1] + Out.m[2][1] * m[3][2]);
		Out.m[3][2] = -(Out.m[0][2] * m[3][0] + Out.m[1][2] * m[3][1] + Out.m[2][2] * m[3][2]);

		Out.m[0][3] = 0.f;
		Out.m[1][3] = 0.f;
		Out.m[2][3] = 0.f;
		Out.m[3][3] = 1.f;
	}
};

struct QuadCtrlPoint_Z
{
    Vec4f m_ControlPoints[4][4];
};

void ToQuat(float* in, float* out);
void ToEuler(float* in, float* out);
void Mat3x3ToEuler(Mat3x3* in, Vec3f* out);
Vec3f GetAxisAngle(float* radians, Quat* q);

void* BaseObjectZ_GetHandle(void* pBaseObject);
uint32_t DynArrayZ_GetSize(void* pDynArray);
void* DynArrayZ_GetItem(void* pDynArray, uint32_t i, uint32_t stride=4);
void GetSurfaceVertices(void* pThis, Vec3f rootPos, Vec3f angle, std::vector<SM64Surface>& outSurfaces);


#endif // UTILS_H_INCLUDED
