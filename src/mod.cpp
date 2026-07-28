#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "safetyhook/safetyhook.hpp"
#include "TinySHA1.hpp"
#include "libsm64/libsm64.h"
extern "C" {
    #include "libsm64/decomp/include/PR/ultratypes.h"
    #include "libsm64/decomp/include/audio_defines.h"
    #include "libsm64/decomp/include/surface_terrains.h"
    #include "libsm64/decomp/include/seq_ids.h"
}

#include "mod.h"
#include "audio.h"
#include "config.h"
#include "marioEffect.h"
#include "quatmath.h"

#define D3DFVF_WALLEVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)

struct WALLEVERTEX
{
	float  x,  y,  z; // position
	float nx, ny, nz; // normal
	D3DCOLOR color;
	float  u,  v;     // texture
};

void Message(const char* sometext)
{
    MessageBoxA(0, sometext, "sm64-wall-e", MB_OK | MB_ICONINFORMATION);
}

static uint8_t* marioTexture;
static float marioTimer = 0;
static int marioId = -1;
static SM64MarioState marioState = {0};
static SM64MarioInputs marioInput = {0};
static SM64MarioGeometryBuffers marioGeometry = {0};
LPDIRECT3DDEVICE9 d3d9Device = 0;
LPDIRECT3DTEXTURE9 marioTextureD3D9 = 0;
WALLEVERTEX* marioVerticesP = 0;
LPD3DXEFFECT marioEffect = 0;
float projMatrix[16] = {0};
float viewMatrix[16] = {0};
static void RenderMario();


auto RegisterCmd = 0x4763b0;
auto RunCmd = 0x476580;
auto ScriptManagerG_Init = 0x6b3c70;
auto ScriptManagerG_GetMainPlayer = 0x6b3740;
auto HandleManagerZ_GetPtr = 0x586a70;
auto LodMoveZ_GetPos = 0x435440;
auto ObjectMoveZ_GetScale = 0x433170;
auto ObjectMoveZ_GetRot = 0x4331e0;
auto LodMoveZ_SetPos = 0x4354b0;
auto LodMoveZ_SetPosAndRot = 0x435680;
auto LodMoveZ_UpdateCollision = 0x435db0;
auto CreaturesMoveG_SetMyFuturePos = 0x5eef10;
auto GameZ_Update = 0x438620;
auto GameZ_GetFirstVp = 0x437dd0;
auto WorldZ_LoadDone = 0x4bd760;
auto CreaturesG_Init = 0x6970a0;
auto CreaturesG_Sleep = 0x5ebb90;
auto CreaturesG_WakeUp = 0x5ebbe0;
auto PlayerG_Init = 0x69b4a0;
auto PlayerG_Suspend = 0x60aa40;
auto PlayerG_Restore = 0x60aa80;
auto PlayerMoveG_Destructor = 0x64b530;
auto PlayerMoveG_Stop = 0x60ea00;
auto PlayerMoveG_Update_SeadZone = 0x60f030;
auto PlayerMoveG_SetMyDynPosAndRot = 0x60e390;
auto PlayerMoveG_IsCurrentMusicForRejected = 0x60e020;
auto PlayerMoveG_GetMusicForRejected = 0x60e0c0;
auto CameraMoveG_Init = 0x5e2f10;
auto CameraEngineZ_GetCameraNode = 0x40ae90;
auto P_WALLE_0x608ee0 = 0x608ee0;
auto MusicManagerG_SetMusicZone = 0x644370;
auto D3D_RendererZ_Init = 0x59c750;
auto D3D_RendererZ_Shut = 0x59cd50;
auto D3D_RendererZ_BeginRender = 0x5b5120;
auto D3D_RendererZ_EndRender = 0x5b5140;
auto RendererZ_DrawString = 0x5cb8e0;
auto D3D_RendererZ_PushProjMatrix = 0x5aed70;
auto D3D_RendererZ_PushViewMatrix = 0x5aee00;
auto ClearZBuffer = 0x58ffb0;
SafetyHookInline RegisterCmdOrig;
SafetyHookInline RunCmdOrig;
SafetyHookInline ScriptManagerG_Init_Orig;
SafetyHookInline ScriptManagerG_GetMainPlayer_Orig;
SafetyHookInline HandleManagerZ_GetPtr_Orig;
SafetyHookInline LodMoveZ_GetPos_Orig;
SafetyHookInline ObjectMoveZ_GetScale_Orig;
SafetyHookInline ObjectMoveZ_GetRot_Orig;
SafetyHookInline LodMoveZ_SetPos_Orig;
SafetyHookInline LodMoveZ_SetPosAndRot_Orig;
SafetyHookInline LodMoveZ_UpdateCollision_Orig;
SafetyHookInline CreaturesMoveG_SetMyFuturePos_Orig;
SafetyHookInline GameZ_Update_Orig;
SafetyHookInline GameZ_GetFirstVp_Orig;
SafetyHookInline WorldZ_LoadDone_Orig;
SafetyHookInline CreaturesG_Init_Orig;
SafetyHookInline CreaturesG_Sleep_Orig;
SafetyHookInline CreaturesG_WakeUp_Orig;
SafetyHookInline PlayerG_Init_Orig;
SafetyHookInline PlayerG_Suspend_Orig;
SafetyHookInline PlayerG_Restore_Orig;
SafetyHookInline PlayerMoveG_Destructor_Orig;
SafetyHookInline PlayerMoveG_Stop_Orig;
SafetyHookInline PlayerMoveG_Update_SeadZone_Orig;
SafetyHookInline PlayerMoveG_SetMyDynPosAndRot_Orig;
SafetyHookInline PlayerMoveG_IsCurrentMusicForRejected_Orig;
SafetyHookInline PlayerMoveG_GetMusicForRejected_Orig;
SafetyHookInline CameraMoveG_Init_Orig;
SafetyHookInline CameraEngineZ_GetCameraNode_Orig;
SafetyHookInline P_WALLE_0x608ee0_Orig;
SafetyHookInline MusicManagerG_SetMusicZone_Orig;
SafetyHookInline D3D_RendererZ_Init_Orig;
SafetyHookInline D3D_RendererZ_Shut_Orig;
SafetyHookInline D3D_RendererZ_BeginRender_Orig;
SafetyHookInline D3D_RendererZ_EndRender_Orig;
SafetyHookInline RendererZ_DrawString_Orig;
SafetyHookInline D3D_RendererZ_PushProjMatrix_Orig;
SafetyHookInline D3D_RendererZ_PushViewMatrix_Orig;
SafetyHookInline ClearZBuffer_Orig;
SafetyHookInline D3D_Clear_Orig;
safetyhook::MidHook D3D_RendererZ_PushProjMatrix_MidOrig;
safetyhook::MidHook D3D_RendererZ_PushViewMatrix_MidOrig;
SAFETYHOOK_STDCALL HRESULT D3D_Clear_Hook(LPDIRECT3DDEVICE9, DWORD, const D3DRECT*, DWORD, D3DCOLOR, float, DWORD);

void* gData = (void*)0x3f0018;
void* ConsoleZ = 0;
void* PlayerG = 0;
void* ScriptManagerG = 0;
void* HandleManagerZ = 0;
void* GameZ_Vp = 0;
void* GameZ = 0;
void* CameraMoveG = 0;

SAFETYHOOK_THISCALL void RegisterCmdHook(void* pThis, const char* cmd, void* param_3)
{
	printf("Register: %x '%s'\n", cmd, pThis);
	RegisterCmdOrig.thiscall<void>(pThis, cmd, param_3);
}

SAFETYHOOK_THISCALL bool RunCmdHook(void* pThis, const char* cmd, void* unknown1)
{
	ConsoleZ = pThis;
	//bool dontLog = (strncmp(cmd, "MENU", 4) == 0);
	bool dontLog = true;
	if (!dontLog)
		printf("cmd='%s' ", cmd);
	bool res = RunCmdOrig.thiscall<bool>(pThis, cmd, unknown1);
	if (!dontLog)
		printf("result: %d\n", res);
	return res;
}

// 0x6b3c70 PC, 0x198294 Mac
SAFETYHOOK_THISCALL void ScriptManagerG_Init_Hook(void* pThis)
{
	printf("ScriptManager_G::Init(): %x\n", pThis);
	ScriptManagerG = pThis;
	ScriptManagerG_Init_Orig.thiscall<void>(pThis);
}

// 0x6b3740 PC
SAFETYHOOK_THISCALL void* ScriptManagerG_GetMainPlayer_Hook(void* pThis, uint32_t param_2)
{
	void* pPlayer = ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(pThis, param_2);
	return pPlayer;
}

// 0x586a70 PC
SAFETYHOOK_THISCALL void* HandleManagerZ_GetPtr_Hook(void* pThis, void* param_2)
{
	if (HandleManagerZ != pThis) HandleManagerZ = pThis;
	void* result = HandleManagerZ_GetPtr_Orig.thiscall<void*>(pThis, param_2);
	return result;
}

// 0x435440 PC
SAFETYHOOK_THISCALL float* LodMoveZ_GetPos_Hook(void* pThis, int param_2)
{
	float* result = LodMoveZ_GetPos_Orig.thiscall<float*>(pThis, param_2);
	//printf("LodMove_Z::GetPos(): %x, %d = {%.3f, %.3f, %.3f}\n", pThis, param_2, result[0], result[1], result[2]);
	return result;
}

// 0x433170 PC
SAFETYHOOK_THISCALL float ObjectMoveZ_GetScale_Hook(void* pThis, int param_2)
{
	return ObjectMoveZ_GetScale_Orig.thiscall<float>(pThis, param_2);
}

// 0x4331e0 PC
SAFETYHOOK_THISCALL float* ObjectMoveZ_GetRot_Hook(void* pThis, int param_2)
{
	return ObjectMoveZ_GetRot_Orig.thiscall<float*>(pThis, param_2);
}

// 0x4354b0 PC
SAFETYHOOK_THISCALL void LodMoveZ_SetPos_Hook(void* pThis, float* pos, int param_3)
{
	//printf("LodMove_Z::SetPos(): %x, {%.3f, %.3f, %.3f}, %d\n", pThis, pos[0], pos[1], pos[2], param_3);
	LodMoveZ_SetPos_Orig.thiscall<void>(pThis, pos, param_3);
}

// 0x435680 PC
SAFETYHOOK_THISCALL void LodMoveZ_SetPosAndRot_Hook(void* pThis, float* pos, float* quat, int param_4)
{
	//printf("LodMove_Z::SetPosAndRot(): %x, {%.3f, %.3f, %.3f}, %d\n", pThis, pos[0], pos[1], pos[2], param_4);
	LodMoveZ_SetPosAndRot_Orig.thiscall<void>(pThis, pos, quat, param_4);
}

SAFETYHOOK_THISCALL void LodMoveZ_UpdateCollision_Hook(void* pThis, void* SeadZoneZ, float* float3_1, float* float3_2, float* float3_3, float* float3_4, float param_6, long param_7)
{
	if (pThis == ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(ScriptManagerG, 0))
	{
		printf("LodMoveZ::UpdateCollision(): this is player (pThis=%x)\n", pThis);
	}
	LodMoveZ_UpdateCollision_Orig.thiscall<void>(pThis, SeadZoneZ, float3_1, float3_2, float3_3, float3_4, param_6, param_7);
}

// 0x5eef10 PC
SAFETYHOOK_THISCALL void CreaturesMoveG_SetMyFuturePos_Hook(void* pThis, float* pos, bool param_3)
{
	CreaturesMoveG_SetMyFuturePos_Orig.thiscall<void>(pThis, pos, param_3);
}

// 0x437dd0 PC
SAFETYHOOK_THISCALL void* GameZ_GetFirstVp_Hook(void* pThis)
{
	GameZ_Vp = pThis;
	return GameZ_GetFirstVp_Orig.thiscall<void*>(pThis);
}

SAFETYHOOK_THISCALL void WorldZ_LoadDone_Hook(void* pThis)
{
	printf("World_Z::LoadDone(): %x\n", pThis);
	WorldZ_LoadDone_Orig.thiscall<void>(pThis);
}

// 0x60aa40 PC
SAFETYHOOK_THISCALL void PlayerG_Suspend_Hook(void* pThis)
{
	printf("Player_G::Suspend(): %x\n", pThis);
	PlayerG_Suspend_Orig.thiscall<void>(pThis);
}

// 0x60aa80 PC
SAFETYHOOK_THISCALL void PlayerG_Restore_Hook(void* pThis)
{
	printf("Player_G::Restore(): %x\n", pThis);
	PlayerG_Restore_Orig.thiscall<void>(pThis);
}

// 0x69b4a0 PC, 0x15ce40 Mac
SAFETYHOOK_THISCALL void PlayerG_Init_Hook(void* pThis)
{
	printf("Player_G::Init(): %x\n", pThis);
	PlayerG_Init_Orig.thiscall<void>(pThis);
	PlayerG = pThis;

	void* pMainPlayer = ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(ScriptManagerG, 0);
	if (!pMainPlayer)
		return;

	void* pPlayerMove = HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, pMainPlayer + 0x70); // field 0x70 is BaseObject_Z
	if (!pPlayerMove)
		return;

	float* quat = ObjectMoveZ_GetRot_Orig.thiscall<float*>(pPlayerMove, 0);
	float* pos = LodMoveZ_GetPos_Orig.thiscall<float*>(pPlayerMove, 0);
	float angle[3] = {0};
	float x = pos[0];
	float y = pos[1];
	float z = pos[2];
	ToEuler(quat, angle);

	// TODO: load level collision

	SM64Surface surfaces[2];
	uint32_t surfaceCount = sizeof(surfaces) / sizeof(SM64Surface);
	int size = 512 * MARIO_SCALE;
    int spawnX = x * MARIO_SCALE;
    int spawnY = y * MARIO_SCALE;
    int spawnZ = z * MARIO_SCALE;

	for (uint32_t i=0; i<surfaceCount; i++)
	{
		surfaces[i].type = SURFACE_DEFAULT;
		surfaces[i].force = 0;
		surfaces[i].terrain = TERRAIN_STONE;
	}

	surfaces[0].vertices[0][0] = spawnX+size/2;	surfaces[0].vertices[0][1] = spawnY;	surfaces[0].vertices[0][2] = spawnZ+size/2;
    surfaces[0].vertices[1][0] = spawnX-size/2;	surfaces[0].vertices[1][1] = spawnY;	surfaces[0].vertices[1][2] = spawnZ-size/2;
    surfaces[0].vertices[2][0] = spawnX-size/2;	surfaces[0].vertices[2][1] = spawnY;	surfaces[0].vertices[2][2] = spawnZ+size/2;

    surfaces[1].vertices[0][0] = spawnX-size/2;	surfaces[1].vertices[0][1] = spawnY;	surfaces[1].vertices[0][2] = spawnZ-size/2;
    surfaces[1].vertices[1][0] = spawnX+size/2;	surfaces[1].vertices[1][1] = spawnY;	surfaces[1].vertices[1][2] = spawnZ+size/2;
    surfaces[1].vertices[2][0] = spawnX+size/2;	surfaces[1].vertices[2][1] = spawnY;	surfaces[1].vertices[2][2] = spawnZ-size/2;

	sm64_static_surfaces_load(surfaces, surfaceCount);

	if (marioId >= 0)
	{
		sm64_mario_delete(marioId);
		printf("Deleted Mario %d\n", marioId);
	}

	marioId = sm64_mario_create(x * MARIO_SCALE, y * MARIO_SCALE, z * MARIO_SCALE);
	if (marioId < 0)
	{
		printf("Failed to spawn Mario at %.2f, %.2f, %.2f\n", x * MARIO_SCALE, y * MARIO_SCALE, z * MARIO_SCALE);
		return;
	}
	printf("Spawned Mario %d at %.2f, %.2f, %.2f\n", marioId, x * MARIO_SCALE, y * MARIO_SCALE, z * MARIO_SCALE);
	marioTimer = 0;
	sm64_set_mario_faceangle(marioId, angle[2]);

	//CreaturesG_Sleep_Orig.thiscall<void>(pMainPlayer);
	CreaturesG_Sleep_Orig.thiscall<void>(pThis);

	// make default player invisible
	void* pNode = HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, pMainPlayer + 0x54);
	float* pNodeColor = (float*)(pNode + 0xfc);
	pNodeColor[3] = 0.f;

	// NOTE: SeadZone_Z has World_Z in 0x2c: (*(World_Z **)(param_1 + 0x2c)
	// get it from the other PlayerMove_G::Update() function that takes SeadZone_Z as a param

	//void* pWorld_Z = HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, GameZ + 0xc); // 0xc is World_ZHdl
	//printf("World_Z = %x, has %d Node_ZHdl's\n", pWorld_Z, (int)(*(uint32_t *)(pWorld_Z + 0xdc) >> 0xe));
	/*
	for (iVar16 = 0; iVar16 < (int)(*(uint *)(this + 0xdc) >> 0xe); iVar16 = iVar16 + 1)
	{
		pNVar13 = (Node_Z *)HandleManager_Z::GetPtr(
			HandleManagerZ, (BaseObject_ZHdl *)(*(int *)(this + 0xe0) + iVar16 * 4)
		);
	}
	*/
}

// 0x64b530 PC, 0x15ac56 Mac
SAFETYHOOK_THISCALL void* PlayerMoveG_Destructor_Hook(void* pThis, char param_2)
{
	printf("PlayerMove_G::~PlayerMove_G(): %x, %d\n", pThis, param_2);
	void* result = PlayerMoveG_Destructor_Orig.thiscall<void*>(pThis, param_2);

	if (marioId >= 0)
	{
		sm64_mario_delete(marioId);
		printf("Deleted Mario %d\n", marioId);
		marioId = -1;
	}

	return result;
}

// 0x60ea00 PC
SAFETYHOOK_THISCALL void PlayerMoveG_Stop_Hook(void* pThis)
{
	printf("PlayerMove_G::Stop(): %x\n", pThis);
	PlayerMoveG_Stop_Orig.thiscall<void>(pThis);
}

SAFETYHOOK_THISCALL void PlayerMoveG_Update_SeadZone_Hook(void* pThis, void* pSeadZone, float* float3_1, float* float3_2, float* float3_3, float param_5, long param_6)
{
	printf("PlayerMove_G::Update() (SeadZone_Z): %x, %x, {%.3f, %.3f, %.3f}, {%.3f, %.3f, %.3f}, {%.3f, %.3f, %.3f}, %.3f, %d\n", pThis, pSeadZone, float3_1[0], float3_1[1], float3_1[2], float3_2[0], float3_2[1], float3_2[2], float3_3[0], float3_3[1], float3_3[2], param_5, param_6);

	/*
	SeadHandle_Z::Load((SeadHandle_Z *)(this + 0x44),param_1);
	SeadHandle_Z::Load((SeadHandle_Z *)(this + 0x84),param_1);
	*/
	//void* pWorldZ = *(void **)(pSeadZone + 0x2c);
	//printf("pWorldZ = %x, has %d Node_ZHdl's\n", pWorldZ, (int)(*(uint32_t *)(pWorldZ + 0xdc) >> 0xe));
	printf("pSeadZone has %d / %d nodes\n", *(int *)(pSeadZone + 0x1c), *(int *)(pSeadZone + 0x20));

	PlayerMoveG_Update_SeadZone_Orig.thiscall<void>(pThis, pSeadZone, float3_1, float3_2, float3_3, param_5, param_6);
}

// 0x60e390 PC
SAFETYHOOK_THISCALL void PlayerMoveG_SetMyDynPosAndRot_Hook(void* pThis, float* pos, float* quat, bool param_4, bool param_5, bool param_6)
{
	printf("PlayerMove_G::SetMyDynPosAndRot(): %x, {%.2f, %.2f, %.2f}, {%.2f}, %d, %d, %d\n", pThis, pos[0], pos[1], pos[2], quat[0], param_4, param_5, param_6);
	PlayerMoveG_SetMyDynPosAndRot_Orig.thiscall<void>(pThis, pos, quat, param_4, param_5, param_6);
}

// 0x60e020 PC
SAFETYHOOK_THISCALL bool PlayerMoveG_IsCurrentMusicForRejected_Hook(void* pThis)
{
	if (marioId >= 0) return sm64_get_current_background_music() != 0xffff;
	return PlayerMoveG_IsCurrentMusicForRejected_Orig.thiscall<bool>(pThis);
}

// 0x60e0c0 PC
SAFETYHOOK_THISCALL uint32_t PlayerMoveG_GetMusicForRejected_Hook(void* pThis)
{
	return PlayerMoveG_GetMusicForRejected_Orig.thiscall<uint32_t>(pThis) + (marioId >= 0 ? 0x1000 : 0);
}

// 0x5e2f10 PC
SAFETYHOOK_THISCALL void CameraMoveG_Init_Hook(void* pThis)
{
	printf("CameraMove_G::Init(): %x\n", pThis);
	CameraMoveG = pThis;
	CameraMoveG_Init_Orig.thiscall<void>(pThis);
}

// 0x40ae90 PC
SAFETYHOOK_THISCALL void* CameraEngineZ_GetCameraNode_Hook(void* pThis)
{
	return CameraEngineZ_GetCameraNode_Orig.thiscall<void*>(pThis);
}

// 0x608ee0 PC
SAFETYHOOK_THISCALL uint32_t P_WALLE_0x608ee0_Hook(void* pThis)
{
	// draw tire tracks only if mario is not there
	return (marioId < 0);
}

// 0x644370 PC
SAFETYHOOK_THISCALL void MusicManagerG_SetMusicZone_Hook(void* pThis, uint32_t musicID, float param_3)
{
	void* pMainPlayer = ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(ScriptManagerG, 0);
	void* pPlayerMove = (pMainPlayer) ? HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, pMainPlayer + 0x70) : 0; // field 0x70 is BaseObject_Z

	if (musicID >= 0x1000)
		sm64_play_music(0, getConfig("rejectbot_music") | (getConfig("sm64_music_variation") ? SEQ_VARIATION : 0), 0);
	else if (sm64_get_current_background_music() != 0xffff)
		sm64_stop_background_music(sm64_get_current_background_music());

	MusicManagerG_SetMusicZone_Orig.thiscall<void>(pThis, musicID, param_3);
}

// 0x4198f0 PC, 0x4c6a8 Mac
float quat[4] = {0};
SAFETYHOOK_THISCALL void GameZ_Update_Hook(void* pThis, float dt)
{
	//printf("Game_Z::Update(): %x, dt=%f\n", pThis, dt);
	GameZ = pThis;

	void* pMainPlayer = ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(ScriptManagerG, 0);
	if (marioId >= 0)
	{
		void* pPlayerMove = (pMainPlayer) ? HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, pMainPlayer + 0x70) : 0; // field 0x70 is BaseObject_Z

		//float* pos = LodMoveZ_GetPos_Orig.thiscall<float*>(pPlayerMove, 0);

		marioTimer += dt;
		while (marioTimer > 1.f/30.f)
		{
			marioTimer -= 1.f/30.f;

			if (pPlayerMove && CameraMoveG)
			{
				void* CamNodeBase = CameraEngineZ_GetCameraNode_Orig.thiscall<void*>(CameraMoveG);
				void* CamNode = HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, CamNodeBase);
				void* CameraZ = (void*)(*(int *)(CamNode + 0x130));


				// from PlayerMove_G::GetPlayerInput() (0x60f530)
				void* PInput_G = (void*)(pPlayerMove + 0xac8);

				marioInput.buttonA = *(uint8_t *)(PInput_G + 0x20); // jump
				marioInput.buttonB = *(uint8_t *)(PInput_G + 0x34); // action / attack
				marioInput.buttonZ = *(uint8_t *)(PInput_G + 0x0c); // crouch
				marioInput.stickX = *(float *)(PInput_G + 0x280);
				marioInput.stickY = -*(float *)(PInput_G + 0x288);
				marioInput.camLookX = *(float *)(CameraZ + 0x080);
				marioInput.camLookZ = -*(float *)(CameraZ + 0x078);
				//float rStickX = *(float *)(PInput_G + 0x28c);
				//float rStickY = *(float *)(PInput_G + 0x294);
				//uint8_t rejectBotMusic = *(uint8_t *)(PInput_G + 0x70);
				//uint8_t firstPerson = *(uint8_t *)(PInput_G + 0x5c);
				//uint8_t laser = *(uint8_t *)(PInput_G + 0x48); // also applies to 0x84 for some reason
			}

			sm64_mario_tick(marioId, &marioInput, &marioState, &marioGeometry);
		}

		if (pPlayerMove)
		{
			//float* quat = ObjectMoveZ_GetRot_Orig.thiscall<float*>(pPlayerMove, 0);
			//printf("%.3f %.3f %.3f %.3f\n", quat[0], quat[1], quat[2], quat[3]);
			ToQuat(marioState.angle, quat);

			float pos[3] = {0};
			for (int i=0; i<3; i++) pos[i] = marioState.position[i] / MARIO_SCALE;
			PlayerMoveG_SetMyDynPosAndRot_Orig.thiscall<void>(pPlayerMove, pos, quat, true, true, true);
		}
	}

	GameZ_Update_Orig.thiscall<void>(pThis, dt);
}

// 0x6970a0 PC, 0x187ea6 Mac
SAFETYHOOK_THISCALL void CreaturesG_Init_Hook(void* pThis, char param_2)
{
	printf("Creatures_G::Init(): %x, %d\n", pThis, param_2);
	CreaturesG_Init_Orig.thiscall<void>(pThis, param_2);
}

// 0x5ebb90 PC
SAFETYHOOK_THISCALL void CreaturesG_Sleep_Hook(void* pThis)
{
	printf("Creatures_G::Sleep(): %x\n", pThis);
	CreaturesG_Sleep_Orig.thiscall<void>(pThis);
}

// 0x5ebbe0 PC
SAFETYHOOK_THISCALL void CreaturesG_WakeUp_Hook(void* pThis)
{
	printf("Creatures_G::WakeUp(): %x\n", pThis);
	CreaturesG_WakeUp_Orig.thiscall<void>(pThis);
}

// 0x59c750 PC
SAFETYHOOK_THISCALL uint32_t D3D_RendererZ_Init_Hook(int* pThis, int width, int height, int param_4, char param_5)
{
	printf("D3D_Renderer_Z::Init(): %x %d %d %d %d\n", pThis, width, height, param_4, param_5);
	uint32_t result = D3D_RendererZ_Init_Orig.thiscall<uint32_t>(pThis, width, height, param_4, param_5);
	d3d9Device = (LPDIRECT3DDEVICE9)(pThis[0xbac]);
	printf("IDirect3DDevice9: %x\n", d3d9Device);

	// https://www.unknowncheats.me/forum/direct3d/66594-d3d9-vtables.html
	auto vmt = *(uintptr_t**)(d3d9Device);
	auto D3D_Clear = vmt[43];
	D3D_Clear_Orig = safetyhook::create_inline((void*)D3D_Clear, (void*)&D3D_Clear_Hook);
	printf("IDirect3DDevice9::Clear(): %x\n", D3D_Clear);

	if (SUCCEEDED(d3d9Device->CreateTexture(SM64_TEXTURE_WIDTH, SM64_TEXTURE_HEIGHT, 0, 0, D3DFMT_A8B8G8R8, D3DPOOL_MANAGED, &marioTextureD3D9, 0)))
	{
		D3DLOCKED_RECT lockRect;
		marioTextureD3D9->LockRect(0, &lockRect, 0, 0);
		memcpy(lockRect.pBits, marioTexture, SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT * 4);
		marioTextureD3D9->UnlockRect(0);
	}
	else
		printf("Failed to create SM64 texture on D3D9\n");

	marioVerticesP = new WALLEVERTEX[SM64_GEO_MAX_TRIANGLES * 3];

	ID3DXBuffer *errorBuffer = 0;
	if (D3DXCreateEffect(d3d9Device, EFFECT_STR, strlen(EFFECT_STR), 0, 0, 0, 0, &marioEffect, &errorBuffer) != D3D_OK)
	{
		Message(errorBuffer ? (const char*)errorBuffer->GetBufferPointer() : "Error loading effect file");
	}
	else
		printf("D3DX9 marioEffect compiled\n");

	return result;
}

// 0x59cd50 PC
SAFETYHOOK_THISCALL void D3D_RendererZ_Shut_Hook(void* pThis)
{
	printf("D3D_Renderer_Z::Shut(): %x\n", pThis);
	D3D_RendererZ_Shut_Orig.thiscall<void>(pThis);
}

// 0x5b5120 PC
SAFETYHOOK_THISCALL void D3D_RendererZ_BeginRender_Hook(void* pThis)
{
	D3D_RendererZ_BeginRender_Orig.thiscall<void>(pThis);
}

// 0x5b5140 PC
SAFETYHOOK_THISCALL void D3D_RendererZ_EndRender_Hook(void* pThis, float param_2)
{
	float pos[3] = {32, 32, 0};
	float color[3] = {1, 1, 1};
	RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, "Mario info", color, 0, 1, true);


	if (marioId < 0)
	{
		pos[1] += 8;
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, "Not spawned", color, 0, 1, true);
	}
	else
	{
		char buf[64] = {0};

		pos[1] += 8;
		sprintf(buf, "ID: %d", marioId);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "position %.2f %.2f %.2f", marioState.position[0], marioState.position[1], marioState.position[2]);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "velocity %.2f %.2f %.2f", marioState.velocity[0], marioState.velocity[1], marioState.velocity[2]);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "forwardVelocity %.2f", marioState.forwardVelocity);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "angle %.2f %.2f %.2f", marioState.angle[0], marioState.angle[1], marioState.angle[2]);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "health %03x", marioState.health);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		/*
		// dump
		if (CameraMoveG)
		{
			void* CamNodeBase = CameraEngineZ_GetCameraNode_Orig.thiscall<void*>(CameraMoveG);
			void* CamNode = HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, CamNodeBase);
			void* CameraZ = (void*)(*(int *)(CamNode + 0x130));

			for (int i=0x000; i<0x500; i+=4)
			{
				int offset = i - 0x000;
				pos[0] = 284;
				pos[1] = offset/4*8;
				while (pos[1] > 720-8)
				{
					pos[0] += 224;
					pos[1] -= 720;
				}
				sprintf(buf, "0x%03x=%d (%.3f)", i, *(int *)(CameraZ + i), *(float *)(CameraZ + i));
				RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);
			}
		}
		*/
	}

	D3D_RendererZ_EndRender_Orig.thiscall<void>(pThis, param_2);
}

// 0x5cb8e0 PC
SAFETYHOOK_THISCALL void RendererZ_DrawString_Hook(void* pThis, float* pos, char* text, float* color, int param_4, float param_5, bool onTop)
{
	RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, text, color, param_4, param_5, onTop);
}

// 0x5aed70 PC
SAFETYHOOK_THISCALL void D3D_RendererZ_PushProjMatrix_Hook(void* pThis, float* mat4x4)
{
	// wanted proj matrix is pushed in either FUN_005b82e0 (0x5b83fd) or FUN_00596050 (0x5965ba)
	D3D_RendererZ_PushProjMatrix_Orig.thiscall<void>(pThis, mat4x4);
}

// 0x5aee00 PC
SAFETYHOOK_THISCALL void D3D_RendererZ_PushViewMatrix_Hook(void* pThis, float* mat4x4)
{
	// wanted view matrix is pushed in either FUN_005b82e0 (0x5b8371) or FUN_00596050 (0x596381)
	D3D_RendererZ_PushViewMatrix_Orig.thiscall<void>(pThis, mat4x4);
}

int callCount = 0;
// 0x58ffb0 PC
SAFETYHOOK_THISCALL void ClearZBuffer_Hook(void* pThis, void* param_2, int param_3)
{
	callCount++;
	ClearZBuffer_Orig.thiscall<void>(pThis, param_2, param_3);
	if (callCount == 1) RenderMario();
}


SAFETYHOOK_NOINLINE void D3D_RendererZ_PushProjMatrix_MidHook(SafetyHookContext& ctx)
{
	// yoink the 3D projection matrix
	float* mtx = (float*)(ctx.eax);
	memcpy(projMatrix, mtx, sizeof(float)*16);
}

SAFETYHOOK_NOINLINE void D3D_RendererZ_PushViewMatrix_MidHook(SafetyHookContext& ctx)
{
	// yoink the view matrix
	float* mtx = (float*)(ctx.ecx);
	memcpy(viewMatrix, mtx, sizeof(float)*16);
}


SAFETYHOOK_STDCALL HRESULT D3D_Clear_Hook(LPDIRECT3DDEVICE9 pThis, DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	if (Flags & D3DCLEAR_TARGET) callCount = 0;
	return D3D_Clear_Orig.stdcall<HRESULT>(pThis, Count, pRects, Flags, Color, Z, Stencil);
}


static void RenderMario()
{
	if (marioId < 0) return;

	WALLEVERTEX* targetVertices = marioVerticesP;
	for (uint32_t i=0; i<marioGeometry.numTrianglesUsed*3; i++)
	{
		targetVertices[i].x = (-marioState.position[0] + marioGeometry.position[i*3+0]) / MARIO_SCALE;
		targetVertices[i].y = (-marioState.position[1] + marioGeometry.position[i*3+1]) / MARIO_SCALE;
		targetVertices[i].z = (-marioState.position[2] + marioGeometry.position[i*3+2]) / MARIO_SCALE;
		targetVertices[i].nx = marioGeometry.normal[i*3+0];
		targetVertices[i].ny = marioGeometry.normal[i*3+1];
		targetVertices[i].nz = marioGeometry.normal[i*3+2];
		targetVertices[i].color = D3DCOLOR_ARGB(255, (UINT)(marioGeometry.color[i*3+0]*255), (UINT)(marioGeometry.color[i*3+1]*255), (UINT)(marioGeometry.color[i*3+2]*255));
		targetVertices[i].u = marioGeometry.uv[i*2+0];
		targetVertices[i].v = marioGeometry.uv[i*2+1];
	}

	D3DXMATRIX projX(projMatrix);
	D3DXMATRIX viewX(viewMatrix), viewXX(viewMatrix);
	D3DXMATRIX worldX(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	D3DXMatrixInverse(&viewX, 0, &viewXX);
	D3DXMatrixTranslation(&worldX, marioState.position[0] / MARIO_SCALE,  marioState.position[1] / MARIO_SCALE,  marioState.position[2] / MARIO_SCALE);

	marioEffect->SetMatrix("gProjMat", &projX);
	marioEffect->SetMatrix("gViewMat", &viewX);
	marioEffect->SetMatrix("gWorldMat", &worldX);
	marioEffect->SetTexture("gTexture", marioTextureD3D9);
	marioEffect->SetTechnique("SingleTexture");

	unsigned int numPasses = 0;
	marioEffect->Begin(&numPasses, 0);

	for(unsigned int i = 0; i < numPasses; ++i)
	{
		marioEffect->BeginPass(i);

		d3d9Device->SetFVF(D3DFVF_WALLEVERTEX);
		HRESULT result = d3d9Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, marioGeometry.numTrianglesUsed, marioVerticesP, sizeof(WALLEVERTEX));

		marioEffect->EndPass();

		if (result != D3D_OK)
		{
			printf("error rendering pass %d out of %d: %d\n", i, numPasses, result);
			break;
		}
	}

	marioEffect->End();
}


void modMain()
{
	AllocConsole();
	SetConsoleTitle("WALL-E Hook");
	freopen("CONOUT$", "w", stdout);

	loadConfig();

	FILE* f = fopen("sm64.z64", "rb");
	if (!f)
	{
		// try outside scripts folder
		FILE* f = fopen("../sm64.z64", "rb");
		if (!f)
		{
			Message("Super Mario 64 US ROM not found!\nPlease provide a ROM with the filename \"sm64.z64\"\nin the same directory as the game .exe.");
			return;
		}
	}

	// load ROM into memory
	uint8_t *romBuffer;
	size_t romFileLength;

	fseek(f, 0, SEEK_END);
	romFileLength = ftell(f);
	fseek(f, 0, SEEK_SET);

	romBuffer = new uint8_t[romFileLength + 1];
	fread(romBuffer, romFileLength, 1, f);
	romBuffer[romFileLength] = 0;
	fclose(f);

	// check ROM SHA1 to avoid crash
	if (!getConfig("skip_sha1_checksum"))
	{
		sha1::SHA1 s;
		char hexdigest[256];
		uint32_t digest[5];
		s.processBytes(romBuffer, romFileLength);
		s.getDigest(digest);
		sprintf(hexdigest, "%08x", digest[0]);
		if (strcmp(hexdigest, "9bef1128"))
		{
			char msg[128];
			sprintf(msg, "Super Mario 64 US ROM checksum does not match!\nYou have the wrong ROM.\n\nExpected: 9bef1128\nYour copy: %s", hexdigest);
			Message(msg);
			delete[] romBuffer;
			return;
		}
	}

	// Mario texture is 704x64 RGBA
	marioTexture = new uint8_t[4 * SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT];

	// load libsm64
	sm64_global_init(romBuffer, marioTexture);
	sm64_audio_init(romBuffer);
	//sm64_set_sound_volume(0.5f);

	delete[] romBuffer;

	audio_thread_init();
	sm64_play_sound_global(SOUND_MENU_STAR_SOUND);

	marioGeometry.position = new float[3 * 3 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.normal   = new float[3 * 3 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.color    = new float[3 * 3 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.uv       = new float[2 * 3 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.numTrianglesUsed = 0;

	//RegisterCmdOrig                            = safetyhook::create_inline((void*)RegisterCmd, (void*)&RegisterCmdHook);
	RunCmdOrig                                 = safetyhook::create_inline((void*)RunCmd, (void*)&RunCmdHook);
	ScriptManagerG_Init_Orig                   = safetyhook::create_inline((void*)ScriptManagerG_Init, (void*)&ScriptManagerG_Init_Hook);
	ScriptManagerG_GetMainPlayer_Orig          = safetyhook::create_inline((void*)ScriptManagerG_GetMainPlayer, (void*)&ScriptManagerG_GetMainPlayer_Hook);
	HandleManagerZ_GetPtr_Orig                 = safetyhook::create_inline((void*)HandleManagerZ_GetPtr, (void*)&HandleManagerZ_GetPtr_Hook);
	LodMoveZ_GetPos_Orig                       = safetyhook::create_inline((void*)LodMoveZ_GetPos, (void*)&LodMoveZ_GetPos_Hook);
	ObjectMoveZ_GetScale_Orig                  = safetyhook::create_inline((void*)ObjectMoveZ_GetScale, (void*)&ObjectMoveZ_GetScale_Hook);
	ObjectMoveZ_GetRot_Orig                    = safetyhook::create_inline((void*)ObjectMoveZ_GetRot, (void*)&ObjectMoveZ_GetRot_Hook);
	LodMoveZ_SetPos_Orig                       = safetyhook::create_inline((void*)LodMoveZ_SetPos, (void*)&LodMoveZ_SetPos_Hook);
	LodMoveZ_SetPosAndRot_Orig                 = safetyhook::create_inline((void*)LodMoveZ_SetPosAndRot, (void*)&LodMoveZ_SetPosAndRot_Hook);
	LodMoveZ_UpdateCollision_Orig              = safetyhook::create_inline((void*)LodMoveZ_UpdateCollision, (void*)&LodMoveZ_UpdateCollision_Hook);
	CreaturesMoveG_SetMyFuturePos_Orig         = safetyhook::create_inline((void*)CreaturesMoveG_SetMyFuturePos, (void*)&CreaturesMoveG_SetMyFuturePos_Hook);
	GameZ_Update_Orig                          = safetyhook::create_inline((void*)GameZ_Update, (void*)&GameZ_Update_Hook);
	GameZ_GetFirstVp_Orig                      = safetyhook::create_inline((void*)GameZ_GetFirstVp, (void*)&GameZ_GetFirstVp_Hook);
	WorldZ_LoadDone_Orig                       = safetyhook::create_inline((void*)WorldZ_LoadDone, (void*)&WorldZ_LoadDone_Hook);
	PlayerG_Init_Orig                          = safetyhook::create_inline((void*)PlayerG_Init, (void*)&PlayerG_Init_Hook);
	PlayerG_Suspend_Orig                       = safetyhook::create_inline((void*)PlayerG_Suspend, (void*)&PlayerG_Suspend_Hook);
	PlayerG_Restore_Orig                       = safetyhook::create_inline((void*)PlayerG_Restore, (void*)&PlayerG_Restore_Hook);
	PlayerMoveG_Destructor_Orig                = safetyhook::create_inline((void*)PlayerMoveG_Destructor, (void*)&PlayerMoveG_Destructor_Hook);
	PlayerMoveG_Stop_Orig                      = safetyhook::create_inline((void*)PlayerMoveG_Stop, (void*)&PlayerMoveG_Stop_Hook);
	PlayerMoveG_Update_SeadZone_Orig           = safetyhook::create_inline((void*)PlayerMoveG_Update_SeadZone, (void*)&PlayerMoveG_Update_SeadZone_Hook);
	PlayerMoveG_SetMyDynPosAndRot_Orig         = safetyhook::create_inline((void*)PlayerMoveG_SetMyDynPosAndRot, (void*)&PlayerMoveG_SetMyDynPosAndRot_Hook);
	PlayerMoveG_IsCurrentMusicForRejected_Orig = safetyhook::create_inline((void*)PlayerMoveG_IsCurrentMusicForRejected, (void*)&PlayerMoveG_IsCurrentMusicForRejected_Hook);
	PlayerMoveG_GetMusicForRejected_Orig       = safetyhook::create_inline((void*)PlayerMoveG_GetMusicForRejected, (void*)&PlayerMoveG_GetMusicForRejected_Hook);
	CameraMoveG_Init_Orig                      = safetyhook::create_inline((void*)CameraMoveG_Init, (void*)&CameraMoveG_Init_Hook);
	CameraEngineZ_GetCameraNode_Orig           = safetyhook::create_inline((void*)CameraEngineZ_GetCameraNode, (void*)&CameraEngineZ_GetCameraNode_Hook);
	P_WALLE_0x608ee0_Orig                      = safetyhook::create_inline((void*)P_WALLE_0x608ee0, (void*)&P_WALLE_0x608ee0_Hook);
	MusicManagerG_SetMusicZone_Orig            = safetyhook::create_inline((void*)MusicManagerG_SetMusicZone, (void*)&MusicManagerG_SetMusicZone_Hook);
	CreaturesG_Init_Orig                       = safetyhook::create_inline((void*)CreaturesG_Init, (void*)&CreaturesG_Init_Hook);
	CreaturesG_Sleep_Orig                      = safetyhook::create_inline((void*)CreaturesG_Sleep, (void*)&CreaturesG_Sleep_Hook);
	CreaturesG_WakeUp_Orig                     = safetyhook::create_inline((void*)CreaturesG_WakeUp, (void*)&CreaturesG_WakeUp_Hook);
	D3D_RendererZ_Init_Orig                    = safetyhook::create_inline((void*)D3D_RendererZ_Init, (void*)&D3D_RendererZ_Init_Hook);
	D3D_RendererZ_Shut_Orig                    = safetyhook::create_inline((void*)D3D_RendererZ_Shut, (void*)&D3D_RendererZ_Shut_Hook);
	D3D_RendererZ_BeginRender_Orig             = safetyhook::create_inline((void*)D3D_RendererZ_BeginRender, (void*)&D3D_RendererZ_BeginRender_Hook);
	D3D_RendererZ_EndRender_Orig               = safetyhook::create_inline((void*)D3D_RendererZ_EndRender, (void*)&D3D_RendererZ_EndRender_Hook);
	RendererZ_DrawString_Orig                  = safetyhook::create_inline((void*)RendererZ_DrawString, (void*)&RendererZ_DrawString_Hook);
	ClearZBuffer_Orig                          = safetyhook::create_inline((void*)ClearZBuffer, &ClearZBuffer_Hook);
	D3D_RendererZ_PushProjMatrix_MidOrig       = safetyhook::create_mid((void*)0x5b83fd, &D3D_RendererZ_PushProjMatrix_MidHook);
	D3D_RendererZ_PushViewMatrix_MidOrig       = safetyhook::create_mid((void*)0x596381, &D3D_RendererZ_PushViewMatrix_MidHook);
	//D3D_RendererZ_PushProjMatrix_Orig          = safetyhook::create_inline((void*)D3D_RendererZ_PushProjMatrix, &D3D_RendererZ_PushProjMatrix_Hook);
	//D3D_RendererZ_PushViewMatrix_Orig          = safetyhook::create_inline((void*)D3D_RendererZ_PushViewMatrix, &D3D_RendererZ_PushViewMatrix_Hook);
}

void modExit()
{
	sm64_global_terminate();
}
