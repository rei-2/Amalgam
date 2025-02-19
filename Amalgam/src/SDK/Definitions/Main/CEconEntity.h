#pragma once
#include "CBaseAnimating.h"

class CEconEntity : public CBaseAnimating
{
public:
	NETVAR(m_iItemDefinitionIndex, int, "CEconEntity", "m_iItemDefinitionIndex");

	VIRTUAL(UpdateAttachmentModels, void, void*, this, 213);
};