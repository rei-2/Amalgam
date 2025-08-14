#pragma once
#include "CBaseHandle.h"
#include "IClientEntity.h"
#include "CCollisionProperty.h"
#include "CParticleProperty.h"
#include "../Main/UtlVector.h"
#include "../Misc/CInterpolatedVar.h"
#include "../Definitions.h"
#include "../../../Utils/Memory/Memory.h"
#include "../../../Utils/Signatures/Signatures.h"
#include "../../../Utils/NetVars/NetVars.h"

MAKE_SIGNATURE(CBaseEntity_SetAbsOrigin, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 E8 ? ? ? ? F3 0F 10 83", 0x0);
MAKE_SIGNATURE(CBaseEntity_SetAbsAngles, "client.dll", "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 48 8B FA 48 8B D9 E8 ? ? ? ? F3 0F 10 83", 0x0);
MAKE_SIGNATURE(CBaseEntity_SetAbsVelocity, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? F3 0F 10 81 ? ? ? ? 48 8B DA 0F 2E 02", 0x0);
MAKE_SIGNATURE(CBaseEntity_EstimateAbsVelocity, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 E8 ? ? ? ? 48 3B D8", 0x0);
MAKE_SIGNATURE(CBaseEntity_CreateShadow, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B 41 ? 48 8B F9 48 83 C1 ? FF 90", 0x0);
MAKE_SIGNATURE(CBaseEntity_InvalidateBoneCache, "client.dll", "8B 05 ? ? ? ? FF C8 C7 81", 0x0);

enum CollideType_t
{
	ENTITY_SHOULD_NOT_COLLIDE = 0,
	ENTITY_SHOULD_COLLIDE,
	ENTITY_SHOULD_RESPOND
};

typedef CHandle<CBaseEntity> EHANDLE;

#define MULTIPLAYER_BACKUP 90

class IInterpolatedVar;

class VarMapEntry_t
{
public:
	unsigned short type;
	unsigned short m_bNeedsToInterpolate;
	void* data;
	IInterpolatedVar* watcher;
};

struct VarMapping_t
{
	CUtlVector<VarMapEntry_t> m_Entries;
	int m_nInterpolatedEntries;
	float m_lastInterpolationTime;
};

class CBaseEntity : public IClientEntity
{
public:
	NETVAR(m_flAnimTime, float, "CBaseEntity", "m_flAnimTime");
	NETVAR(m_flSimulationTime, float, "CBaseEntity", "m_flSimulationTime");
	NETVAR(m_ubInterpolationFrame, int, "CBaseEntity", "m_ubInterpolationFrame");
	NETVAR(m_vecOrigin, Vec3, "CBaseEntity", "m_vecOrigin");
	NETVAR(m_angRotation, Vec3, "CBaseEntity", "m_angRotation");
	NETVAR(m_nModelIndex, int, "CBaseEntity", "m_nModelIndex");
	NETVAR(m_fEffects, int, "CBaseEntity", "m_fEffects");
	NETVAR(m_nRenderMode, int, "CBaseEntity", "m_nRenderMode");
	NETVAR(m_nRenderFX, int, "CBaseEntity", "m_nRenderFX");
	NETVAR(m_clrRender, Color_t, "CBaseEntity", "m_clrRender");
	NETVAR(m_iTeamNum, int, "CBaseEntity", "m_iTeamNum");
	NETVAR(m_CollisionGroup, int, "CBaseEntity", "m_CollisionGroup");
	NETVAR(m_flElasticity, float, "CBaseEntity", "m_flElasticity");
	NETVAR(m_flShadowCastDistance, float, "CBaseEntity", "m_flShadowCastDistance");
	NETVAR(m_hOwnerEntity, EHANDLE, "CBaseEntity", "m_hOwnerEntity");
	NETVAR(m_hEffectEntity, EHANDLE, "CBaseEntity", "m_hEffectEntity");
	NETVAR(moveparent, int, "CBaseEntity", "moveparent");
	NETVAR(m_iParentAttachment, int, "CBaseEntity", "m_iParentAttachment");
	NETVAR(m_Collision, CCollisionProperty*, "CBaseEntity", "m_Collision");
	NETVAR(m_vecMinsPreScaled, Vec3, "CBaseEntity", "m_vecMinsPreScaled");
	NETVAR(m_vecMaxsPreScaled, Vec3, "CBaseEntity", "m_vecMaxsPreScaled");
	NETVAR(m_vecMins, Vec3, "CBaseEntity", "m_vecMins");
	NETVAR(m_vecMaxs, Vec3, "CBaseEntity", "m_vecMaxs");
	NETVAR(m_nSolidType, int, "CBaseEntity", "m_nSolidType");
	NETVAR(m_usSolidFlags, int, "CBaseEntity", "m_usSolidFlags");
	NETVAR(m_nSurroundType, int, "CBaseEntity", "m_nSurroundType");
	NETVAR(m_triggerBloat, int, "CBaseEntity", "m_triggerBloat");
	NETVAR(m_bUniformTriggerBloat, bool, "CBaseEntity", "m_bUniformTriggerBloat");
	NETVAR(m_vecSpecifiedSurroundingMinsPreScaled, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMinsPreScaled");
	NETVAR(m_vecSpecifiedSurroundingMaxsPreScaled, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMaxsPreScaled");
	NETVAR(m_vecSpecifiedSurroundingMins, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMins");
	NETVAR(m_vecSpecifiedSurroundingMaxs, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMaxs");
	NETVAR(m_iTextureFrameIndex, int, "CBaseEntity", "m_iTextureFrameIndex");
	NETVAR(m_PredictableID, int, "CBaseEntity", "m_PredictableID");
	NETVAR(m_bIsPlayerSimulated, bool, "CBaseEntity", "m_bIsPlayerSimulated");
	NETVAR(m_bSimulatedEveryTick, bool, "CBaseEntity", "m_bSimulatedEveryTick");
	NETVAR(m_bAnimatedEveryTick, bool, "CBaseEntity", "m_bAnimatedEveryTick");
	NETVAR(m_bAlternateSorting, bool, "CBaseEntity", "m_bAlternateSorting");
	NETVAR(m_nModelIndexOverrides, void*, "CBaseEntity", "m_nModelIndexOverrides");
	NETVAR(movetype, int, "CBaseEntity", "movetype");
	
	NETVAR_OFF(m_flOldSimulationTime, float, "CBaseEntity", "m_flSimulationTime", 4);
	NETVAR_OFF(m_flGravity, float, "CTFPlayer", "m_nWaterLevel", -24);
	NETVAR_OFF(m_MoveType, byte, "CTFPlayer", "m_nWaterLevel", -4);
	NETVAR_OFF(m_MoveCollide, byte, "CTFPlayer", "m_nWaterLevel", -3);
	NETVAR_OFF(m_nWaterType, byte, "CTFPlayer", "m_nWaterLevel", 1);
	NETVAR_OFF(m_Particles, CParticleProperty*, "CBaseEntity", "m_flElasticity", -56);
	NETVAR_OFF(m_iv_vecVelocity, CInterpolatedVar<Vec3>, "CBaseEntity", "m_iTeamNum", 156);
	NETVAR_OFF(m_iv_vecOrigin, CInterpolatedVar<Vec3>, "CBaseEntity", "m_flShadowCastDistance", 84);
	NETVAR_OFF(m_iv_angRotation, CInterpolatedVar<Vec3>, "CBaseEntity", "m_vecOrigin", -128);
	inline CBaseEntity* GetMoveParent()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseEntity", "moveparent") - 8;
		auto m_pMoveParent = reinterpret_cast<EHANDLE*>(uintptr_t(this) + nOffset);

		return m_pMoveParent ? m_pMoveParent->Get() : nullptr;
	}
	inline CBaseEntity* NextMovePeer()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseEntity", "moveparent") - 16;
		auto m_pMovePeer = reinterpret_cast<EHANDLE*>(uintptr_t(this) + nOffset);

		return m_pMovePeer ? m_pMovePeer->Get() : nullptr;
	}
	inline CBaseEntity* FirstMoveChild()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseEntity", "moveparent") - 24;
		auto m_pMoveChild = reinterpret_cast<EHANDLE*>(uintptr_t(this) + nOffset);

		return m_pMoveChild ? m_pMoveChild->Get() : nullptr;
	}
	inline CBaseEntity* const GetRootMoveParent()
	{
		CBaseEntity* ent = this;
		CBaseEntity* parent = GetMoveParent();

		while (parent)
		{
			ent = parent;
			parent = ent->GetMoveParent();
		}

		return ent;
	}

	inline bool IsPlayer()
	{
		return GetClassID() == ETFClassID::CTFPlayer;
	}
	inline bool IsBuilding()
	{
		switch (GetClassID())
		{
		case ETFClassID::CObjectDispenser:
		case ETFClassID::CObjectSentrygun:
		case ETFClassID::CObjectTeleporter:
			return true;
		}
		return false;
	}
	inline bool IsSentrygun()
	{
		return GetClassID() == ETFClassID::CObjectSentrygun;
	}
	inline bool IsDispenser()
	{
		return GetClassID() == ETFClassID::CObjectDispenser;
	}
	inline bool IsTeleporter()
	{
		return GetClassID() == ETFClassID::CObjectTeleporter;
	}
	inline bool IsProjectile()
	{
		switch (GetClassID())
		{
		case ETFClassID::CBaseProjectile:
		case ETFClassID::CBaseGrenade:
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
		case ETFClassID::CTFStunBall:
		case ETFClassID::CTFBall_Ornament:
		case ETFClassID::CTFProjectile_Jar:
		case ETFClassID::CTFProjectile_Cleaver:
		case ETFClassID::CTFProjectile_JarGas:
		case ETFClassID::CTFProjectile_JarMilk:
		case ETFClassID::CTFProjectile_SpellBats:
		case ETFClassID::CTFProjectile_SpellKartBats:
		case ETFClassID::CTFProjectile_SpellMeteorShower:
		case ETFClassID::CTFProjectile_SpellMirv:
		case ETFClassID::CTFProjectile_SpellPumpkin:
		case ETFClassID::CTFProjectile_SpellSpawnBoss:
		case ETFClassID::CTFProjectile_SpellSpawnHorde:
		case ETFClassID::CTFProjectile_SpellSpawnZombie:
		case ETFClassID::CTFProjectile_SpellTransposeTeleport:
		case ETFClassID::CTFProjectile_Throwable:
		case ETFClassID::CTFProjectile_ThrowableBreadMonster:
		case ETFClassID::CTFProjectile_ThrowableBrick:
		case ETFClassID::CTFProjectile_ThrowableRepel:
		case ETFClassID::CTFBaseRocket:
		case ETFClassID::CTFFlameRocket:
		case ETFClassID::CTFProjectile_Arrow:
		case ETFClassID::CTFProjectile_GrapplingHook:
		case ETFClassID::CTFProjectile_HealingBolt:
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_BallOfFire:
		case ETFClassID::CTFProjectile_MechanicalArmOrb:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_SpellFireball:
		case ETFClassID::CTFProjectile_SpellLightningOrb:
		case ETFClassID::CTFProjectile_SpellKartOrb:
		case ETFClassID::CTFProjectile_EnergyBall:
		case ETFClassID::CTFProjectile_Flare:
		case ETFClassID::CTFBaseProjectile:
		case ETFClassID::CTFProjectile_EnergyRing:
			//case ETFClassID::CTFProjectile_Syringe:
			return true;
		}
		return false;
	}
	inline bool IsPickup()
	{
		switch (GetClassID())
		{
		case ETFClassID::CBaseAnimating:
			return I::ModelInfoClient->GetModelName(GetModel())[24] != 'h';
		case ETFClassID::CTFAmmoPack:
			return true;
		}
		return false;
	}
	inline bool IsNPC()
	{
		switch (GetClassID())
		{
		case ETFClassID::CTFBaseBoss:
		case ETFClassID::CTFTankBoss:
		case ETFClassID::CMerasmus:
		case ETFClassID::CEyeballBoss:
		case ETFClassID::CHeadlessHatman:
		case ETFClassID::CZombie:
			return true;
		}
		return false;
	}
	inline bool IsBomb()
	{
		switch (GetClassID())
		{
		case ETFClassID::CTFPumpkinBomb:
		case ETFClassID::CTFGenericBomb:
			return true;
		}
		return false;
	}
	inline bool IsWearable()
	{
		switch (GetClassID())
		{
		case ETFClassID::CTFWearable:
		case ETFClassID::CTFPowerupBottle:
		case ETFClassID::CTFWearableCampaignItem:
		case ETFClassID::CTFWearableDemoShield:
		case ETFClassID::CTFWeaponPDAExpansion_Dispenser:
		case ETFClassID::CTFWeaponPDAExpansion_Teleporter:
		case ETFClassID::CTFWearableLevelableItem:
		case ETFClassID::CTFWearableRazorback:
		case ETFClassID::CTFWearableRobotArm:
		case ETFClassID::CTFWearableVM:
		case ETFClassID::CTFWearableItem:
			return true;
		}
		return false;
	}

	//VIRTUAL(IsPlayer, bool, 132, this);
	VIRTUAL(IsBaseCombatCharacter, bool, 133, this);
	//VIRTUAL(IsNPC, bool, 135, this);
	VIRTUAL(IsNextBot, bool, 136, this);
	VIRTUAL(IsBaseObject, bool, 137, this);
	VIRTUAL(IsBaseCombatWeapon, bool, 138, this);
	VIRTUAL(IsCombatItem, bool, 140, this);
	VIRTUAL(EyePosition, Vec3, 142, this);
	VIRTUAL(EyeAngles, Vec3&, 143, this);
	VIRTUAL(UpdateVisibility, void, 91, this);

	SIGNATURE_ARGS(SetAbsOrigin, void, CBaseEntity, (const Vec3& vOrigin), this, std::ref(vOrigin));
	SIGNATURE_ARGS(SetAbsAngles, void, CBaseEntity, (const Vec3& vAngles), this, std::ref(vAngles));
	SIGNATURE_ARGS(SetAbsVelocity, void, CBaseEntity, (const Vec3& vVelocity), this, std::ref(vVelocity));
	SIGNATURE_ARGS(EstimateAbsVelocity, void, CBaseEntity, (Vec3& vVelocity), this, std::ref(vVelocity));
	SIGNATURE(InvalidateBoneCache, void, CBaseEntity, this);
	SIGNATURE(CreateShadow, void, CBaseEntity, this);
	inline Vec3 GetAbsVelocity()
	{
		Vec3 vOut;
		EstimateAbsVelocity(vOut);
		return vOut;
	}

	Vec3 GetSize();
	Vec3 GetOffset();
	Vec3 GetCenter();
	Vec3 GetRenderCenter();
	int IsInValidTeam();
	int SolidMask();
};