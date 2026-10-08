#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <d3dx9.h>
#include "utils.h"
#include "globals.h"
extern "C" {
    #include "libsm64/decomp/include/surface_terrains.h"
}

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
	float q_x = in[0];
	float q_y = in[1];
	float q_z = in[2];
	float q_w = in[3];
    out[0] = -asinf(2 * (q_w * q_y - q_z * q_x)) + M_PI;
    out[1] = -atan2f(2 * (q_w * q_z + q_x * q_y), 1.f - 2.f * (q_y*q_y + q_z*q_z)) + M_PI;
    out[2] = atan2f(2 * (q_w * q_x + q_y * q_z), 1.f - 2.f * (q_x*q_x + q_y*q_y)) + M_PI;

	/*
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
	*/
}

void Mat3x3ToEuler(Mat3x3* in, Vec3f* out)
{
	float sy = std::hypot(in->m[0][0], in->m[1][0]);
    bool singular = sy < 1e-9;

    if (!singular)
    {
        out->x = atan2f(in->m[2][1], in->m[2][2]);
        out->y = atan2f(-in->m[2][0], sy);
        out->z = atan2f(in->m[1][0], in->m[0][0]);
    }
    else
    {
        out->x = atan2f(-in->m[1][2], in->m[1][1]);
        out->y = atan2f(-in->m[2][0], sy);
        out->z = 0.f; // deterministic representative at gimbal lock
    }
}

Vec3f GetAxisAngle(float* radians, Quat* q)
{
	float length = sqrtf( q->v.x * q->v.x + q->v.y * q->v.y + q->v.z * q->v.z );
	*radians = 2.0f * atan2f( length, q->w );
	if ( length > 0.0f )
	{
		float invLength = 1.0f / length;
		Vec3f axis = { invLength * q->v.x, invLength * q->v.y, invLength * q->v.z };
		return axis;
	}

	return (Vec3f){0,0,0};
}

uint32_t BaseObjectZ_GetName(void* pBaseObject)
{
	return *(uint32_t*)(pBaseObject+4);
}

void* BaseObjectZ_GetHandle(void* pBaseObject)
{
	return pBaseObject+8;
}

uint32_t DynArrayZ_GetSize(void* pDynArray)
{
	return *(uint32_t *)(pDynArray) >> 0xe;
}

void* DynArrayZ_GetItem(void* pDynArray, uint32_t i, uint32_t stride)
{
	uint32_t size = DynArrayZ_GetSize(pDynArray);
	if (i < 0 || i >= size) return 0;

	void* data = *(void **)(pDynArray+4);
	return data + i*stride;
}

static inline void Vec3_Add(Vec3f& pOut, Vec3f pLeft, Vec3f pRight)
{
	pOut.x = pLeft.x + pRight.x;
	pOut.y = pLeft.y + pRight.y;
	pOut.z = pLeft.z + pRight.z;
}

static inline void Vec3_Add_Scale(Vec3f& pOut, Vec3f pLeft, float scale, Vec3f pRight)
{
	pOut.x = pLeft.x + (pRight.x * scale);
	pOut.y = pLeft.y + (pRight.y * scale);
	pOut.z = pLeft.z + (pRight.z * scale);
}

static inline void Vec3_Scale(Vec3f& pOut, float scale, Vec3f& pIn)
{
	pOut.x = pIn.x * scale;
	pOut.y = pIn.y * scale;
	pOut.z = pIn.z * scale;
}

static inline void Vec4_Add(Vec4f& pOut, Vec4f pLeft, Vec4f pRight)
{
	pOut.x = pLeft.x + pRight.x;
	pOut.y = pLeft.y + pRight.y;
	pOut.z = pLeft.z + pRight.z;
	pOut.w = pLeft.w + pRight.w;
}

static inline void Vec4_Sub(Vec4f& pOut, Vec4f pLeft, Vec4f pRight)
{
	pOut.x = pLeft.x - pRight.x;
	pOut.y = pLeft.y - pRight.y;
	pOut.z = pLeft.z - pRight.z;
	pOut.w = 1.0f;
}

static inline void Vec4_Add_Scale(Vec4f& pOut, Vec4f pLeft, float scale, Vec4f pRight)
{
	pOut.x = pLeft.x + (pRight.x * scale);
	pOut.y = pLeft.y + (pRight.y * scale);
	pOut.z = pLeft.z + (pRight.z * scale);
	pOut.w = pLeft.w + (pRight.w * scale);
}

static inline void Vec4_Scale(Vec4f& pOut, float scale, Vec4f pIn)
{
	pOut.x = pIn.x * scale;
	pOut.y = pIn.y * scale;
	pOut.z = pIn.z * scale;
	pOut.w = pIn.w * scale;
}

static inline float Vec4_Dist2(Vec4f i_V1, Vec4f i_V2)
{
    Vec3f l_Delta = {
    	i_V1.x - i_V2.x,
    	i_V1.y - i_V2.y,
    	i_V1.z - i_V2.z
    };
    return l_Delta.x * l_Delta.x + l_Delta.y * l_Delta.y + l_Delta.z * l_Delta.z;
}

static inline void Vec4_Cross(Vec4f& o_Vec, Vec4f i_A, Vec4f i_B)
{
    o_Vec.x = i_A.y * i_B.z - i_A.z * i_B.y;
    o_Vec.y = i_A.z * i_B.x - i_A.x * i_B.z;
    o_Vec.z = i_A.x * i_B.y - i_A.y * i_B.x;
    o_Vec.w = 1.0f;
}

static inline float Vec4_Dot(Vec4f i_A, Vec4f i_B)
{
    return i_A.x * i_B.x + i_A.y * i_B.y + i_A.z * i_B.z;
}

static void ConcatMat4x4(const Mat4x4& a1, const Mat4x4& a2, Mat4x4& a3)
{
    Mat4x4 l_Result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            l_Result.m[i][j] = a1.m[i][0] * a2.m[0][j] + a1.m[i][1] * a2.m[1][j] + a1.m[i][2] * a2.m[2][j] + a1.m[i][3] * a2.m[3][j];
        }
    }
    a3 = l_Result;
}

Mat4x4 Mat4x4::operator*(const Mat4x4& i_Matrix) const
{
    Mat4x4 l_Mat;
    ConcatMat4x4(*this, i_Matrix, l_Mat);
    return l_Mat;
}

static void GetQuadPatchCtrlPoint(void *pThis, void *pPatch, QuadCtrlPoint_Z *param_2)
{
	float *pfVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	uint16_t uVar5;
	uint16_t uVar6;
	uint16_t uVar7;
	int iVar8;
	int iVar9;
	int iVar10;
	int iVar11;
	int iVar12;
	int iVar13;
	int iVar14;
	uint32_t uVar15;
	int iVar16;
	uint32_t uVar17;
	uint32_t uVar18;
	uint32_t uVar19;
	Vec4f local_9c;
	Vec4f local_8c;
	Vec4f local_7c;
	Vec4f local_6c;

	uVar5 = *(uint16_t *)pPatch;
	uVar17 = uVar5 >> 1 & 1;
	uVar15 = uVar5 >> 2 & 1;
	uVar18 = uVar5 >> 3 & 1;
	uVar19 = uVar5 >> 4 & 1;
	uVar5 = *(uint16_t *)(pPatch + 6);
	uVar6 = *(uint16_t *)(pPatch + 8);
	uVar7 = *(uint16_t *)(pPatch + 10);
	iVar12 = (uint32_t)*(uint16_t *)(pPatch + 4) * 8;
	iVar8 = uVar17 * 2;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar8 + *(int *)(pThis + 0xac) + iVar12) * 0xc);
	param_2->m_ControlPoints[0][0].x = *pfVar1;
	param_2->m_ControlPoints[0][0].y = pfVar1[1];
	param_2->m_ControlPoints[0][0].z = pfVar1[2];
	param_2->m_ControlPoints[0][0].w = 1.0f;
	iVar16 = (uint32_t)uVar5 * 8;
	iVar9 = uVar15 * 2;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar9 + *(int *)(pThis + 0xac) + iVar16) * 0xc);
	param_2->m_ControlPoints[0][3].x = *pfVar1;
	param_2->m_ControlPoints[0][3].y = pfVar1[1];
	param_2->m_ControlPoints[0][3].z = pfVar1[2];
	param_2->m_ControlPoints[0][3].w = 1.0f;
	iVar13 = (uint32_t)uVar6 * 8;
	iVar10 = uVar18 * 2;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar10 + *(int *)(pThis + 0xac) + iVar13) * 0xc);
	param_2->m_ControlPoints[3][3].x = *pfVar1;
	param_2->m_ControlPoints[3][3].y = pfVar1[1];
	param_2->m_ControlPoints[3][3].z = pfVar1[2];
	param_2->m_ControlPoints[3][3].w = 1.0f;
	iVar14 = (uint32_t)uVar7 * 8;
	iVar11 = uVar19 * 2;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar11 + *(int *)(pThis + 0xac) + iVar14) * 0xc);
	param_2->m_ControlPoints[3][0].x = *pfVar1;
	param_2->m_ControlPoints[3][0].y = pfVar1[1];
	param_2->m_ControlPoints[3][0].z = pfVar1[2];
	param_2->m_ControlPoints[3][0].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar8 + iVar12 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[0][1].x = *pfVar1;
	param_2->m_ControlPoints[0][1].y = pfVar1[1];
	param_2->m_ControlPoints[0][1].z = pfVar1[2];
	param_2->m_ControlPoints[0][1].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar12 + (1 - uVar17) * 2 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[0][2].x = *pfVar1;
	param_2->m_ControlPoints[0][2].y = pfVar1[1];
	param_2->m_ControlPoints[0][2].z = pfVar1[2];
	param_2->m_ControlPoints[0][2].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar9 + iVar16 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[1][3].x = *pfVar1;
	param_2->m_ControlPoints[1][3].y = pfVar1[1];
	param_2->m_ControlPoints[1][3].z = pfVar1[2];
	param_2->m_ControlPoints[1][3].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar16 + (1 - uVar15) * 2 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[2][3].x = *pfVar1;
	param_2->m_ControlPoints[2][3].y = pfVar1[1];
	param_2->m_ControlPoints[2][3].z = pfVar1[2];
	param_2->m_ControlPoints[2][3].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar10 + iVar13 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[3][2].x = *pfVar1;
	param_2->m_ControlPoints[3][2].y = pfVar1[1];
	param_2->m_ControlPoints[3][2].z = pfVar1[2];
	param_2->m_ControlPoints[3][2].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar13 + (1 - uVar18) * 2 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[3][1].x = *pfVar1;
	param_2->m_ControlPoints[3][1].y = pfVar1[1];
	param_2->m_ControlPoints[3][1].z = pfVar1[2];
	param_2->m_ControlPoints[3][1].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar11 + iVar14 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[2][0].x = *pfVar1;
	param_2->m_ControlPoints[2][0].y = pfVar1[1];
	param_2->m_ControlPoints[2][0].z = pfVar1[2];
	param_2->m_ControlPoints[2][0].w = 1.0f;
	pfVar1 = (float *)(*(int *)(pThis + 0x7c) +
					(uint32_t)*(uint16_t *)(iVar14 + (1 - uVar19) * 2 + 4 + *(int *)(pThis + 0xac)) * 0xc);
	param_2->m_ControlPoints[1][0].x = *pfVar1;
	param_2->m_ControlPoints[1][0].y = pfVar1[1];
	param_2->m_ControlPoints[1][0].z = pfVar1[2];
	param_2->m_ControlPoints[1][0].w = 1.0f;
	Vec4_Add(local_6c, param_2->m_ControlPoints[1][0], param_2->m_ControlPoints[0][1]);
	fVar2 = param_2->m_ControlPoints[0][0].z;
	fVar3 = param_2->m_ControlPoints[0][0].y;
	fVar4 = param_2->m_ControlPoints[0][0].x;
	param_2->m_ControlPoints[1][1].w = 1.0f;
	param_2->m_ControlPoints[1][1].z = local_6c.z - fVar2;
	param_2->m_ControlPoints[1][1].y = local_6c.y - fVar3;
	param_2->m_ControlPoints[1][1].x = local_6c.x - fVar4;
	Vec4_Add(local_7c, param_2->m_ControlPoints[0][2], param_2->m_ControlPoints[1][3]);
	fVar2 = param_2->m_ControlPoints[0][3].z;
	fVar3 = param_2->m_ControlPoints[0][3].y;
	fVar4 = param_2->m_ControlPoints[0][3].x;
	param_2->m_ControlPoints[1][2].w = 1.0f;
	param_2->m_ControlPoints[1][2].z = local_7c.z - fVar2;
	param_2->m_ControlPoints[1][2].y = local_7c.y - fVar3;
	param_2->m_ControlPoints[1][2].x = local_7c.x - fVar4;
	Vec4_Add(local_8c, param_2->m_ControlPoints[2][3], param_2->m_ControlPoints[3][2]);
	fVar2 = param_2->m_ControlPoints[3][3].z;
	fVar3 = param_2->m_ControlPoints[3][3].y;
	fVar4 = param_2->m_ControlPoints[3][3].x;
	param_2->m_ControlPoints[2][2].w = 1.0f;
	param_2->m_ControlPoints[2][2].z = local_8c.z - fVar2;
	param_2->m_ControlPoints[2][2].y = local_8c.y - fVar3;
	param_2->m_ControlPoints[2][2].x = local_8c.x - fVar4;
	Vec4_Add(local_9c, param_2->m_ControlPoints[3][1], param_2->m_ControlPoints[2][0]);
	fVar2 = param_2->m_ControlPoints[3][0].z;
	fVar3 = param_2->m_ControlPoints[3][0].y;
	fVar4 = param_2->m_ControlPoints[3][0].x;
	param_2->m_ControlPoints[2][1].w = 1.0f;
	param_2->m_ControlPoints[2][1].z = local_9c.z - fVar2;
	param_2->m_ControlPoints[2][1].y = local_9c.y - fVar3;
	param_2->m_ControlPoints[2][1].x = local_9c.x - fVar4;
}


void GetSurfaceVertices(void* pThis, Vec3f rootPos, Vec3f angle, std::vector<SM64Surface>& outSurfaces)
{
	QuadCtrlPoint_Z l_CtrlPoints;
	//rootPos.x *= MARIO_SCALE;
	//rootPos.y *= MARIO_SCALE;
	//rootPos.z *= MARIO_SCALE;

	//printf("get patchID %d for Surface_Z %x\n", patchID, pThis);
	uint32_t numPatches = DynArrayZ_GetSize(pThis + 0xa0);
	Vec4f out[16];

	// Get vertices from tessellating control points
	for (uint32_t patchID = 0; patchID < numPatches; patchID++)
	{
		void* pPatch = DynArrayZ_GetItem(pThis + 0xa0, patchID, 0xb0);
		GetQuadPatchCtrlPoint(pThis, pPatch, &l_CtrlPoints);

		int l_Lod = 3;
		Vec4f l_D0[4];
		Vec4f l_D1[4];
		Vec4f l_D2[4];
		Vec4f l_D3[4];
		Vec4f* l_Out = out;
		float l_Step = 1.0f / (float)l_Lod;
		float l_Step2 = l_Step * l_Step;
		float l_Step3 = l_Step2 * l_Step;
		int i;
		int j;

		for (i = 0; i < 4; i++) {
			Vec4f l_A;
			Vec4f l_B;
			Vec4f l_C;
			Vec4f l_D;

			Vec4_Scale(l_A, -1.0f, l_CtrlPoints.m_ControlPoints[0][i]);
			Vec4_Add_Scale(l_A, l_A, 3.0f, l_CtrlPoints.m_ControlPoints[1][i]);
			Vec4_Add_Scale(l_A, l_A, -3.0f, l_CtrlPoints.m_ControlPoints[2][i]);
			Vec4_Add_Scale(l_A, l_A, 1.0f, l_CtrlPoints.m_ControlPoints[3][i]);

			Vec4_Scale(l_B, 3.0f, l_CtrlPoints.m_ControlPoints[0][i]);
			Vec4_Add_Scale(l_B, l_B, -6.0f, l_CtrlPoints.m_ControlPoints[1][i]);
			Vec4_Add_Scale(l_B, l_B, 3.0f, l_CtrlPoints.m_ControlPoints[2][i]);

			Vec4_Scale(l_C, -3.0f, l_CtrlPoints.m_ControlPoints[0][i]);
			Vec4_Add_Scale(l_C, l_C, 3.0f, l_CtrlPoints.m_ControlPoints[1][i]);

			l_D0[i] = l_D = l_CtrlPoints.m_ControlPoints[0][i];

			Vec4_Scale(l_D1[i], l_Step3, l_A);
			Vec4_Add_Scale(l_D1[i], l_D1[i], l_Step2, l_B);
			Vec4_Add_Scale(l_D1[i], l_D1[i], l_Step, l_C);

			Vec4_Scale(l_D3[i], 6.0f * (l_Step3), l_A);
			Vec4_Add_Scale(l_D2[i], l_D3[i], l_Step2, l_B * 2.0f);
		}

		for (i = 0; i < l_Lod + 1; i++) {
			Vec4f l_A;
			Vec4f l_B;
			Vec4f l_C;
			Vec4f l_D;
			Vec4f l_Cur;
			Vec4f l_Delta1;
			Vec4f l_Delta2;
			Vec4f l_Delta3;

			Vec4_Scale(l_A, -1.0f, l_D0[0]);
			Vec4_Add_Scale(l_A, l_A, 3.0f, l_D0[1]);
			Vec4_Add_Scale(l_A, l_A, -3.0f, l_D0[2]);
			Vec4_Add_Scale(l_A, l_A, 1.0f, l_D0[3]);

			Vec4_Scale(l_B, 3.0f, l_D0[0]);
			Vec4_Add_Scale(l_B, l_B, -6.0f, l_D0[1]);
			Vec4_Add_Scale(l_B, l_B, 3.0f, l_D0[2]);

			Vec4_Scale(l_C, -3.0f, l_D0[0]);
			Vec4_Add_Scale(l_C, l_C, 3.0f, l_D0[1]);

			l_Cur = l_D = l_D0[0];

			Vec4_Scale(l_Delta1, l_Step3, l_A);
			Vec4_Add_Scale(l_Delta1, l_Delta1, l_Step2, l_B);
			Vec4_Add_Scale(l_Delta1, l_Delta1, l_Step, l_C);

			Vec4_Scale(l_Delta3, 6.0f * (l_Step3), l_A);
			Vec4_Add_Scale(l_Delta2, l_Delta3, 2.0f * l_Step2, l_B);

			for (j = 0; j < l_Lod + 1; j++) {
				*l_Out = l_Cur;
				l_Out++;

				Vec4_Add(l_Cur, l_Cur, l_Delta1);
				Vec4_Add(l_Delta1, l_Delta1, l_Delta2);
				Vec4_Add(l_Delta2, l_Delta2, l_Delta3);
			}

			for (j = 0; j < 4; j++) {
				Vec4_Add(l_D0[j], l_D0[j], l_D1[j]);
				Vec4_Add(l_D1[j], l_D1[j], l_D2[j]);
				Vec4_Add(l_D2[j], l_D2[j], l_D3[j]);
			}
		}

		Vec4f* l_Vtx0 = out;
		Vec4f* l_Vtx1 = out + l_Lod + 1;
		Vec4f* l_Vtx2 = l_Vtx1 + 1;
		Vec4f* l_Vtx3 = out + 1;

		for (i=0; i<l_Lod; i++)
		{
			for (j=0; j<l_Lod; j++)
			{
				SM64Surface surf1 = {
					SURFACE_DEFAULT,
					0,
					TERRAIN_STONE,
					{
						{rootPos.x, rootPos.y, rootPos.z},
						{angle.x, angle.y, angle.z}
					},
					{
						{(int)(l_Vtx0->x * MARIO_SCALE), (int)(l_Vtx0->y * MARIO_SCALE), (int)(l_Vtx0->z * MARIO_SCALE)},
						{(int)(l_Vtx1->x * MARIO_SCALE), (int)(l_Vtx1->y * MARIO_SCALE), (int)(l_Vtx1->z * MARIO_SCALE)},
						{(int)(l_Vtx2->x * MARIO_SCALE), (int)(l_Vtx2->y * MARIO_SCALE), (int)(l_Vtx2->z * MARIO_SCALE)},
					}
				};

				SM64Surface surf2 = {
					SURFACE_DEFAULT,
					0,
					TERRAIN_STONE,
					{
						{rootPos.x, rootPos.y, rootPos.z},
						{angle.x, angle.y, angle.z}
					},
					{
						{(int)(l_Vtx0->x * MARIO_SCALE), (int)(l_Vtx0->y * MARIO_SCALE), (int)(l_Vtx0->z * MARIO_SCALE)},
						{(int)(l_Vtx2->x * MARIO_SCALE), (int)(l_Vtx2->y * MARIO_SCALE), (int)(l_Vtx2->z * MARIO_SCALE)},
						{(int)(l_Vtx3->x * MARIO_SCALE), (int)(l_Vtx3->y * MARIO_SCALE), (int)(l_Vtx3->z * MARIO_SCALE)},
					}
				};

				outSurfaces.push_back(surf1);
				outSurfaces.push_back(surf2);

				l_Vtx0++;
				l_Vtx1++;
				l_Vtx2++;
				l_Vtx3++;
			}

			l_Vtx0++;
            l_Vtx1++;
            l_Vtx2++;
            l_Vtx3++;
		}
	}
}
