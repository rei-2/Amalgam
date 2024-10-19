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



/*
inline float SolveCubic(float b, float c, float d)
{
	float p = c - powf(b, 2) / 3;
	float q = 2 * powf(b, 3) / 27 - b * c / 3 + d;

	if (p == 0.f)
		return pow(q, 1.f / 3);
	if (q == 0.f)
		return 0.f;

	float t = sqrt(fabs(p) / 3);
	float g = q / (2.f / 3) / (p * t);
	if (p > 0.f)
		return -2 * t * sinh(asinh(g) / 3) - b / 3;

	if (4 * powf(p, 3) + 27 * powf(q, 2) < 0.f)
		return 2 * t * cos(acos(g) / 3) - b / 3;
	if (q > 0.f)
		return -2 * t * cosh(acosh(-g) / 3) - b / 3;
	return 2 * t * cosh(acosh(g) / 3) - b / 3;
}
inline std::vector<float> SolveQuartic(float b, float c, float d, float e)
{
	std::vector<float> vRoots = {};

	float p = c - powf(b, 2) / (8.f / 3);
	float q = powf(b, 3) / 8 - b * c / 2 + d;
	float m = SolveCubic(p, powf(p, 2) / 4 + powf(b, 4) / (256.f / 3) - e + b * d / 4 - powf(b, 2) * c / 16, -powf(q, 2) / 8);
	if (m < 0.f)
		return vRoots;

	float sqrt_2m = sqrt(2 * m);
	if (q == 0.f)
	{
		if (-m - p > 0.f)
		{
			float flDelta = sqrt(2 * (-m - p));
			vRoots.push_back(-b / 4 + (sqrt_2m - flDelta) / 2);
			vRoots.push_back(-b / 4 - (sqrt_2m - flDelta) / 2);
			vRoots.push_back(-b / 4 + (sqrt_2m + flDelta) / 2);
			vRoots.push_back(-b / 4 - (sqrt_2m + flDelta) / 2);
		}
		if (-m - p == 0.f)
		{
			vRoots.push_back(-b / 4 - sqrt_2m / 2);
			vRoots.push_back(-b / 4 + sqrt_2m / 2);
		}
	}
	else
	{
		if (-m - p + q / sqrt_2m >= 0.f)
		{
			float flDelta = sqrt(2 * (-m - p + q / sqrt_2m));
			vRoots.push_back((-sqrt_2m + flDelta) / 2 - b / 4);
			vRoots.push_back((-sqrt_2m - flDelta) / 2 - b / 4);
		}
		if (-m - p - q / sqrt_2m >= 0.f)
		{
			float flDelta = sqrt(2 * (-m - p - q / sqrt_2m));
			vRoots.push_back((sqrt_2m + flDelta) / 2 - b / 4);
			vRoots.push_back((sqrt_2m - flDelta) / 2 - b / 4);
		}
	}
	return vRoots;
}
*/

void CMovementSimulation::Store()
{
	if (Vars::Aimbot::Projectile::StrafePrediction.Value)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			auto& vRecords = mRecords[pPlayer->entindex()];

			bool bLocal = pPlayer->entindex() == I::EngineClient->GetLocalPlayer();
			Vec3 vVelocity = bLocal ? F::EnginePrediction.vVelocity : pPlayer->m_vecVelocity();
			Vec3 vOrigin = bLocal ? F::EnginePrediction.vOrigin : pPlayer->m_vecOrigin();
			Vec3 vDirection = bLocal ? Math::RotatePoint(F::EnginePrediction.vDirection, {}, { 0, F::EnginePrediction.vAngles.y, 0 }) : Vec3(pPlayer->m_vecVelocity().x, pPlayer->m_vecVelocity().y, 0.f);
			Vec3 vAngles = bLocal ? F::EnginePrediction.vAngles : pPlayer->GetEyeAngles();

			if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant() || vVelocity.IsZero())
			{
				vRecords.clear();
				continue;
			}
			else if (!H::Entities.GetDeltaTime(pPlayer))
				continue;

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
					if (trace.DidHit())
					{
						pRecord = nullptr;
						vRecords.clear();
					}
				}
			}

			{
				//bool bSet = false; // debug

				static auto sv_accelerate = U::ConVars.FindVar("sv_accelerate");
				static auto sv_friction = U::ConVars.FindVar("sv_friction");
				static auto sv_stopspeed = U::ConVars.FindVar("sv_stopspeed");

				float flAccel = sv_accelerate->GetFloat();
				float flFriction = sv_friction->GetFloat();
				float flStopSpeed = sv_stopspeed->GetFloat();

				float flMaxSpeed = pPlayer->TeamFortress_CalculateMaxSpeed() / (pPlayer->IsDucking() ? 3 : 1);
				float flEntFriction = 1.f;

				//if (bLocal)
				{
					if (tRecord.m_iMode != 0)
						tRecord.m_vDirection = Vec3(pPlayer->m_vecVelocity().x, pPlayer->m_vecVelocity().y, 0).GetNorm() * flMaxSpeed;
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
									auto vRoots = SolveQuartic(flB / flA, flC / flA, flD / flA, flE / flA);
									if (vRoots.size())
										flMult = std::clamp(*std::max_element(vRoots.begin(), vRoots.end()), 1.f, flMult);
								}
								vNewVelocity *= flMult;

								vWishDir = vNewVelocity - vOldVelocity;
								flAccelSpeed = vWishDir.Normalize();
							}
						}

						SDK::Output("flAccelSpeed", std::format("{}", flAccelSpeed).c_str());
						flAccelSpeed = flAccelSpeed < (flAccel * flMaxSpeed * flEntFriction * TICK_INTERVAL) * 0.35f ? 0.f : flMaxSpeed; // this might be a bad solution?

						bSet = true;
						tRecord.m_vDirection = vWishDir * flAccelSpeed;
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

				//G::LineStorage.push_back({ { tRecord.m_vOrigin, tRecord.m_vOrigin + tRecord.m_vDirection }, I::GlobalVars->curtime + 0.5f, bSet ? Color_t(SDK::RandomInt(200, 255), SDK::RandomInt(200, 255), SDK::RandomInt(200, 255), 255) : Color_t(0, 0, 0, 255) });
			}
			
			
			vRecords.push_front(tRecord);
			if (vRecords.size() > 66)
				vRecords.pop_back();
		}
	}
	else
		mRecords.clear();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		auto& vSimTimes = mSimTimes[pPlayer->entindex()];

		if (pEntity == H::Entities.GetLocal() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant())
		{
			vSimTimes.clear();
			continue;
		}

		float flDeltaTime = H::Entities.GetDeltaTime(pPlayer);
		if (!flDeltaTime)
			continue;

		vSimTimes.push_front(flDeltaTime);
		if (vSimTimes.size() > Vars::Aimbot::Projectile::DeltaCount.Value)
			vSimTimes.pop_back();
	}
}



bool CMovementSimulation::Initialize(CBaseEntity* pEntity, PlayerStorage& playerStorageOut, bool useHitchance, bool cancelStrafe)
{
	G::LineStorage.clear();
	if (!pEntity || !pEntity->IsPlayer() || !pEntity->As<CTFPlayer>()->IsAlive())
	{
		playerStorageOut.m_bInitFailed = playerStorageOut.m_bFailed = true;
		return false;
	}

	auto pPlayer = pEntity->As<CTFPlayer>();
	playerStorageOut.m_pPlayer = pPlayer;
	playerStorageOut.m_pPlayer->SetCurrentCmd(&DummyCmd);

	// store player restore data
	Store(playerStorageOut);

	// store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;

	// the hacks that make it work
	{
		if (pPlayer->m_fFlags() & FL_DUCKING)
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
			pPlayer->m_hGroundEntity() = nullptr; // fix for velocity.z being set to 0 even if in air

			pPlayer->m_vecBaseVelocity() = Vec3(); // residual basevelocity causes issues
			if (pPlayer->IsOnGround())
				pPlayer->m_vecVelocity().z = std::min(pPlayer->m_vecVelocity().z, 0.f); // step fix
		}

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
		const auto& vRecords = mRecords[playerStorageOut.m_pPlayer->entindex()];
		const auto iSamples = vRecords.size();

		float flCurrentChance = 1.f, flAverageYaw = 0.f;
		for (size_t i = 0; i < iSamples; i++)
		{
			if (vRecords.size() <= i + 2)
				break;

			const auto& pRecord1 = vRecords[i], &pRecord2 = vRecords[i + 1];
			const float flYaw1 = Math::VelocityToAngles(pRecord1.m_vVelocity).y, flYaw2 = Math::VelocityToAngles(pRecord2.m_vVelocity).y;
			const float flTime1 = pRecord1.m_flSimTime, flTime2 = pRecord2.m_flSimTime;

			float flYaw = (flYaw1 - flYaw2) / TIME_TO_TICKS(flTime1 - flTime2);
			flYaw = fmodf(flYaw + 180.f, 360.f);
			flYaw += flYaw < 0 ? 180.f : -180.f;
			flAverageYaw += flYaw;

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

	for (int i = 0; i < H::Entities.GetChoke(pPlayer); i++)
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
	if (playerStorage.m_PlayerData.m_fFlags & FL_DUCKING)
		playerStorage.m_MoveData.m_flMaxSpeed /= 3;

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

	playerStorage.m_MoveData.m_flClientMaxSpeed = playerStorage.m_MoveData.m_flMaxSpeed;
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

bool CMovementSimulation::GetYawDifference(const std::deque<MoveData>& vRecords, size_t i, float* flYaw, float flStraightFuzzyValue)
{
	if (vRecords.size() <= i + 2)
		return false;

	const auto& pRecord1 = vRecords[i], &pRecord2 = vRecords[i + 1];
	const float flYaw1 = Math::VelocityToAngles(pRecord1.m_vDirection).y, flYaw2 = Math::VelocityToAngles(pRecord2.m_vDirection).y;
	const float flTime1 = pRecord1.m_flSimTime, flTime2 = pRecord2.m_flSimTime;

	const int iTicks = std::max(TIME_TO_TICKS(flTime1 - flTime2), 1);
	*flYaw = (flYaw1 - flYaw2) / iTicks;
	*flYaw = fmodf(*flYaw + 180.f, 360.f);
	*flYaw += *flYaw < 0 ? 180.f : -180.f;

	static int iSign = 0;
	const int iLastSign = iSign;
	const int iCurSign = iSign = *flYaw > 0 ? 1 : -1;

	if (fabsf(*flYaw) * pRecord1.m_vVelocity.Length2D() * iTicks < flStraightFuzzyValue) // dumb way to get straight bool
		return false;

	return !i || iLastSign == iCurSign;
}

float CMovementSimulation::GetAverageYaw(PlayerStorage& playerStorage, const int iSamples)
{
	const auto& vRecords = mRecords[playerStorage.m_pPlayer->entindex()];

	bool bGround = playerStorage.m_pPlayer->IsOnGround();
	float flStraightFuzzyValue = bGround ? Vars::Aimbot::Projectile::GroundStraightFuzzyValue.Value : Vars::Aimbot::Projectile::AirStraightFuzzyValue.Value;

	float flAverageYaw = 0.f; size_t i = 0;
	for (; i < iSamples; i++)
	{
		float flYaw;
		if (!GetYawDifference(vRecords, i, &flYaw, flStraightFuzzyValue))
			break;
		flAverageYaw += flYaw;
	}
	if (i < 3) // valid strafes too low
		return 0.f;

	float flLowMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumDistance.Value : Vars::Aimbot::Projectile::AirLowMinimumDistance.Value;
	float flLowMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundLowMinimumSamples.Value : Vars::Aimbot::Projectile::AirLowMinimumSamples.Value;
	float flHighMinimumDistance = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumDistance.Value : Vars::Aimbot::Projectile::AirHighMinimumDistance.Value;
	float flHighMinimumSamples = bGround ? Vars::Aimbot::Projectile::GroundHighMinimumSamples.Value : Vars::Aimbot::Projectile::AirHighMinimumSamples.Value;

	float flDistance = 0.f;
	if (auto pLocal = H::Entities.GetLocal())
		flDistance = pLocal->m_vecOrigin().DistTo(playerStorage.m_pPlayer->m_vecOrigin());

	unsigned long long iMinimumStrafes = (flDistance < flLowMinimumDistance) ? flLowMinimumSamples : Math::RemapValClamped(flDistance, flLowMinimumDistance, flHighMinimumDistance, flLowMinimumSamples + 1, flHighMinimumSamples);
	flAverageYaw /= std::max(i, iMinimumStrafes);

	SDK::Output("MovementSimulation", std::format("flAverageYaw calculated to {} from {}", flAverageYaw, i).c_str(), { 100, 255, 150, 255 }, Vars::Debug::Logging.Value);

	return flAverageYaw;
}

bool CMovementSimulation::StrafePrediction(PlayerStorage& playerStorage, const int iSamples)
{
	if (playerStorage.m_pPlayer->IsOnGround()
		? !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Ground)
		: !(Vars::Aimbot::Projectile::StrafePrediction.Value & Vars::Aimbot::Projectile::StrafePredictionEnum::Air))
		return false;

	playerStorage.m_flAverageYaw = GetAverageYaw(playerStorage, iSamples);

	return true;
}

void CMovementSimulation::RunTick(PlayerStorage& playerStorage, bool bPath)
{
	if (playerStorage.m_bFailed || !playerStorage.m_pPlayer || !playerStorage.m_pPlayer->IsPlayer())
		return;

	if (bPath)
		playerStorage.m_vPath.push_back(playerStorage.m_MoveData.m_vecAbsOrigin);

	//make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;

	float flCorrection = 0.f;
	if (playerStorage.m_flAverageYaw)
	{
		if (!playerStorage.m_pPlayer->IsOnGround())
			flCorrection = 90.f * sign(playerStorage.m_flAverageYaw);

		playerStorage.m_MoveData.m_vecViewAngles.y += playerStorage.m_flAverageYaw + flCorrection;
	}
	//else
	//	playerStorage.m_MoveData.m_vecViewAngles.y = Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity).y;

	/*
	{
		Vec3 vMove = Vec3(playerStorage.m_MoveData.m_flForwardMove, -playerStorage.m_MoveData.m_flSideMove, playerStorage.m_MoveData.m_flUpMove);

		Vec3 v1 = playerStorage.m_MoveData.m_vecAbsOrigin;
		Vec3 v2 = v1 + (vMove.IsZero() ? Vec3(0, 0, 1) : Math::RotatePoint(vMove.GetNorm(), {}, { 0, playerStorage.m_MoveData.m_vecViewAngles.y, 0 })) * Vars::Visuals::Simulation::SeparatorLength.Value;

		G::LineStorage.push_back({ { v1, v2 }, I::GlobalVars->curtime + 5.f, Color_t(SDK::RandomInt(0, 255), SDK::RandomInt(0, 255), SDK::RandomInt(0, 255), 255) });
	}
	*/

	I::GameMovement->ProcessMovement(playerStorage.m_pPlayer, &playerStorage.m_MoveData);

	playerStorage.m_flSimTime += TICK_INTERVAL;
	playerStorage.m_bPredictNetworked = playerStorage.m_flSimTime >= playerStorage.m_flPredictedSimTime;
	if (playerStorage.m_bPredictNetworked)
	{
		playerStorage.m_vPredictedOrigin = playerStorage.m_MoveData.m_vecAbsOrigin;
		playerStorage.m_flPredictedSimTime += playerStorage.m_flPredictedDelta;
	}

	playerStorage.m_MoveData.m_vecViewAngles.y -= flCorrection;
}

void CMovementSimulation::Restore(PlayerStorage& playerStorage)
{
	if (playerStorage.m_bInitFailed || !playerStorage.m_pPlayer)
		return;

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