#include "CBaseProjectile.h"

#include "../../SDK.h"

bool CTFGrenadePipebombProjectile::HasStickyEffects()
{
	return m_iType() == TF_GL_MODE_REMOTE_DETONATE || m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE;
}