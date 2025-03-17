#include "CheaterDetection.h"

#include "../Players/PlayerUtils.h"
#include "../Records/Records.h"

bool CCheaterDetection::ShouldScan()
{
	if (!Vars::CheaterDetection::Methods.Value || I::EngineClient->IsPlayingDemo())
		return false;

	static float flOldTime = I::GlobalVars->curtime;
	const float flCurTime = I::GlobalVars->curtime;
	const bool bShouldSkip = TIME_TO_TICKS(flCurTime - flOldTime) != 1;
	flOldTime = flCurTime;
	if (bShouldSkip)
		return false;

	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (pNetChan && (pNetChan->GetTimeSinceLastReceived() > TICK_INTERVAL * 2 || pNetChan->IsTimingOut()))
		return false;

	return true;
}

bool CCheaterDetection::InvalidPitch(CTFPlayer* pEntity)
{
	return Vars::CheaterDetection::Methods.Value & Vars::CheaterDetection::MethodsEnum::InvalidPitch && fabsf(pEntity->m_angEyeAnglesX()) == 90.f;
}

bool CCheaterDetection::IsChoking(CTFPlayer* pEntity)
{
	bool bReturn = mData[pEntity].m_PacketChoking.m_bInfract;
	mData[pEntity].m_PacketChoking.m_bInfract = false;

	return Vars::CheaterDetection::Methods.Value & Vars::CheaterDetection::MethodsEnum::PacketChoking && bReturn;
}

bool CCheaterDetection::IsFlicking(CTFPlayer* pEntity) // awful
{
	auto& vAngles = mData[pEntity].m_AimFlicking.m_vAngles;
	if (!(Vars::CheaterDetection::Methods.Value & Vars::CheaterDetection::MethodsEnum::AimFlicking))
	{
		vAngles.clear();
		return false;
	}

	vAngles.emplace_front(pEntity->GetEyeAngles(), false);
	if (vAngles.size() > 3)
		vAngles.pop_back();

	if (vAngles.size() != 3 || !vAngles[0].m_bAttacking && !vAngles[1].m_bAttacking && !vAngles[2].m_bAttacking
		|| Math::CalcFov(vAngles[0].m_vAngle, vAngles[1].m_vAngle) < Vars::CheaterDetection::MinimumFlick.Value
		|| Math::CalcFov(vAngles[0].m_vAngle, vAngles[2].m_vAngle) > Vars::CheaterDetection::MaximumNoise.Value * (TICK_INTERVAL / 0.015f))
		return false;

	vAngles.clear();
	return true;
}

bool CCheaterDetection::IsDuckSpeed(CTFPlayer* pEntity)
{
	if (!(Vars::CheaterDetection::Methods.Value & Vars::CheaterDetection::MethodsEnum::DuckSpeed)
		|| !pEntity->IsDucking() || !pEntity->IsOnGround()
		|| pEntity->m_vecVelocity().Length2D() < pEntity->m_flMaxspeed() * 0.5f)
	{
		mData[pEntity].m_DuckSpeed.m_iStartTick = 0;
		return false;
	}

	if (!mData[pEntity].m_DuckSpeed.m_iStartTick)
		mData[pEntity].m_DuckSpeed.m_iStartTick = I::GlobalVars->tickcount;

	if (I::GlobalVars->tickcount - mData[pEntity].m_DuckSpeed.m_iStartTick > TIME_TO_TICKS(1))
	{
		mData[pEntity].m_DuckSpeed.m_iStartTick = 0;
		return true;
	}

	return false;
}

void CCheaterDetection::Infract(CTFPlayer* pEntity, std::string sReason)
{
	bool bMark = false;
	if (Vars::CheaterDetection::DetectionsRequired.Value)
	{
		mData[pEntity].m_iDetections++;
		bMark = mData[pEntity].m_iDetections >= Vars::CheaterDetection::DetectionsRequired.Value;
	}

	F::Records.CheatDetection(mData[pEntity].m_sName, bMark ? "marked" : "infracted", sReason);
	if (bMark)
	{
		mData[pEntity].m_iDetections = 0;
		F::PlayerUtils.AddTag(mData[pEntity].m_uFriendsID, F::PlayerUtils.TagToIndex(CHEATER_TAG), true, mData[pEntity].m_sName);
	}
}

void CCheaterDetection::Run()
{
	if (!ShouldScan() || !I::EngineClient->IsConnected() || I::EngineClient->IsPlayingDemo())
		return;

	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!H::Entities.GetDeltaTime(pPlayer->entindex()))
			continue;

		PlayerInfo_t pi{};
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant()
			|| !I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi) || pi.fakeplayer || F::PlayerUtils.HasTag(pi.friendsID, F::PlayerUtils.TagToIndex(CHEATER_TAG)))
		{
			mData[pPlayer].m_PacketChoking = {};
			mData[pPlayer].m_AimFlicking = {};
			mData[pPlayer].m_DuckSpeed = {};
			continue;
		}

		mData[pPlayer].m_uFriendsID = pi.friendsID;
		mData[pPlayer].m_sName = F::PlayerUtils.GetPlayerName(pPlayer->entindex(), pi.name);

		if (InvalidPitch(pPlayer))
			Infract(pPlayer, "invalid pitch");
		if (IsChoking(pPlayer))
			Infract(pPlayer, "choking packets");
		if (IsFlicking(pPlayer))
			Infract(pPlayer, "flicking");
		if (IsDuckSpeed(pPlayer))
			Infract(pPlayer, "duck speed");
	}
}

void CCheaterDetection::Reset()
{
	mData.clear();
}

void CCheaterDetection::ReportChoke(CTFPlayer* pEntity, int iChoke)
{
	if (Vars::CheaterDetection::Methods.Value & Vars::CheaterDetection::MethodsEnum::PacketChoking)
	{
		mData[pEntity].m_PacketChoking.m_vChokes.push_back(iChoke);
		if (mData[pEntity].m_PacketChoking.m_vChokes.size() == 3)
		{
			mData[pEntity].m_PacketChoking.m_bInfract = true; // check for last 3 choke amounts
			for (auto& iChoke : mData[pEntity].m_PacketChoking.m_vChokes)
			{
				if (iChoke < Vars::CheaterDetection::MinimumChoking.Value)
					mData[pEntity].m_PacketChoking.m_bInfract = false;
			}
			mData[pEntity].m_PacketChoking.m_vChokes.clear();
		}
	}
	else
		mData[pEntity].m_PacketChoking.m_vChokes.clear();
}

void CCheaterDetection::ReportDamage(IGameEvent* pEvent)
{
	if (!(Vars::CheaterDetection::Methods.Value & Vars::CheaterDetection::MethodsEnum::AimFlicking))
		return;

	int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
	if (iIndex == I::EngineClient->GetLocalPlayer())
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(iIndex)->As<CTFPlayer>();
	if (!pEntity || !pEntity->IsPlayer() || pEntity->IsDormant())
		return;

	auto pWeapon = pEntity->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
	switch (SDK::GetWeaponType(pWeapon))
	{
	case EWeaponType::UNKNOWN:
	case EWeaponType::PROJECTILE:
		return;
	}

	if (!mData[pEntity->As<CTFPlayer>()].m_AimFlicking.m_vAngles.empty())
		mData[pEntity->As<CTFPlayer>()].m_AimFlicking.m_vAngles.back().m_bAttacking = true;
}