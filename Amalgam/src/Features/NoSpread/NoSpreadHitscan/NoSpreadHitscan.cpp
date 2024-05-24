#include "NoSpreadHitscan.h"

#include "../../TickHandler/TickHandler.h"
#include <regex>
#include <numeric>

void CNoSpreadHitscan::Reset(bool bResetPrint)
{
	bWaitingForPlayerPerf = false;
	flServerTime = 0.f;
	flFloatTimeDelta = 0.f;

	iSeed = 0;
	flMantissaStep = 0;

	bSynced = false;
	if (bResetPrint)
		iBestSync = 0;
}

bool CNoSpreadHitscan::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bCreateMove)
{
	if (G::WeaponType != EWeaponType::HITSCAN)
		return false;

	if (pWeapon->GetWeaponSpread() <= 0.f)
		return false;

	return bCreateMove ? G::IsAttacking : true;
}

int CNoSpreadHitscan::GetSeed(CUserCmd* pCmd)
{
	static auto sv_usercmd_custom_random_seed = U::ConVars.FindVar("sv_usercmd_custom_random_seed");
	if (sv_usercmd_custom_random_seed ? sv_usercmd_custom_random_seed->GetBool() : true)
	{
		const float flFloatTime = float(SDK::PlatFloatTime()) + flFloatTimeDelta;
		//SDK::Output("Seed Prediction", std::format("{}\n", flFloatTime).c_str());

		const float flTime = (flFloatTime) * 1000;
		return std::bit_cast<int32_t>(flTime) & 255;
	}
	else
		return pCmd->random_seed; // i don't think this is right
}

float CNoSpreadHitscan::CalcMantissaStep(float val)
{
	// Calculate the delta to the next representable value
	const float nextValue = std::nextafter(val, std::numeric_limits<float>::infinity());
	const float mantissaStep = (nextValue - val) * 1000;

	// Get the closest mantissa (next power of 2)
	return powf(2, ceilf(logf(mantissaStep) / logf(2)));
}

std::string CNoSpreadHitscan::GetFormat(int m_ServerTime)
{
	const int iDays = m_ServerTime / 86400;
	const int iHours = m_ServerTime / 3600 % 24;
	const int iMinutes = m_ServerTime / 60 % 60;
	const int iSeconds = m_ServerTime % 60;

	if (iDays)
		return std::format("{}d {}h", iDays, iHours);
	else if (iHours)
		return std::format("{}h {}m", iHours, iMinutes);
	else
		return std::format("{}m {}s", iMinutes, iSeconds);
}

void CNoSpreadHitscan::AskForPlayerPerf()
{
	if (!Vars::Aimbot::General::NoSpread.Value)
		return Reset();

	static Timer playerperfTimer{};
	if (playerperfTimer.Run(50) && !bWaitingForPlayerPerf && I::EngineClient->IsInGame())
	{
		I::EngineClient->ClientCmd_Unrestricted("playerperf");
		bWaitingForPlayerPerf = true;
	}
}

bool CNoSpreadHitscan::ParsePlayerPerf(bf_read& msgData)
{
	if (!Vars::Aimbot::General::NoSpread.Value)
		return false;

	char rawMsg[256] = {};

	msgData.ReadString(rawMsg, sizeof(rawMsg), true);
	msgData.Seek(0);

	std::string msg(rawMsg);
	msg.erase(msg.begin());

	std::smatch matches = {};
	std::regex_match(msg, matches, std::regex(R"((\d+.\d+)\s\d+\s\d+\s\d+.\d+\s\d+.\d+\svel\s\d+.\d+)"));

	if (matches.size() == 2)
	{
		bWaitingForPlayerPerf = false;

		// credits to kgb for idea
		const float flNewServerTime = std::stof(matches[1].str());
		if (flNewServerTime < flServerTime)
			return true;

		flServerTime = flNewServerTime;
		flFloatTimeDelta = flServerTime - float(SDK::PlatFloatTime());

		flMantissaStep = CalcMantissaStep(flServerTime);
		const int iSynced = flMantissaStep < 4.f ? 2 : 1;
		bSynced = iSynced == 1;

		if (!iBestSync || iBestSync == 2 && bSynced)
		{
			iBestSync = iSynced;
			SDK::Output("Seed Prediction", bSynced ? std::format("Synced ({})", flFloatTimeDelta).c_str() : "Not synced, step too low", Vars::Menu::Theme::Accent.Value);
			SDK::Output("Seed Prediction", std::format("Age {}; Step {}", GetFormat(flServerTime), CalcMantissaStep(flServerTime)).c_str(), Vars::Menu::Theme::Accent.Value);
		}

		return true;
	}

	return std::regex_match(msg, std::regex(R"(\d+.\d+\s\d+\s\d+)"));
}

void CNoSpreadHitscan::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	iSeed = GetSeed(pCmd);
	if (!bSynced || !ShouldRun(pLocal, pWeapon, true))
		return;

	// credits to cathook for average spread stuff
	const float flSpread = pWeapon->GetWeaponSpread();
	auto tfWeaponInfo = pWeapon->GetWeaponInfo();
	int iBulletsPerShot = tfWeaponInfo ? tfWeaponInfo->GetWeaponData(0).m_nBulletsPerShot : 1;
	iBulletsPerShot = static_cast<int>(SDK::AttribHookValue(static_cast<float>(iBulletsPerShot), "mult_bullets_per_shot", pWeapon));

	std::vector<Vec3> vBulletCorrections = {};
	Vec3 vAverageSpread = {};
	for (int iBullet = 0; iBullet < iBulletsPerShot; iBullet++)
	{
		SDK::RandomSeed(iSeed + iBullet);

		if (!iBullet) // Check if we'll get a guaranteed perfect shot
		{
			bool bDoubletap = false; // if we are doubletapping and firerate fast enough, prioritize later bullets
			int iTicks = F::Ticks.GetTicks(pLocal);
			if (iTicks && tfWeaponInfo)
			{
				float flDoubletapTime = TICKS_TO_TIME(iTicks);
				float flFireRate = tfWeaponInfo->GetWeaponData(0).m_flTimeFireDelay;
				bDoubletap = flDoubletapTime > flFireRate * 2;
			}

			if (!bDoubletap)
			{
				const float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();
				if ((iBulletsPerShot == 1 && flTimeSinceLastShot > 1.25f) || (iBulletsPerShot > 1 && flTimeSinceLastShot > 0.25f))
					return;
			}
		}

		const float x = SDK::RandomFloat(-0.5f, 0.5f) + SDK::RandomFloat(-0.5f, 0.5f);
		const float y = SDK::RandomFloat(-0.5f, 0.5f) + SDK::RandomFloat(-0.5f, 0.5f);

		Vec3 forward, right, up;
		Math::AngleVectors(pCmd->viewangles, &forward, &right, &up);

		Vec3 vFixedSpread = forward + (right * x * flSpread) + (up * y * flSpread);
		vFixedSpread.Normalize();
		vAverageSpread += vFixedSpread;

		vBulletCorrections.push_back(vFixedSpread);
	}
	vAverageSpread /= static_cast<float>(iBulletsPerShot);

	const auto cFixedSpread = std::ranges::min_element(vBulletCorrections,
		[&](const Vec3& lhs, const Vec3& rhs)
		{
			return lhs.DistTo(vAverageSpread) < rhs.DistTo(vAverageSpread);
		});

	if (cFixedSpread == vBulletCorrections.end())
		return;

	Vec3 vFixedAngles{};
	Math::VectorAngles(*cFixedSpread, vFixedAngles);

	pCmd->viewangles += pCmd->viewangles - vFixedAngles;
	Math::ClampAngles(pCmd->viewangles);

	G::SilentAngles = true;
}