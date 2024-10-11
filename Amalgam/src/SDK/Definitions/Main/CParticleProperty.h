#pragma once
#include "UtlVector.h"

class CBaseEntity;
struct ParticleEffectList_t;

class CParticleProperty
{
public:
    CBaseEntity* m_pOuter;
    CUtlVector<ParticleEffectList_t> m_ParticleEffects;
    int m_iDormancyChangedAtFrame;

    friend class CBaseEntity;
};