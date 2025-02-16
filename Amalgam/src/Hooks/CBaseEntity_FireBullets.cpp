#include "../SDK/SDK.h"

#include "../Features/Visuals/Visuals.h"

MAKE_SIGNATURE(CBaseEntity_FireBullets, "client.dll", "48 89 74 24 ? 55 57 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F3 41 0F 10 58", 0x0);

MAKE_HOOK(CBaseEntity_FireBullets, S::CBaseEntity_FireBullets(), void,
	void* rcx, CBaseCombatWeapon* pWeapon, const FireBulletsInfo_t& info, bool bDoEffects, int nDamageType, int nCustomDamageType)
{
	auto pLocal = H::Entities.GetLocal();
	auto pPlayer = reinterpret_cast<CBaseEntity*>(rcx);
	if (!pLocal || pPlayer != pLocal)
		return CALL_ORIGINAL(rcx, pWeapon, info, bDoEffects, nDamageType, nCustomDamageType);

	const Vec3 vStart = info.m_vecSrc;
	const Vec3 vEnd = vStart + info.m_vecDirShooting * info.m_flDistance;
	CGameTrace trace = {};
	CTraceFilterHitscan filter = {}; filter.pSkip = pLocal;
	SDK::Trace(vStart, vEnd, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);

	const int iAttachment = pWeapon->LookupAttachment("muzzle");
	pWeapon->GetAttachment(iAttachment, trace.startpos);

	const bool bCrit = nDamageType & DMG_CRITICAL || pLocal->IsCritBoosted();
	const int iTeam = pLocal->m_iTeamNum();

	auto& sString = bCrit ? Vars::Visuals::Particles::CritTrail.Value : Vars::Visuals::Particles::BulletTrail.Value;
	auto uHash = FNV1A::Hash32(sString.c_str());
	if (!pLocal->IsInValidTeam() || uHash == FNV1A::Hash32Const("Off") || uHash == FNV1A::Hash32Const("Default"))
		CALL_ORIGINAL(rcx, pWeapon, info, bDoEffects, nDamageType, nCustomDamageType);
	else if (uHash == FNV1A::Hash32Const("Machina"))
		H::Particles.ParticleTracer(iTeam == TF_TEAM_RED ? "dxhr_sniper_rail_red" : "dxhr_sniper_rail_blue", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("C.A.P.P.E.R"))
		H::Particles.ParticleTracer(iTeam == TF_TEAM_RED ? "bullet_tracer_raygun_red" : "bullet_tracer_raygun_blue", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("Short Circuit"))
		H::Particles.ParticleTracer(iTeam == TF_TEAM_RED ? "dxhr_lightningball_hit_zap_red" : "dxhr_lightningball_hit_zap_blue", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("Merasmus ZAP"))
		H::Particles.ParticleTracer("merasmus_zap", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("Merasmus ZAP 2"))
		H::Particles.ParticleTracer("merasmus_zap_beam02", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("Big Nasty"))
		H::Particles.ParticleTracer(iTeam == TF_TEAM_RED ? "bullet_bignasty_tracer01_blue" : "bullet_bignasty_tracer01_red", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("Distortion Trail"))
		H::Particles.ParticleTracer("tfc_sniper_distortion_trail", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("Black Ink"))
		H::Particles.ParticleTracer("merasmus_zap_beam01", trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
	else if (uHash == FNV1A::Hash32Const("Line"))
	{
		bool bClear = false;
		for (auto& Line : G::LineStorage)
		{
			if (I::GlobalVars->curtime - (Line.m_flTime - 5.f) > 0)
			{
				bClear = true;
				break;
			}
		}
		if (bClear)
			G::LineStorage.clear();

		G::LineStorage.push_back({ { trace.startpos, trace.endpos }, I::GlobalVars->curtime + 5.f, Vars::Colors::Line.Value, true });
	}
	else if (uHash == FNV1A::Hash32Const("Beam"))
	{
		BeamInfo_t beamInfo;
		beamInfo.m_nType = 0;
		beamInfo.m_pszModelName = FNV1A::Hash32(Vars::Visuals::Beams::Model.Value.c_str()) != FNV1A::Hash32Const("") ? Vars::Visuals::Beams::Model.Value.c_str() : "sprites/physbeam.vmt";
		beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = Vars::Visuals::Beams::Life.Value;
		beamInfo.m_flWidth = Vars::Visuals::Beams::Width.Value;
		beamInfo.m_flEndWidth = Vars::Visuals::Beams::EndWidth.Value;
		beamInfo.m_flFadeLength = Vars::Visuals::Beams::FadeLength.Value;
		beamInfo.m_flAmplitude = Vars::Visuals::Beams::Amplitude.Value;
		beamInfo.m_flBrightness = Vars::Visuals::Beams::Brightness.Value;
		beamInfo.m_flSpeed = Vars::Visuals::Beams::Speed.Value;
		beamInfo.m_nStartFrame = 0;
		beamInfo.m_flFrameRate = 0;
		beamInfo.m_flRed = Vars::Visuals::Beams::BeamColor.Value.r;
		beamInfo.m_flGreen = Vars::Visuals::Beams::BeamColor.Value.g;
		beamInfo.m_flBlue = Vars::Visuals::Beams::BeamColor.Value.b;
		beamInfo.m_nSegments = Vars::Visuals::Beams::Segments.Value;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = Vars::Visuals::Beams::Flags.Value;
		beamInfo.m_vecStart = trace.startpos;
		beamInfo.m_vecEnd = trace.endpos;

		if (auto pBeam = I::ViewRenderBeams->CreateBeamPoints(beamInfo))
			I::ViewRenderBeams->DrawBeam(pBeam);
	}
	else
		H::Particles.ParticleTracer(sString.c_str(), trace.startpos, trace.endpos, pLocal->entindex(), iAttachment, true);
}