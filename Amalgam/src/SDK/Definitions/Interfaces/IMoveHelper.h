#pragma once
#include "Interface.h"
#include "../Main/CBaseHandle.h"
#include "../Definitions.h"
#include "../Types.h"

class CGameTrace;
class IPhysicsSurfaceProps;
enum PLAYER_ANIM;

typedef CBaseHandle EntityHandle_t;

#define INVALID_ENTITY_HANDLE INVALID_EHANDLE_INDEX

class IMoveHelper
{
public:
	virtual	char const* GetName(EntityHandle_t handle) const = 0;
	virtual void ResetTouchList(void) = 0;
	virtual bool AddToTouched(const CGameTrace& tr, const Vector& impactvelocity) = 0;
	virtual void ProcessImpacts(void) = 0;
	virtual void Con_NPrintf(int idx, char const* fmt, ...) = 0;
	virtual void StartSound(const Vector& origin, int channel, char const* sample, float volume, soundlevel_t soundlevel, int fFlags, int pitch) = 0;
	virtual void StartSound(const Vector& origin, const char* soundname) = 0;
	virtual void PlaybackEventFull(int flags, int clientindex, unsigned short eventindex, float delay, Vector& origin, Vector& angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2) = 0;
	virtual bool PlayerFallingDamage(void) = 0;
	virtual void PlayerSetAnimation(PLAYER_ANIM playerAnim) = 0;
	virtual IPhysicsSurfaceProps* GetSurfaceProps(void) = 0;
	virtual bool IsWorldEntity(const CBaseHandle& handle) = 0;

	inline CBasePlayer* GetHost()
	{
		return *reinterpret_cast<CBasePlayer**>(uintptr_t(this) + 40);
	}

	inline void SetHost(CBasePlayer* pHost)
	{
		*reinterpret_cast<CBasePlayer**>(uintptr_t(this) + 40) = pHost;
	}

protected:
	virtual ~IMoveHelper() {}
};

MAKE_INTERFACE_SIGNATURE(IMoveHelper, MoveHelper, "client.dll", "48 8B 0D ? ? ? ? 48 8B 01 FF 50 ? 0F B7 D7", 0x0, 1);