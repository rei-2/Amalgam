#include "MovementSimulation.h"

#include "../../EnginePrediction/EnginePrediction.h"
#include <numeric>

//we'll use this to set current player's command, without it CGameMovement::CheckInterval will try to access a nullptr
static CUserCmd DummyCmd = {};

void CMovementSimulation::Store(PlayerStorage& playerStorage)
{
	playerStorage.m_PlayerData.m_vecOrigin = playerStorage.m_pPlayer->m_vecOrigin();
	playerStorage.m_PlayerData.m_vecVelocity = playerStorage.m_pPlayer->m_vecVelocity();
	playerStorage.m_PlayerData.m_vecBaseVelocity = playerStorage.m_pPlayer->m_vecBaseVelocity();
	playerStorage.m_PlayerData.m_vecViewOffset = playerStorage.m_pPlayer->m_vecViewOffset();
	playerStorage.m_PlayerData.m_hGroundEntity = playerStorage.m_pPlayer->m_hGroundEntity();
	playerStorage.m_PlayerData.m_fFlags = playerStorage.m_pPlayer->m_fFlags();
	playerStorage.m_PlayerData.m_flDucktime = playerStorage.m_pPlayer->m_flDucktime();
	playerStorage.m_PlayerData.m_flDuckJumpTime = playerStorage.m_pPlayer->m_flDuckJumpTime();
	playerStorage.m_PlayerData.m_bDucked = playerStorage.m_pPlayer->m_bDucked();
	playerStorage.m_PlayerData.m_bDucking = playerStorage.m_pPlayer->m_bDucking();
	playerStorage.m_PlayerData.m_bInDuckJump = playerStorage.m_pPlayer->m_bInDuckJump();
	playerStorage.m_PlayerData.m_flModelScale = playerStorage.m_pPlayer->m_flModelScale();
	playerStorage.m_PlayerData.m_nButtons = playerStorage.m_pPlayer->m_nButtons();
	playerStorage.m_PlayerData.m_flLastMovementStunChange = playerStorage.m_pPlayer->m_flLastMovementStunChange();
	playerStorage.m_PlayerData.m_flStunLerpTarget = playerStorage.m_pPlayer->m_flStunLerpTarget();
	playerStorage.m_PlayerData.m_bStunNeedsFadeOut = playerStorage.m_pPlayer->m_bStunNeedsFadeOut();
	playerStorage.m_PlayerData.m_flPrevTauntYaw = playerStorage.m_pPlayer->m_flPrevTauntYaw();
	playerStorage.m_PlayerData.m_flTauntYaw = playerStorage.m_pPlayer->m_flTauntYaw();
	playerStorage.m_PlayerData.m_flCurrentTauntMoveSpeed = playerStorage.m_pPlayer->m_flCurrentTauntMoveSpeed();
	playerStorage.m_PlayerData.m_iKartState = playerStorage.m_pPlayer->m_iKartState();
	playerStorage.m_PlayerData.m_flVehicleReverseTime = playerStorage.m_pPlayer->m_flVehicleReverseTime();
	playerStorage.m_PlayerData.m_flHypeMeter = playerStorage.m_pPlayer->m_flHypeMeter();
	playerStorage.m_PlayerData.m_flMaxspeed = playerStorage.m_pPlayer->m_flMaxspeed();
	playerStorage.m_PlayerData.m_nAirDucked = playerStorage.m_pPlayer->m_nAirDucked();
	playerStorage.m_PlayerData.m_bJumping = playerStorage.m_pPlayer->m_bJumping();
	playerStorage.m_PlayerData.m_iAirDash = playerStorage.m_pPlayer->m_iAirDash();
	playerStorage.m_PlayerData.m_flWaterJumpTime = playerStorage.m_pPlayer->m_flWaterJumpTime();
	playerStorage.m_PlayerData.m_flSwimSoundTime = playerStorage.m_pPlayer->m_flSwimSoundTime();
	playerStorage.m_PlayerData.m_surfaceProps = playerStorage.m_pPlayer->m_surfaceProps();
	playerStorage.m_PlayerData.m_pSurfaceData = playerStorage.m_pPlayer->m_pSurfaceData();
	playerStorage.m_PlayerData.m_surfaceFriction = playerStorage.m_pPlayer->m_surfaceFriction();
	playerStorage.m_PlayerData.m_chTextureType = playerStorage.m_pPlayer->m_chTextureType();
	playerStorage.m_PlayerData.m_vecPunchAngle = playerStorage.m_pPlayer->m_vecPunchAngle();
	playerStorage.m_PlayerData.m_vecPunchAngleVel = playerStorage.m_pPlayer->m_vecPunchAngleVel();
	playerStorage.m_PlayerData.m_MoveType = playerStorage.m_pPlayer->m_MoveType();
	playerStorage.m_PlayerData.m_MoveCollide = playerStorage.m_pPlayer->m_MoveCollide();
	playerStorage.m_PlayerData.m_vecLadderNormal = playerStorage.m_pPlayer->m_vecLadderNormal();
	playerStorage.m_PlayerData.m_flGravity = playerStorage.m_pPlayer->m_flGravity();
	playerStorage.m_PlayerData.m_nWaterLevel = playerStorage.m_pPlayer->m_nWaterLevel();
	playerStorage.m_PlayerData.m_nWaterType = playerStorage.m_pPlayer->m_nWaterType();
	playerStorage.m_PlayerData.m_flFallVelocity = playerStorage.m_pPlayer->m_flFallVelocity();
	playerStorage.m_PlayerData.m_nPlayerCond = playerStorage.m_pPlayer->m_nPlayerCond();
	playerStorage.m_PlayerData.m_nPlayerCondEx = playerStorage.m_pPlayer->m_nPlayerCondEx();
	playerStorage.m_PlayerData.m_nPlayerCondEx2 = playerStorage.m_pPlayer->m_nPlayerCondEx2();
	playerStorage.m_PlayerData.m_nPlayerCondEx3 = playerStorage.m_pPlayer->m_nPlayerCondEx3();
	playerStorage.m_PlayerData.m_nPlayerCondEx4 = playerStorage.m_pPlayer->m_nPlayerCondEx4();
	playerStorage.m_PlayerData._condition_bits = playerStorage.m_pPlayer->_condition_bits();
}

void CMovementSimulation::Reset(PlayerStorage& playerStorage)
{
	if (auto pGameRules = I::TFGameRules->Get())
	{
		if (auto pViewVectors = pGameRules->GetViewVectors())
		{
			pViewVectors->m_vHullMin = Vec3(-24, -24, 0);
			pViewVectors->m_vHullMax = Vec3(24, 24, 82);
			pViewVectors->m_vDuckHullMin = Vec3(-24, -24, 0);
			pViewVectors->m_vDuckHullMax = Vec3(24, 24, 62);
		}
	}

	playerStorage.m_pPlayer->m_vecOrigin() = playerStorage.m_PlayerData.m_vecOrigin;
	playerStorage.m_pPlayer->m_vecVelocity() = playerStorage.m_PlayerData.m_vecVelocity;
	playerStorage.m_pPlayer->m_vecBaseVelocity() = playerStorage.m_PlayerData.m_vecBaseVelocity;
	playerStorage.m_pPlayer->m_vecViewOffset() = playerStorage.m_PlayerData.m_vecViewOffset;
	playerStorage.m_pPlayer->m_hGroundEntity() = playerStorage.m_PlayerData.m_hGroundEntity;
	playerStorage.m_pPlayer->m_fFlags() = playerStorage.m_PlayerData.m_fFlags;
	playerStorage.m_pPlayer->m_flDucktime() = playerStorage.m_PlayerData.m_flDucktime;
	playerStorage.m_pPlayer->m_flDuckJumpTime() = playerStorage.m_PlayerData.m_flDuckJumpTime;
	playerStorage.m_pPlayer->m_bDucked() = playerStorage.m_PlayerData.m_bDucked;
	playerStorage.m_pPlayer->m_bDucking() = playerStorage.m_PlayerData.m_bDucking;
	playerStorage.m_pPlayer->m_bInDuckJump() = playerStorage.m_PlayerData.m_bInDuckJump;
	playerStorage.m_pPlayer->m_flModelScale() = playerStorage.m_PlayerData.m_flModelScale;
	playerStorage.m_pPlayer->m_nButtons() = playerStorage.m_PlayerData.m_nButtons;
	playerStorage.m_pPlayer->m_flLastMovementStunChange() = playerStorage.m_PlayerData.m_flLastMovementStunChange;
	playerStorage.m_pPlayer->m_flStunLerpTarget() = playerStorage.m_PlayerData.m_flStunLerpTarget;
	playerStorage.m_pPlayer->m_bStunNeedsFadeOut() = playerStorage.m_PlayerData.m_bStunNeedsFadeOut;
	playerStorage.m_pPlayer->m_flPrevTauntYaw() = playerStorage.m_PlayerData.m_flPrevTauntYaw;
	playerStorage.m_pPlayer->m_flTauntYaw() = playerStorage.m_PlayerData.m_flTauntYaw;
	playerStorage.m_pPlayer->m_flCurrentTauntMoveSpeed() = playerStorage.m_PlayerData.m_flCurrentTauntMoveSpeed;
	playerStorage.m_pPlayer->m_iKartState() = playerStorage.m_PlayerData.m_iKartState;
	playerStorage.m_pPlayer->m_flVehicleReverseTime() = playerStorage.m_PlayerData.m_flVehicleReverseTime;
	playerStorage.m_pPlayer->m_flHypeMeter() = playerStorage.m_PlayerData.m_flHypeMeter;
	playerStorage.m_pPlayer->m_flMaxspeed() = playerStorage.m_PlayerData.m_flMaxspeed;
	playerStorage.m_pPlayer->m_nAirDucked() = playerStorage.m_PlayerData.m_nAirDucked;
	playerStorage.m_pPlayer->m_bJumping() = playerStorage.m_PlayerData.m_bJumping;
	playerStorage.m_pPlayer->m_iAirDash() = playerStorage.m_PlayerData.m_iAirDash;
	playerStorage.m_pPlayer->m_flWaterJumpTime() = playerStorage.m_PlayerData.m_flWaterJumpTime;
	playerStorage.m_pPlayer->m_flSwimSoundTime() = playerStorage.m_PlayerData.m_flSwimSoundTime;
	playerStorage.m_pPlayer->m_surfaceProps() = playerStorage.m_PlayerData.m_surfaceProps;
	playerStorage.m_pPlayer->m_pSurfaceData() = playerStorage.m_PlayerData.m_pSurfaceData;
	playerStorage.m_pPlayer->m_surfaceFriction() = playerStorage.m_PlayerData.m_surfaceFriction;
	playerStorage.m_pPlayer->m_chTextureType() = playerStorage.m_PlayerData.m_chTextureType;
	playerStorage.m_pPlayer->m_vecPunchAngle() = playerStorage.m_PlayerData.m_vecPunchAngle;
	playerStorage.m_pPlayer->m_vecPunchAngleVel() = playerStorage.m_PlayerData.m_vecPunchAngleVel;
	playerStorage.m_pPlayer->m_MoveType() = playerStorage.m_PlayerData.m_MoveType;
	playerStorage.m_pPlayer->m_MoveCollide() = playerStorage.m_PlayerData.m_MoveCollide;
	playerStorage.m_pPlayer->m_vecLadderNormal() = playerStorage.m_PlayerData.m_vecLadderNormal;
	playerStorage.m_pPlayer->m_flGravity() = playerStorage.m_PlayerData.m_flGravity;
	playerStorage.m_pPlayer->m_nWaterLevel() = playerStorage.m_PlayerData.m_nWaterLevel;
	playerStorage.m_pPlayer->m_nWaterType() = playerStorage.m_PlayerData.m_nWaterType;
	playerStorage.m_pPlayer->m_flFallVelocity() = playerStorage.m_PlayerData.m_flFallVelocity;
	playerStorage.m_pPlayer->m_nPlayerCond() = playerStorage.m_PlayerData.m_nPlayerCond;
	playerStorage.m_pPlayer->m_nPlayerCondEx() = playerStorage.m_PlayerData.m_nPlayerCondEx;
	playerStorage.m_pPlayer->m_nPlayerCondEx2() = playerStorage.m_PlayerData.m_nPlayerCondEx2;
	playerStorage.m_pPlayer->m_nPlayerCondEx3() = playerStorage.m_PlayerData.m_nPlayerCondEx3;
	playerStorage.m_pPlayer->m_nPlayerCondEx4() = playerStorage.m_PlayerData.m_nPlayerCondEx4;
	playerStorage.m_pPlayer->_condition_bits() = playerStorage.m_PlayerData._condition_bits;
}


static inline float GetPlayerGravity(CTFPlayer* pEntity)
{
	static auto sv_gravity = U::ConVars.FindVar("sv_gravity");

	float flGravity = pEntity->m_flGravity() ? pEntity->m_flGravity() : 1.f;
	if (auto pGameRules = I::TFGameRules->Get())
		return sv_gravity->GetFloat() * pGameRules->m_flGravityMultiplier() * flGravity;
	return sv_gravity->GetFloat() * flGravity;
}

void CMovementSimulation::Store()
{
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		auto& vRecords = mRecords[pPlayer->entindex()];

		if (pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->m_vecVelocity().IsZero())
		{
			vRecords.clear();
			continue;
		}
		else if (!H::Entities.GetDeltaTime(pPlayer->entindex()))
			continue;

		bool bLocal = pPlayer->entindex() == I::EngineClient->GetLocalPlayer() && !I::EngineClient->IsPlayingDemo();
		Vec3 vVelocity = bLocal ? F::EnginePrediction.m_vVelocity : pPlayer->m_vecVelocity();
		Vec3 vOrigin = bLocal ? F::EnginePrediction.m_vOrigin : pPlayer->m_vecOrigin();
		Vec3 vDirection = bLocal ? Math::RotatePoint(F::EnginePrediction.m_vDirection, {}, { 0, F::EnginePrediction.m_vAngles.y, 0 }) : Vec3(vVelocity.x, vVelocity.y, 0.f);
		Vec3 vAngles = bLocal ? F::EnginePrediction.m_vAngles : pPlayer->GetEyeAngles();

		MoveData tRecord = {
			vDirection,
			vAngles,
			pPlayer->m_flSimulationTime(),
			pPlayer->IsOnGround() ? 0 : pPlayer->m_nWaterLevel() < 2 ? 1 : 2,
			vVelocity,
			vOrigin
		};
		MoveData* pRecord = !vRecords.empty() ? &vRecords.front() : nullptr;
			
		if (pRecord)
		{
			/*
			if (tRecord.m_iMode != pRecord->m_iMode)
			{
				pRecord = nullptr;
				vRecords.clear();
			}
			else // does this eat up fps? i can't tell currently
			*/
			{
				CGameTrace trace = {};
				CTraceFilterWorldAndPropsOnly filter = {};
				SDK::TraceHull(pRecord->m_vOrigin, pRecord->m_vOrigin + pRecord->m_vVelocity * TICK_INTERVAL, pPlayer->m_vecMins() + 0.125f, pPlayer->m_vecMaxs() - 0.125f, MASK_PLAYERSOLID, &filter, &trace);
				if (trace.DidHit() && trace.plane.normal.z < 0.707f)
				{
					pRecord = nullptr;
					vRecords.clear();
				}
			}
		}

		{
			/*
			bool bSet = false; // debug

			static auto sv_accelerate = U::ConVars.FindVar("sv_accelerate");
			static auto sv_friction = U::ConVars.FindVar("sv_friction");
			static auto sv_stopspeed = U::ConVars.FindVar("sv_stopspeed");

			float flAccel = sv_accelerate->GetFloat();
			float flFriction = sv_friction->GetFloat();
			float flStopSpeed = sv_stopspeed->GetFloat();

			float flMaxSpeed = SDK::MaxSpeed(pPlayer);
			float flEntFriction = 1.f;
			*/

			float flMaxSpeed = SDK::MaxSpeed(pPlayer);

			//if (bLocal)
			{
				if (tRecord.m_iMode != 0 || bLocal && Vars::Misc::Movement::Bunnyhop.Value && G::Buttons & IN_JUMP && !(F::EnginePrediction.m_iFlags & FL_DUCKING))
					tRecord.m_vDirection = Vec3(vVelocity.x, vVelocity.y, 0).Normalized() * flMaxSpeed;
			}
			//else
			{
				// proof of concept that didn't really work out (most notably due to the origin tolerance)
				// there is probably a way to implement ground movement in a way that's beneficial, but i didn't care enough to fiddle around with it

				/*
				switch (pRecord ? tRecord.m_iMode : 3)
				{
				case 0: // walkmove
				{
					Vec3 vOldVelocity = pRecord->m_vVelocity, vNewVelocity = tRecord.m_vVelocity;
					vOldVelocity.z = vNewVelocity.z = 0;
					vOldVelocity /= std::max(vOldVelocity.Length2D() / flMaxSpeed, 1.f);
					vNewVelocity /= std::max(vNewVelocity.Length2D() / flMaxSpeed, 1.f);

					// friction
					float flSpeed = vOldVelocity.Length2D();
					if (flSpeed >= 0.1f)
					{
						float flControl = std::max(flStopSpeed, flSpeed);
						float flDrop = flControl * flFriction * flEntFriction * TICK_INTERVAL;
						float flNewSpeed = std::max(flSpeed - flDrop, 0.f);
						if (flNewSpeed != flSpeed)
						{
							flNewSpeed /= flSpeed;
							vOldVelocity *= flNewSpeed;
						}
					}

					Vec3 vWishDir = vNewVelocity - vOldVelocity;
					float flAccelSpeed = vWishDir.Normalize();
					float flMaxAccel = flAccel * flMaxSpeed * flEntFriction * TICK_INTERVAL;
					if (flAccelSpeed < flMaxAccel * 0.999f && vNewVelocity.Length2D() > flMaxSpeed * 0.999f)
					{
						//SDK::Output("Quadratic", std::format("{} < {} && {} > {}", flAccelSpeed, flMaxAccel * 0.999f, vNewVelocity.Length2D(), flMaxSpeed * 0.999f).c_str());
						float flMult;
						{
							// quadratic
							float flA = powf(vNewVelocity.x, 2.f) + powf(vNewVelocity.y, 2.f);
							float flB = -2 * vNewVelocity.x * vOldVelocity.x - 2 * vNewVelocity.y * vOldVelocity.y;
							float flC = powf(vOldVelocity.x, 2.f) + powf(vOldVelocity.y, 2.f) - powf(flMaxAccel, 2.f);
							flMult = (-flB + sqrt(fabsf(powf(flB, 2.f) - 4 * flA * flC))) / (2 * flA);
						}
						vNewVelocity *= flMult;

						vWishDir = vNewVelocity - vOldVelocity;
						flAccelSpeed = vWishDir.Normalize();

						float flCurSpeed = vOldVelocity.Dot(vWishDir);
						float flAccelSpeed2 = std::max(flMaxSpeed - flCurSpeed, 0.f);
						if (flAccelSpeed2 < flMaxAccel)
						{
							//SDK::Output("Quartic", std::format("{} < {}", flAccelSpeed2, flMaxAccel).c_str());
							vNewVelocity = tRecord.m_vVelocity;
							vNewVelocity.z = 0;
							vNewVelocity /= std::max(vNewVelocity.Length2D() / flMaxSpeed, 1.f);
							{
								// quartic
								float flA = pow((powf(vNewVelocity.x, 2.f) + powf(vNewVelocity.y, 2.f)) / flMaxSpeed, 2);
								float flB = (2 * (powf(vNewVelocity.x, 2.f) + powf(vNewVelocity.y, 2.f)) * (-vOldVelocity.x * vNewVelocity.x - vOldVelocity.y * vNewVelocity.y)) / pow(flMaxSpeed, 2);
								float flC = pow((-vOldVelocity.x * vNewVelocity.x - vOldVelocity.y * vNewVelocity.y) / flMaxSpeed, 2) - powf(vNewVelocity.x, 2.f) - powf(vNewVelocity.y, 2.f);
								float flD = 2 * vNewVelocity.x * vOldVelocity.x + 2 * vNewVelocity.y * vOldVelocity.y;
								float flE = -powf(vOldVelocity.x, 2.f) - powf(vOldVelocity.y, 2.f);
								//SDK::Output("Values", std::format("\na={}\nb={}\nc={}\nd={}\ne={}", flA, flB, flC, flD, flE).c_str());
								auto vRoots = Math::SolveQuartic(flA, flB, flC, flD, flE);
								if (vRoots.size())
									flMult = std::clamp(*std::max_element(vRoots.begin(), vRoots.end()), 1.f, flMult);
							}
							vNewVelocity *= flMult;

							vWishDir = vNewVelocity - vOldVelocity;
							flAccelSpeed = vWishDir.Normalize();
						}
					}

					SDK::Output("flAccelSpeed", std::format("{}", flAccelSpeed).c_str(), {}, Vars::Debug::Info.Value);
					flAccelSpeed = flAccelSpeed < (flAccel * flMaxSpeed * flEntFriction * TICK_INTERVAL) * 0.35f ? 0.f : flMaxSpeed; // this might be a bad solution?

					if (!tRecord.m_vDirection.IsZero())
					{
						bSet = true;

						if (Vars::Debug::Info.Value)
						{
							G::LineStorage.clear();
							G::LineStorage.push_back({ { tRecord.m_vOrigin, tRecord.m_vOrigin + tRecord.m_vDirection }, I::GlobalVars->curtime + 0.1f, { 255, 0, 0, 255 } });
							G::LineStorage.push_back({ { tRecord.m_vOrigin, tRecord.m_vOrigin + vWishDir * flAccelSpeed }, I::GlobalVars->curtime + 0.1f, { 0, 255, 0, 255 } });
						}

						float flAng1 = Math::VelocityToAngles(tRecord.m_vDirection).y, flAng2 = Math::VelocityToAngles(vWishDir).y;
						float flDiff = flAng1 - flAng2;
						flDiff = fmodf(flDiff + 180.f, 360.f);
						flDiff += flDiff < 0 ? 180.f : -180.f;
							
						SDK::Output("Diff", std::format("{}", flDiff).c_str(), {}, Vars::Debug::Info.Value);
							
						if (flAccelSpeed > 0 && !vWishDir.IsZero() && fabsf(flDiff) > 45.f)
							tRecord.m_vDirection = vWishDir * flAccelSpeed;
						else
							tRecord.m_vDirection = tRecord.m_vDirection.Normalized() * flAccelSpeed;
					}
					//SDK::Output("Angle", std::format("{}", tRecord.m_vDirection.Ang).c_str());
					//SDK::Output("WishDir", std::format("{}, {}, ({})", vWishDir.x, vWishDir.y, flAccelSpeed).c_str());
					//SDK::Output("Velocity", std::format("{}, {}, {} ({}, {}; {})", vNewVelocity.x, vNewVelocity.y, vNewVelocity.z, vNewVelocity.Length2D(), vNewVelocity.Length2D(), flMaxSpeed * 0.999f).c_str());

					break;
				}
				/*
				case 1: // airmove
				{
					Vec3 vOldVelocity = pRecord->m_vVelocity, vNewVelocity = tRecord.m_vVelocity;
					vOldVelocity.z = vNewVelocity.z = 0; // for wishdir

					Vec3 vWishDir = vNewVelocity - vOldVelocity;
					float flAccelSpeed = vWishDir.Normalize();

					SDK::Output("flAccelSpeed", std::format("{}, {}", flAccelSpeed, flMaxSpeed).c_str());

					// just assume forward if we can't get a dir large enough
					if (flAccelSpeed < 10.f) // this might be a bad solution?
						break;

					bSet = true;
					tRecord.m_vDirection = vWishDir * flMaxSpeed;
					//SDK::Output("WishDir", std::format("{}, {}, ({})", vWishDir.x, vWishDir.y, flAccelSpeed).c_str());

					break;
				}
				case 2: // watermove
				{
					// maybe implement idk

					break;
				}
				* /
				//case 3: // none, just use velocity
				}
				*/
			}

			//if (Vars::Debug::Info.Value)
			//	G::LineStorage.push_back({ { tRecord.m_vOrigin, tRecord.m_vOrigin + tRecord.m_vDirection }, I::GlobalVars->curtime + 0.5f, bSet ? Color_t(SDK::StdRandomInt(200, 255), SDK::StdRandomInt(200, 255), SDK::StdRandomInt(200, 255), 255) : Color_t(0, 0, 0, 255) });
		}
			
		vRecords.push_front(tRecord);
		if (vRecords.size() > 66)
			vRecords.pop_back();
	}

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		auto& vSimTimes = mSimTimes[pPlayer->entindex()];

		if (pEntity->entindex() == I::EngineClient->GetLocalPlayer() || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
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



bool CMovementSimulation::Initialize(CBaseEntity* pEntity, PlayerStorage& playerStorageOut, bool useHitchance, bool cancelStrafe)
{
	if (!pEntity || !pEntity->IsPlayer() || !pEntity->As<CTFPlayer>()->IsAlive())
	{
		playerStorageOut.m_bInitFailed = playerStorageOut.m_bFailed = true;
		return false;
	}

	auto pPlayer = pEntity->As<CTFPlayer>();
	playerStorageOut.m_pPlayer = pPlayer;

	I::MoveHelper->SetHost(pPlayer);
	pPlayer->SetCurrentCmd(&DummyCmd);

	// store player restore data
	Store(playerStorageOut);

	// store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;

	// the hacks that make it work
	{
		if (auto pAvgVelocity = H::Entities.GetAvgVelocity(pPlayer->entindex()))
			pPlayer->m_vecVelocity() = *pAvgVelocity; // only use average velocity here

		if (pPlayer->IsDucking())
		{
			pPlayer->m_fFlags() &= ~FL_DUCKING; // breaks origin's z if FL_DUCKING is not removed
			pPlayer->m_bDucked() = true; // (mins/maxs will be fine when ducking as long as m_bDucked is true)
			pPlayer->m_flDucktime() = 0.f;
			pPlayer->m_flDuckJumpTime() = 0.f;
			pPlayer->m_bDucking() = false;
			pPlayer->m_bInDuckJump() = false;
		}

		if (pPlayer != H::Entities.GetLocal())
		{
			pPlayer->m_vecBaseVelocity() = Vec3(); // residual basevelocity causes issues
			if (pPlayer->IsOnGround())
				pPlayer->m_vecVelocity().z = std::min(pPlayer->m_vecVelocity().z, 0.f); // step fix
			else
				pPlayer->m_hGroundEntity() = nullptr; // fix for velocity.z being set to 0 even if in air
		}
		else if (Vars::Misc::Movement::Bunnyhop.Value && G::Buttons & IN_JUMP)
			playerStorageOut.m_bBunnyHop = true;

		// fixes issues with origin tolerance
		if (auto pGameRules = I::TFGameRules->Get())
		{
			if (auto pViewVectors = pGameRules->GetViewVectors())
			{
				pViewVectors->m_vHullMin = Vec3(-24, -24, 0) + 0.125f;
				pViewVectors->m_vHullMax = Vec3(24, 24, 82) - 0.125f;
				pViewVectors->m_vDuckHullMin = Vec3(-24, -24, 0) + 0.125f;
				pViewVectors->m_vDuckHullMax = Vec3(24, 24, 62) - 0.125f;
			}
		}

		//for some reason if xy vel is zero it doesn't predict
		if (fabsf(pPlayer->m_vecVelocity().x) < 0.01f)
			pPlayer->m_vecVelocity().x = 0.015f;
		if (fabsf(pPlayer->m_vecVelocity().y) < 0.01f)
			pPlayer->m_vecVelocity().y = 0.015f;
	}

	// setup move data
	if (!SetupMoveData(playerStorageOut))
	{
		playerStorageOut.m_bFailed = true;
		return false;
	}

	const int iStrafeSamples = pPlayer->IsOnGround() ? Vars::Aimbot::Projectile::GroundSamples.Value : Vars::Aimbot::Projectile::AirSamples.Value;

	// calculate strafe if desired
	bool bCalculated = cancelStrafe ? false : StrafePrediction(playerStorageOut, iStrafeSamples);

	// really hope this doesn't work like shit
	if (useHitchance && bCalculated && !pPlayer->m_vecVelocity().IsZero() && Vars::Aimbot::Projectile::Hitchance.Value)
	{
		const auto& vRecords = mRecords[pPlayer->entindex()];
		const auto iSamples = vRecords.size();

		float flCurrentChance = 1.f, flAverageYaw = 0.f;
		for (size_t i = 0; i < iSamples; i++)
		{
			if (vRecords.size() <= i + 2)
				break;

			const auto& pRecord1 = vRecords[i], &pRecord2 = vRecords[i + 1];
			const float flYaw1 = Math::VelocityToAngles(pRecord1.m_vDirection).y, flYaw2 = Math::VelocityToAngles(pRecord2.m_vDirection).y;
			const float flTime1 = pRecord1.m_flSimTime, flTime2 = pRecord2.m_flSimTime;
			const int iTicks = std::max(TIME_TO_TICKS(flTime1 - flTime2), 1);

			float flYaw = flYaw1 - flYaw2;
			flYaw = fmodf(flYaw + 180.f, 360.f);
			flYaw += flYaw < 0 ? 180.f : -180.f;
			flYaw /= iTicks;
			flAverageYaw += flYaw;
			if (playerStorageOut.m_MoveData.m_flMaxSpeed)
				flYaw *= std::clamp(pRecord1.m_vVelocity.Length2D() / playerStorageOut.m_MoveData.m_flMaxSpeed, 0.f, 1.f);

			if ((i + 1) % iStrafeSamples == 0 || i == iSamples - 1)
			{
				flAverageYaw /= i % iStrafeSamples + 1;
				if (fabsf(playerStorageOut.m_flAverageYaw - flAverageYaw) > 0.5f)
					flCurrentChance -= 1.f / ((iSamples - 1) / float(iStrafeSamples) + 1);
				flAverageYaw = 0.f;
			}
		}

		if (flCurrentChance < Vars::Aimbot::Projectile::Hitchance.Value / 100)
		{
			SDK::Output("MovementSimulation", std::format("Hitchance ({}% < {}%)", flCurrentChance * 100, Vars::Aimbot::Projectile::Hitchance.Value).c_str(), { 80, 200, 120, 255 }, Vars::Debug::Logging.Value);

			playerStorageOut.m_bFailed = true;
			return false;
		}
	}

	for (int i = 0; i < H::Entities.GetChoke(pPlayer->entindex()); i++)
		RunTick(playerStorageOut);

	return true;
}

bool CMovementSimulation::SetupMoveData(PlayerStorage& playerStorage)
{
	if (!playerStorage.m_pPlayer)
		return false;

	const auto& vRecords = mRecords[playerStorage.m_pPlayer->entindex()];

	playerStorage.m_MoveData.m_bFirstRunOfFunctions = false;
	playerStorage.m_MoveData.m_bGameCodeMovedPlayer = false;
	playerStorage.m_MoveData.m_nPlayerHandle = reinterpret_cast<IHandleEntity*>(playerStorage.m_pPlayer)->GetRefEHandle();

	playerStorage.m_MoveData.m_vecAbsOrigin = playerStorage.m_pPlayer->m_vecOrigin();
	playerStorage.m_MoveData.m_vecVelocity = playerStorage.m_pPlayer->m_vecVelocity();
	playerStorage.m_MoveData.m_flMaxSpeed = SDK::MaxSpeed(playerStorage.m_pPlayer);
	playerStorage.m_MoveData.m_flClientMaxSpeed = playerStorage.m_MoveData.m_flMaxSpeed;

	{
		if (!vRecords.empty())
		{
			auto& tRecord = vRecords.front();
			playerStorage.m_MoveData.m_vecViewAngles = tRecord.m_vAngles;
			if (!tRecord.m_vDirection.IsZero())
			{
				Vec3 vMove = Math::RotatePoint(tRecord.m_vDirection, {}, { 0, -tRecord.m_vAngles.y, 0 });
				playerStorage.m_MoveData.m_flForwardMove = vMove.x;
				playerStorage.m_MoveData.m_flSideMove = -vMove.y;
				playerStorage.m_MoveData.m_flUpMove = vMove.z;
			}
		}
		else
		{
			playerStorage.m_MoveData.m_vecViewAngles = { 0.f, Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity).y, 0.f };
			Vec3 vForward, vRight; Math::AngleVectors(playerStorage.m_MoveData.m_vecViewAngles, &vForward, &vRight, nullptr);
			playerStorage.m_MoveData.m_flForwardMove = (playerStorage.m_MoveData.m_vecVelocity.y - vRight.y / vRight.x * playerStorage.m_MoveData.m_vecVelocity.x) / (vForward.y - vRight.y / vRight.x * vForward.x);
			playerStorage.m_MoveData.m_flSideMove = (playerStorage.m_MoveData.m_vecVelocity.x - vForward.x * playerStorage.m_MoveData.m_flForwardMove) / vRight.x;
			if (playerStorage.m_pPlayer->m_vecVelocity().Length2D() <= playerStorage.m_MoveData.m_flMaxSpeed * 0.1f)
				playerStorage.m_MoveData.m_flForwardMove = playerStorage.m_MoveData.m_flSideMove = 0.f;
		}
		//SDK::Output("Shit", std::format("{}, {}, {}", playerStorage.m_MoveData.m_vecViewAngles.y, playerStorage.m_MoveData.m_flForwardMove, playerStorage.m_MoveData.m_flSideMove).c_str());
	}

	playerStorage.m_MoveData.m_vecAngles = playerStorage.m_MoveData.m_vecOldAngles = playerStorage.m_MoveData.m_vecViewAngles;
	if (auto pConstraintEntity = playerStorage.m_pPlayer->m_hConstraintEntity().Get())
		playerStorage.m_MoveData.m_vecConstraintCenter = pConstraintEntity->GetAbsOrigin();
	else
		playerStorage.m_MoveData.m_vecConstraintCenter = playerStorage.m_pPlayer->m_vecConstraintCenter();
	playerStorage.m_MoveData.m_flConstraintRadius = playerStorage.m_pPlayer->m_flConstraintRadius();
	playerStorage.m_MoveData.m_flConstraintWidth = playerStorage.m_pPlayer->m_flConstraintWidth();
	playerStorage.m_MoveData.m_flConstraintSpeedFactor = playerStorage.m_pPlayer->m_flConstraintSpeedFactor();

	playerStorage.m_flPredictedDelta = GetPredictedDelta(playerStorage.m_pPlayer);
	playerStorage.m_flSimTime = playerStorage.m_pPlayer->m_flSimulationTime();
	playerStorage.m_flPredictedSimTime = playerStorage.m_flSimTime + playerStorage.m_flPredictedDelta;
	playerStorage.m_vPredictedOrigin = playerStorage.m_MoveData.m_vecAbsOrigin;

	return true;
}

static int GetYawDifference(std::deque<MoveData>& vRecords, size_t i, float* pYaw, float flStraightFuzzyValue, int iMaxChanges = 0, int iMaxChangeTime = 0, float flMaxSpeed = 0.f)
{
	if (vRecords.size() <= i + 2)
		return false;

	const auto& pRecord1 = vRecords[i], &pRecord2 = vRecords[i + 1];
	if (pRecord1.m_iMode != pRecord2.m_iMode)
		return 2;

	const float flYaw1 = Math::VelocityToAngles(pRecord1.m_vDirection).y, flYaw2 = Math::VelocityToAngles(pRecord2.m_vDirection).y;
	const float flTime1 = pRecord1.m_flSimTime, flTime2 = pRecord2.m_flSimTime;
	const int iTicks = std::max(TIME_TO_TICKS(flTime1 - flTime2), 1);

	*pYaw = flYaw1 - flYaw2;
	*pYaw = fmodf(*pYaw + 180.f, 360.f);
	*pYaw += *pYaw < 0 ? 180.f : -180.f;
	*pYaw /= iTicks;
	if (flMaxSpeed)
		*pYaw *= std::clamp(pRecord1.m_vVelocity.Length2D() / flMaxSpeed, 0.f, 1.f);

	static int iSign = 0;
	const int iLastSign = iSign;
	const int iCurSign = iSign = *pYaw > 0 ? 1 : -1;

	static int iChanges, iStart;
	bool bStraight = fabsf(*pYaw) * pRecord1.m_vVelocity.Length2D() * iTicks <= flStraightFuzzyValue; // dumb way to get straight bool
	bool bChanged = iLastSign && iCurSign && iLastSign != iCurSign;
	if (!i)
	{
		iChanges = 0, iStart = TIME_TO_TICKS(flTime1);
		if (bStraight && ++iChanges > iMaxChanges)
			return false;
		return true;
	}
	if ((bChanged || bStraight) && ++iChanges > iMaxChanges)
		return false;
	if (iChanges && iStart - TIME_TO_TICKS(flTime2) > iMaxChangeTime)
		return false;
	return true;
}

void CMovementSimulation::GetAverageYaw(PlayerStorage& playerStorage, int iSamples)
{
	auto pPlayer = playerStorage.m_pPlayer;
	auto& vRecords = mRecords[pPlayer->entindex()];

	bool bGround = pPlayer->IsOnGround();
	float flStraightFuzzyValue = bGround ? Vars::Aimbot::Projectile::GroundStraightFuzzyValue.Value : Vars::Aimbot::Projectile::AirStraightFuzzyValue.Value;
	int iMaxChanges = bGround ? Vars::Aimbot::Projectile::GroundMaxChanges.Value : Vars::Aimbot::Projectile::AirMaxChanges.Value;
	int iMaxChangeTime = bGround ? Vars::Aimbot::Projectile::GroundMaxChangeTime.Value : Vars::Aimbot::Projectile::AirMaxChangeTime.Value;
	float flLowMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumDistance.Value : Vars::Aimbot::Projectile::AirLowMinimumDistance.Value;
	float flLowMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumSamples.Value : Vars::Aimbot::Projectile::AirLowMinimumSamples.Value;
	float flHighMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumDistance.Value : Vars::Aimbot::Projectile::AirHighMinimumDistance.Value;
	float flHighMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumSamples.Value : Vars::Aimbot::Projectile::AirHighMinimumSamples.Value;
	float flNewWeight = (bGround ? Vars::Aimbot::Projectile::GroundNewWeight.Value : Vars::Aimbot::Projectile::AirNewWeight.Value) / 100;
	float flOldWeight = (bGround ? Vars::Aimbot::Projectile::GroundOldWeight.Value : Vars::Aimbot::Projectile::AirOldWeight.Value) / 100;
	float flMaxWeight = std::max(flNewWeight, flOldWeight);
	if (!flMaxWeight)
		return;

	float flAverageYaw = 0.f, flAverageDelta = 0.f, flTotalWeight = 0.f;
	int iTicks = 0, iSkips = 0, iDeltaChanges = 0;
	for (; iTicks < iSamples; iTicks++)
	{
		float flYaw = 0.f;
		int iResult = GetYawDifference(vRecords, iTicks, &flYaw, flStraightFuzzyValue, iMaxChanges, iMaxChangeTime, playerStorage.m_MoveData.m_flMaxSpeed);
		SDK::Output("GetYawDifference", std::format("{}: {}, {}", iTicks, flYaw, iResult).c_str(), { 50, 127, 75, 255 }, Vars::Debug::Logging.Value);
		if (!iResult)
			break;
		else if (iResult == 2)
		{
			iSkips++;
			continue;
		}

		float flMult = 1.f;
		/*
		if (!pPlayer->IsOnGround())
		{
			float flPrevVelZ = vRecords[iTicks].m_vVelocity.z - GetPlayerGravity(pPlayer) * TICK_INTERVAL;
			flMult = 0.f < flPrevVelZ && flPrevVelZ <= 250.f ? 0.25f : 1.f; // flEntFriction will be 0.25f if flPrevVelZ is between 0.f and 250.f
		}
		*/

		float flWeight = powf(Math::RemapValClamped(iTicks, 0, iSamples, flNewWeight, flOldWeight) / flMaxWeight, flMaxWeight);
		flYaw *= flWeight / flMult, flTotalWeight += flWeight;

		flAverageYaw += flYaw;
		if (Vars::Aimbot::Projectile::StrafeDelta.Value)
		{
			static float flStaticYaw;
			float flLastYaw = flStaticYaw;
			flStaticYaw = flYaw;
			if (iTicks > 0)
			{
				static float flStaticDelta;
				float flLastDelta = flStaticDelta;
				float flDelta = flStaticDelta = flLastYaw - flYaw;
				if (iTicks > 1 && sign(flDelta) != sign(flLastDelta))
					iDeltaChanges++;

				flAverageDelta += flDelta;
			}
		}
	}
	iTicks -= iSkips;

	if (!flTotalWeight || iTicks <= 3) // valid strafes not high enough
		return;
	if (iTicks > 6) // valid strafes probably fine for this
		flAverageDelta /= std::max(iDeltaChanges, 1);
	else
		flAverageDelta = 0.f;

	float flDistance = 0.f;
	if (auto pLocal = H::Entities.GetLocal())
		flDistance = pLocal->m_vecOrigin().DistTo(playerStorage.m_pPlayer->m_vecOrigin());

	int iMinimumStrafes = flDistance < flLowMinimumDistance ? flLowMinimumSamples : Math::RemapValClamped(flDistance, flLowMinimumDistance, flHighMinimumDistance, flLowMinimumSamples + 1, flHighMinimumSamples);
	flAverageYaw /= flTotalWeight;
	flAverageYaw *= std::min(float(iTicks) / iMinimumStrafes, 1.f);
	flAverageDelta = std::clamp(flAverageDelta / std::max(iTicks, iMinimumStrafes), -0.05f, 0.05f);
	if (fabsf(flAverageYaw) < 0.1f)
		return;

	SDK::Output("MovementSimulation", std::format("flAverageYaw calculated to {} ({}) from {} ({}) {}", flAverageYaw, flAverageDelta, iTicks, iMinimumStrafes, pPlayer->entindex() == I::EngineClient->GetLocalPlayer() ? "(local)" : "").c_str(), {100, 255, 150, 255}, Vars::Debug::Logging.Value);
	
	playerStorage.m_flAverageYaw = flAverageYaw;
	playerStorage.m_flAverageDelta = flAverageDelta;
}

bool CMovementSimulation::StrafePrediction(PlayerStorage& playerStorage, int iSamples)
{
	if (playerStorage.m_pPlayer->IsOnGround()
		? !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Ground)
		: !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Air))
		return false;

	GetAverageYaw(playerStorage, iSamples);
	return true;
}

void CMovementSimulation::RunTick(PlayerStorage& playerStorage, bool bPath)
{
	if (playerStorage.m_bFailed || !playerStorage.m_pPlayer || !playerStorage.m_pPlayer->IsPlayer())
		return;

	if (bPath)
		playerStorage.m_vPath.push_back(playerStorage.m_MoveData.m_vecAbsOrigin);

	// make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;

	float flCorrection = 0.f;
	if (playerStorage.m_flAverageYaw)
	{
		float flYaw = playerStorage.m_flAverageYaw, flDelta = playerStorage.m_flAverageDelta;

		float flMult = 1.f;
		if (!playerStorage.m_pPlayer->IsOnGround())
		{
			if (!playerStorage.m_pPlayer->InCond(TF_COND_SHIELD_CHARGE))
				flCorrection = 90.f * sign(flYaw);

			float flPrevVelZ = playerStorage.m_MoveData.m_vecVelocity.z + GetPlayerGravity(playerStorage.m_pPlayer) * TICK_INTERVAL;
			flMult = 0.f < flPrevVelZ && flPrevVelZ <= 250.f ? 0.25f : 1.f; // flEntFriction will be 0.25f if flPrevVelZ is between 0.f and 250.f
		}
		playerStorage.m_MoveData.m_vecViewAngles.y += flYaw * flMult + flCorrection;

		//playerStorage.m_flAverageYaw = std::clamp(flYaw + flDelta* flMult, sign(flYaw) == -1 ? flYaw : 0, sign(flYaw) == 1 ? flYaw : 0);
		playerStorage.m_flAverageYaw = sign(flYaw) == -1 ? std::min(flYaw + flDelta * flMult, 0.f) : std::max(flYaw + flDelta * flMult, 0.f);
	}
	else
	{
		if (!playerStorage.m_pPlayer->IsOnGround())
			playerStorage.m_MoveData.m_flForwardMove = playerStorage.m_MoveData.m_flSideMove = 0.f;
		//else
		//	playerStorage.m_MoveData.m_vecViewAngles.y = Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity).y;
	}

	/*
	{
		Vec3 vMove = Vec3(playerStorage.m_MoveData.m_flForwardMove, -playerStorage.m_MoveData.m_flSideMove, playerStorage.m_MoveData.m_flUpMove);

		Vec3 v1 = playerStorage.m_MoveData.m_vecAbsOrigin;
		Vec3 v2 = v1 + (vMove.IsZero() ? Vec3(0, 0, 1) : Math::RotatePoint(vMove.Normalized() * 12, {}, { 0, playerStorage.m_MoveData.m_vecViewAngles.y, 0 }));

		G::LineStorage.push_back({ { v1, v2 }, I::GlobalVars->curtime + 5.f, Color_t(SDK::RandomInt(0, 255), SDK::RandomInt(0, 255), SDK::RandomInt(0, 255), 255) });
	}
	*/

	float flOldSpeed = playerStorage.m_MoveData.m_flClientMaxSpeed, flOldSide = playerStorage.m_MoveData.m_flSideMove;
	if (playerStorage.m_PlayerData.m_fFlags & FL_DUCKING && playerStorage.m_pPlayer->IsOnGround())
		playerStorage.m_MoveData.m_flClientMaxSpeed /= 3;

	if (playerStorage.m_bBunnyHop && !(playerStorage.m_PlayerData.m_fFlags & FL_DUCKING))
	{
		playerStorage.m_MoveData.m_nOldButtons = 0;
		playerStorage.m_MoveData.m_nButtons |= IN_JUMP;
	}

	I::GameMovement->ProcessMovement(playerStorage.m_pPlayer, &playerStorage.m_MoveData);

	playerStorage.m_MoveData.m_flClientMaxSpeed = flOldSpeed;

	playerStorage.m_flSimTime += TICK_INTERVAL;
	playerStorage.m_bPredictNetworked = playerStorage.m_flSimTime >= playerStorage.m_flPredictedSimTime;
	if (playerStorage.m_bPredictNetworked)
	{
		playerStorage.m_vPredictedOrigin = playerStorage.m_MoveData.m_vecAbsOrigin;
		playerStorage.m_flPredictedSimTime += playerStorage.m_flPredictedDelta;
	}

	if (playerStorage.m_flAverageYaw)
		playerStorage.m_MoveData.m_vecViewAngles.y -= flCorrection;
	else if (playerStorage.m_pPlayer->IsOnGround() && !playerStorage.m_MoveData.m_flForwardMove && !playerStorage.m_MoveData.m_flSideMove && !playerStorage.m_MoveData.m_vecVelocity.IsZero())
	{
		Vec3 vMove = Math::RotatePoint(playerStorage.m_MoveData.m_vecVelocity * Vec3(1, 1, 0), {}, { 0, -playerStorage.m_MoveData.m_vecViewAngles.y, 0 });
		playerStorage.m_MoveData.m_flForwardMove = vMove.x;
		playerStorage.m_MoveData.m_flSideMove = -vMove.y;
	}
}

void CMovementSimulation::Restore(PlayerStorage& playerStorage)
{
	if (playerStorage.m_bInitFailed || !playerStorage.m_pPlayer)
		return;

	I::MoveHelper->SetHost(nullptr);
	playerStorage.m_pPlayer->SetCurrentCmd(nullptr);

	Reset(playerStorage);

	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
	I::GlobalVars->frametime = m_flOldFrametime;

	const bool bInitFailed = playerStorage.m_bInitFailed, bFailed = playerStorage.m_bFailed;
	memset(&playerStorage, 0, sizeof(PlayerStorage));
	playerStorage.m_bInitFailed = bInitFailed, playerStorage.m_bFailed = bFailed;
}

float CMovementSimulation::GetPredictedDelta(CBaseEntity* pEntity)
{
	auto& vSimTimes = mSimTimes[pEntity->entindex()];
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