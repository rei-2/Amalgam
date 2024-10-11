#include "SDK.h"

#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Visuals/Notifications/Notifications.h"
#include <random>

#pragma warning (disable : 6385)

BOOL CALLBACK TeamFortressWindow(HWND hwnd, LPARAM lParam)
{
	char windowTitle[1024];
	GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
	if (std::string(windowTitle).find("Team Fortress 2 - ") == 0) // support both dx9 & vulkan
		*reinterpret_cast<HWND*>(lParam) = hwnd;

	return TRUE;
}




void SDK::Output(const char* cFunction, const char* cLog, Color_t cColor, bool bConsole, bool bChat, bool bToast, bool bDebug)
{
	if (cLog)
	{
		if (bConsole)
		{
			I::CVar->ConsoleColorPrintf(cColor, "[%s] ", cFunction);
			I::CVar->ConsoleColorPrintf({}, "%s\n", cLog);
		}
		if (bChat)
			I::ClientModeShared->m_pChatElement->ChatPrintf(0, std::format("{}[{}]\x1 {}", cColor.ToHex(), cFunction, cLog).c_str());
		if (bToast)
			F::Notifications.Add(std::format("[{}] {}", cFunction, cLog));
		if (bDebug)
			OutputDebugStringA(std::format("[{}] {}\n", cFunction, cLog).c_str());
	}
	else
	{
		if (bConsole)
			I::CVar->ConsoleColorPrintf(cColor, "%s\n", cFunction);
		if (bChat)
			I::ClientModeShared->m_pChatElement->ChatPrintf(0, std::format("{}{}\x1", cColor.ToHex(), cFunction).c_str());
		if (bToast)
			F::Notifications.Add(std::format("{}", cFunction));
		if (bDebug)
			OutputDebugStringA(std::format("{}\n", cFunction).c_str());
	}
}

HWND SDK::GetTeamFortressWindow()
{
	static HWND hwWindow = nullptr;
	while (!hwWindow)
	{
		EnumWindows(TeamFortressWindow, reinterpret_cast<LPARAM>(&hwWindow));
		Sleep(100);
	}
	return hwWindow;
}

bool SDK::IsGameWindowInFocus()
{
	return GetForegroundWindow() == GetTeamFortressWindow();
}

std::wstring SDK::ConvertUtf8ToWide(const std::string& source)
{
	std::wstring result(source.length(), 0);
	MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, result.data(), int(source.length()));
	return result;
}

std::string SDK::ConvertWideToUTF8(const std::wstring& source)
{
	std::string result(source.length(), 0);
	WideCharToMultiByte(CP_UTF8, 0, source.c_str(), -1, result.data(), int(source.length()), NULL, NULL);
	return result;
}

double SDK::PlatFloatTime()
{
	static auto fnPlatFloatTime = reinterpret_cast<double(*)()>(GetProcAddress(GetModuleHandleA("tier0.dll"), "Plat_FloatTime"));
	return fnPlatFloatTime();
}

int SDK::StdRandomInt(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(min, max);
	return distr(gen);
}

float SDK::StdRandomFloat(float min, float max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distr(min, max);
	return distr(gen);
}

int SDK::SeedFileLineHash(int seedvalue, const char* sharedname, int additionalSeed)
{
	CRC32_t retval;

	CRC32_Init(&retval);

	CRC32_ProcessBuffer(&retval, &seedvalue, sizeof(int));
	CRC32_ProcessBuffer(&retval, &additionalSeed, sizeof(int));
	CRC32_ProcessBuffer(&retval, sharedname, int(strlen(sharedname)));

	CRC32_Final(&retval);

	return static_cast<int>(retval);
}

int SDK::SharedRandomInt(unsigned iseed, const char* sharedname, int iMinVal, int iMaxVal, int additionalSeed)
{
	const int seed = SeedFileLineHash(iseed, sharedname, additionalSeed);
	I::UniformRandomStream->SetSeed(seed);
	return I::UniformRandomStream->RandomInt(iMinVal, iMaxVal);
}

void SDK::RandomSeed(int iSeed)
{
	static auto fnRandomSeed = reinterpret_cast<void(*)(uint32_t)>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed"));
	fnRandomSeed(iSeed);
}

int SDK::RandomInt(int iMinVal, int iMaxVal)
{
	static auto fnRandomInt = reinterpret_cast<int(*)(int, int)>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomInt"));
	return fnRandomInt(iMinVal, iMaxVal);
}

float SDK::RandomFloat(float flMinVal, float flMaxVal)
{
	static auto fnRandomFloat = reinterpret_cast<float(*)(float, float)>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat"));
	return fnRandomFloat(flMinVal, flMaxVal);
}

int SDK::HandleToIDX(unsigned int pHandle)
{
	return pHandle & 0xFFF;
}

bool SDK::W2S(const Vec3& vOrigin, Vec3& m_vScreen)
{
	const auto& worldToScreen = H::Draw.m_WorldToProjection.As3x4();

	float w = worldToScreen[3][0] * vOrigin[0] + worldToScreen[3][1] * vOrigin[1] + worldToScreen[3][2] * vOrigin[2] + worldToScreen[3][3];
	m_vScreen.z = 0;

	if (w > 0.001f)
	{
		const float fl1DBw = 1 / w;
		m_vScreen.x = (H::Draw.m_nScreenW / 2.f) + (0.5 * ((worldToScreen[0][0] * vOrigin[0] + worldToScreen[0][1] * vOrigin[1] + worldToScreen[0][2] * vOrigin[2] + worldToScreen[0][3]) * fl1DBw) * H::Draw.m_nScreenW + 0.5);
		m_vScreen.y = (H::Draw.m_nScreenH / 2.f) - (0.5 * ((worldToScreen[1][0] * vOrigin[0] + worldToScreen[1][1] * vOrigin[1] + worldToScreen[1][2] * vOrigin[2] + worldToScreen[1][3]) * fl1DBw) * H::Draw.m_nScreenH + 0.5);
		return true;
	}

	return false;
}

bool SDK::IsOnScreen(CBaseEntity* pEntity, const matrix3x4& transform, float* pLeft, float* pRight, float* pTop, float* pBottom)
{
	Vec3 vMins = pEntity->m_vecMins(), vMaxs = pEntity->m_vecMaxs();

	bool bInit = false;
	float flLeft = 0.f, flRight = 0.f, flTop = 0.f, flBottom = 0.f;

	const Vec3 vPoints[] = {
		Vec3(0.f, 0.f, vMins.z),
		Vec3(0.f, 0.f, vMaxs.z),
		Vec3(vMins.x, vMins.y, (vMins.z + vMaxs.z) * 0.5f),
		Vec3(vMins.x, vMaxs.y, (vMins.z + vMaxs.z) * 0.5f),
		Vec3(vMaxs.x, vMins.y, (vMins.z + vMaxs.z) * 0.5f),
		Vec3(vMaxs.x, vMaxs.y, (vMins.z + vMaxs.z) * 0.5f)
	};
	for (int n = 0; n < 6; n++)
	{
		Vec3 vPoint; Math::VectorTransform(vPoints[n], transform, vPoint);

		Vec3 vScreenPos;
		if (!W2S(vPoint, vScreenPos))
			continue;

		flLeft = bInit ? std::min(flLeft, vScreenPos.x) : vScreenPos.x;
		flRight = bInit ? std::max(flRight, vScreenPos.x) : vScreenPos.x;
		flTop = bInit ? std::max(flTop, vScreenPos.y) : vScreenPos.y;
		flBottom = bInit ? std::min(flBottom, vScreenPos.y) : vScreenPos.y;
		bInit = true;
	}

	if (!bInit)
		return false;

	if (pLeft) *pLeft = flLeft;
	if (pRight) *pRight = flRight;
	if (pTop) *pTop = flTop;
	if (pBottom) *pBottom = flBottom;

	return !(flRight < 0 || flLeft > H::Draw.m_nScreenW || flTop < 0 || flBottom > H::Draw.m_nScreenH);
}

bool SDK::IsOnScreen(CBaseEntity* pEntity, Vec3 vOrigin)
{
	Vec3 vMins = pEntity->m_vecMins(), vMaxs = pEntity->m_vecMaxs();

	bool bInit = false;
	float flLeft = 0.f, flRight = 0.f, flTop = 0.f, flBottom = 0.f;

	const Vec3 vPoints[] = {
		Vec3(0.f, 0.f, vMins.z),
		Vec3(0.f, 0.f, vMaxs.z),
		Vec3(vMins.x, vMins.y, (vMins.z + vMaxs.z) * 0.5f),
		Vec3(vMins.x, vMaxs.y, (vMins.z + vMaxs.z) * 0.5f),
		Vec3(vMaxs.x, vMins.y, (vMins.z + vMaxs.z) * 0.5f),
		Vec3(vMaxs.x, vMaxs.y, (vMins.z + vMaxs.z) * 0.5f)
	};
	for (int n = 0; n < 6; n++)
	{
		Vec3 vPoint = vOrigin + vPoints[n];

		Vec3 vScreenPos;
		if (!W2S(vPoint, vScreenPos))
			continue;

		flLeft = bInit ? std::min(flLeft, vScreenPos.x) : vScreenPos.x;
		flRight = bInit ? std::max(flRight, vScreenPos.x) : vScreenPos.x;
		flTop = bInit ? std::max(flTop, vScreenPos.y) : vScreenPos.y;
		flBottom = bInit ? std::min(flBottom, vScreenPos.y) : vScreenPos.y;
		bInit = true;
	}

	if (!bInit)
		return false;

	return !(flRight < 0 || flLeft > H::Draw.m_nScreenW || flTop < 0 || flBottom > H::Draw.m_nScreenH);
}

bool SDK::IsOnScreen(CBaseEntity* pEntity)
{
	if (auto pOwner = pEntity->m_hOwnerEntity().Get())
		pEntity = pOwner;

	return IsOnScreen(pEntity, pEntity->entindex() == I::EngineClient->GetLocalPlayer() ? F::EnginePrediction.vOrigin : pEntity->GetAbsOrigin());
}

void SDK::Trace(const Vec3& vecStart, const Vec3& vecEnd, unsigned int nMask, ITraceFilter* pFilter, CGameTrace* pTrace)
{
	Ray_t ray;
	ray.Init(vecStart, vecEnd);
	I::EngineTrace->TraceRay(ray, nMask, pFilter, pTrace);

	if (Vars::Debug::VisualizeTraces.Value)
		G::LineStorage.push_back({ { vecStart, Vars::Debug::VisualizeTraceHits.Value ? pTrace->endpos : vecEnd }, I::GlobalVars->curtime + 0.015f });
}

void SDK::TraceHull(const Vec3& vecStart, const Vec3& vecEnd, const Vec3& vecHullMin, const Vec3& vecHullMax, unsigned int nMask, ITraceFilter* pFilter, CGameTrace* pTrace)
{
	Ray_t ray;
	ray.Init(vecStart, vecEnd, vecHullMin, vecHullMax);
	I::EngineTrace->TraceRay(ray, nMask, pFilter, pTrace);

	if (Vars::Debug::VisualizeTraces.Value)
	{
		G::LineStorage.push_back({ { vecStart, Vars::Debug::VisualizeTraceHits.Value ? pTrace->endpos : vecEnd }, I::GlobalVars->curtime + 0.015f });
		if (!(vecHullMax - vecHullMin).IsZero())
		{
			G::BoxStorage.push_back({ vecStart, vecHullMin, vecHullMax, {}, I::GlobalVars->curtime + 0.015f, {}, { 0, 0, 0, 0 } });
			G::BoxStorage.push_back({ Vars::Debug::VisualizeTraceHits.Value ? pTrace->endpos : vecEnd, vecHullMin, vecHullMax, {}, I::GlobalVars->curtime + 0.015f, {}, { 0, 0, 0, 0 } });
		}
	}
}

bool SDK::VisPos(CBaseEntity* pSkip, const CBaseEntity* pEntity, const Vec3& from, const Vec3& to, unsigned int nMask)
{
	CGameTrace trace = {};
	CTraceFilterHitscan filter = {}; filter.pSkip = pSkip;
	Trace(from, to, nMask, &filter, &trace);
	if (trace.DidHit())
		return trace.m_pEnt && trace.m_pEnt == pEntity;
	return true;
}
bool SDK::VisPosProjectile(CBaseEntity* pSkip, const CBaseEntity* pEntity, const Vec3& from, const Vec3& to, unsigned int nMask)
{
	CGameTrace trace = {};
	CTraceFilterProjectile filter = {}; filter.pSkip = pSkip;
	Trace(from, to, nMask, &filter, &trace);
	if (trace.DidHit())
		return trace.m_pEnt && trace.m_pEnt == pEntity;
	return true;
}
bool SDK::VisPosWorld(CBaseEntity* pSkip, const CBaseEntity* pEntity, const Vec3& from, const Vec3& to, unsigned int nMask)
{
	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {}; filter.pSkip = pSkip;
	Trace(from, to, nMask, &filter, &trace);
	if (trace.DidHit())
		return trace.m_pEnt && trace.m_pEnt == pEntity;
	return true;
}

int SDK::GetRoundState()
{
	if (auto pGameRules = I::TFGameRules->Get())
		return pGameRules->m_iRoundState();
	return 0;
}

EWeaponType SDK::GetWeaponType(CTFWeaponBase* pWeapon, EWeaponType* pSecondaryType)
{
	if (pSecondaryType)
		*pSecondaryType = EWeaponType::UNKNOWN;
	if (!pWeapon)
		return EWeaponType::UNKNOWN;

	if (pSecondaryType)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_BAT_WOOD:
		case TF_WEAPON_BAT_GIFTWRAP:
			*pSecondaryType = EWeaponType::PROJECTILE;
		}
	}

	if (pWeapon->GetSlot() == EWeaponSlot::SLOT_MELEE || pWeapon->GetWeaponID() == TF_WEAPON_BUILDER)
		return EWeaponType::MELEE;

	switch (pWeapon->m_iItemDefinitionIndex())
	{
	case Soldier_s_TheBuffBanner:
	case Soldier_s_FestiveBuffBanner:
	case Soldier_s_TheBattalionsBackup:
	case Soldier_s_TheConcheror:
	case Scout_s_BonkAtomicPunch:
	case Scout_s_CritaCola:
		EWeaponType::UNKNOWN;
	}

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_INVIS:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_GRAPPLINGHOOK:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_ROCKETPACK:
		return EWeaponType::UNKNOWN;
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_FLAME_BALL:
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_FLAREGUN:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_DRG_POMSON:
	case TF_WEAPON_FLAREGUN_REVENGE:
	case TF_WEAPON_RAYGUN:
	case TF_WEAPON_CANNON:
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	case TF_WEAPON_FLAMETHROWER:
	case TF_WEAPON_CLEAVER:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_LUNCHBOX:
		return EWeaponType::PROJECTILE;
	}

	return EWeaponType::HITSCAN;
}

const char* SDK::GetClassByIndex(const int nClass)
{
	static const char* szClasses[] = {
		"unknown",
		"scout",
		"sniper",
		"soldier",
		"demoman",
		"medic",
		"heavy",
		"pyro",
		"spy",
		"engineer"
	};

	return (nClass < 10 && nClass > 0) ? szClasses[nClass] : szClasses[0];
}

bool SDK::IsAttacking(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, const CUserCmd* pCmd, bool bTickBase)
{
	if (pCmd->weaponselect)
		return false;

	int iTickBase = bTickBase ? I::GlobalVars->tickcount : pLocal->m_nTickBase();
	float flTickBase = bTickBase ? I::GlobalVars->curtime : TICKS_TO_TIME(iTickBase);

	if (pWeapon->GetSlot() == SLOT_MELEE)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_KNIFE:
			return ((pCmd->buttons & IN_ATTACK) && G::CanPrimaryAttack);
		case TF_WEAPON_BAT_WOOD:
		case TF_WEAPON_BAT_GIFTWRAP:
		{
			static int iThrowTick = 0;
			if (G::CanSecondaryAttack && pWeapon->HasPrimaryAmmoForShot() && pCmd->buttons & IN_ATTACK2 && iThrowTick + 5 < iTickBase)
				iThrowTick = iTickBase + 11;
			if (iThrowTick >= iTickBase)
				G::CanPrimaryAttack = G::CanSecondaryAttack = G::IsThrowing = true;
			if (iThrowTick == iTickBase)
				return true;
		}
		}

		return TIME_TO_TICKS(pWeapon->m_flSmackTime()) == iTickBase - 1;
	}

	const int iWeaponID = pWeapon->GetWeaponID();
	switch (iWeaponID)
	{
	case TF_WEAPON_COMPOUND_BOW:
		return !(pCmd->buttons & IN_ATTACK) && pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f;
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_STICKY_BALL_LAUNCHER:
	case TF_WEAPON_GRENADE_STICKY_BALL:
	{
		float flCharge = pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? flTickBase - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f;
		const float flAmount = Math::RemapValClamped(flCharge, 0.f, SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f);
		return !(pCmd->buttons & IN_ATTACK) && flAmount > 0.f || flAmount == 1.f;
	}
	case TF_WEAPON_CANNON:
	{
		float flMortar = SDK::AttribHookValue(0.f, "grenade_launcher_mortar_mode", pWeapon);
		if (!flMortar)
			return pCmd->buttons & IN_ATTACK && G::CanPrimaryAttack;

		float flCharge = pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() > 0.f ? flMortar - (pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() - flTickBase) : 0.f;
		const float flAmount = Math::RemapValClamped(flCharge, 0.f, SDK::AttribHookValue(0.f, "grenade_launcher_mortar_mode", pWeapon), 0.f, 1.f);
		return !(pCmd->buttons & IN_ATTACK) && flAmount > 0.f || flAmount == 1.f;
	}
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		return !(pCmd->buttons & IN_ATTACK) && pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() > 0.f;
	case TF_WEAPON_CLEAVER: // we can randomly use attack2 to fire
	{
		static int iThrowTick = 0;
		if (G::CanPrimaryAttack && pWeapon->HasPrimaryAmmoForShot() && pCmd->buttons & (IN_ATTACK | IN_ATTACK2) && iThrowTick < iTickBase)
			iThrowTick = iTickBase + 11;
		if (iThrowTick >= iTickBase)
			G::CanPrimaryAttack = G::IsThrowing = true;
		return iThrowTick == iTickBase;
	}
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_JAR_GAS:
	{
		static int iThrowTick = 0;
		if (G::CanPrimaryAttack && pWeapon->HasPrimaryAmmoForShot() && pCmd->buttons & IN_ATTACK && iThrowTick < iTickBase)
			iThrowTick = iTickBase + 11;
		if (iThrowTick >= iTickBase)
			G::CanPrimaryAttack = G::IsThrowing = true;
		return iThrowTick == iTickBase;
	}
	case TF_WEAPON_GRAPPLINGHOOK:
	{
		if (G::LastUserCmd->buttons & IN_ATTACK || !(pCmd->buttons & IN_ATTACK))
			return false;

		if (pLocal->InCond(TF_COND_GRAPPLINGHOOK))
			return false;

		Vec3 vPos, vAngle; GetProjectileFireSetup(pLocal, pCmd->viewangles, { 23.5f, -8.f, -3.f }, vPos, vAngle, false);
		Vec3 vForward; Math::AngleVectors(vAngle, &vForward);

		CGameTrace trace = {};
		CTraceFilterHitscan filter = {}; filter.pSkip = pLocal;
		static auto tf_grapplinghook_max_distance = U::ConVars.FindVar("tf_grapplinghook_max_distance");
		const float flGrappleDistance = tf_grapplinghook_max_distance ? tf_grapplinghook_max_distance->GetFloat() : 2000.f;
		Trace(vPos, vPos + vForward * flGrappleDistance, MASK_SOLID, &filter, &trace);
		return trace.DidHit() && !(trace.surface.flags & 0x0004 /*SURF_SKY*/);
	}
	case TF_WEAPON_MINIGUN:
		return (pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_FIRING || pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_SPINNING) &&
			pCmd->buttons & IN_ATTACK && G::CanPrimaryAttack;
	}

	if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
	{
		static bool bLoading = false, bFiring = false;

		if (pWeapon->m_iClip1() == 0)
			bLoading = false,
			bFiring = false;
		else if (!bFiring)
			bLoading = true;

		if ((bFiring || bLoading && !(pCmd->buttons & IN_ATTACK)) && G::CanPrimaryAttack)
		{
			bFiring = true;
			bLoading = false;
			return true; //!pWeapon->IsInReload();
		}

		return false;
	}

	return pCmd->buttons & IN_ATTACK && G::CanPrimaryAttack;
}

float SDK::MaxSpeed(CTFPlayer* pPlayer, bool bIncludeCrouch, bool bIgnoreSpecialAbility)
{
	float flSpeed = pPlayer->TeamFortress_CalculateMaxSpeed(bIgnoreSpecialAbility);

	if (pPlayer->InCond(TF_COND_SPEED_BOOST) ||
		pPlayer->InCond(TF_COND_HALLOWEEN_SPEED_BOOST))
		flSpeed *= 1.35f;
	if (bIncludeCrouch && pPlayer->IsDucking())
		flSpeed /= 3;

	return flSpeed;
}

void SDK::FixMovement(CUserCmd* pCmd, const Vec3& vTargetAngle)
{
	const Vec3 vMove = { pCmd->forwardmove, pCmd->sidemove, pCmd->upmove };

	Vec3 vMoveAng = {};
	Math::VectorAngles(vMove, vMoveAng);

	const float flSpeed = vMove.Length2D();
	const float flYaw = DEG2RAD(vTargetAngle.y - pCmd->viewangles.y + vMoveAng.y);

	pCmd->forwardmove = (cos(flYaw) * flSpeed);
	pCmd->sidemove = (sin(flYaw) * flSpeed);
}

bool SDK::StopMovement(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (pLocal->m_vecVelocity().IsZero())
	{
		pCmd->forwardmove = 0.f;
		pCmd->sidemove = 0.f;
		return false;
	}

	if (!G::IsAttacking)
	{
		const float direction = Math::VelocityToAngles(pLocal->m_vecVelocity() * -1).y;
		pCmd->viewangles = { 90, direction, 0 };
		pCmd->sidemove = 0; pCmd->forwardmove = 0;
		return true;
	}
	else
	{
		Vec3 direction = pLocal->m_vecVelocity().toAngle();
		direction.y = pCmd->viewangles.y - direction.y;
		const Vec3 negatedDirection = direction.fromAngle() * -pLocal->m_vecVelocity().Length2D();
		pCmd->forwardmove = negatedDirection.x;
		pCmd->sidemove = negatedDirection.y;
		return false;
	}
}

Vec3 SDK::ComputeMove(const CUserCmd* pCmd, CTFPlayer* pLocal, Vec3& a, Vec3& b)
{
	const Vec3 diff = (b - a);
	if (diff.Length() == 0.f)
		return { 0.f, 0.f, 0.f };

	const float x = diff.x;
	const float y = diff.y;
	const Vec3 vSilent(x, y, 0);
	Vec3 ang;
	Math::VectorAngles(vSilent, ang);
	const float yaw = DEG2RAD(ang.y - pCmd->viewangles.y);
	const float pitch = DEG2RAD(ang.x - pCmd->viewangles.x);
	Vec3 move = { cos(yaw) * 450.f, -sin(yaw) * 450.f, -cos(pitch) * 450.f };

	// Only apply upmove in water
	if (!(I::EngineTrace->GetPointContents(pLocal->GetEyePosition()) & CONTENTS_WATER))
		move.z = pCmd->upmove;
	return move;
}

void SDK::WalkTo(CUserCmd* pCmd, CTFPlayer* pLocal, Vec3& a, Vec3& b, float scale)
{
	// Calculate how to get to a vector
	const auto result = ComputeMove(pCmd, pLocal, a, b);

	// Push our move to usercmd
	pCmd->forwardmove = result.x * scale;
	pCmd->sidemove = result.y * scale;
	pCmd->upmove = result.z * scale;
}

void SDK::WalkTo(CUserCmd* pCmd, CTFPlayer* pLocal, Vec3& pDestination)
{
	Vec3 localPos = pLocal->m_vecOrigin();
	WalkTo(pCmd, pLocal, localPos, pDestination, 1.f);
}



class CTraceFilterSetup : public ITraceFilter // trace filter solely for GetProjectileFireSetup
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask) override;
	TraceType_t GetTraceType() const override;
	CBaseEntity* pSkip = nullptr;
};
bool CTraceFilterSetup::ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask)
{
	if (!pServerEntity || pServerEntity == pSkip)
		return false;

	auto pEntity = reinterpret_cast<CBaseEntity*>(pServerEntity);
	auto pLocal = H::Entities.GetLocal();

	const int iTargetTeam = pEntity->m_iTeamNum(), iLocalTeam = pLocal ? pLocal->m_iTeamNum() : iTargetTeam;

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFAmmoPack:
	case ETFClassID::CFuncAreaPortalWindow:
	case ETFClassID::CFuncRespawnRoomVisualizer:
	case ETFClassID::CTFReviveMarker: return false;
	case ETFClassID::CTFMedigunShield:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter: return true;
	case ETFClassID::CTFPlayer: return iTargetTeam != iLocalTeam;
	}

	return true;
}
TraceType_t CTraceFilterSetup::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

void SDK::GetProjectileFireSetup(CTFPlayer* pPlayer, const Vec3& vAngIn, Vec3 vOffset, Vec3& vPosOut, Vec3& vAngOut, bool bPipes, bool bInterp)
{
	static auto cl_flipviewmodels = U::ConVars.FindVar("cl_flipviewmodels");
	if (cl_flipviewmodels && cl_flipviewmodels->GetBool())
		vOffset.y *= -1.f;

	const Vec3 vShootPos = bInterp ? pPlayer->GetEyePosition() : pPlayer->GetShootPos();

	Vec3 vForward, vRight, vUp; Math::AngleVectors(vAngIn, &vForward, &vRight, &vUp);
	vPosOut = vShootPos + (vForward * vOffset.x) + (vRight * vOffset.y) + (vUp * vOffset.z);

	if (bPipes)
		vAngOut = vAngIn;
	else
	{
		Vec3 vEndPos = vShootPos + (vForward * 2000.f);

		CGameTrace trace = {};
		CTraceFilterSetup filter = {};
		Trace(vShootPos, vEndPos, MASK_SOLID, &filter, &trace);
		if (trace.DidHit() && trace.fraction > 0.1f)
			vEndPos = trace.endpos;

		Math::VectorAngles(vEndPos - vPosOut, vAngOut);
	}
}

void SDK::GetProjectileFireSetupAirblast(CTFPlayer* pPlayer, const Vec3& vAngIn, Vec3 vPosIn, Vec3& vAngOut, bool bInterp)
{
	const Vec3 vShootPos = bInterp ? pPlayer->GetEyePosition() : pPlayer->GetShootPos();

	Vec3 vForward; Math::AngleVectors(vAngIn, &vForward);

	Vec3 vEndPos = vShootPos + (vForward * MAX_TRACE_LENGTH);
	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};
	Trace(vShootPos, vEndPos, MASK_SOLID, &filter, &trace);

	Math::VectorAngles(trace.endpos - vPosIn, vAngOut);
}