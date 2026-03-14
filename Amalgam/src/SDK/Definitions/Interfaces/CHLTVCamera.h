#pragma once
#include "../Main/CUserCmd.h"
#include "../Misc/CGameEventListener.h"

class CHLTVCamera : CGameEventListener
{
public:
	int			m_nCameraMode;
	int			m_iCameraMan;
	Vector		m_vCamOrigin;
	QAngle		m_aCamAngle;
	int			m_iTraget1;
	int			m_iTraget2;
	float		m_flFOV;
	float		m_flOffset;
	float		m_flDistance;
	float		m_flLastDistance;
	float		m_flTheta;
	float		m_flPhi;
	float		m_flInertia;
	float		m_flLastAngleUpdateTime;
	bool		m_bEntityPacketReceived;
	int			m_nNumSpectators;
	char		m_szTitleText[64];
	CUserCmd	m_LastCmd;
	Vector		m_vecVelocity;
};