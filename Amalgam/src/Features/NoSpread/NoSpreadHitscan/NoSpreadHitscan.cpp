#include "NoSpreadHitscan.h"

#include "../../TickHandler/TickHandler.h"
#include <regex>
#include <numeric>

void CNoSpreadHitscan::Reset()
{
	m_bWaitingForPlayerPerf = false;
	m_flServerTime = 0.f;
	m_vTimeDeltas.clear();
	m_dTimeDelta = 0.0;

	m_iSeed = 0;
	m_flMantissaStep = 0.f;

	m_bSynced = false;
}

bool CNoSpreadHitscan::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bCreateMove)
{
	if (G::PrimaryWeaponType != EWeaponType::HITSCAN)
		return false;

	if (pWeapon->GetWeaponSpread() <= 0.f)
		return false;

	return bCreateMove ? G::Attacking == 1 : true;
}

int CNoSpreadHitscan::GetSeed(CUserCmd* pCmd)
{
	static auto sv_usercmd_custom_random_seed = U::ConVars.FindVar("sv_usercmd_custom_random_seed");
	if (sv_usercmd_custom_random_seed ? sv_usercmd_custom_random_seed->GetBool() : true)
	{
		double dFloatTime = SDK::PlatFloatTime() + m_dTimeDelta;
		float flTime = float(dFloatTime * 1000.0);
		return std::bit_cast<int32_t>(flTime) & 255;
	}
	else
		return pCmd->random_seed; // i don't think this is right
}

float CNoSpreadHitscan::CalcMantissaStep(float val)
{
	// Calculate the delta to the next representable value
	float nextValue = std::nextafter(val, std::numeric_limits<float>::infinity());
	float mantissaStep = (nextValue - val) * 1000;

	// Get the closest mantissa (next power of 2)
	return powf(2, ceilf(logf(mantissaStep) / logf(2)));
}

std::string CNoSpreadHitscan::GetFormat(int m_ServerTime)
{
	int iDays = m_ServerTime / 86400;
	int iHours = m_ServerTime / 3600 % 24;
	int iMinutes = m_ServerTime / 60 % 60;
	int iSeconds = m_ServerTime % 60;

	if (iDays)
		return std::format("{}d {}h", iDays, iHours);
	else if (iHours)
		return std::format("{}h {}m", iHours, iMinutes);
	else
		return std::format("{}m {}s", iMinutes, iSeconds);
}

void CNoSpreadHitscan::AskForPlayerPerf()
{
	if (!Vars::Aimbot::General::NoSpread.Value || !I::EngineClient->IsInGame())
		return Reset();

	static Timer playerperfTimer{};
	if (playerperfTimer.Run(50) && !m_bWaitingForPlayerPerf)
	{
		I::ClientState->SendStringCmd("playerperf");
		m_bWaitingForPlayerPerf = true;
		m_dRequestTime = SDK::PlatFloatTime();
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
		m_bWaitingForPlayerPerf = false;

		// credits to kgb for idea
		float flNewServerTime = std::stof(matches[1].str());
		if (flNewServerTime < m_flServerTime)
			return true;

		m_flServerTime = flNewServerTime;

		double dRecieveTime = SDK::PlatFloatTime();
		double dResponseTime = dRecieveTime - m_dRequestTime;

		m_vTimeDeltas.push_back(m_flServerTime - dRecieveTime + dResponseTime);
		while (!m_vTimeDeltas.empty() && m_vTimeDeltas.size() > Vars::Aimbot::General::NoSpreadAverage.Value)
			m_vTimeDeltas.pop_front();
		m_dTimeDelta = std::reduce(m_vTimeDeltas.begin(), m_vTimeDeltas.end()) / m_vTimeDeltas.size() + TICKS_TO_TIME(Vars::Aimbot::General::NoSpreadOffset.Value);

		float flMantissaStep = CalcMantissaStep(m_flServerTime);
		m_bSynced = flMantissaStep >= 1.f;

		if (flMantissaStep > m_flMantissaStep && (m_bSynced || !m_flMantissaStep))
		{
			SDK::Output("Seed Prediction", m_bSynced ? std::format("Synced ({})", m_dTimeDelta).c_str() : "Not synced, step too low", Vars::Menu::Theme::Accent.Value);
			SDK::Output("Seed Prediction", std::format("Age {}; Step {}", GetFormat(m_flServerTime), flMantissaStep).c_str(), Vars::Menu::Theme::Accent.Value);
		}
		m_flMantissaStep = flMantissaStep;

		return true;
	}

	return std::regex_match(msg, std::regex(R"(\d+.\d+\s\d+\s\d+)"));
}

void CNoSpreadHitscan::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	m_iSeed = GetSeed(pCmd);
	if (!m_bSynced || !ShouldRun(pLocal, pWeapon, true))
		return;

	// credits to cathook for average spread stuff
	float flSpread = pWeapon->GetWeaponSpread();
	int iBulletsPerShot = pWeapon->GetBulletsPerShot();
	float flFireRate = std::ceilf(pWeapon->GetFireRate() / TICK_INTERVAL) * TICK_INTERVAL;

	std::vector<Vec3> vBulletCorrections = {};
	Vec3 vAverageSpread = {};
	for (int iBullet = 0; iBullet < iBulletsPerShot; iBullet++)
	{
		SDK::RandomSeed(m_iSeed + iBullet);

		if (!iBullet) // check if we'll get a guaranteed perfect shot
		{
			// if we are doubletapping and firerate is fast enough, prioritize later bullets
			int iTicks = F::Ticks.GetTicks();
			if (!iTicks || iTicks < TIME_TO_TICKS(flFireRate) * 2)
			{
				float flTimeSinceLastShot = TICKS_TO_TIME(pLocal->m_nTickBase()) - pWeapon->m_flLastFireTime();
				if ((iBulletsPerShot == 1 && flTimeSinceLastShot > 1.25f) || (iBulletsPerShot > 1 && flTimeSinceLastShot > 0.25f))
					return;
			}
		}

		const float x = SDK::RandomFloat(-0.5f, 0.5f) + SDK::RandomFloat(-0.5f, 0.5f);
		const float y = SDK::RandomFloat(-0.5f, 0.5f) + SDK::RandomFloat(-0.5f, 0.5f);

		Vec3 vForward, vRight, vUp; Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, &vUp);
		Vec3 vFixedSpread = vForward + (vRight * x * flSpread) + (vUp * y * flSpread);
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

void CNoSpreadHitscan::Draw(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::SeedPrediction) || !pLocal || !pLocal->IsAlive() || !Vars::Aimbot::General::NoSpread.Value)
		return;

	{
		auto pWeapon = H::Entities.GetWeapon();
		if (!pWeapon || !ShouldRun(pLocal, pWeapon))
			return;
	}

	int x = Vars::Menu::SeedPredictionDisplay.Value.x;
	int y = Vars::Menu::SeedPredictionDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(1);

	EAlign align = ALIGN_TOP;
	if (x <= 100 + H::Draw.Scale(50, Scale_Round))
	{
		x -= H::Draw.Scale(42, Scale_Round);
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - 100 - H::Draw.Scale(50, Scale_Round))
	{
		x += H::Draw.Scale(42, Scale_Round);
		align = ALIGN_TOPRIGHT;
	}

	const auto& cColor = m_bSynced ? Vars::Menu::Theme::Active.Value : Vars::Menu::Theme::Inactive.Value;

	H::Draw.StringOutlined(fFont, x, y, cColor, Vars::Menu::Theme::Background.Value, align, std::format("Uptime {}", GetFormat(m_flServerTime)).c_str());
	H::Draw.StringOutlined(fFont, x, y += nTall, cColor, Vars::Menu::Theme::Background.Value, align, std::format("Mantissa step {}", m_flMantissaStep).c_str());
}