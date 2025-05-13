#include "PlayerArrows.h"

void CPlayerArrows::DrawArrowTo(const Vec3& vFromPos, const Vec3& vToPos, Color_t tColor)
{
	float flMaxDistance = Vars::ESP::FOVArrows::MaxDistance.Value;
	float flMap = Math::RemapVal(vFromPos.DistTo(vToPos), flMaxDistance, flMaxDistance * 0.9f, 0.f, 1.f);
	tColor.a = byte(flMap * 255.f);
	if (!tColor.a)
		return;

	Vec2 vCenter = { H::Draw.m_nScreenW / 2.f, H::Draw.m_nScreenH / 2.f };
	Vec3 vScreenPos;
	if (SDK::W2S(vToPos, vScreenPos, true))
	{
		float flMin = std::min(vCenter.x, vCenter.y); float flMax = std::max(vCenter.x, vCenter.y);
		float flDist = sqrt(powf(vScreenPos.x - vCenter.x, 2) + powf(vScreenPos.y - vCenter.y, 2));
		float flTransparency = 1.f - std::clamp((flDist - flMin) / (flMin != flMax ? flMax - flMin : 1), 0.f, 1.f);
		tColor.a = byte(std::max(float(tColor.a) - flTransparency * 255, 0.f));
		if (!tColor.a)
			return;
	}

	Vec3 vAngle; Math::VectorAngles({ vCenter.x - vScreenPos.x, vCenter.y - vScreenPos.y, 0 }, vAngle);
	const float flDeg = DEG2RAD(vAngle.y);
	const float flCos = cos(flDeg);
	const float flSin = sin(flDeg);

	float flOffset = -Vars::ESP::FOVArrows::Offset.Value;
	float flScale = H::Draw.Scale(25);
	Vec2 v1 = { flOffset, flScale / 2 },
		v2 = { flOffset, -flScale / 2 },
		v3 = { flOffset - flScale * sqrt(3.f) / 2, 0 };
	H::Draw.FillPolygon(
		{
			{ { vCenter.x + v1.x * flCos - v1.y * flSin, vCenter.y + v1.y * flCos + v1.x * flSin } },
			{ { vCenter.x + v2.x * flCos - v2.y * flSin, vCenter.y + v2.y * flCos + v2.x * flSin } },
			{ { vCenter.x + v3.x * flCos - v3.y * flSin, vCenter.y + v3.y * flCos + v3.x * flSin } }
		}, tColor
	);
}

void CPlayerArrows::Run(CTFPlayer* pLocal)
{
	if (!Vars::ESP::FOVArrows::Enabled.Value)
		return;

	const Vec3 vLocalPos = pLocal->GetEyePosition();
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->IsDormant() && !H::Entities.GetDormancy(pPlayer->entindex()) || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->InCond(TF_COND_STEALTHED))
			continue;

		Color_t tColor = H::Color.GetEntityDrawColor(pLocal, pEntity, Vars::Colors::Relative.Value);
		if (pPlayer->InCond(TF_COND_DISGUISED))
			tColor = Vars::Colors::Target.Value;

		DrawArrowTo(vLocalPos, pPlayer->GetCenter(), tColor);
	}
}
