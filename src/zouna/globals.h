#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

// lifted from ratatouille decomp
// https://github.com/ZounaModding/RatDecomp/blob/main/src/Engine/includes/Main_Z.h#L146
struct Globals
{
	void* UnkMgr_0x4;
    void* Cons;
    void* MainRdr;
    void* ClassMgr; // ClassManager_Z (also HandleManager_Z)
    void* WorldMgr;
    void* MaterialMgr;
    void* ScriptMgr;
    void* ColSurfaceCache;
    void* UnkMgr_0x24;
    void* ColTriangleCache;
    void* MatrixBuffer;
    void* ManipulatorMgr;
    void* GameMgr;
    void* UnkMgr_0x38;
    void* AnimMgr;
    void* EffectMgr;
    void* SystemDatas;
    void* ObjectBankMgr;
    void* InputMgr;
    void* SoundMgr;
    void* MovieMgr;
    void* ScriptInputMgr;
    void* SavingMgr;
    void* ParticlesMgr;
    void* UnkMgr_0x64;
    void* SurfaceCache;
    void* UnkMgr_0x6c;
    void* StreamMgr;
    void* VolatileMgr;
    void* UnkMgr_0x78;
    void* NetMgr;
    void* UnkMgr_0x80;
    void* XRamMgr;
};

#endif // GLOBALS_H_INCLUDED
