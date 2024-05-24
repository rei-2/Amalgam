#pragma once
#include "../../Definitions/Types.h"
#include "../../Definitions/Main/CBaseEntity.h"

MAKE_SIGNATURE(CTE_DispatchEffect, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 48 8B FA 48 8D 4C 24", 0x0);
MAKE_SIGNATURE(Get_ParticleSystemIndex, "client.dll", "40 53 48 83 EC ? 48 8B D9 48 85 C9 74 ? 48 8B 0D ? ? ? ? 48 8B D3 48 8B 01 FF 50 ? 3D", 0x0);
MAKE_SIGNATURE(UTIL_ParticleTracer, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 4C 89 74 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 49 63 F1", 0x0);

enum ParticleAttachment_t
{
	PATTACH_ABSORIGIN = 0,			// Create at absorigin, but don't follow
	PATTACH_ABSORIGIN_FOLLOW,		// Create at absorigin, and update to follow the entity
	PATTACH_CUSTOMORIGIN,			// Create at a custom origin, but don't follow
	PATTACH_POINT,					// Create on attachment point, but don't follow
	PATTACH_POINT_FOLLOW,			// Create on attachment point, and update to follow the entity

	PATTACH_WORLDORIGIN,			// Used for control points that don't attach to an entity

	PATTACH_ROOTBONE_FOLLOW,		// Create at the root bone of the entity, and update to follow

	MAX_PATTACH_TYPES,
};

struct te_tf_particle_effects_colors_t
{
	Vec3 m_vecColor1;
	Vec3 m_vecColor2;
};

struct te_tf_particle_effects_control_point_t
{
	ParticleAttachment_t m_eParticleAttachment;
	Vec3 m_vecOffset;
};

class CEffectData
{
public:
	Vec3									m_vOrigin;
	Vec3									m_vStart;
	Vec3									m_vNormal;
	Vec3									m_vAngles;
	int										m_fFlags;
	int										m_nEntIndex;
	float									m_flScale;
	float									m_flMagnitude;
	float									m_flRadius;
	int										m_nAttachmentIndex;
	short									m_nSurfaceProp;
	int										m_nMaterial;
	int										m_nDamageType;
	int										m_nHitBox;
	unsigned char							m_nColor;
	bool									m_bCustomColors;
	te_tf_particle_effects_colors_t			m_CustomColors;
	bool									m_bControlPoint1;
	te_tf_particle_effects_control_point_t	m_ControlPoint1;
	int										m_iEffectName;
};

class CParticles
{
public:
	inline void DispatchEffect(const char* pName, const CEffectData& data)
	{
		static auto fnDispatchEffect = S::CTE_DispatchEffect.As<int(__cdecl*)(const char*, const CEffectData&)>();
		fnDispatchEffect(pName, data);
	}

	inline int GetParticleSystemIndex(const char* pParticleSystemName)
	{
		static auto fnGetParticleSystemIndex = S::Get_ParticleSystemIndex.As<int(__cdecl*)(const char*)>();
		return fnGetParticleSystemIndex(pParticleSystemName);
	}

	inline void DispatchParticleEffect(int iEffectIndex, const Vec3& vecOrigin, const Vec3& vecStart, const Vec3& vecAngles, CBaseEntity* pEntity = nullptr)
	{
		CEffectData data{};
		data.m_nHitBox = iEffectIndex;
		data.m_vOrigin = vecOrigin;
		data.m_vStart = vecStart;
		data.m_vAngles = vecAngles;

		if (pEntity) {
			data.m_nEntIndex = pEntity->entindex();
			data.m_fFlags |= (1 << 0);
			data.m_nDamageType = 2;
		}
		else {
			data.m_nEntIndex = 0;
		}

		data.m_bCustomColors = true;

		DispatchEffect("ParticleEffect", data);
	}

	inline void DispatchParticleEffect(const char* pszParticleName, const Vec3& vecOrigin, const Vec3& vecAngles, CBaseEntity* pEntity = nullptr)
	{
		const int iIndex = GetParticleSystemIndex(pszParticleName);
		DispatchParticleEffect(iIndex, vecOrigin, vecOrigin, vecAngles, pEntity);
	}

	inline void ParticleTracer(const char* pszTracerEffectName, const Vector& vecStart, const Vector& vecEnd, int iEntIndex, int iAttachment, bool bWhiz)
	{
		static auto fnParticleTracer = S::UTIL_ParticleTracer.As<void(__cdecl*)(const char*, const Vec3&, const Vec3&, int, int, bool)>();
		fnParticleTracer(pszTracerEffectName, vecStart, vecEnd, iEntIndex, iAttachment, bWhiz);
	}
};

ADD_FEATURE_CUSTOM(CParticles, Particles, H)