#include "CBaseProjectile.h"

#include "../../SDK.h"

bool CTFProjectile_Arrow::CanHeadshot()
{
	return m_iProjectileType() == TF_PROJECTILE_ARROW || m_iProjectileType() == TF_PROJECTILE_FESTIVE_ARROW;
}

bool CTFGrenadePipebombProjectile::HasStickyEffects()
{
	return m_iType() == TF_GL_MODE_REMOTE_DETONATE || m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE;
}