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
}

#include "mod.h"
#include "audio.h"
#include "config.h"

void Message(const char* sometext)
{
    MessageBoxA(0, sometext, "libsm64-wall-e", MB_OK | MB_ICONINFORMATION);
}

static uint8_t* marioTexture;
static uint16_t marioIndices[SM64_GEO_MAX_TRIANGLES * 3];


auto RegisterCmd = 0x4763b0;
auto RunCmd = 0x476580;
auto CreaturesG_Init = 0x6970a0;
auto PlayerG_Init = 0x69b4a0;
auto RendererZ_DrawString = 0x5cb8e0;
auto D3D_RendererZ_Init = 0x59c750;
auto D3D_RendererZ_BeginRender = 0x5b5120;
auto D3D_RendererZ_EndRender = 0x5b5140;
SafetyHookInline RegisterCmdOrig;
SafetyHookInline RunCmdOrig;
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

SAFETYHOOK_THISCALL bool RunCmdHook(void* pThis, const char* cmd, void* unknown1)
{
	bool dontLog = (strncmp(cmd, "MENU", 4) == 0);
	//bool dontLog = true;
	if (!dontLog)
		printf("cmd='%s' ", cmd);
	bool res = RunCmdOrig.thiscall<bool>(pThis, cmd, unknown1);
	if (!dontLog)
		printf("result: %d\n", res);
	return res;
}

// 0x69b4a0 PC, 0x15ce40 Mac
SAFETYHOOK_THISCALL void PlayerG_Init_Hook(void* pThis)
{
	printf("Player_G::Init(): %x\n", pThis);
	PlayerG_Init_Orig.thiscall<void>(pThis);
}

// 0x6970a0 PC, 0x187ea6 Mac
SAFETYHOOK_THISCALL void CreaturesG_Init_Hook(void* pThis, char param_2)
{
	printf("Creatures_G::Init(): %x, %d\n", pThis, param_2);
	CreaturesG_Init_Orig.thiscall<void>(pThis, param_2);
}

// 0x59c750 PC
SAFETYHOOK_THISCALL uint32_t D3D_RendererZ_Init_Hook(void* pThis, int param_2, int param_3, int param_4, char param_5)
{
	printf("D3D_Renderer_Z::Init(): %x %d %d %d %d\n", pThis, param_2, param_3, param_4, param_5, param_5);
	return D3D_RendererZ_Init_Orig.thiscall<uint32_t>(pThis, param_2, param_3, param_4, param_5);
}

// 0x5b5120 PC
SAFETYHOOK_THISCALL void D3D_RendererZ_BeginRender_Hook(void* pThis)
{
	printf("D3D_Renderer_Z::BeginRender(): %x\n", pThis);
	D3D_RendererZ_BeginRender_Orig.thiscall<void>(pThis);
}

// 0x5b5140 PC
SAFETYHOOK_THISCALL void D3D_RendererZ_EndRender_Hook(void* pThis, float param_2)
{
	float pos[3] = {32, 32, 0};
	float color[3] = {1, 1, 1};
	RendererZ_DrawString_Orig.thiscall<void>(pThis, pos, "rendering my own string", color, 0, 1, true);

	printf("D3D_Renderer_Z::EndRender(): %x\n", pThis);
	D3D_RendererZ_EndRender_Orig.thiscall<void>(pThis, param_2);
}

// 0x5cb8e0 PC
SAFETYHOOK_THISCALL void RendererZ_DrawString_Hook(void* pThis, float* pos, char* text, float* color, int param_4, float param_5, bool onTop)
{
	printf("RendererZ::DrawString(): %x - '%s' - %.2f,%.2f,%.2f - %.2f,%.2f,%.2f - %d - %.2f - %d\n", pThis, text, pos[0], pos[1], pos[2], color[0], color[1], color[2], param_4, param_5, onTop);
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

	//RegisterCmdOrig = safetyhook::create_inline((void*)RegisterCmd, (void*)&RegisterCmdHook);
	//RunCmdOrig = safetyhook::create_inline((void*)RunCmd, (void*)&RunCmdHook);
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
