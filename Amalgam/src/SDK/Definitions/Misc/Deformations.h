#pragma once
#include "../Types.h"

enum DeformationType_t
{
	DEFORMATION_CLAMP_TO_BOX_IN_WORLDSPACE = 1
};

struct DeformationBase_t
{
	DeformationType_t m_eType;
};

struct BoxDeformation_t : DeformationBase_t
{
	Vector m_SourceMins;
	float m_flPad0;
	Vector m_SourceMaxes;
	float m_flPad1;
	Vector m_ClampMins;
	float m_flPad2;
	Vector m_ClampMaxes;
	float m_flPad3;
};