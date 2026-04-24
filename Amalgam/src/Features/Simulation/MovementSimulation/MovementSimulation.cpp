#include "MovementSimulation.h"

#include "../../EnginePrediction/EnginePrediction.h"
#include <numeric>

void CMovementSimulation::Store(MoveStorage& tMoveStorage)
{
	auto pMap = tMoveStorage.m_pPlayer->GetPredDescMap();
	if (!pMap)
		return;

	size_t iSize = tMoveStorage.m_pPlayer->GetIntermediateDataSize();
	tMoveStorage.m_pData = reinterpret_cast<byte*>(I::MemAlloc->Alloc(iSize));

	CPredictionCopy copy = { PC_NETWORKED_ONLY, tMoveStorage.m_pData, PC_DATA_PACKED, tMoveStorage.m_pPlayer, PC_DATA_NORMAL };
	copy.TransferData("MovementSimulationStore", tMoveStorage.m_pPlayer->entindex(), pMap);
}

void CMovementSimulation::Reset(MoveStorage& tMoveStorage)
{
	if (tMoveStorage.m_pData)
	{
		auto pMap = tMoveStorage.m_pPlayer->GetPredDescMap();
		if (!pMap)
			return;

		CPredictionCopy copy = { PC_NETWORKED_ONLY, tMoveStorage.m_pPlayer, PC_DATA_NORMAL, tMoveStorage.m_pData, PC_DATA_PACKED };
		copy.TransferData("MovementSimulationReset", tMoveStorage.m_pPlayer->entindex(), pMap);

		I::MemAlloc->Free(tMoveStorage.m_pData);
		tMoveStorage.m_pData = nullptr;
	}
}

static inline void HandleMovement(CTFPlayer* pPlayer, MoveData* pLastRecord, MoveData& tCurRecord, std::deque<MoveData>& vRecords)
{
	bool bLocal = pPlayer->entindex() == I::EngineClient->GetLocalPlayer();

	if (pLastRecord)
	{
		/*
		if (tRecord.m_iMode != pLastRecord->m_iMode)
		{
			pLastRecord = nullptr;
			vRecords.clear();
		}
		else */
		{	// does this eat up fps? i can't tell currently
			CGameTrace trace = {};
			CTraceFilterWorldAndPropsOnly filter = {};
			filter.pSkip = pPlayer;

			SDK::TraceHull(pLastRecord->m_vOrigin, pLastRecord->m_vOrigin + pLastRecord->m_vVelocity * TICK_INTERVAL, pPlayer->m_vecMins() + PLAYER_ORIGIN_COMPRESSION, pPlayer->m_vecMaxs() - PLAYER_ORIGIN_COMPRESSION, pPlayer->SolidMask(), &filter, &trace);
			if (trace.DidHit() && trace.plane.normal.z < 0.707f)
			{
				pLastRecord = nullptr;
				vRecords.clear();
			}
		}
	}
	if (!pLastRecord)
		return;

	if (pPlayer->InCond(TF_COND_SHIELD_CHARGE))
	{
		G::DummyCmd.forwardmove = 450.f;
		G::DummyCmd.sidemove = 0.f;
		SDK::FixMovement(&G::DummyCmd, bLocal ? G::CurrentUserCmd->viewangles : pPlayer->GetEyeAngles(), {});
		tCurRecord.m_vDirection.x = G::DummyCmd.forwardmove;
		tCurRecord.m_vDirection.y = -G::DummyCmd.sidemove;
		return;
	}

	switch (tCurRecord.m_iMode)
	{
	case MoveEnum::Ground:
	{
		if (bLocal && Vars::Misc::Movement::Bunnyhop.Value && G::OriginalCmd.buttons & IN_JUMP)
		{
			float flMaxSpeed = SDK::MaxSpeed(pPlayer, true);
			tCurRecord.m_vDirection = tCurRecord.m_vVelocity.Normalized2D() * flMaxSpeed;
		}
		break;
	}
	case MoveEnum::Air:
	{
		float flMaxSpeed = SDK::MaxSpeed(pPlayer, true);
		tCurRecord.m_vDirection = tCurRecord.m_vVelocity.Normalized2D() * flMaxSpeed;
		break;
	}
	case MoveEnum::Swim:
	{
		tCurRecord.m_vDirection *= 2;
	}
	}
}

void CMovementSimulation::Store()
{
	if (I::EngineClient->IsPlayingDemo())
		return;

	for (auto pEntity : H::Entities.GetGroup(EntityEnum::PlayerAll))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		auto& vRecords = m_mRecords[pPlayer->entindex()];
		if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant() || pPlayer->m_vecVelocity().IsZero())
		{
			vRecords.clear();
			continue;
		}
		else if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || !H::Entities.GetDeltaTime(pPlayer->entindex()))
			continue;

		Vec3 vVelocity = pPlayer->m_vecVelocity();
		Vec3 vOrigin = pPlayer->m_vecOrigin();
		Vec3 vDirection = vVelocity.To2D();

		MoveData* pLastRecord = !vRecords.empty() ? &vRecords.front() : nullptr;
		vRecords.emplace_front(
			vDirection,
			pPlayer->m_flSimulationTime(),
			pPlayer->IsSwimming() ? MoveEnum::Swim : pPlayer->IsOnGround() ? MoveEnum::Ground : MoveEnum::Air,
			vVelocity,
			vOrigin
		);
		MoveData& tCurRecord = vRecords.front();
		if (vRecords.size() > 66)
			vRecords.pop_back();

		HandleMovement(pPlayer, pLastRecord, tCurRecord, vRecords);
	}

	for (auto pEntity : H::Entities.GetGroup(EntityEnum::PlayerAll))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		auto& vSimTimes = m_mSimTimes[pPlayer->entindex()];
		if (pEntity->entindex() == I::EngineClient->GetLocalPlayer() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant())
		{
			vSimTimes.clear();
			continue;
		}

		float flDeltaTime = H::Entities.GetDeltaTime(pPlayer->entindex());
		if (!flDeltaTime)
			continue;

		vSimTimes.push_front(flDeltaTime);
		if (vSimTimes.size() > Vars::Aimbot::Projectile::DeltaCount.Value)
			vSimTimes.pop_back();
	}
}

void CMovementSimulation::StorePlayer(CTFPlayer* pPlayer, CMoveData& tMoveData, float flTime)
{
	auto& vRecords = m_mRecords[pPlayer->entindex()];
	if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant() || pPlayer->m_vecVelocity().IsZero())
	{
		vRecords.clear();
		return;
	}

	Vec3 vVelocity = tMoveData.m_vecVelocity;
	Vec3 vOrigin = tMoveData.m_vecAbsOrigin;
	Vec3 vDirection = Math::RotatePoint({ tMoveData.m_flForwardMove, -tMoveData.m_flSideMove, tMoveData.m_flUpMove }, {}, { 0, tMoveData.m_vecViewAngles.y, 0 });

	MoveData* pLastRecord = !vRecords.empty() ? &vRecords.front() : nullptr;
	vRecords.emplace_front(
		vDirection,
		flTime,
		pPlayer->IsSwimming() ? MoveEnum::Swim : pPlayer->IsOnGround() ? MoveEnum::Ground : MoveEnum::Air,
		vVelocity,
		vOrigin
	);
	MoveData& tCurRecord = vRecords.front();
	if (vRecords.size() > 66)
		vRecords.pop_back();

	HandleMovement(pPlayer, pLastRecord, tCurRecord, vRecords);
}



bool CMovementSimulation::Initialize(CBaseEntity* pEntity, MoveStorage& tMoveStorage, bool bHitchance, bool bStrafe)
{
	if (!pEntity || !pEntity->IsPlayer() || !pEntity->As<CTFPlayer>()->IsAlive())
	{
		tMoveStorage.m_bInitFailed = tMoveStorage.m_bFailed = true;
		return false;
	}

	auto pPlayer = pEntity->As<CTFPlayer>();
	tMoveStorage.m_pPlayer = pPlayer;

	// store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;

	// store restore data
	Store(tMoveStorage);

	// the hacks that make it work
	I::MoveHelper->SetHost(pPlayer);
	pPlayer->m_pCurrentCommand() = &G::DummyCmd;

	if (auto pAvgVelocity = H::Entities.GetAvgVelocity(pPlayer->entindex()))
		pPlayer->m_vecVelocity() = *pAvgVelocity; // only use average velocity here

	if (pPlayer->m_bDucked() = pPlayer->IsDucking())
	{
		pPlayer->m_fFlags() &= ~FL_DUCKING; // breaks origin's z if FL_DUCKING is not removed
		pPlayer->m_flDucktime() = 0.f;
		pPlayer->m_flDuckJumpTime() = 0.f;
		pPlayer->m_bDucking() = false;
		pPlayer->m_bInDuckJump() = false;
	}

	if (pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
	{
		pPlayer->m_vecBaseVelocity() = Vec3(); // residual basevelocity causes issues
		if (pPlayer->IsOnGround())
			pPlayer->m_vecVelocity().z = std::min(pPlayer->m_vecVelocity().z, 0.f); // step fix
		else
			pPlayer->m_hGroundEntity() = nullptr; // fix for velocity.z being set to 0 even if in air
	}
	else if (Vars::Misc::Movement::Bunnyhop.Value && G::OriginalCmd.buttons & IN_JUMP)
		tMoveStorage.m_bBunnyHop = true;

	// setup move data
	SetupMoveData(tMoveStorage);

	// calculate strafe if desired
	if (bStrafe)
	{
		if (!StrafePrediction(tMoveStorage, bHitchance))
		{
			tMoveStorage.m_bFailed = true;
			return false;
		}
	}

	tMoveStorage.m_vPath = { tMoveStorage.m_MoveData.m_vecAbsOrigin };
	for (int i = 0; i < H::Entities.GetChoke(pPlayer->entindex()); i++)
		RunTick(tMoveStorage);

	return true;
}

void CMovementSimulation::SetupMoveData(MoveStorage& tMoveStorage)
{
	tMoveStorage.m_MoveData.m_bFirstRunOfFunctions = false;
	tMoveStorage.m_MoveData.m_bGameCodeMovedPlayer = false;
	tMoveStorage.m_MoveData.m_nPlayerHandle = reinterpret_cast<IHandleEntity*>(tMoveStorage.m_pPlayer)->GetRefEHandle();

	tMoveStorage.m_MoveData.m_vecAbsOrigin = tMoveStorage.m_pPlayer->m_vecOrigin();
	tMoveStorage.m_MoveData.m_vecVelocity = tMoveStorage.m_pPlayer->m_vecVelocity();
	tMoveStorage.m_MoveData.m_flMaxSpeed = SDK::MaxSpeed(tMoveStorage.m_pPlayer);
	tMoveStorage.m_MoveData.m_flClientMaxSpeed = tMoveStorage.m_MoveData.m_flMaxSpeed;

	if (!tMoveStorage.m_MoveData.m_vecVelocity.To2D().IsZero())
	{
		int iIndex = tMoveStorage.m_pPlayer->entindex();
		if (iIndex == I::EngineClient->GetLocalPlayer() && G::CurrentUserCmd)
			tMoveStorage.m_MoveData.m_vecViewAngles = G::CurrentUserCmd->viewangles;
		else
		{
			if (!tMoveStorage.m_pPlayer->InCond(TF_COND_SHIELD_CHARGE))
				tMoveStorage.m_MoveData.m_vecViewAngles = { 0.f, Math::VectorAngles(tMoveStorage.m_MoveData.m_vecVelocity).y, 0.f };
			else
				tMoveStorage.m_MoveData.m_vecViewAngles = H::Entities.GetEyeAngles(iIndex);
		}

		const auto& vRecords = m_mRecords[tMoveStorage.m_pPlayer->entindex()];
		if (!vRecords.empty())
		{
			auto& tRecord = vRecords.front();
			if (!tRecord.m_vDirection.IsZero())
			{
				G::DummyCmd.forwardmove = tRecord.m_vDirection.x;
				G::DummyCmd.sidemove = -tRecord.m_vDirection.y;
				G::DummyCmd.upmove = tRecord.m_vDirection.z;
				SDK::FixMovement(&G::DummyCmd, {}, tMoveStorage.m_MoveData.m_vecViewAngles);
				tMoveStorage.m_MoveData.m_flForwardMove = G::DummyCmd.forwardmove;
				tMoveStorage.m_MoveData.m_flSideMove = G::DummyCmd.sidemove;
				tMoveStorage.m_MoveData.m_flUpMove = G::DummyCmd.upmove;
			}
		}
	}

	tMoveStorage.m_MoveData.m_vecAngles = tMoveStorage.m_MoveData.m_vecOldAngles = tMoveStorage.m_MoveData.m_vecViewAngles;
	if (auto pConstraintEntity = tMoveStorage.m_pPlayer->m_hConstraintEntity().Get())
		tMoveStorage.m_MoveData.m_vecConstraintCenter = pConstraintEntity->GetAbsOrigin();
	else
		tMoveStorage.m_MoveData.m_vecConstraintCenter = tMoveStorage.m_pPlayer->m_vecConstraintCenter();
	tMoveStorage.m_MoveData.m_flConstraintRadius = tMoveStorage.m_pPlayer->m_flConstraintRadius();
	tMoveStorage.m_MoveData.m_flConstraintWidth = tMoveStorage.m_pPlayer->m_flConstraintWidth();
	tMoveStorage.m_MoveData.m_flConstraintSpeedFactor = tMoveStorage.m_pPlayer->m_flConstraintSpeedFactor();

	tMoveStorage.m_flPredictedDelta = GetPredictedDelta(tMoveStorage.m_pPlayer);
	tMoveStorage.m_flSimTime = tMoveStorage.m_pPlayer->m_flSimulationTime();
	tMoveStorage.m_flPredictedSimTime = tMoveStorage.m_flSimTime + tMoveStorage.m_flPredictedDelta;
	tMoveStorage.m_vPredictedOrigin = tMoveStorage.m_MoveData.m_vecAbsOrigin;
	tMoveStorage.m_bDirectMove = tMoveStorage.m_pPlayer->IsOnGround() || tMoveStorage.m_pPlayer->IsSwimming();
}

static inline float GetGravity()
{
	static auto sv_gravity = H::ConVars.FindVar("sv_gravity");

	return sv_gravity->GetFloat();
}

static inline float GetFrictionScale(float flVelocityXY, float flTurn, float flVelocityZ, float flMin = 50.f, float flMax = 150.f)
{
	if (0.f >= flVelocityZ || flVelocityZ > 250.f)
		return 1.f;

	static auto sv_airaccelerate = H::ConVars.FindVar("sv_airaccelerate");
	float flScale = std::max(sv_airaccelerate->GetFloat(), 1.f);
	flMin *= flScale, flMax *= flScale;

	// entity friction will be 0.25f if velocity is between 0.f and 250.f
	return Math::RemapVal(fabsf(flVelocityXY * flTurn), flMin, flMax, 1.f, 0.25f);
}

//#define VISUALIZE_RECORDS
#ifdef VISUALIZE_RECORDS
static inline void VisualizeRecords(MoveData& tRecord1, MoveData& tRecord2, Color_t tColor, float flStraightFuzzyValue)
{
	static int iStaticTickcount = I::GlobalVars->tickcount;
	if (I::GlobalVars->tickcount != iStaticTickcount)
	{
		G::LineStorage.clear();
		iStaticTickcount = I::GlobalVars->tickcount;
	}

	const float flYaw1 = Math::VectorAngles(tRecord1.m_vDirection).y, flYaw2 = Math::VectorAngles(tRecord2.m_vDirection).y;
	const float flTime1 = tRecord1.m_flSimTime, flTime2 = tRecord2.m_flSimTime;
	const int iTicks = std::max(TIME_TO_TICKS(flTime1 - flTime2), 1);
	const float flYaw = Math::NormalizeAngle(flYaw1 - flYaw2);
	const bool bStraight = fabsf(flYaw) * tRecord1.m_vVelocity.Length2D() * iTicks < flStraightFuzzyValue; // dumb way to get straight bool

	G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(tRecord1.m_vOrigin, tRecord2.m_vOrigin), I::GlobalVars->curtime + 5.f, tColor);
	G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(tRecord1.m_vOrigin, tRecord1.m_vOrigin + Vec3(0, 0, 5)), I::GlobalVars->curtime + 5.f, tColor);
	if (!bStraight && flYaw)
	{
		Vec3 vVelocity = tRecord1.m_vVelocity.Normalized2D() * 5;
		vVelocity = Math::RotatePoint(vVelocity, {}, { 0, flYaw > 0 ? 90.f : -90.f, 0 });
		if (Vars::Aimbot::Projectile::MovesimFrictionFlags.Value & Vars::Aimbot::Projectile::MovesimFrictionFlagsEnum::CalculateIncrease && tRecord1.m_iMode == MoveEnum::Air)
			vVelocity /= GetFrictionScale(tRecord1.m_vVelocity.Length2D(), flYaw, tRecord1.m_vVelocity.z + GetGravity() * TICK_INTERVAL, 0.f, 56.f);
		G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(tRecord1.m_vOrigin, tRecord1.m_vOrigin + vVelocity), I::GlobalVars->curtime + 5.f, tColor);
	}
}
#endif

static inline bool GetYawDifference(MoveData& tRecord1, MoveData& tRecord2, bool bStart, float* pYaw, float flStraightFuzzyValue, int iMaxChanges = 0, int iMaxChangeTime = 0, float flMaxSpeed = 0.f)
{
	const float flYaw1 = Math::VectorAngles(tRecord1.m_vDirection).y, flYaw2 = Math::VectorAngles(tRecord2.m_vDirection).y;
	const float flTime1 = tRecord1.m_flSimTime, flTime2 = tRecord2.m_flSimTime;
	const int iTicks = std::max(TIME_TO_TICKS(flTime1 - flTime2), 1);

	*pYaw = Math::NormalizeAngle(flYaw1 - flYaw2);
	if (flMaxSpeed && tRecord1.m_iMode != MoveEnum::Air)
		*pYaw *= std::clamp(tRecord1.m_vVelocity.Length2D() / flMaxSpeed, 0.f, 1.f);
	if (Vars::Aimbot::Projectile::MovesimFrictionFlags.Value & Vars::Aimbot::Projectile::MovesimFrictionFlagsEnum::CalculateIncrease && tRecord1.m_iMode == 1)
		*pYaw /= GetFrictionScale(tRecord1.m_vVelocity.Length2D(), *pYaw, tRecord1.m_vVelocity.z + GetGravity() * TICK_INTERVAL, 0.f, 56.f);
	if (fabsf(*pYaw) > 45.f)
		return false;

	static int iChanges, iStart;

	static int iStaticSign = 0;
	const int iLastSign = iStaticSign;
	const int iCurrSign = iStaticSign = *pYaw ? sign(*pYaw) : iStaticSign;

	static bool bStaticZero = false;
	const bool iLastZero = bStaticZero;
	const bool iCurrZero = bStaticZero = !*pYaw;

	const bool bChanged = iCurrSign != iLastSign || iCurrZero && iLastZero;
	const bool bStraight = fabsf(*pYaw) * tRecord1.m_vVelocity.Length2D() * iTicks < flStraightFuzzyValue; // dumb way to get straight bool

	if (bStart)
	{
		iChanges = 0, iStart = TIME_TO_TICKS(flTime1);
		if (bStraight && ++iChanges > iMaxChanges)
			return false;
		return true;
	}
	else
	{
		if ((bChanged || bStraight) && ++iChanges > iMaxChanges)
			return false;
		return iChanges && iStart - TIME_TO_TICKS(flTime2) > iMaxChangeTime ? false : true;
	}
}

void CMovementSimulation::GetAverageYaw(MoveStorage& tMoveStorage, int iSamples)
{
	auto pPlayer = tMoveStorage.m_pPlayer;
	bool bLocal = pPlayer->entindex() == I::EngineClient->GetLocalPlayer();
	auto& vRecords = m_mRecords[pPlayer->entindex()];
	if (vRecords.empty())
		return;

	bool bGround = tMoveStorage.m_bDirectMove; int iMinimumStrafes = 4;
	float flMaxSpeed = SDK::MaxSpeed(tMoveStorage.m_pPlayer, false, true);
	float flLowMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumDistance.Value : Vars::Aimbot::Projectile::AirLowMinimumDistance.Value;
	float flLowMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumSamples.Value : Vars::Aimbot::Projectile::AirLowMinimumSamples.Value;
	float flHighMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumDistance.Value : Vars::Aimbot::Projectile::AirHighMinimumDistance.Value;
	float flHighMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumSamples.Value : Vars::Aimbot::Projectile::AirHighMinimumSamples.Value;

	float flAverageYaw = 0.f; int iTicks = 0, iSkips = 0;
	iSamples = std::min(iSamples, int(vRecords.size()));
	size_t i = 1; for (; i < iSamples; i++)
	{
		auto& tRecord1 = vRecords[i - 1];
		auto& tRecord2 = vRecords[i];
		if (tRecord1.m_iMode != tRecord2.m_iMode)
		{
			iSkips++;
			continue;
		}

		bGround = tRecord1.m_iMode != MoveEnum::Air;
		float flStraightFuzzyValue = bGround ? Vars::Aimbot::Projectile::GroundStraightFuzzyValue.Value : Vars::Aimbot::Projectile::AirStraightFuzzyValue.Value;
		int iMaxChanges = bLocal ? 0 : bGround ? Vars::Aimbot::Projectile::GroundMaxChanges.Value : Vars::Aimbot::Projectile::AirMaxChanges.Value;
		int iMaxChangeTime = bLocal ? 0 : bGround ? Vars::Aimbot::Projectile::GroundMaxChangeTime.Value : Vars::Aimbot::Projectile::AirMaxChangeTime.Value;
		iMinimumStrafes = 4 + iMaxChanges;
#ifdef VISUALIZE_RECORDS
		VisualizeRecords(tRecord1, tRecord2, { 255, 0, 0 }, flStraightFuzzyValue);
#endif

		float flYaw = 0.f;
		bool bResult = GetYawDifference(tRecord1, tRecord2, !iTicks, &flYaw, flStraightFuzzyValue, iMaxChanges, iMaxChangeTime, flMaxSpeed);
		SDK::Output("GetYawDifference", std::format("{} ({}): {}, {}", i, iTicks, flYaw, bResult).c_str(), { 50, 127, 75 }, Vars::Debug::Logging.Value);
		if (!bResult)
			break;

		flAverageYaw += flYaw;
		iTicks += std::max(TIME_TO_TICKS(tRecord1.m_flSimTime - tRecord2.m_flSimTime), 1);
	}
#ifdef VISUALIZE_RECORDS
	size_t i2 = i; for (; i2 < iSamples; i2++)
	{
		auto& tRecord1 = vRecords[i2 - 1];
		auto& tRecord2 = vRecords[i2];

		float flStraightFuzzyValue = bGround ? Vars::Aimbot::Projectile::GroundStraightFuzzyValue.Value : Vars::Aimbot::Projectile::AirStraightFuzzyValue.Value;
		VisualizeRecords(tRecord1, tRecord2, { 0, 0, 0 }, flStraightFuzzyValue);
	}
	/*
	for (; i2 < vRecords.size(); i2++)
	{
		auto& tRecord1 = vRecords[i2 - 1];
		auto& tRecord2 = vRecords[i2];

		float flStraightFuzzyValue = bGround ? Vars::Aimbot::Projectile::GroundStraightFuzzyValue.Value : Vars::Aimbot::Projectile::AirStraightFuzzyValue.Value;
		VisualizeRecords(tRecord1, tRecord2, { 0, 0, 0, 100 }, flStraightFuzzyValue);
	}
	*/
#endif
	if (i <= size_t(iMinimumStrafes + iSkips)) // valid strafes not high enough
		return;

	int iMinimum = flLowMinimumSamples;
	if (!bLocal)
	{
		float flDistance = 0.f;
		if (auto pLocal = H::Entities.GetLocal())
			flDistance = pLocal->m_vecOrigin().DistTo(tMoveStorage.m_pPlayer->m_vecOrigin());
		iMinimum = flDistance < flLowMinimumDistance ? flLowMinimumSamples : Math::RemapVal(flDistance, flLowMinimumDistance, flHighMinimumDistance, flLowMinimumSamples + 1, flHighMinimumSamples);
	}

	flAverageYaw /= std::max(iTicks, iMinimum);
	if (fabsf(flAverageYaw) < 0.36f)
		return;

	tMoveStorage.m_flAverageYaw = flAverageYaw;
	SDK::Output("MovementSimulation", std::format("flAverageYaw calculated to {} from {} ({}){}", flAverageYaw, iTicks, iMinimum, bLocal ? " (local)" : "").c_str(), { 100, 255, 150 }, Vars::Debug::Logging.Value);
}

bool CMovementSimulation::StrafePrediction(MoveStorage& tMoveStorage, bool bHitchance)
{
	if (tMoveStorage.m_bDirectMove
		? !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Ground)
		: !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Air))
		return true;

	const int iStrafeSamples = tMoveStorage.m_bDirectMove
		? Vars::Aimbot::Projectile::GroundSamples.Value
		: Vars::Aimbot::Projectile::AirSamples.Value;

	GetAverageYaw(tMoveStorage, iStrafeSamples);

	// really hope this doesn't work like shit
	if (bHitchance && !tMoveStorage.m_pPlayer->m_vecVelocity().IsZero() && Vars::Aimbot::Projectile::HitChance.Value)
	{
		const auto& vRecords = m_mRecords[tMoveStorage.m_pPlayer->entindex()];
		const auto iSamples = vRecords.size();

		float flCurrentChance = 1.f, flAverageYaw = 0.f;
		for (size_t i = 0; i < iSamples; i++)
		{
			if (vRecords.size() <= i + 2)
				break;

			const auto& pRecord1 = vRecords[i], & pRecord2 = vRecords[i + 1];
			const float flYaw1 = Math::VectorAngles(pRecord1.m_vDirection).y, flYaw2 = Math::VectorAngles(pRecord2.m_vDirection).y;
			const float flTime1 = pRecord1.m_flSimTime, flTime2 = pRecord2.m_flSimTime;
			const int iTicks = std::max(TIME_TO_TICKS(flTime1 - flTime2), 1);

			float flYaw = Math::NormalizeAngle(flYaw1 - flYaw2) / iTicks;
			flAverageYaw += flYaw;
			if (tMoveStorage.m_MoveData.m_flMaxSpeed)
				flYaw *= std::clamp(pRecord1.m_vVelocity.Length2D() / tMoveStorage.m_MoveData.m_flMaxSpeed, 0.f, 1.f);

			if ((i + 1) % iStrafeSamples == 0 || i == iSamples - 1)
			{
				flAverageYaw /= i % iStrafeSamples + 1;
				if (fabsf(tMoveStorage.m_flAverageYaw - flAverageYaw) > 0.5f)
					flCurrentChance -= 1.f / ((iSamples - 1) / float(iStrafeSamples) + 1);
				flAverageYaw = 0.f;
			}
		}

		if (flCurrentChance < Vars::Aimbot::Projectile::HitChance.Value / 100)
		{
			SDK::Output("MovementSimulation", std::format("Hitchance ({}% < {}%)", flCurrentChance * 100, Vars::Aimbot::Projectile::HitChance.Value).c_str(), { 80, 200, 120 }, Vars::Debug::Logging.Value);
			return false;
		}
	}

	return true;
}

bool CMovementSimulation::SetDuck(MoveStorage& tMoveStorage, bool bDuck) // this only touches origin, bounds
{
	if (bDuck == tMoveStorage.m_pPlayer->m_bDucked())
		return true;

	auto pGameRules = I::TFGameRules();
	auto pViewVectors = pGameRules ? pGameRules->GetViewVectors() : nullptr;
	float flScale = tMoveStorage.m_pPlayer->m_flModelScale();

	if (!tMoveStorage.m_pPlayer->IsOnGround())
	{
		Vec3 vHullMins = (pViewVectors ? pViewVectors->m_vHullMin : Vec3(-24, -24, 0)) * flScale;
		Vec3 vHullMaxs = (pViewVectors ? pViewVectors->m_vHullMax : Vec3(24, 24, 82)) * flScale;
		Vec3 vDuckHullMins = (pViewVectors ? pViewVectors->m_vDuckHullMin : Vec3(-24, -24, 0)) * flScale;
		Vec3 vDuckHullMaxs = (pViewVectors ? pViewVectors->m_vDuckHullMax : Vec3(24, 24, 62)) * flScale;

		if (bDuck)
			tMoveStorage.m_MoveData.m_vecAbsOrigin += (vHullMaxs - vHullMins) - (vDuckHullMaxs - vDuckHullMins);
		else
		{
			Vec3 vOrigin = tMoveStorage.m_MoveData.m_vecAbsOrigin - ((vHullMaxs - vHullMins) - (vDuckHullMaxs - vDuckHullMins));

			CGameTrace trace = {};
			CTraceFilterWorldAndPropsOnly filter = {};
			SDK::TraceHull(vOrigin, vOrigin, vHullMins, vHullMaxs, tMoveStorage.m_pPlayer->SolidMask(), &filter, &trace);
			if (trace.DidHit())
				return false;

			tMoveStorage.m_MoveData.m_vecAbsOrigin = vOrigin;
		}
	}
	tMoveStorage.m_pPlayer->m_bDucked() = bDuck;

	return true;
}

void CMovementSimulation::SetBounds(CTFPlayer* pPlayer)
{
	if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer())
		return;

	// fixes issues with origin compression
	if (auto pGameRules = I::TFGameRules())
	{
		if (auto pViewVectors = pGameRules->GetViewVectors())
		{
			pViewVectors->m_vHullMin = Vec3(-24, -24, 0) + PLAYER_ORIGIN_COMPRESSION;
			pViewVectors->m_vHullMax = Vec3(24, 24, 82) - PLAYER_ORIGIN_COMPRESSION;
			pViewVectors->m_vDuckHullMin = Vec3(-24, -24, 0) + PLAYER_ORIGIN_COMPRESSION;
			pViewVectors->m_vDuckHullMax = Vec3(24, 24, 62) - PLAYER_ORIGIN_COMPRESSION;
		}
	}
}

void CMovementSimulation::RestoreBounds(CTFPlayer* pPlayer)
{
	if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer())
		return;

	if (auto pGameRules = I::TFGameRules())
	{
		if (auto pViewVectors = pGameRules->GetViewVectors())
		{
			pViewVectors->m_vHullMin = Vec3(-24, -24, 0);
			pViewVectors->m_vHullMax = Vec3(24, 24, 82);
			pViewVectors->m_vDuckHullMin = Vec3(-24, -24, 0);
			pViewVectors->m_vDuckHullMax = Vec3(24, 24, 62);
		}
	}
}

void CMovementSimulation::RunTick(MoveStorage& tMoveStorage, bool bPath, RunTickCallback* pCallback)
{
	if (tMoveStorage.m_bFailed || !tMoveStorage.m_pPlayer || !tMoveStorage.m_pPlayer->IsPlayer())
		return;

	// make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
	SetBounds(tMoveStorage.m_pPlayer);

	float flCorrection = 0.f;
	if (tMoveStorage.m_flAverageYaw)
	{
		float flMult = 1.f;
		if (!tMoveStorage.m_bDirectMove && !tMoveStorage.m_pPlayer->InCond(TF_COND_SHIELD_CHARGE))
		{
			flCorrection = 90.f * sign(tMoveStorage.m_flAverageYaw);
			if (Vars::Aimbot::Projectile::MovesimFrictionFlags.Value & Vars::Aimbot::Projectile::MovesimFrictionFlagsEnum::RunReduce)
				flMult = GetFrictionScale(tMoveStorage.m_MoveData.m_vecVelocity.Length2D(), tMoveStorage.m_flAverageYaw, tMoveStorage.m_MoveData.m_vecVelocity.z + GetGravity() * TICK_INTERVAL);
		}
		tMoveStorage.m_MoveData.m_vecViewAngles.y += tMoveStorage.m_flAverageYaw * flMult + flCorrection;
	}
	else if (!tMoveStorage.m_bDirectMove)
		tMoveStorage.m_MoveData.m_flForwardMove = tMoveStorage.m_MoveData.m_flSideMove = 0.f;

	float flOldSpeed = tMoveStorage.m_MoveData.m_flClientMaxSpeed;
	if (tMoveStorage.m_pPlayer->m_bDucked() && tMoveStorage.m_pPlayer->IsOnGround() && !tMoveStorage.m_pPlayer->IsSwimming())
		tMoveStorage.m_MoveData.m_flClientMaxSpeed /= 3;

	if (tMoveStorage.m_bBunnyHop && tMoveStorage.m_pPlayer->IsOnGround() && !tMoveStorage.m_pPlayer->m_bDucked())
	{
		tMoveStorage.m_MoveData.m_nOldButtons = 0;
		tMoveStorage.m_MoveData.m_nButtons |= IN_JUMP;
	}

	I::GameMovement->ProcessMovement(tMoveStorage.m_pPlayer, &tMoveStorage.m_MoveData);
	if (pCallback)
		(*pCallback)(tMoveStorage.m_MoveData);

	tMoveStorage.m_MoveData.m_flClientMaxSpeed = flOldSpeed;

	tMoveStorage.m_flSimTime += TICK_INTERVAL;
	tMoveStorage.m_bPredictNetworked = tMoveStorage.m_flSimTime >= tMoveStorage.m_flPredictedSimTime;
	if (tMoveStorage.m_bPredictNetworked)
	{
		tMoveStorage.m_vPredictedOrigin = tMoveStorage.m_MoveData.m_vecAbsOrigin;
		tMoveStorage.m_flPredictedSimTime += tMoveStorage.m_flPredictedDelta;
	}
	bool bLastbDirectMove = tMoveStorage.m_bDirectMove;
	tMoveStorage.m_bDirectMove = tMoveStorage.m_pPlayer->IsOnGround() || tMoveStorage.m_pPlayer->IsSwimming();

	if (tMoveStorage.m_flAverageYaw)
		tMoveStorage.m_MoveData.m_vecViewAngles.y -= flCorrection;
	else if (tMoveStorage.m_bDirectMove && !bLastbDirectMove
		&& !tMoveStorage.m_MoveData.m_flForwardMove && !tMoveStorage.m_MoveData.m_flSideMove
		&& tMoveStorage.m_MoveData.m_vecVelocity.Length2D() > tMoveStorage.m_MoveData.m_flMaxSpeed * 0.015f)
	{
		Vec3 vDirection = tMoveStorage.m_MoveData.m_vecVelocity.Normalized2D() * 450.f;
		G::DummyCmd.forwardmove = vDirection.x, G::DummyCmd.sidemove = -vDirection.y;
		SDK::FixMovement(&G::DummyCmd, {}, tMoveStorage.m_MoveData.m_vecViewAngles);
		tMoveStorage.m_MoveData.m_flForwardMove = G::DummyCmd.forwardmove, tMoveStorage.m_MoveData.m_flSideMove = G::DummyCmd.sidemove;
	}

	RestoreBounds(tMoveStorage.m_pPlayer);

	if (bPath)
		tMoveStorage.m_vPath.push_back(tMoveStorage.m_MoveData.m_vecAbsOrigin);
}

void CMovementSimulation::RunTick(MoveStorage& tMoveStorage, bool bPath, RunTickCallback fCallback)
{
	RunTick(tMoveStorage, bPath, &fCallback);
}

void CMovementSimulation::Restore(MoveStorage& tMoveStorage)
{
	if (tMoveStorage.m_bInitFailed || !tMoveStorage.m_pPlayer)
		return;

	I::MoveHelper->SetHost(nullptr);
	tMoveStorage.m_pPlayer->m_pCurrentCommand() = nullptr;

	Reset(tMoveStorage);

	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
	I::GlobalVars->frametime = m_flOldFrametime;
}

float CMovementSimulation::GetPredictedDelta(CBaseEntity* pEntity)
{
	auto& vSimTimes = m_mSimTimes[pEntity->entindex()];
	if (!vSimTimes.empty())
	{
		switch (Vars::Aimbot::Projectile::DeltaMode.Value)
		{
		case 0: return std::reduce(vSimTimes.begin(), vSimTimes.end()) / vSimTimes.size();
		case 1: return *std::max_element(vSimTimes.begin(), vSimTimes.end());
		}
	}
	return TICK_INTERVAL;
}