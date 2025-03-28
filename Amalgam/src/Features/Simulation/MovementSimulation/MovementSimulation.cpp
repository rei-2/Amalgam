#include "MovementSimulation.h"

#include "../../EnginePrediction/EnginePrediction.h"
#include <numeric>

//we'll use this to set current player's command, without it CGameMovement::CheckInterval will try to access a nullptr
static CUserCmd DummyCmd = {};

void CMovementSimulation::Store(PlayerStorage& tStorage)
{
	tStorage.m_PlayerData.m_vecOrigin = tStorage.m_pPlayer->m_vecOrigin();
	tStorage.m_PlayerData.m_vecVelocity = tStorage.m_pPlayer->m_vecVelocity();
	tStorage.m_PlayerData.m_vecBaseVelocity = tStorage.m_pPlayer->m_vecBaseVelocity();
	tStorage.m_PlayerData.m_vecViewOffset = tStorage.m_pPlayer->m_vecViewOffset();
	tStorage.m_PlayerData.m_hGroundEntity = tStorage.m_pPlayer->m_hGroundEntity();
	tStorage.m_PlayerData.m_fFlags = tStorage.m_pPlayer->m_fFlags();
	tStorage.m_PlayerData.m_flDucktime = tStorage.m_pPlayer->m_flDucktime();
	tStorage.m_PlayerData.m_flDuckJumpTime = tStorage.m_pPlayer->m_flDuckJumpTime();
	tStorage.m_PlayerData.m_bDucked = tStorage.m_pPlayer->m_bDucked();
	tStorage.m_PlayerData.m_bDucking = tStorage.m_pPlayer->m_bDucking();
	tStorage.m_PlayerData.m_bInDuckJump = tStorage.m_pPlayer->m_bInDuckJump();
	tStorage.m_PlayerData.m_flModelScale = tStorage.m_pPlayer->m_flModelScale();
	tStorage.m_PlayerData.m_nButtons = tStorage.m_pPlayer->m_nButtons();
	tStorage.m_PlayerData.m_flLastMovementStunChange = tStorage.m_pPlayer->m_flLastMovementStunChange();
	tStorage.m_PlayerData.m_flStunLerpTarget = tStorage.m_pPlayer->m_flStunLerpTarget();
	tStorage.m_PlayerData.m_bStunNeedsFadeOut = tStorage.m_pPlayer->m_bStunNeedsFadeOut();
	tStorage.m_PlayerData.m_flPrevTauntYaw = tStorage.m_pPlayer->m_flPrevTauntYaw();
	tStorage.m_PlayerData.m_flTauntYaw = tStorage.m_pPlayer->m_flTauntYaw();
	tStorage.m_PlayerData.m_flCurrentTauntMoveSpeed = tStorage.m_pPlayer->m_flCurrentTauntMoveSpeed();
	tStorage.m_PlayerData.m_iKartState = tStorage.m_pPlayer->m_iKartState();
	tStorage.m_PlayerData.m_flVehicleReverseTime = tStorage.m_pPlayer->m_flVehicleReverseTime();
	tStorage.m_PlayerData.m_flHypeMeter = tStorage.m_pPlayer->m_flHypeMeter();
	tStorage.m_PlayerData.m_flMaxspeed = tStorage.m_pPlayer->m_flMaxspeed();
	tStorage.m_PlayerData.m_nAirDucked = tStorage.m_pPlayer->m_nAirDucked();
	tStorage.m_PlayerData.m_bJumping = tStorage.m_pPlayer->m_bJumping();
	tStorage.m_PlayerData.m_iAirDash = tStorage.m_pPlayer->m_iAirDash();
	tStorage.m_PlayerData.m_flWaterJumpTime = tStorage.m_pPlayer->m_flWaterJumpTime();
	tStorage.m_PlayerData.m_flSwimSoundTime = tStorage.m_pPlayer->m_flSwimSoundTime();
	tStorage.m_PlayerData.m_surfaceProps = tStorage.m_pPlayer->m_surfaceProps();
	tStorage.m_PlayerData.m_pSurfaceData = tStorage.m_pPlayer->m_pSurfaceData();
	tStorage.m_PlayerData.m_surfaceFriction = tStorage.m_pPlayer->m_surfaceFriction();
	tStorage.m_PlayerData.m_chTextureType = tStorage.m_pPlayer->m_chTextureType();
	tStorage.m_PlayerData.m_vecPunchAngle = tStorage.m_pPlayer->m_vecPunchAngle();
	tStorage.m_PlayerData.m_vecPunchAngleVel = tStorage.m_pPlayer->m_vecPunchAngleVel();
	tStorage.m_PlayerData.m_MoveType = tStorage.m_pPlayer->m_MoveType();
	tStorage.m_PlayerData.m_MoveCollide = tStorage.m_pPlayer->m_MoveCollide();
	tStorage.m_PlayerData.m_vecLadderNormal = tStorage.m_pPlayer->m_vecLadderNormal();
	tStorage.m_PlayerData.m_flGravity = tStorage.m_pPlayer->m_flGravity();
	tStorage.m_PlayerData.m_nWaterLevel = tStorage.m_pPlayer->m_nWaterLevel();
	tStorage.m_PlayerData.m_nWaterType = tStorage.m_pPlayer->m_nWaterType();
	tStorage.m_PlayerData.m_flFallVelocity = tStorage.m_pPlayer->m_flFallVelocity();
	tStorage.m_PlayerData.m_nPlayerCond = tStorage.m_pPlayer->m_nPlayerCond();
	tStorage.m_PlayerData.m_nPlayerCondEx = tStorage.m_pPlayer->m_nPlayerCondEx();
	tStorage.m_PlayerData.m_nPlayerCondEx2 = tStorage.m_pPlayer->m_nPlayerCondEx2();
	tStorage.m_PlayerData.m_nPlayerCondEx3 = tStorage.m_pPlayer->m_nPlayerCondEx3();
	tStorage.m_PlayerData.m_nPlayerCondEx4 = tStorage.m_pPlayer->m_nPlayerCondEx4();
	tStorage.m_PlayerData._condition_bits = tStorage.m_pPlayer->_condition_bits();
}

void CMovementSimulation::Reset(PlayerStorage& tStorage)
{
	tStorage.m_pPlayer->m_vecOrigin() = tStorage.m_PlayerData.m_vecOrigin;
	tStorage.m_pPlayer->m_vecVelocity() = tStorage.m_PlayerData.m_vecVelocity;
	tStorage.m_pPlayer->m_vecBaseVelocity() = tStorage.m_PlayerData.m_vecBaseVelocity;
	tStorage.m_pPlayer->m_vecViewOffset() = tStorage.m_PlayerData.m_vecViewOffset;
	tStorage.m_pPlayer->m_hGroundEntity() = tStorage.m_PlayerData.m_hGroundEntity;
	tStorage.m_pPlayer->m_fFlags() = tStorage.m_PlayerData.m_fFlags;
	tStorage.m_pPlayer->m_flDucktime() = tStorage.m_PlayerData.m_flDucktime;
	tStorage.m_pPlayer->m_flDuckJumpTime() = tStorage.m_PlayerData.m_flDuckJumpTime;
	tStorage.m_pPlayer->m_bDucked() = tStorage.m_PlayerData.m_bDucked;
	tStorage.m_pPlayer->m_bDucking() = tStorage.m_PlayerData.m_bDucking;
	tStorage.m_pPlayer->m_bInDuckJump() = tStorage.m_PlayerData.m_bInDuckJump;
	tStorage.m_pPlayer->m_flModelScale() = tStorage.m_PlayerData.m_flModelScale;
	tStorage.m_pPlayer->m_nButtons() = tStorage.m_PlayerData.m_nButtons;
	tStorage.m_pPlayer->m_flLastMovementStunChange() = tStorage.m_PlayerData.m_flLastMovementStunChange;
	tStorage.m_pPlayer->m_flStunLerpTarget() = tStorage.m_PlayerData.m_flStunLerpTarget;
	tStorage.m_pPlayer->m_bStunNeedsFadeOut() = tStorage.m_PlayerData.m_bStunNeedsFadeOut;
	tStorage.m_pPlayer->m_flPrevTauntYaw() = tStorage.m_PlayerData.m_flPrevTauntYaw;
	tStorage.m_pPlayer->m_flTauntYaw() = tStorage.m_PlayerData.m_flTauntYaw;
	tStorage.m_pPlayer->m_flCurrentTauntMoveSpeed() = tStorage.m_PlayerData.m_flCurrentTauntMoveSpeed;
	tStorage.m_pPlayer->m_iKartState() = tStorage.m_PlayerData.m_iKartState;
	tStorage.m_pPlayer->m_flVehicleReverseTime() = tStorage.m_PlayerData.m_flVehicleReverseTime;
	tStorage.m_pPlayer->m_flHypeMeter() = tStorage.m_PlayerData.m_flHypeMeter;
	tStorage.m_pPlayer->m_flMaxspeed() = tStorage.m_PlayerData.m_flMaxspeed;
	tStorage.m_pPlayer->m_nAirDucked() = tStorage.m_PlayerData.m_nAirDucked;
	tStorage.m_pPlayer->m_bJumping() = tStorage.m_PlayerData.m_bJumping;
	tStorage.m_pPlayer->m_iAirDash() = tStorage.m_PlayerData.m_iAirDash;
	tStorage.m_pPlayer->m_flWaterJumpTime() = tStorage.m_PlayerData.m_flWaterJumpTime;
	tStorage.m_pPlayer->m_flSwimSoundTime() = tStorage.m_PlayerData.m_flSwimSoundTime;
	tStorage.m_pPlayer->m_surfaceProps() = tStorage.m_PlayerData.m_surfaceProps;
	tStorage.m_pPlayer->m_pSurfaceData() = tStorage.m_PlayerData.m_pSurfaceData;
	tStorage.m_pPlayer->m_surfaceFriction() = tStorage.m_PlayerData.m_surfaceFriction;
	tStorage.m_pPlayer->m_chTextureType() = tStorage.m_PlayerData.m_chTextureType;
	tStorage.m_pPlayer->m_vecPunchAngle() = tStorage.m_PlayerData.m_vecPunchAngle;
	tStorage.m_pPlayer->m_vecPunchAngleVel() = tStorage.m_PlayerData.m_vecPunchAngleVel;
	tStorage.m_pPlayer->m_MoveType() = tStorage.m_PlayerData.m_MoveType;
	tStorage.m_pPlayer->m_MoveCollide() = tStorage.m_PlayerData.m_MoveCollide;
	tStorage.m_pPlayer->m_vecLadderNormal() = tStorage.m_PlayerData.m_vecLadderNormal;
	tStorage.m_pPlayer->m_flGravity() = tStorage.m_PlayerData.m_flGravity;
	tStorage.m_pPlayer->m_nWaterLevel() = tStorage.m_PlayerData.m_nWaterLevel;
	tStorage.m_pPlayer->m_nWaterType() = tStorage.m_PlayerData.m_nWaterType;
	tStorage.m_pPlayer->m_flFallVelocity() = tStorage.m_PlayerData.m_flFallVelocity;
	tStorage.m_pPlayer->m_nPlayerCond() = tStorage.m_PlayerData.m_nPlayerCond;
	tStorage.m_pPlayer->m_nPlayerCondEx() = tStorage.m_PlayerData.m_nPlayerCondEx;
	tStorage.m_pPlayer->m_nPlayerCondEx2() = tStorage.m_PlayerData.m_nPlayerCondEx2;
	tStorage.m_pPlayer->m_nPlayerCondEx3() = tStorage.m_PlayerData.m_nPlayerCondEx3;
	tStorage.m_pPlayer->m_nPlayerCondEx4() = tStorage.m_PlayerData.m_nPlayerCondEx4;
	tStorage.m_pPlayer->_condition_bits() = tStorage.m_PlayerData._condition_bits;
}


float CMovementSimulation::GetPlayerGravity(CTFPlayer* pEntity)
{
	static auto sv_gravity = U::ConVars.FindVar("sv_gravity");

	return sv_gravity->GetFloat();
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
			pPlayer->m_nWaterLevel() >= 2 ? 2 : pPlayer->IsOnGround() ? 0 : 1,
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
				SDK::TraceHull(pRecord->m_vOrigin, pRecord->m_vOrigin + pRecord->m_vVelocity * TICK_INTERVAL, pPlayer->m_vecMins() + 0.125f, pPlayer->m_vecMaxs() - 0.125f, pPlayer->SolidMask(), &filter, &trace);
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
				switch (tRecord.m_iMode)
				{
				case 0:
					if (bLocal && Vars::Misc::Movement::Bunnyhop.Value && G::OriginalMove.m_iButtons & IN_JUMP)
						tRecord.m_vDirection = (vVelocity * Vec3(1, 1, 0)).Normalized() * flMaxSpeed;
					break;
				case 1:
					tRecord.m_vDirection = (vVelocity * Vec3(1, 1, 0)).Normalized() * flMaxSpeed;
					break;
				case 2:
					tRecord.m_vDirection *= 2;
				}
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

					SDK::Output("flAccelSpeed", std::format("{}", flAccelSpeed).c_str(), {}, Vars::Debug::Logging.Value);
					flAccelSpeed = flAccelSpeed < (flAccel * flMaxSpeed * flEntFriction * TICK_INTERVAL) * 0.35f ? 0.f : flMaxSpeed; // this might be a bad solution?

					if (!tRecord.m_vDirection.IsZero())
					{
						bSet = true;

						if (Vars::Debug::Info.Value)
						{
							G::LineStorage.clear();
							G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(tRecord.m_vOrigin, tRecord.m_vOrigin + tRecord.m_vDirection), I::GlobalVars->curtime + 0.1f, Color_t(255, 0, 0) });
							G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(tRecord.m_vOrigin, tRecord.m_vOrigin + vWishDir * flAccelSpeed), I::GlobalVars->curtime + 0.1f, Color_t(0, 255, 0));
						}

						float flAng1 = Math::VelocityToAngles(tRecord.m_vDirection).y, flAng2 = Math::VelocityToAngles(vWishDir).y;
						float flDiff = Math::NormalizeAngle(flAng1 - flAng2);
							
						SDK::Output("Diff", std::format("{}", flDiff).c_str(), {}, Vars::Debug::Logging.Value);
							
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
			//	G::LineStorage.emplace_back({ std::pair<Vec3, Vec3>(tRecord.m_vOrigin, tRecord.m_vOrigin + tRecord.m_vDirection), I::GlobalVars->curtime + 0.5f, bSet ? Color_t(SDK::StdRandomInt(200, 255), SDK::StdRandomInt(200, 255), SDK::StdRandomInt(200, 255)) : Color_t(0, 0, 0) });
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



bool CMovementSimulation::Initialize(CBaseEntity* pEntity, PlayerStorage& tStorage, bool useHitchance, bool cancelStrafe)
{
	if (!pEntity || !pEntity->IsPlayer() || !pEntity->As<CTFPlayer>()->IsAlive())
	{
		tStorage.m_bInitFailed = tStorage.m_bFailed = true;
		return false;
	}

	auto pPlayer = pEntity->As<CTFPlayer>();
	tStorage.m_pPlayer = pPlayer;

	I::MoveHelper->SetHost(pPlayer);
	pPlayer->SetCurrentCmd(&DummyCmd);

	// store player restore data
	Store(tStorage);

	// store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;

	// the hacks that make it work
	{
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

		if (pPlayer != H::Entities.GetLocal())
		{
			pPlayer->m_vecBaseVelocity() = Vec3(); // residual basevelocity causes issues
			if (pPlayer->IsOnGround())
				pPlayer->m_vecVelocity().z = std::min(pPlayer->m_vecVelocity().z, 0.f); // step fix
			else
				pPlayer->m_hGroundEntity() = nullptr; // fix for velocity.z being set to 0 even if in air
		}
		else if (Vars::Misc::Movement::Bunnyhop.Value && G::OriginalMove.m_iButtons & IN_JUMP)
			tStorage.m_bBunnyHop = true;
	}

	// setup move data
	if (!SetupMoveData(tStorage))
	{
		tStorage.m_bFailed = true;
		return false;
	}

	const int iStrafeSamples = tStorage.m_bDirectMove
		? Vars::Aimbot::Projectile::GroundSamples.Value
		: Vars::Aimbot::Projectile::AirSamples.Value;

	// calculate strafe if desired
	bool bCalculated = cancelStrafe ? false : StrafePrediction(tStorage, iStrafeSamples);

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

			float flYaw = Math::NormalizeAngle(flYaw1 - flYaw2) / iTicks;
			flAverageYaw += flYaw;
			if (tStorage.m_MoveData.m_flMaxSpeed)
				flYaw *= std::clamp(pRecord1.m_vVelocity.Length2D() / tStorage.m_MoveData.m_flMaxSpeed, 0.f, 1.f);

			if ((i + 1) % iStrafeSamples == 0 || i == iSamples - 1)
			{
				flAverageYaw /= i % iStrafeSamples + 1;
				if (fabsf(tStorage.m_flAverageYaw - flAverageYaw) > 0.5f)
					flCurrentChance -= 1.f / ((iSamples - 1) / float(iStrafeSamples) + 1);
				flAverageYaw = 0.f;
			}
		}

		if (flCurrentChance < Vars::Aimbot::Projectile::Hitchance.Value / 100)
		{
			SDK::Output("MovementSimulation", std::format("Hitchance ({}% < {}%)", flCurrentChance * 100, Vars::Aimbot::Projectile::Hitchance.Value).c_str(), { 80, 200, 120 }, Vars::Debug::Logging.Value);

			tStorage.m_bFailed = true;
			return false;
		}
	}

	for (int i = 0; i < H::Entities.GetChoke(pPlayer->entindex()); i++)
		RunTick(tStorage);

	return true;
}

bool CMovementSimulation::SetupMoveData(PlayerStorage& tStorage)
{
	if (!tStorage.m_pPlayer)
		return false;

	const auto& vRecords = mRecords[tStorage.m_pPlayer->entindex()];

	tStorage.m_MoveData.m_bFirstRunOfFunctions = false;
	tStorage.m_MoveData.m_bGameCodeMovedPlayer = false;
	tStorage.m_MoveData.m_nPlayerHandle = reinterpret_cast<IHandleEntity*>(tStorage.m_pPlayer)->GetRefEHandle();

	tStorage.m_MoveData.m_vecAbsOrigin = tStorage.m_pPlayer->m_vecOrigin();
	tStorage.m_MoveData.m_vecVelocity = tStorage.m_pPlayer->m_vecVelocity();
	tStorage.m_MoveData.m_flMaxSpeed = SDK::MaxSpeed(tStorage.m_pPlayer);
	tStorage.m_MoveData.m_flClientMaxSpeed = tStorage.m_MoveData.m_flMaxSpeed;

	if (!(tStorage.m_MoveData.m_vecVelocity * Vec3(1, 1, 0)).IsZero())
	{
		if (!vRecords.empty())
		{
			auto& tRecord = vRecords.front();
			tStorage.m_MoveData.m_vecViewAngles = tRecord.m_vAngles;
			if (!tRecord.m_vDirection.IsZero())
			{
				Vec3 vMove = Math::RotatePoint(tRecord.m_vDirection, {}, { 0, -tRecord.m_vAngles.y, 0 });
				tStorage.m_MoveData.m_flForwardMove = vMove.x;
				tStorage.m_MoveData.m_flSideMove = -vMove.y;
				tStorage.m_MoveData.m_flUpMove = vMove.z;
			}
		}
		else
		{
			tStorage.m_MoveData.m_vecViewAngles = { 0.f, Math::VelocityToAngles(tStorage.m_MoveData.m_vecVelocity).y, 0.f };
			Vec3 vForward, vRight; Math::AngleVectors(tStorage.m_MoveData.m_vecViewAngles, &vForward, &vRight, nullptr);
			tStorage.m_MoveData.m_flForwardMove = (tStorage.m_MoveData.m_vecVelocity.y - vRight.y / vRight.x * tStorage.m_MoveData.m_vecVelocity.x) / (vForward.y - vRight.y / vRight.x * vForward.x);
			tStorage.m_MoveData.m_flSideMove = (tStorage.m_MoveData.m_vecVelocity.x - vForward.x * tStorage.m_MoveData.m_flForwardMove) / vRight.x;
			if (tStorage.m_pPlayer->m_vecVelocity().Length2D() <= tStorage.m_MoveData.m_flMaxSpeed * 0.1f)
				tStorage.m_MoveData.m_flForwardMove = tStorage.m_MoveData.m_flSideMove = 0.f;
		}
		//SDK::Output("Move", std::format("{}: {}, {}, {}", !vRecords.empty(), tStorage.m_MoveData.m_vecViewAngles.y, tStorage.m_MoveData.m_flForwardMove, tStorage.m_MoveData.m_flSideMove).c_str());
	}

	tStorage.m_MoveData.m_vecAngles = tStorage.m_MoveData.m_vecOldAngles = tStorage.m_MoveData.m_vecViewAngles;
	if (auto pConstraintEntity = tStorage.m_pPlayer->m_hConstraintEntity().Get())
		tStorage.m_MoveData.m_vecConstraintCenter = pConstraintEntity->GetAbsOrigin();
	else
		tStorage.m_MoveData.m_vecConstraintCenter = tStorage.m_pPlayer->m_vecConstraintCenter();
	tStorage.m_MoveData.m_flConstraintRadius = tStorage.m_pPlayer->m_flConstraintRadius();
	tStorage.m_MoveData.m_flConstraintWidth = tStorage.m_pPlayer->m_flConstraintWidth();
	tStorage.m_MoveData.m_flConstraintSpeedFactor = tStorage.m_pPlayer->m_flConstraintSpeedFactor();

	tStorage.m_flPredictedDelta = GetPredictedDelta(tStorage.m_pPlayer);
	tStorage.m_flSimTime = tStorage.m_pPlayer->m_flSimulationTime();
	tStorage.m_flPredictedSimTime = tStorage.m_flSimTime + tStorage.m_flPredictedDelta;
	tStorage.m_vPredictedOrigin = tStorage.m_MoveData.m_vecAbsOrigin;
	tStorage.m_bDirectMove = tStorage.m_pPlayer->IsOnGround() || tStorage.m_pPlayer->m_nWaterLevel() >= 2;

	return true;
}

static bool GetYawDifference(MoveData& tRecord1, MoveData& tRecord2, bool bStart, float* pYaw, float flStraightFuzzyValue, int iMaxChanges = 0, int iMaxChangeTime = 0, float flMaxSpeed = 0.f)
{
	const float flYaw1 = Math::VelocityToAngles(tRecord1.m_vDirection).y, flYaw2 = Math::VelocityToAngles(tRecord2.m_vDirection).y;
	const float flTime1 = tRecord1.m_flSimTime, flTime2 = tRecord2.m_flSimTime;
	const int iTicks = std::max(TIME_TO_TICKS(flTime1 - flTime2), 1);

	*pYaw = Math::NormalizeAngle(flYaw1 - flYaw2); // / iTicks
	if (flMaxSpeed)
		*pYaw *= std::clamp(tRecord1.m_vVelocity.Length2D() / flMaxSpeed, 0.f, 1.f);

	static int iSign = 0;
	const int iLastSign = iSign;
	const int iCurSign = iSign = *pYaw > 0 ? 1 : -1;

	static int iChanges, iStart;
	bool bStraight = fabsf(*pYaw) * tRecord1.m_vVelocity.Length2D() * iTicks <= flStraightFuzzyValue; // dumb way to get straight bool
	bool bChanged = iLastSign && iCurSign && iLastSign != iCurSign;
	if (bStart)
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

void CMovementSimulation::GetAverageYaw(PlayerStorage& tStorage, int iSamples)
{
	auto pPlayer = tStorage.m_pPlayer;
	auto& vRecords = mRecords[pPlayer->entindex()];
	if (vRecords.empty())
		return;

	bool bGround = tStorage.m_bDirectMove;
	float flStraightFuzzyValue = bGround ? Vars::Aimbot::Projectile::GroundStraightFuzzyValue.Value : Vars::Aimbot::Projectile::AirStraightFuzzyValue.Value;
	int iMaxChanges = bGround ? Vars::Aimbot::Projectile::GroundMaxChanges.Value : Vars::Aimbot::Projectile::AirMaxChanges.Value;
	int iMaxChangeTime = bGround ? Vars::Aimbot::Projectile::GroundMaxChangeTime.Value : Vars::Aimbot::Projectile::AirMaxChangeTime.Value;
	float flLowMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumDistance.Value : Vars::Aimbot::Projectile::AirLowMinimumDistance.Value;
	float flLowMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumSamples.Value : Vars::Aimbot::Projectile::AirLowMinimumSamples.Value;
	float flHighMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumDistance.Value : Vars::Aimbot::Projectile::AirHighMinimumDistance.Value;
	float flHighMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumSamples.Value : Vars::Aimbot::Projectile::AirHighMinimumSamples.Value;

	float flAverageYaw = 0.f; int iTicks = 0, iTicks2 = 0;
	iSamples = std::min(iSamples, int(vRecords.size()));
	for (size_t i = 1; i < iSamples; i++)
	{
		auto& tRecord1 = vRecords[i - 1];
		auto& tRecord2 = vRecords[i];
		if (tRecord1.m_iMode != tRecord2.m_iMode)
			continue;

		float flYaw = 0.f;
		bool bResult = GetYawDifference(tRecord1, tRecord2, !iTicks, &flYaw, flStraightFuzzyValue, iMaxChanges, iMaxChangeTime, tStorage.m_MoveData.m_flMaxSpeed);
		SDK::Output("GetYawDifference", std::format("{} ({}): {}, {}", i, iTicks, flYaw, bResult).c_str(), { 50, 127, 75 }, Vars::Debug::Logging.Value);
		if (!bResult)
			break;

		flAverageYaw += flYaw;
		iTicks += std::max(TIME_TO_TICKS(tRecord1.m_flSimTime - tRecord2.m_flSimTime), 1);
		iTicks2++;
	}
	if (iTicks <= 3) // valid strafes not high enough
		return;

	SDK::Output("Ticks", std::format("{}, {}", iTicks, iTicks2).c_str(), { 150, 255, 175 }, Vars::Debug::Logging.Value);

	int iMinimum = 1;
	if (pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
	{
		float flDistance = 0.f;
		if (auto pLocal = H::Entities.GetLocal())
			flDistance = pLocal->m_vecOrigin().DistTo(tStorage.m_pPlayer->m_vecOrigin());
		iMinimum = flDistance < flLowMinimumDistance ? flLowMinimumSamples : Math::RemapVal(flDistance, flLowMinimumDistance, flHighMinimumDistance, flLowMinimumSamples + 1, flHighMinimumSamples);
	}

	flAverageYaw /= std::max(iTicks, iMinimum);
	tStorage.m_flAverageYaw = flAverageYaw;

	SDK::Output("MovementSimulation", std::format("flAverageYaw calculated to {} from {} ({}) {}", flAverageYaw, iTicks, iMinimum, pPlayer->entindex() == I::EngineClient->GetLocalPlayer() ? "(local)" : "").c_str(), { 100, 255, 150 }, Vars::Debug::Logging.Value);
}

bool CMovementSimulation::StrafePrediction(PlayerStorage& tStorage, int iSamples)
{
	if (tStorage.m_bDirectMove
		? !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Ground)
		: !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Air))
		return false;

	GetAverageYaw(tStorage, iSamples);
	return true;
}

bool CMovementSimulation::SetDuck(PlayerStorage& tStorage, bool bDuck) // this only touches origin, bounds
{
	if (bDuck == tStorage.m_pPlayer->m_bDucked())
		return true;

	auto pGameRules = I::TFGameRules();
	auto pViewVectors = pGameRules ? pGameRules->GetViewVectors() : nullptr;
	float flScale = tStorage.m_pPlayer->m_flModelScale();

	if (!tStorage.m_pPlayer->IsOnGround())
	{
		Vec3 vHullMins = (pViewVectors ? pViewVectors->m_vHullMin : Vec3(-24, -24, 0)) * flScale;
		Vec3 vHullMaxs = (pViewVectors ? pViewVectors->m_vHullMax : Vec3(24, 24, 82)) * flScale;
		Vec3 vDuckHullMins = (pViewVectors ? pViewVectors->m_vDuckHullMin : Vec3(-24, -24, 0)) * flScale;
		Vec3 vDuckHullMaxs = (pViewVectors ? pViewVectors->m_vDuckHullMax : Vec3(24, 24, 62)) * flScale;

		if (bDuck)
			tStorage.m_MoveData.m_vecAbsOrigin += (vHullMaxs - vHullMins) - (vDuckHullMaxs - vDuckHullMins);
		else
		{
			Vec3 vOrigin = tStorage.m_MoveData.m_vecAbsOrigin - ((vHullMaxs - vHullMins) - (vDuckHullMaxs - vDuckHullMins));

			CGameTrace trace = {};
			CTraceFilterWorldAndPropsOnly filter = {};
			SDK::TraceHull(vOrigin, vOrigin, vHullMins, vHullMaxs, tStorage.m_pPlayer->SolidMask(), &filter, &trace);
			if (trace.DidHit())
				return false;

			tStorage.m_MoveData.m_vecAbsOrigin = vOrigin;
		}
	}
	tStorage.m_pPlayer->m_bDucked() = bDuck;

	return true;
}

void CMovementSimulation::SetBounds(CTFPlayer* pPlayer)
{
	if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer())
		return;

	// fixes issues with origin tolerance
	if (auto pGameRules = I::TFGameRules())
	{
		if (auto pViewVectors = pGameRules->GetViewVectors())
		{
			pViewVectors->m_vHullMin = Vec3(-24, -24, 0) + 0.125f;
			pViewVectors->m_vHullMax = Vec3(24, 24, 82) - 0.125f;
			pViewVectors->m_vDuckHullMin = Vec3(-24, -24, 0) + 0.125f;
			pViewVectors->m_vDuckHullMax = Vec3(24, 24, 62) - 0.125f;
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

void CMovementSimulation::RunTick(PlayerStorage& tStorage, bool bPath, std::function<void(CMoveData&)>* pCallback)
{
	if (tStorage.m_bFailed || !tStorage.m_pPlayer || !tStorage.m_pPlayer->IsPlayer())
		return;

	if (bPath)
		tStorage.m_vPath.push_back(tStorage.m_MoveData.m_vecAbsOrigin);

	// make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
	SetBounds(tStorage.m_pPlayer);

	float flCorrection = 0.f;
	if (tStorage.m_flAverageYaw)
	{
		float flYaw = tStorage.m_flAverageYaw;

		float flMult = 1.f;
		if (!tStorage.m_bDirectMove)
		{
			if (!tStorage.m_pPlayer->InCond(TF_COND_SHIELD_CHARGE))
				flCorrection = 90.f * sign(flYaw);

			float flPrevVelZ = tStorage.m_MoveData.m_vecVelocity.z + GetPlayerGravity(tStorage.m_pPlayer) * TICK_INTERVAL;
			flMult = 0.f < flPrevVelZ && flPrevVelZ <= 250.f ? 0.25f : 1.f; // flEntFriction will be 0.25f if flPrevVelZ is between 0.f and 250.f
		}
		tStorage.m_MoveData.m_vecViewAngles.y += flYaw * flMult + flCorrection;
	}
	else if (!tStorage.m_bDirectMove)
		tStorage.m_MoveData.m_flForwardMove = tStorage.m_MoveData.m_flSideMove = 0.f;

	float flOldSpeed = tStorage.m_MoveData.m_flClientMaxSpeed;
	if (tStorage.m_pPlayer->m_bDucked() && tStorage.m_pPlayer->IsOnGround() && tStorage.m_pPlayer->m_nWaterLevel() < 2)
		tStorage.m_MoveData.m_flClientMaxSpeed /= 3;

	if (tStorage.m_bBunnyHop && tStorage.m_pPlayer->IsOnGround() && !tStorage.m_pPlayer->m_bDucked())
	{
		tStorage.m_MoveData.m_nOldButtons = 0;
		tStorage.m_MoveData.m_nButtons |= IN_JUMP;
	}

	I::GameMovement->ProcessMovement(tStorage.m_pPlayer, &tStorage.m_MoveData);
	if (pCallback)
		(*pCallback)(tStorage.m_MoveData);

	tStorage.m_MoveData.m_flClientMaxSpeed = flOldSpeed;

	tStorage.m_flSimTime += TICK_INTERVAL;
	tStorage.m_bPredictNetworked = tStorage.m_flSimTime >= tStorage.m_flPredictedSimTime;
	if (tStorage.m_bPredictNetworked)
	{
		tStorage.m_vPredictedOrigin = tStorage.m_MoveData.m_vecAbsOrigin;
		tStorage.m_flPredictedSimTime += tStorage.m_flPredictedDelta;
	}
	bool bLastbDirectMove = tStorage.m_bDirectMove;
	tStorage.m_bDirectMove = tStorage.m_pPlayer->IsOnGround() || tStorage.m_pPlayer->m_nWaterLevel() >= 2;

	if (tStorage.m_flAverageYaw)
		tStorage.m_MoveData.m_vecViewAngles.y -= flCorrection;
	else if (tStorage.m_bDirectMove && !bLastbDirectMove
		&& !tStorage.m_MoveData.m_flForwardMove && !tStorage.m_MoveData.m_flSideMove
		&& !(tStorage.m_MoveData.m_vecVelocity * Vec3(1, 1, 0)).IsZero())
	{
		float flMaxSpeed = SDK::MaxSpeed(tStorage.m_pPlayer);
		Vec3 vDirection = (tStorage.m_MoveData.m_vecVelocity * Vec3(1, 1, 0)).Normalized() * flMaxSpeed;
		Vec3 vMove = Math::RotatePoint(vDirection, {}, { 0, -tStorage.m_MoveData.m_vecViewAngles.y, 0 });
		tStorage.m_MoveData.m_flForwardMove = vMove.x;
		tStorage.m_MoveData.m_flSideMove = -vMove.y;
	}

	RestoreBounds(tStorage.m_pPlayer);
}

void CMovementSimulation::RunTick(PlayerStorage& tStorage, bool bPath, std::function<void(CMoveData&)> fCallback)
{
	RunTick(tStorage, bPath, &fCallback);
}

void CMovementSimulation::Restore(PlayerStorage& tStorage)
{
	if (tStorage.m_bInitFailed || !tStorage.m_pPlayer)
		return;

	I::MoveHelper->SetHost(nullptr);
	tStorage.m_pPlayer->SetCurrentCmd(nullptr);

	Reset(tStorage);

	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
	I::GlobalVars->frametime = m_flOldFrametime;

	/*
	const bool bInitFailed = tStorage.m_bInitFailed, bFailed = tStorage.m_bFailed;
	memset(&tStorage, 0, sizeof(PlayerStorage));
	tStorage.m_bInitFailed = bInitFailed, tStorage.m_bFailed = bFailed;
	*/
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