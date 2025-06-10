#include "Resolver.h"

#include "../Backtrack/Backtrack.h"
#include "../Players/PlayerUtils.h"
#include "../Ticks/Ticks.h"
#include "../Output/Output.h"

void CResolver::Reset()
{
	m_mResolverData.clear();
	m_mSniperDots.clear();

	m_iWaitingForTarget = -1;
	m_flWaitingForDamage = 0.f;
	m_bWaitingForHeadshot = false;
}

void CResolver::StoreSniperDots(CTFPlayerResource* pResource)
{
	m_mSniperDots.clear();
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::MISC_DOTS))
	{
		if (auto pOwner = pEntity->m_hOwnerEntity().Get())
		{
			int iUserID = pResource->m_iUserID(pOwner->entindex());
			m_mSniperDots[iUserID] = pEntity->m_vecOrigin();
		}
	}
}

std::optional<float> CResolver::GetPitchForSniperDot(CTFPlayer* pEntity, CTFPlayerResource* pResource)
{
	if (m_mSniperDots.contains(pEntity->entindex()))
	{
		int iUserID = pResource->m_iUserID(pEntity->entindex());

		const Vec3 vOrigin = m_mSniperDots[iUserID];
		const Vec3 vEyeOrigin = pEntity->m_vecOrigin() + pEntity->GetViewOffset();
		const Vec3 vDelta = vOrigin - vEyeOrigin;
		Vec3 vAngles; Math::VectorAngles(vDelta, vAngles);
		return vAngles.x;
	}

	return std::nullopt;
}

void CResolver::FrameStageNotify()
{
	if (!Vars::Resolver::Enabled.Value || !I::EngineClient->IsInGame() || I::EngineClient->IsPlayingDemo())
		return;

	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return;

	StoreSniperDots(pResource);

	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			continue;

		int iUserID = pResource->m_iUserID(pPlayer->entindex());
		auto& tData = m_mResolverData[iUserID];

		if (tData.m_flYaw)
			tData.m_bYaw = true;
		else
			tData.m_bYaw = false;

		if (H::Entities.GetDeltaTime(pPlayer->entindex()) && fabsf(pPlayer->m_angEyeAnglesX()) == 90.f)
		{
			if (auto flPitch = GetPitchForSniperDot(pPlayer, pResource))
				tData.m_flPitch = flPitch.value();
			else if (!tData.m_bFirstOOBPitch && tData.m_bAutoSetPitch || tData.m_bInversePitch)
				tData.m_flPitch = -pPlayer->m_angEyeAnglesX(); // set to inverse by default
			tData.m_bPitch = true, tData.m_bFirstOOBPitch = true;
		}
		else
			tData.m_bPitch = false;
	}
}

void CResolver::CreateMove(CTFPlayer* pLocal)
{
	if (!pLocal)
		return;

	if (m_iWaitingForTarget != -1 && m_flWaitingForDamage < TICKS_TO_TIME(pLocal->m_nTickBase()))
	{
		if (auto pTarget = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(m_iWaitingForTarget))->As<CTFPlayer>())
		{
			auto& tData = m_mResolverData[m_iWaitingForTarget];

			tData.m_iMissCount++;

			float& flYaw = tData.m_flYaw;
			float& flPitch = tData.m_flPitch;
			bool bAutoYaw = tData.m_bAutoSetYaw && Vars::Resolver::AutoResolveYawAmount.Value;
			bool bAutoPitch = tData.m_bAutoSetPitch && Vars::Resolver::AutoResolvePitchAmount.Value;

			if (bAutoYaw)
			{
				if (tData.m_iHitCount > 0 && tData.m_iHitCount > tData.m_iMissCount / 3)
				{
					float flTargetYaw = tData.m_flLastSuccessfulYaw;
					float flDiff = Math::NormalizeAngle(flTargetYaw - flYaw);
					
					flYaw = Math::NormalizeAngle(flYaw + (flDiff * 0.5f) + 
						(Vars::Resolver::AutoResolveYawAmount.Value * 0.25f * (SDK::RandomInt(0, 1) ? 1 : -1)));
				}
				else
				{
					flYaw = Math::NormalizeAngle(flYaw + Vars::Resolver::AutoResolveYawAmount.Value);
				}

				tData.m_flLastYawAttempt = flYaw;
				F::Backtrack.ResolverUpdate(pTarget);
				F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(m_iWaitingForTarget), "Adaptive", "yaw", flYaw);
			}

			if (bAutoPitch
				&& (!bAutoYaw || fabsf(flYaw) < fabsf(Vars::Resolver::AutoResolveYawAmount.Value / 2))
				&& fabsf(pTarget->m_angEyeAnglesX()) == 90.f && !m_mSniperDots.contains(m_iWaitingForTarget))
			{
				if (tData.m_iHitCount > 0 && tData.m_iHitCount > tData.m_iMissCount / 3)
				{
					float flTargetPitch = tData.m_flLastSuccessfulPitch;
					float flDiff = Math::NormalizeAngle(flTargetPitch - flPitch, 180.f);
					
					flPitch = Math::NormalizeAngle(flPitch + (flDiff * 0.5f) + 
						(Vars::Resolver::AutoResolvePitchAmount.Value * 0.25f * (SDK::RandomInt(0, 1) ? 1 : -1)), 180.f);
				}
				else
				{
					flPitch = Math::NormalizeAngle(flPitch + Vars::Resolver::AutoResolvePitchAmount.Value, 180.f);
				}

				tData.m_flLastPitchAttempt = flPitch;
				F::Backtrack.ResolverUpdate(pTarget);
				F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(m_iWaitingForTarget), "Adaptive", "pitch", flPitch);
			}

			m_iWaitingForTarget = -1;
			m_flWaitingForDamage = 0.f;
			m_bWaitingForHeadshot = false;
		}
	}

	if (!Vars::Resolver::Enabled.Value)
		return;

	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return;

	auto getPlayerClosestToFOV = [&]()
		{
			CTFPlayer* pClosest = nullptr;
			float flMinFOV = 180.f;

			const Vec3 vLocalPos = F::Ticks.GetShootPos();
			const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

			for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
			{
				if (pEntity->entindex() == pLocal->entindex())
					continue;

				Vec3 vCurPos = pEntity->GetCenter();
				Vec3 vCurAngleTo = Math::CalcAngle(vLocalPos, vCurPos);
				float flCurFOVTo = Math::CalcFov(vLocalAngles, vCurAngleTo);

				if (flCurFOVTo < flMinFOV)
				{
					pClosest = pEntity->As<CTFPlayer>();
					flMinFOV = flCurFOVTo;
				}
			}

			return pClosest;
		};

	if (Vars::Resolver::CycleYaw.Value)
	{
		if (SDK::PlatFloatTime() > m_flLastYawCycle + 0.5f)
		{
			if (auto pTarget = getPlayerClosestToFOV())
			{
				int iUserID = pResource->m_iUserID(pTarget->entindex());
				auto& tData = m_mResolverData[iUserID];

				float& flYaw = tData.m_flYaw;
				flYaw = Math::NormalizeAngle(flYaw + Vars::Resolver::CycleYaw.Value);

				F::Backtrack.ResolverUpdate(pTarget);
				F::Output.ReportResolver(pTarget->entindex(), "Cycling", "yaw", flYaw);
			}

			m_flLastYawCycle = SDK::PlatFloatTime();
		}
	}
	else
		m_flLastYawCycle = 0.f;

	if (Vars::Resolver::CyclePitch.Value)
	{
		if (SDK::PlatFloatTime() > m_flLastPitchCycle + 0.5f)
		{
			if (auto pTarget = getPlayerClosestToFOV())
			{
				int iUserID = pResource->m_iUserID(pTarget->entindex());
				auto& tData = m_mResolverData[iUserID];

				if (fabsf(pTarget->m_angEyeAnglesX()) != 90.f)
					F::Output.ReportResolver("Target not using out of bounds pitch");

				float& flPitch = tData.m_flPitch;

				flPitch += Vars::Resolver::CyclePitch.Value;
				flPitch = Math::NormalizeAngle(flPitch + Vars::Resolver::CyclePitch.Value, 180.f);

				F::Backtrack.ResolverUpdate(pTarget);
				F::Output.ReportResolver(pTarget->entindex(), "Cycling", "pitch", flPitch);
			}

			m_flLastPitchCycle = SDK::PlatFloatTime();
		}
	}
	else
		m_flLastPitchCycle = 0.f;

	if (Vars::Resolver::CycleMinwalk.Value)
	{
		if (SDK::PlatFloatTime() > m_flLastMinwalkCycle + 0.5f)
		{
			if (auto pTarget = getPlayerClosestToFOV())
			{
				int iUserID = pResource->m_iUserID(pTarget->entindex());
				auto& tData = m_mResolverData[iUserID];

				bool& bMinwalk = tData.m_bMinwalk;
				bMinwalk = !bMinwalk;

				F::Backtrack.ResolverUpdate(pTarget);
				F::Output.ReportResolver(pTarget->entindex(), "Cycling", "minwalk", bMinwalk);
			}

			m_flLastMinwalkCycle = SDK::PlatFloatTime();
		}
	}
	else
		m_flLastMinwalkCycle = 0.f;

	if (Vars::Resolver::CycleView.Value)
	{
		if (SDK::PlatFloatTime() > m_flLastViewCycle + 0.5f)
		{
			if (auto pTarget = getPlayerClosestToFOV())
			{
				int iUserID = pResource->m_iUserID(pTarget->entindex());
				auto& tData = m_mResolverData[iUserID];

				bool& bView = tData.m_bView;
				bView = !bView;

				F::Backtrack.ResolverUpdate(pTarget);
				F::Output.ReportResolver(pTarget->entindex(), "Cycling", "view", std::string(bView ? "view to local" : "static"));
			}

			m_flLastViewCycle = SDK::PlatFloatTime();
		}
	}
	else
		m_flLastViewCycle = 0.f;
}

void CResolver::HitscanRan(CTFPlayer* pLocal, CTFPlayer* pTarget, CTFWeaponBase* pWeapon, int iHitbox)
{
	if (!Vars::Resolver::Enabled.Value || !Vars::Resolver::AutoResolve.Value
		|| Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Smooth || pLocal->m_iTeamNum() == pTarget->m_iTeamNum())
		return;

	if (Vars::Resolver::AutoResolveCheatersOnly.Value && !F::PlayerUtils.HasTag(pTarget->entindex(), CHEATER_TAG))
		return;

	if (Vars::Resolver::AutoResolveHeadshotOnly.Value && (iHitbox != HITBOX_HEAD || !G::CanHeadshot))
		return;

	if (pWeapon->GetWeaponSpread())
	{
		// check for perfect bullet
		const float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();
		if (flTimeSinceLastShot <= (pWeapon->GetBulletsPerShot() > 1 ? 0.25f : 1.25f))
			return;
	}

	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return;

	m_iWaitingForTarget = pResource->m_iUserID(pTarget->entindex());
	m_flWaitingForDamage = I::GlobalVars->curtime + F::Backtrack.GetReal(MAX_FLOWS, false) * 1.5f + 0.1f;
	if (iHitbox == HITBOX_HEAD && G::CanHeadshot)
	{
		// not dealing with ambassador's range check right now
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
			m_bWaitingForHeadshot = true;
		}
	}
}

void CResolver::PlayerHurt(IGameEvent* pEvent)
{
	if (m_iWaitingForTarget == -1
		|| I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")) != I::EngineClient->GetLocalPlayer()
		|| pEvent->GetInt("userid") != m_iWaitingForTarget)
		return;

	if (m_bWaitingForHeadshot && !pEvent->GetBool("crit"))
		return;

	auto& tData = m_mResolverData[m_iWaitingForTarget];
	tData.m_iHitCount++;
	
	if (tData.m_bYaw)
		tData.m_flLastSuccessfulYaw = tData.m_flYaw;
		
	if (tData.m_bPitch)
		tData.m_flLastSuccessfulPitch = tData.m_flPitch;

	m_iWaitingForTarget = -1;
	m_flWaitingForDamage = 0.f;
	m_bWaitingForHeadshot = false;
}

void CResolver::SetYaw(int iUserID, float flValue, bool bAuto)
{
	auto& tData = m_mResolverData[iUserID];

	if (bAuto)
	{
		tData.m_bAutoSetYaw = true;

		F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(iUserID), "Set", "yaw", "auto");
	}
	else
	{
		tData.m_flYaw = flValue;
		tData.m_bAutoSetYaw = false;

		F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(iUserID), "Set", "yaw", flValue);
	}

	F::Backtrack.ResolverUpdate(I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(iUserID))->As<CTFPlayer>());
}

void CResolver::SetPitch(int iUserID, float flValue, bool bInverse, bool bAuto)
{
	auto& tData = m_mResolverData[iUserID];

	if (bAuto)
	{
		tData.m_bInversePitch = false;
		tData.m_bAutoSetPitch = true;

		F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(iUserID), "Set", "pitch", "auto");
	}
	else if (bInverse)
	{
		tData.m_bInversePitch = true;
		tData.m_bAutoSetPitch = false;

		F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(iUserID), "Set", "pitch", "inverse");
	}
	else
	{
		auto pPlayer = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(iUserID))->As<CTFPlayer>();
		if (pPlayer && fabsf(pPlayer->m_angEyeAnglesX()) != 90.f)
			F::Output.ReportResolver("Target not using out of bounds pitch");

		tData.m_flPitch = flValue;
		tData.m_bInversePitch = false;
		tData.m_bAutoSetPitch = false;

		F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(iUserID), "Set", "pitch", flValue);
	}

	F::Backtrack.ResolverUpdate(I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(iUserID))->As<CTFPlayer>());
}

void CResolver::SetMinwalk(int iUserID, bool bValue)
{
	auto& tData = m_mResolverData[iUserID];

	tData.m_bMinwalk = bValue;

	F::Backtrack.ResolverUpdate(I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(iUserID))->As<CTFPlayer>());
	F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(iUserID), "Set", "minwalk", bValue);
}

void CResolver::SetView(int iUserID, bool bValue)
{
	auto& tData = m_mResolverData[iUserID];

	tData.m_bView = bValue;

	F::Backtrack.ResolverUpdate(I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(iUserID))->As<CTFPlayer>());
	F::Output.ReportResolver(I::EngineClient->GetPlayerForUserID(iUserID), "Set", "view", std::string(bValue ? "view to local" : "static"));
}

bool CResolver::GetAngles(CTFPlayer* pPlayer, float* pYaw, float* pPitch, bool* pMinwalk, bool bFake)
{
	if (!Vars::Resolver::Enabled.Value)
		return false;

	if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
		return false;

	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return false;

	int iUserID = pResource->m_iUserID(pPlayer->entindex());
	auto& tData = m_mResolverData[iUserID];

	bool bYaw = tData.m_bYaw, bPitch = tData.m_bPitch;
	
	if (pYaw)
	{
		*pYaw = pPlayer->m_angEyeAnglesY();
		if (bYaw && !bFake)
		{
			auto pLocal = H::Entities.GetLocal();
			if (pLocal && tData.m_bView)
				*pYaw = Math::CalcAngle(pPlayer->m_vecOrigin(), pLocal->m_vecOrigin()).y;
			*pYaw += tData.m_flYaw;
		}
	}

	if (pPitch)
	{
		*pPitch = pPlayer->m_angEyeAnglesX();
		if (bPitch)
			*pPitch = tData.m_flPitch;
	}

	if (pMinwalk)
		*pMinwalk = tData.m_bMinwalk;

	return bYaw || bPitch;
}