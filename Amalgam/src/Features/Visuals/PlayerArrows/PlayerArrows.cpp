#include "PlayerArrows.h"

bool CPlayerArrows::ShouldRun(CTFPlayer* pLocal)
{
	if (!Vars::Visuals::FOVArrows::Enabled.Value)
		return false;

	if (!pLocal->IsAlive() || pLocal->IsStunned())
		return false;

	if (pLocal->IsInBumperKart() || pLocal->IsAGhost())
		return false;

	return true;
}

void CPlayerArrows::DrawArrowTo(const Vec3& vecFromPos, const Vec3& vecToPos, Color_t color)
{
	const auto cx = static_cast<float>(H::Draw.m_nScreenW / 2);
	const auto cy = static_cast<float>(H::Draw.m_nScreenH / 2);

	bool onScreen; Vec3 vScreenPos = {};
	{
		const matrix3x4& worldToScreen = H::Draw.m_WorldToProjection.As3x4();
		float w = worldToScreen[3][0] * vecToPos[0] + worldToScreen[3][1] * vecToPos[1] + worldToScreen[3][2] * vecToPos[2] + worldToScreen[3][3];
		vScreenPos.z = 0;

		onScreen = w > 0.f;
		float fl1DBw = 1 / abs(w);
		vScreenPos.x = cx + (0.5 * ((worldToScreen[0][0] * vecToPos[0] + worldToScreen[0][1] * vecToPos[1] + worldToScreen[0][2] * vecToPos[2] + worldToScreen[0][3]) * fl1DBw) * cx + 0.5);
		vScreenPos.y = cy - (0.5 * ((worldToScreen[1][0] * vecToPos[0] + worldToScreen[1][1] * vecToPos[1] + worldToScreen[1][2] * vecToPos[2] + worldToScreen[1][3]) * fl1DBw) * cy + 0.5);
	}

	/*
	if (onScreen && !(vScreenPos.x < 0 ||
		vScreenPos.x > H::Draw.m_nScreenW ||
		vScreenPos.y < 0 ||
		vScreenPos.y > H::Draw.m_nScreenH))
		return;
	*/

	if (onScreen)
	{
		float minc = std::min(cx, cy); float maxc = std::max(cx, cy);
		float dist = sqrt(powf(vScreenPos.x - cx, 2) + powf(vScreenPos.y - cy, 2)) * 2;
		float transparency;
		if (minc != maxc)
			transparency = 1 - std::clamp((dist - minc) / (maxc - minc), 0.f, 1.f);
		else
			transparency = 1 - std::clamp(dist - minc, 0.f, 1.f);
		color.a = static_cast<byte>(std::max(static_cast<float>(color.a) - transparency * 255.f, 0.f));
	}

	if (color.a == 0)
		return;

	Vec2
		p1 = { static_cast<float>(-Vars::Visuals::FOVArrows::Offset.Value), 12.5f },
		p2 = { static_cast<float>(-Vars::Visuals::FOVArrows::Offset.Value), -12.5f },
		p3 = { static_cast<float>(-Vars::Visuals::FOVArrows::Offset.Value) - 25.f * sqrt(3.f) / 2.f, 0.f };

	auto angle = Vec3();
	Math::VectorAngles(Vec3(cx - vScreenPos.x, cy - vScreenPos.y, 0), angle);
	const float deg = DEG2RAD(angle.y);
	const float _cos = cos(deg);
	const float _sin = sin(deg);

	Vertex_t t1, t2, t3;
		t1.Init({ cx + p1.x * _cos - p1.y * _sin, cy + p1.y * _cos + p1.x * _sin });
		t2.Init({ cx + p2.x * _cos - p2.y * _sin, cy + p2.y * _cos + p2.x * _sin });
		t3.Init({ cx + p3.x * _cos - p3.y * _sin, cy + p3.y * _cos + p3.x * _sin });
	std::array<Vertex_t, 3> verts{ t1, t2, t3 };
	H::Draw.Polygon(3, verts.data(), color);
}

void CPlayerArrows::Run(CTFPlayer* pLocal)
{
	if (!ShouldRun(pLocal))
		return;

	const Vec3 vLocalPos = pLocal->GetEyePosition();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer->IsAlive() || pPlayer->IsCloaked() || pPlayer->IsAGhost())
			continue;

		Vec3 vEnemyPos = pPlayer->GetCenter();

		Color_t color;
		if (!Vars::Colors::Relative.Value)
		{
			if (pLocal->m_iTeamNum() == 2)
				color = Vars::Colors::TeamBlu.Value;
			else
				color = Vars::Colors::TeamRed.Value;
		}
		else
			color = Vars::Colors::Enemy.Value;
		if (pPlayer->InCond(TF_COND_DISGUISED))
			color = Vars::Colors::Cloak.Value;

		auto MapFloat = [&](float x, float in_min, float in_max, float out_min, float out_max) -> float
		{
			return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		};
		const float fMap = std::clamp(MapFloat(vLocalPos.DistTo(vEnemyPos), Vars::Visuals::FOVArrows::MaxDist.Value, Vars::Visuals::FOVArrows::MaxDist.Value * 0.9f, 0.0f, 1.0f), 0.0f, 1.0f);
		color.a = static_cast<byte>(fMap * 255.f);

		DrawArrowTo(vLocalPos, vEnemyPos, color);
	}
}
