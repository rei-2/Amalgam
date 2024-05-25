#include "CheaterDetection.h"

#include "../Players/PlayerUtils.h"
#include "../Records/Records.h"

bool CCheaterDetection::ShouldScan()
{
	if (!Vars::CheaterDetection::Methods.Value || I::EngineClient->IsPlayingTimeDemo())
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
	return Vars::CheaterDetection::Methods.Value & (1 << 0) && fabsf(pEntity->m_angEyeAnglesX()) > 89.9f;
}

bool CCheaterDetection::IsChoking(CTFPlayer* pEntity)
{
	const bool bReturn = mData[pEntity].bChoke;
	mData[pEntity].bChoke = false;

	return Vars::CheaterDetection::Methods.Value & (1 << 1) && bReturn;
}

bool CCheaterDetection::IsFlicking(CTFPlayer* pEntity) // this is aggravating
{
	auto& vAngles = mData[pEntity].vAngles;
	if (!(Vars::CheaterDetection::Methods.Value & (1 << 2)))
	{
		vAngles.clear();
		return false;
	}

	if (vAngles.size() != 3 || !vAngles[0].second && !vAngles[1].second && !vAngles[2].second)
		return false;

	if (Math::CalcFov(vAngles[0].first, vAngles[1].first) < Vars::CheaterDetection::MinimumFlick.Value)
		return false;

	if (Math::CalcFov(vAngles[0].first, vAngles[2].first) > Vars::CheaterDetection::MaximumNoise.Value * (TICK_INTERVAL / 0.015f))
		return false;

	vAngles.clear();
	return true;
}

bool CCheaterDetection::IsDuckSpeed(CTFPlayer* pEntity)
{
	if (!(Vars::CheaterDetection::Methods.Value & (1 << 3))
		|| !pEntity->IsDucking() || !pEntity->OnSolid() // this may break on movement sim
		|| pEntity->m_vecVelocity().Length2D() < pEntity->m_flMaxspeed() * 0.5f)
	{
		mData[pEntity].iDuckSpeed = 0;
		return false;
	}

	mData[pEntity].iDuckSpeed++;
	if (mData[pEntity].iDuckSpeed > 20)
	{
		mData[pEntity].iDuckSpeed = 0;
		return true;
	}

	return false;
}

void CCheaterDetection::Infract(CTFPlayer* pEntity, std::string sReason)
{
	mData[pEntity].iDetections++;
	const bool bMark = mData[pEntity].iDetections >= Vars::CheaterDetection::DetectionsRequired.Value;

	F::Records.CheatDetection(mData[pEntity].sName, bMark ? "marked" : "infracted", sReason);
	if (bMark)
	{
		mData[pEntity].iDetections = 0;
		F::PlayerUtils.AddTag(mData[pEntity].friendsID, "Cheater", true, mData[pEntity].sName);
	}
}

void CCheaterDetection::Run()
{
	if (!ShouldScan() || !I::EngineClient->IsConnected())
		return;

	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!mData[pPlayer].bShouldScan)
			continue;

		mData[pPlayer].bShouldScan = false;

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

void CCheaterDetection::Fill() // maybe just run here
{
	if (!Vars::CheaterDetection::Methods.Value || I::EngineClient->IsPlayingTimeDemo())
		return;

	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		mData[pPlayer].bShouldScan = false;

		PlayerInfo_t pi{};
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant()
			|| !I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi) || pi.fakeplayer || F::PlayerUtils.HasTag(pi.friendsID, "Cheater"))
		{
			mData[pPlayer].vChokes.clear();
			mData[pPlayer].bChoke = false;
			mData[pPlayer].vAngles.clear();
			mData[pPlayer].iDuckSpeed = 0;
			continue;
		}

		if (pPlayer->m_flSimulationTime() == pPlayer->m_flOldSimulationTime())
			continue;

		mData[pPlayer].bShouldScan = true;
		mData[pPlayer].friendsID = pi.friendsID;
		mData[pPlayer].sName = pi.name;
		mData[pPlayer].vAngles.push_back({ pPlayer->GetEyeAngles(), mData[pPlayer].bDamage });
		mData[pPlayer].bDamage = false;
		if (mData[pPlayer].vAngles.size() > 3)
			mData[pPlayer].vAngles.pop_front();
	}
}

void CCheaterDetection::Reset()
{
	mData.clear();
}

void CCheaterDetection::ReportChoke(CTFPlayer* pEntity, int iChoke)
{
	if (Vars::CheaterDetection::Methods.Value & (1 << 1))
	{
		mData[pEntity].vChokes.push_back(iChoke);
		if (mData[pEntity].vChokes.size() == 3)
		{
			mData[pEntity].bChoke = true; // check for last 3 choke amounts
			for (auto& iChoke : mData[pEntity].vChokes)
			{
				if (iChoke < Vars::CheaterDetection::MinimumChoking.Value)
					mData[pEntity].bChoke = false;
			}
			mData[pEntity].vChokes.clear();
		}
	}
	else
		mData[pEntity].vChokes.clear();
}

void CCheaterDetection::ReportDamage(IGameEvent* pEvent)
{
	if (!(Vars::CheaterDetection::Methods.Value & (1 << 2)))
		return;

	const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
	if (iIndex == I::EngineClient->GetLocalPlayer())
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
	if (!pEntity || pEntity->IsDormant())
		return;

	switch (SDK::GetWeaponType(pEntity->As<CTFPlayer>()->m_hActiveWeapon().Get()->As<CTFWeaponBase>()))
	{
	case EWeaponType::UNKNOWN:
	case EWeaponType::PROJECTILE:
		return;
	}

	mData[pEntity->As<CTFPlayer>()].bDamage = true;
}