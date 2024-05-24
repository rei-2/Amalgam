#pragma once
#include "../Types.h"

#pragma warning (disable : 26812)

enum LightType_t
{
	MATERIAL_LIGHT_DISABLE = 0,
	MATERIAL_LIGHT_POINT,
	MATERIAL_LIGHT_DIRECTIONAL,
	MATERIAL_LIGHT_SPOT
};

enum LightType_OptimizationFlags_t
{
	LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION0 = 1,
	LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION1 = 2,
	LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION2 = 4,
	LIGHTTYPE_OPTIMIZATIONFLAGS_DERIVED_VALUES_CALCED = 8
};

struct LightDesc_t
{
public:
	LightType_t m_Type;
	Vector m_Color;
	Vector m_Position;
	Vector m_Direction;
	float  m_Range;
	float m_Falloff;
	float m_Attenuation0;
	float m_Attenuation1;
	float m_Attenuation2;
	float m_Theta;
	float m_Phi;
	float m_ThetaDot;
	float m_PhiDot;
	unsigned int m_Flags;
	float OneOver_ThetaDot_Minus_PhiDot;
	float m_RangeSquared;

	bool IsDirectionWithinLightCone(const Vector& rdir) const
	{
		return ((m_Type != MATERIAL_LIGHT_SPOT) || (rdir.Dot(m_Direction) >= m_PhiDot));
	}

	float OneOverThetaDotMinusPhiDot() const
	{
		return OneOver_ThetaDot_Minus_PhiDot;
	}
};