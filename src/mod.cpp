#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

#include "safetyhook/safetyhook.hpp"
#include "TinySHA1.hpp"
#include "libsm64/libsm64.h"
extern "C" {
    #include "libsm64/decomp/include/PR/ultratypes.h"
    #include "libsm64/decomp/include/audio_defines.h"
    #include "libsm64/decomp/include/surface_terrains.h"
}

#include "mod.h"
#include "audio.h"
#include "config.h"

void Message(const char* sometext)
{
    MessageBoxA(0, sometext, "sm64-wall-e", MB_OK | MB_ICONINFORMATION);
}

static uint8_t* marioTexture;
static uint16_t marioIndices[SM64_GEO_MAX_TRIANGLES * 3];
static float marioTimer = 0;
static int marioId = -1;
static SM64MarioState marioState = {0};
static SM64MarioInputs marioInput = {0};
static SM64MarioGeometryBuffers marioGeometry = {0};


auto RegisterCmd = 0x4763b0;
auto RunCmd = 0x476580;
auto ScriptManagerG_Init = 0x6b3c70;
auto ScriptManagerG_GetMainPlayer = 0x6b3740;
auto HandleManagerZ_GetPtr = 0x435440;
auto GameZ_Update = 0x438620;
auto CreaturesG_Init = 0x6970a0;
auto PlayerG_Init = 0x69b4a0;
auto RendererZ_DrawString = 0x5cb8e0;
auto D3D_RendererZ_Init = 0x59c750;
auto D3D_RendererZ_BeginRender = 0x5b5120;
auto D3D_RendererZ_EndRender = 0x5b5140;
SafetyHookInline RegisterCmdOrig;
SafetyHookInline RunCmdOrig;
SafetyHookInline ScriptManagerG_Init_Orig;
SafetyHookInline ScriptManagerG_GetMainPlayer_Orig;
SafetyHookInline HandleManagerZ_GetPtr_Orig;
SafetyHookInline GameZ_Update_Orig;
SafetyHookInline CreaturesG_Init_Orig;
SafetyHookInline PlayerG_Init_Orig;
SafetyHookInline RendererZ_DrawString_Orig;
SafetyHookInline D3D_RendererZ_Init_Orig;
SafetyHookInline D3D_RendererZ_BeginRender_Orig;
SafetyHookInline D3D_RendererZ_EndRender_Orig;


SAFETYHOOK_THISCALL void RegisterCmdHook(void* pThis, const char* cmd, void* param_3)
{
	printf("Register: %x '%s'\n", cmd, pThis);
	RegisterCmdOrig.thiscall<void>(pThis, cmd, param_3);
}

void* ConsoleZ;
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

void* ScriptManagerG = 0;
// 0x6b3c70 PC, 0x198294 Mac
SAFETYHOOK_THISCALL void ScriptManagerG_Init_Hook(void* pThis)
{
	printf("ScriptManager_G::Init(): %x\n", pThis);
	ScriptManagerG = pThis;
	ScriptManagerG_Init_Orig.thiscall<void>(pThis);
}

SAFETYHOOK_THISCALL void* ScriptManagerG_GetMainPlayer_Hook(void* pThis, uint32_t param_2)
{
	void* pPlayer = ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(pThis, param_2);
	return pPlayer;
}

void* HandleManagerZ = 0;
// 0x435440 PC
SAFETYHOOK_THISCALL void* HandleManagerZ_GetPtr_Hook(void* pThis, void* BaseObject_Z_param)
{
	if (HandleManagerZ != pThis) HandleManagerZ = pThis;
	void* result = HandleManagerZ_GetPtr_Orig.thiscall<void*>(pThis, BaseObject_Z_param);
	return result;
}

// 0x4198f0 PC, 0x4c6a8 Mac
SAFETYHOOK_THISCALL void GameZ_Update_Hook(void* pThis, float dt)
{
	//printf("Game_Z::Update(): %x, dt=%f\n", pThis, dt);
	GameZ_Update_Orig.thiscall<void>(pThis, dt);

	void* pMainPlayer = ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(ScriptManagerG, 0);
	if (!pMainPlayer)
		return;

	void* pHandle = HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, pMainPlayer + 0x70); // field 0x70 is BaseObject_Z
	if (!pHandle)
		return;

	float x = *(float*)(pHandle + 0);
	float y = *(float*)(pHandle + 4);
	float z = *(float*)(pHandle + 8);

	if (marioId < 0) return;

	marioTimer += dt;
	while (marioTimer > 1.f/30.f)
	{
		marioTimer -= 1.f/30.f;
		sm64_mario_tick(marioId, &marioInput, &marioState, &marioGeometry);
	}
}

void* PlayerG = 0;
// 0x69b4a0 PC, 0x15ce40 Mac
SAFETYHOOK_THISCALL void PlayerG_Init_Hook(void* pThis)
{
	printf("Player_G::Init(): %x\n", pThis);
	PlayerG_Init_Orig.thiscall<void>(pThis);
	PlayerG = pThis;

	void* pMainPlayer = ScriptManagerG_GetMainPlayer_Orig.thiscall<void*>(ScriptManagerG, 0);
	if (!pMainPlayer)
		return;

	void* pHandle = HandleManagerZ_GetPtr_Orig.thiscall<void*>(HandleManagerZ, pMainPlayer + 0x70); // field 0x70 is BaseObject_Z
	if (!pHandle)
		return;

	float x = *(float*)(pHandle + 0);
	float y = *(float*)(pHandle + 4);
	float z = *(float*)(pHandle + 8);

	// TODO: load level collision

	// multiply coordinates when converting from game to libsm64
	// divide coordinates when converting from libsm64 to game
	SM64Surface surfaces[2];
	uint32_t surfaceCount = sizeof(surfaces) / sizeof(SM64Surface);
	int width = 256 * MARIO_SCALE;
    int spawnX = x * MARIO_SCALE;
    int spawnY = y * MARIO_SCALE;
    int spawnZ = z * MARIO_SCALE;

	for (uint32_t i=0; i<surfaceCount; i++)
	{
		surfaces[i].type = SURFACE_DEFAULT;
		surfaces[i].force = 0;
		surfaces[i].terrain = TERRAIN_STONE;
	}

	surfaces[0].vertices[0][0] = spawnX + width/2 + 128;	surfaces[0].vertices[0][1] = spawnY;	surfaces[0].vertices[0][2] = spawnZ+128;
    surfaces[0].vertices[1][0] = spawnX - width/2 - 128;	surfaces[0].vertices[1][1] = spawnY;	surfaces[0].vertices[1][2] = spawnZ-128;
    surfaces[0].vertices[2][0] = spawnX - width/2 - 128;	surfaces[0].vertices[2][1] = spawnY;	surfaces[0].vertices[2][2] = spawnZ+128;

    surfaces[1].vertices[0][0] = spawnX - width/2 - 128;	surfaces[1].vertices[0][1] = spawnY;	surfaces[1].vertices[0][2] = spawnZ-128;
    surfaces[1].vertices[1][0] = spawnX + width/2 + 128;	surfaces[1].vertices[1][1] = spawnY;	surfaces[1].vertices[1][2] = spawnZ+128;
    surfaces[1].vertices[2][0] = spawnX + width/2 + 128;	surfaces[1].vertices[2][1] = spawnY;	surfaces[1].vertices[2][2] = spawnZ-128;

	sm64_static_surfaces_load(surfaces, surfaceCount);

	marioId = sm64_mario_create(x * MARIO_SCALE, y * MARIO_SCALE, z * MARIO_SCALE);
	if (marioId < 0)
	{
		printf("Failed to spawn Mario at %.2f, %.2f, %.2f\n", x * MARIO_SCALE, y * MARIO_SCALE, z * MARIO_SCALE);
		return;
	}
	printf("Spawned Mario %d at %.2f, %.2f, %.2f\n", marioId, x * MARIO_SCALE, y * MARIO_SCALE, z * MARIO_SCALE);
	marioTimer = 0;
}

// 0x6970a0 PC, 0x187ea6 Mac
SAFETYHOOK_THISCALL void CreaturesG_Init_Hook(void* pThis, char param_2)
{
	printf("Creatures_G::Init(): %x, %d\n", pThis, param_2);
	CreaturesG_Init_Orig.thiscall<void>(pThis, param_2);
}

// 0x59c750 PC
SAFETYHOOK_THISCALL uint32_t D3D_RendererZ_Init_Hook(void* pThis, int width, int height, int param_4, char param_5)
{
	printf("D3D_Renderer_Z::Init(): %x %d %d %d %d\n", pThis, width, height, param_4, param_5, param_5);
	return D3D_RendererZ_Init_Orig.thiscall<uint32_t>(pThis, width, height, param_4, param_5);
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
		char buf[128] = {0};

		pos[1] += 8;
		sprintf(buf, "ID: %d", marioId);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "position: %.2f %.2f %.2f", marioState.position[0], marioState.position[1], marioState.position[2]);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "velocity: %.2f %.2f %.2f", marioState.velocity[0], marioState.velocity[1], marioState.velocity[2]);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "forwardVelocity: %.2f", marioState.forwardVelocity);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "faceAngle: %.2f", marioState.faceAngle);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);

		pos[1] += 8;
		sprintf(buf, "health: %03x", marioState.health);
		RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, buf, color, 0, 1, true);
	}

	D3D_RendererZ_EndRender_Orig.thiscall<void>(pThis, param_2);
}

// 0x5cb8e0 PC
SAFETYHOOK_THISCALL void RendererZ_DrawString_Hook(void* pThis, float* pos, char* text, float* color, int param_4, float param_5, bool onTop)
{
	RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, text, color, param_4, param_5, onTop);
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

	for(int i=0; i<3*SM64_GEO_MAX_TRIANGLES; i++) marioIndices[i] = i;
	delete[] romBuffer;

	audio_thread_init();
	sm64_play_sound_global(SOUND_MENU_STAR_SOUND);

	marioGeometry.position = new float[9 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.normal   = new float[9 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.color    = new float[9 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.uv       = new float[6 * SM64_GEO_MAX_TRIANGLES];
    marioGeometry.numTrianglesUsed = 0;

	//RegisterCmdOrig = safetyhook::create_inline((void*)RegisterCmd, (void*)&RegisterCmdHook);
	RunCmdOrig = safetyhook::create_inline((void*)RunCmd, (void*)&RunCmdHook);
	ScriptManagerG_Init_Orig = safetyhook::create_inline((void*)ScriptManagerG_Init, (void*)&ScriptManagerG_Init_Hook);
	ScriptManagerG_GetMainPlayer_Orig = safetyhook::create_inline((void*)ScriptManagerG_GetMainPlayer, (void*)&ScriptManagerG_GetMainPlayer_Hook);
	HandleManagerZ_GetPtr_Orig = safetyhook::create_inline((void*)HandleManagerZ_GetPtr, (void*)&HandleManagerZ_GetPtr_Hook);
	GameZ_Update_Orig = safetyhook::create_inline((void*)GameZ_Update, (void*)&GameZ_Update_Hook);
	PlayerG_Init_Orig = safetyhook::create_inline((void*)PlayerG_Init, (void*)&PlayerG_Init_Hook);
	CreaturesG_Init_Orig = safetyhook::create_inline((void*)CreaturesG_Init, (void*)&CreaturesG_Init_Hook);
	RendererZ_DrawString_Orig = safetyhook::create_inline((void*)RendererZ_DrawString, (void*)&RendererZ_DrawString_Hook);
	D3D_RendererZ_Init_Orig = safetyhook::create_inline((void*)D3D_RendererZ_Init, (void*)&D3D_RendererZ_Init_Hook);
	D3D_RendererZ_BeginRender_Orig = safetyhook::create_inline((void*)D3D_RendererZ_BeginRender, (void*)&D3D_RendererZ_BeginRender_Hook);
	D3D_RendererZ_EndRender_Orig = safetyhook::create_inline((void*)D3D_RendererZ_EndRender, (void*)&D3D_RendererZ_EndRender_Hook);
}

void modExit()
{
	sm64_global_terminate();
}
