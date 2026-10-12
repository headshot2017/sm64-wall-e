#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include <stdint.h>

// lifted from ratatouille decomp
// https://github.com/ZounaModding/RatDecomp/blob/main/src/Engine/includes/Main_Z.h#L146
struct Globals
{
	uint8_t* UnkMgr_0x4;
    uint8_t* Cons;
    uint8_t* MainRdr;
    uint8_t* ClassMgr; // ClassManager_Z (also HandleManager_Z)
    uint8_t* WorldMgr;
    uint8_t* MaterialMgr;
    uint8_t* ScriptMgr;
    uint8_t* ColSurfaceCache;
    uint8_t* UnkMgr_0x24;
    uint8_t* ColTriangleCache;
    uint8_t* MatrixBuffer;
    uint8_t* ManipulatorMgr;
    uint8_t* GameMgr;
    uint8_t* UnkMgr_0x38;
    uint8_t* AnimMgr;
    uint8_t* EffectMgr;
    uint8_t* SystemDatas;
    uint8_t* ObjectBankMgr;
    uint8_t* InputMgr;
    uint8_t* SoundMgr;
    uint8_t* MovieMgr;
    uint8_t* ScriptInputMgr;
    uint8_t* SavingMgr;
    uint8_t* ParticlesMgr;
    uint8_t* UnkMgr_0x64;
    uint8_t* SurfaceCache;
    uint8_t* UnkMgr_0x6c;
    uint8_t* StreamMgr;
    uint8_t* VolatileMgr;
    uint8_t* UnkMgr_0x78;
    uint8_t* NetMgr;
    uint8_t* UnkMgr_0x80;
    uint8_t* XRamMgr;
};

extern Globals* gData;

#endif // GLOBALS_H_INCLUDED
