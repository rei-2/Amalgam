#include "Resolver.h"

#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Backtrack/Backtrack.h"

static std::vector<float> vYawRotations{ 0.0f, 180.0f, 90.0f, -90.0f};

void PResolver::UpdateSniperDots()
{
	mSniperDots.clear();
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::MISC_DOTS))
	{
		if (auto pOwner = pEntity->m_hOwnerEntity().Get())
			mSniperDots[pOwner] = pEntity->m_vecOrigin();
	}
}

std::optional<float> PResolver::GetPitchForSniperDot(CTFPlayer* pEntity)
{
	if (mSniperDots.contains(pEntity))
	{
		const Vec3 vOrigin = mSniperDots[pEntity];
		const Vec3 vEyeOrigin = pEntity->GetEyePosition();
		const Vec3 vDelta = vOrigin - vEyeOrigin;
		Vec3 vAngles;
		Math::VectorAngles(vDelta, vAngles);
		return vAngles.x;
	}
	return std::nullopt;
}

std::optional<float> PResolver::PredictBaseYaw(CTFPlayer* pLocal, CTFPlayer* pEntity)
{
	if (I::GlobalVars->tickcount - mResolverData[pEntity].pLastFireAngles.first.first > 66 || !mResolverData[pEntity].pLastFireAngles.first.first)
	{	// staleness & validity check
		if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
			return std::nullopt;
		return Math::CalcAngle(pEntity->m_vecOrigin(), pLocal->m_vecOrigin()).y;
	}

	bool bFound = false;
	float flSmallestAngleTo = 0.f; float flSmallestFovTo = 360.f;
	for (auto pTarget : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pTarget->As<CTFPlayer>();
		if (!pPlayer || pPlayer->IsAGhost() || !pPlayer->IsAlive() || pPlayer->m_iTeamNum() == pEntity->m_iTeamNum())
			continue;

		const Vec3 vAngleTo = Math::CalcAngle(pEntity->m_vecOrigin(), pPlayer->m_vecOrigin());
		const float flFOVTo = Math::CalcFov(mResolverData[pEntity].pLastFireAngles.second, vAngleTo);
	
		if (flFOVTo < flSmallestFovTo)
		{
			bFound = true;
			flSmallestAngleTo = vAngleTo.y;
			flSmallestFovTo = flFOVTo;
		}
	}

	if (!bFound)
		return std::nullopt;

	return flSmallestAngleTo;
}

bool PResolver::ShouldRun(CTFPlayer* pLocal)
{
	return Vars::AntiHack::Resolver::Resolver.Value && pLocal && pLocal->IsAlive() && !pLocal->IsAGhost() && G::WeaponType == EWeaponType::HITSCAN;
}

bool PResolver::ShouldRunEntity(CTFPlayer* pEntity)
{
	if (!pEntity->OnSolid() && Vars::AntiHack::Resolver::IgnoreAirborne.Value)
		return false;
	if (!pEntity->IsAlive() || pEntity->IsAGhost() || pEntity->IsTaunting())
		return false;

	//if (pEntity->GetSimulationTime() == pEntity->GetOldSimulationTime())
	//	return false; // last networked angles are the same as these, no need to change them
	return true;
}

bool PResolver::KeepOnShot(CTFPlayer* pEntity)
{
	if (abs(I::GlobalVars->tickcount - mResolverData[pEntity].pLastFireAngles.first.first) < 2)
		return true;
	if (mResolverData[pEntity].pLastFireAngles.first.second)
		return true; // this person has not unchoked since shooting
	return false;
}

bool PResolver::IsOnShotPitchReliable(const float flPitch)
{
	if (flPitch > 180)
		return flPitch > 273.f;
	else
		return flPitch < 87.f;
}

float PResolver::GetRealPitch(const float flPitch)
{
	if (flPitch < 157.5f)
		return 89.f;
	else
		return -89.f;
}

void PResolver::SetAngles(const Vec3 vAngles, CTFPlayer* pEntity)
{
	if (auto pAnimState = pEntity->GetAnimState())
	{
		pEntity->m_angEyeAnglesX() = vAngles.x;
		pAnimState->m_flCurrentFeetYaw = vAngles.y;
		pAnimState->m_flGoalFeetYaw = vAngles.y;
		pAnimState->Update(vAngles.y, vAngles.x);
	}
}

int PResolver::GetPitchMode(CTFPlayer* pEntity)
{
	PlayerInfo_t pi{};
	if (!I::EngineClient->GetPlayerInfo(pEntity->entindex(), &pi))
		return 0;
	return mResolverMode[pi.friendsID].first;
}

int PResolver::GetYawMode(CTFPlayer* pEntity)
{
	PlayerInfo_t pi{};
	if (!I::EngineClient->GetPlayerInfo(pEntity->entindex(), &pi))
		return 0;
	return mResolverMode[pi.friendsID].second;
}

void PResolver::OnDormancy(CTFPlayer* pEntity)
{
	mResolverData[pEntity].pLastSniperPitch = {0, 0.f};
	mResolverData[pEntity].flPitchNoise = 0.f;
	mResolverData[pEntity].iPitchNoiseSteps = 0;
	mResolverData[pEntity].pLastFireAngles = {{0, false}, {}};
	mResolverData[pEntity].vOriginalAngles = {};
}

void PResolver::Aimbot(CTFPlayer* pEntity, const bool bHeadshot)
{
	if (abs(I::GlobalVars->tickcount - pWaiting.first) < 66)
		return;

	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return;

	const int iDelay = 6 + TIME_TO_TICKS(G::Lerp + pNetChan->GetLatency(FLOW_INCOMING) + pNetChan->GetLatency(FLOW_OUTGOING));
	pWaiting = {I::GlobalVars->tickcount + iDelay, {pEntity, bHeadshot}};
}

void PResolver::FrameStageNotify(CTFPlayer* pLocal)
{
	if (!ShouldRun(pLocal))
		return;

	UpdateSniperDots();

	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer())
			continue;

		if (pPlayer->IsDormant())
		{
			OnDormancy(pPlayer);
			continue;
		}

		mResolverData[pPlayer].vOriginalAngles = { pPlayer->m_angEyeAnglesX(), pPlayer->m_angEyeAnglesY() };

		if (abs(I::GlobalVars->tickcount - mResolverData[pPlayer].pLastFireAngles.first.first) >= 2)
			mResolverData[pPlayer].pLastFireAngles.first.second = (pPlayer->m_flSimulationTime() == pPlayer->m_flOldSimulationTime()) ? mResolverData[pPlayer].pLastFireAngles.first.second : false;

		if (!ShouldRunEntity(pPlayer))
			continue;
		if (KeepOnShot(pPlayer))
		{
			SetAngles(mResolverData[pPlayer].pLastFireAngles.second, pPlayer);
			continue;
		}

		Vec3 vAdjustedAngle = pPlayer->GetEyeAngles();

		if (std::optional<float> flPitch = GetPitchForSniperDot(pPlayer))
		{
			vAdjustedAngle.x = flPitch.value();

			//	get noise
			if (mResolverData[pPlayer].pLastSniperPitch.first)
			{
				const float flNoise = mResolverData[pPlayer].pLastSniperPitch.second - flPitch.value();
				mResolverData[pPlayer].flPitchNoise *= mResolverData[pPlayer].iPitchNoiseSteps;
				mResolverData[pPlayer].flPitchNoise += flNoise;
				mResolverData[pPlayer].iPitchNoiseSteps++;
				mResolverData[pPlayer].flPitchNoise /= mResolverData[pPlayer].iPitchNoiseSteps;
			}

			mResolverData[pPlayer].pLastSniperPitch = {I::GlobalVars->tickcount, flPitch.value()};
		}
		else if (I::GlobalVars->tickcount - mResolverData[pPlayer].pLastSniperPitch.first < 66 && mResolverData[pPlayer].flPitchNoise < 5.f)
			vAdjustedAngle.x = mResolverData[pPlayer].pLastSniperPitch.second;
		else
		{
			switch (GetPitchMode(pPlayer))
			{
			case 0: break;
			case 1: vAdjustedAngle.x = -89.f; break;				//up
			case 2: vAdjustedAngle.x = 89.f; break;					//down
			case 3: vAdjustedAngle.x = 0.f; break;					//zero
			case 4:													//auto
				if (mResolverData[pPlayer].vOriginalAngles.x >= 90.f)
					vAdjustedAngle.x = -89.f;
				else if (mResolverData[pPlayer].vOriginalAngles.x <= -90.f)
					vAdjustedAngle.x = 89.f;
				break;
			}
		}

		const int iYawMode = GetYawMode(pPlayer);
		if (iYawMode)
		{
			std::optional<float> flTempYaw = PredictBaseYaw(pLocal, pPlayer);
			if (!flTempYaw) { flTempYaw = Math::CalcAngle(pPlayer->m_vecOrigin(), pLocal->m_vecOrigin()).y; }

			const float flBaseYaw = flTempYaw.value();

			switch (iYawMode)
			{
			case 0: break;
			case 1: vAdjustedAngle.y = flBaseYaw; break;			//forward
			case 2: vAdjustedAngle.y = flBaseYaw + 180.f; break;	//backwards
			case 3: vAdjustedAngle.y = flBaseYaw - 90.f; break;		//side1
			case 4: vAdjustedAngle.y = flBaseYaw + 90.f; break;		//side2
			case 5: vAdjustedAngle.y += 180.f; break;				//invert
			case 6:{												//edge
				// TODO: Fix this. Resolver will always correct inversely to the player's pitch if we do not fix the logic here.
				const bool bEdge = vAdjustedAngle.x == 89 ? F::AntiAim.GetEdge(pPlayer, flBaseYaw, false) : !F::AntiAim.GetEdge(pPlayer, flBaseYaw, false);
				vAdjustedAngle.y = flBaseYaw + (bEdge ? 90.f : -90.f);
				break;
			}
			case 7:{												//auto
				vAdjustedAngle.y = vYawRotations[mResolverData[pPlayer].iYawIndex];
				break;
			}
			}
		}

		SetAngles(vAdjustedAngle, pPlayer);
	}
}

void PResolver::CreateMove()
{
	if (I::GlobalVars->tickcount > pWaiting.first && pWaiting.second.first)
	{ 
		mResolverData[pWaiting.second.first].iYawIndex++;
		if (mResolverData[pWaiting.second.first].iYawIndex > 3)
			mResolverData[pWaiting.second.first].iYawIndex = 0;
		pWaiting = {0, {nullptr, false}};
		F::Backtrack.ResolverUpdate(pWaiting.second.first);
	}
}

void PResolver::FXFireBullet(int iIndex, const Vec3 vAngles)
{
	auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
	if (!pEntity)
		return;

	auto pPlayer = pEntity->As<CTFPlayer>();

	Vec3 vAngAdjusted = vAngles;
	Vec3 vAngStore = vAngles;

	if (!IsOnShotPitchReliable(vAngles.x))
	{
		float flAdjustedPitch = vAngles.x;

		vAngStore.x += 360.f;
		vAngStore.x *= -1;

		vAngAdjusted.x = GetRealPitch(flAdjustedPitch);

		if (fabsf(flAdjustedPitch) > 89.f)
			vAngStore.y += 180; // account for likely yaw faking
		while (vAngStore.y > 360)
			vAngStore.y -= 360.f; // hacky fix for previous line
		vAngStore.x += 540; // (360+180)
		SDK::Output("Resolver", std::format("Recieved {:.1f} {:.1f}", vAngles.x, vAngles.y).c_str(), {0, 222, 255, 255});
		SDK::Output("Resolver", std::format("Adjusted {:.1f} {:.1f}", vAngAdjusted.x, vAngAdjusted.y).c_str(), {0, 222, 255, 255});
		SDK::Output("Resolver", std::format("Adjusted 2 {:.1f} {:.1f}", vAngStore.x, vAngStore.y).c_str(), {0, 222, 255, 255});
	}

	mResolverData[pPlayer].pLastFireAngles = { { I::GlobalVars->tickcount, true }, vAngStore};
	SetAngles(vAngAdjusted, pPlayer);
}

void PResolver::OnPlayerHurt(IGameEvent* pEvent)
{
	const bool bLocal = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")) == I::EngineClient->GetLocalPlayer();
	if (!bLocal)
		return;

	auto pVictim = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")));

	if (pVictim == pWaiting.second.first)
	{ 
		if (pWaiting.second.second && G::CanHeadshot)
		{	// should be headshot
			const bool bCrit = pEvent->GetBool("crit");
			if (!bCrit)
				return;
		}
		pWaiting = {0, {nullptr, false}}; 
	}
	return;
}