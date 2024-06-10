#include "Visuals.h"

#include "../ImGui/Render.h"
#include "../Visuals/LocalConditions/LocalConditions.h"
#include "../Backtrack/Backtrack.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../CameraWindow/CameraWindow.h"
#include "../NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "Materials/Materials.h"

#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_stdlib.h>

MAKE_SIGNATURE(RenderLine, "engine.dll", "48 89 5C 24 ? 48 89 74 24 ? 44 89 44 24", 0x0);
MAKE_SIGNATURE(RenderBox, "engine.dll", "48 83 EC ? 8B 84 24 ? ? ? ? 4D 8B D8", 0x0);
MAKE_SIGNATURE(RenderWireframeBox, "engine.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 49 8B F9", 0x0);
MAKE_SIGNATURE(DrawServerHitboxes, "server.dll", "44 88 44 24 ? 53 48 81 EC", 0x0);
MAKE_SIGNATURE(GetServerAnimating, "server.dll", "48 83 EC ? 8B D1 85 C9 7E ? 48 8B 05", 0x0);
MAKE_SIGNATURE(LoadSkys, "engine.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8D 05", 0x0);

void CVisuals::DrawAimbotFOV(CTFPlayer* pLocal)
{
	if (!Vars::Aimbot::General::FOVCircle.Value || !Vars::Colors::FOVCircle.Value.a || !pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->IsTaunting() || pLocal->IsStunned() || pLocal->IsInBumperKart())
		return;

	const float flR = tanf(DEG2RAD(Vars::Aimbot::General::AimFOV.Value) / 2.0f) / tanf(DEG2RAD(pLocal->m_iFOV()) / 2.0f) * H::Draw.m_nScreenW;
	const Color_t clr = Vars::Colors::FOVCircle.Value;
	H::Draw.LineCircle(H::Draw.m_nScreenW / 2, H::Draw.m_nScreenH / 2, flR, 68, clr);
}

void CVisuals::DrawTickbaseText(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 0)))
		return;

	if (!pLocal->IsAlive())
		return;

	const int iTicks = std::clamp(G::ShiftedTicks + G::ChokeAmount, 0, G::MaxShift);

	const DragBox_t dtPos = Vars::Menu::TicksDisplay.Value;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	const int offset = 7 + 12 * Vars::Menu::DPI.Value;
	H::Draw.String(fFont, dtPos.x, dtPos.y + 2, Vars::Menu::Theme::Active.Value, ALIGN_TOP, "Ticks %d / %d", iTicks, G::MaxShift);
	if (G::WaitForShift)
		H::Draw.String(fFont, dtPos.x, dtPos.y + fFont.m_nTall + offset, Vars::Menu::Theme::Active.Value, ALIGN_TOP, "Not Ready");

	const float ratioCurrent = (float)iTicks / (float)G::MaxShift;
	float sizeX = 100 * Vars::Menu::DPI.Value, sizeY = 12 * Vars::Menu::DPI.Value, posX = dtPos.x - sizeX / 2, posY = dtPos.y + 5 + fFont.m_nTall;
	H::Draw.LineRect(posX, dtPos.y + 5 + fFont.m_nTall, sizeX, sizeY, Vars::Menu::Theme::Accent.Value);
	if (iTicks)
	{
		sizeX -= 4, sizeY -= 4, posX = dtPos.x - sizeX / 2;
		H::Draw.StartClipping(posX, posY, sizeX * ratioCurrent, sizeY + 2 /*?*/);
		H::Draw.FillRect(posX, posY + 2, sizeX, sizeY, Vars::Menu::Theme::Accent.Value);
		H::Draw.EndClipping();
	}
}
void CVisuals::DrawTickbaseBars()
{
	/* get rid of me
	
	if (!(Vars::Menu::Indicators.Value & (1 << 0)) || I::EngineVGui->IsGameUIVisible())
		return;

	auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal || !pLocal->As<CTFPlayer>()->IsAlive())
		return;

	const int iTicks = std::clamp(G::ShiftedTicks + G::ChokeAmount, 0, G::MaxShift);

	const DragBox_t dtPos = Vars::Menu::TicksDisplay.Value;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const float ratioCurrent = (float)iTicks / (float)G::MaxShift;

	float sizeX = 100 * Vars::Menu::DPI.Value, sizeY = 12 * Vars::Menu::DPI.Value, posY = dtPos.y + 5 + fFont.m_nTall;
	ImGui::GetBackgroundDrawList()->AddRectFilled(
		ImVec2(dtPos.x - sizeX / 2, posY), ImVec2(dtPos.x + sizeX / 2, posY + sizeY),
		F::Render.Background, 10
	);
	if (iTicks && ratioCurrent)
	{
		sizeX = 96 * Vars::Menu::DPI.Value, sizeY = 8 * Vars::Menu::DPI.Value; posY = posY + 2 * Vars::Menu::DPI.Value;
		ImGui::GetBackgroundDrawList()->PushClipRect(ImVec2(dtPos.x - sizeX / 2, posY), ImVec2(dtPos.x - sizeX / 2 + sizeX * ratioCurrent + 1, posY + sizeY), true);
		ImGui::GetBackgroundDrawList()->AddRectFilled(
			ImVec2(dtPos.x - sizeX / 2, posY), ImVec2(dtPos.x + sizeX / 2, posY + sizeY),
			ImColor(Vars::Menu::Theme::Accent.Value.r, Vars::Menu::Theme::Accent.Value.g, Vars::Menu::Theme::Accent.Value.b, Vars::Menu::Theme::Accent.Value.a), 10
		);
		ImGui::GetBackgroundDrawList()->PopClipRect();
	}
	*/
}

void CVisuals::DrawOnScreenPing(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 3)) || !pLocal || !pLocal->IsAlive())
		return;

	auto pResource = H::Entities.GetPR();
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pResource || !pNetChan)
		return;

	const float flLatencyIn = pNetChan->GetLatency(FLOW_INCOMING) * 1000.f;
	const float flLatencyOut = pNetChan->GetLatency(FLOW_OUTGOING) * 1000.f;
	const float flFake = std::min((F::Backtrack.GetFake() + (F::Backtrack.flFakeInterp > G::Lerp ? F::Backtrack.flFakeInterp : 0.f)) * 1000.f, F::Backtrack.flMaxUnlag * 1000.f);
	const float flLatency = F::Backtrack.GetReal() * 1000.f;
	const int iLatencyScoreboard = pResource->GetPing(pLocal->entindex());

	int x = Vars::Menu::PingDisplay.Value.x;
	int y = Vars::Menu::PingDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}

	if (flFake || Vars::Backtrack::Interp.Value && Vars::Backtrack::Enabled.Value)
	{
		if (flLatency > 0.f)
		{
			if (!Vars::Debug::Info.Value)
				H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Real %.0f (+ %.0f) ms", flLatency, flFake);
			else
				H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "In %.0f, Out %.0f (+ %.0f) ms", flLatencyIn, flLatencyOut, flFake);
		}
		else
			H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Syncing");
	}
	else
	{
		if (!Vars::Debug::Info.Value)
			H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Real %.0f ms", flLatency);
		else
			H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "In %.0f, Out %.0f ms", flLatencyIn, flLatencyOut);
	}
	H::Draw.String(fFont, x, y += fFont.m_nTall + 1, Vars::Menu::Theme::Active.Value, align, "Scoreboard %d ms", iLatencyScoreboard);
	if (Vars::Debug::Info.Value)
	{
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, align, "iTickCount %i (%i, %i, %i)", F::Backtrack.iTickCount, TIME_TO_TICKS(F::Backtrack.GetReal()), TIME_TO_TICKS(flLatencyIn / 1000), TIME_TO_TICKS(flLatencyOut / 1000));
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, align, "G::AnticipatedChoke %i", G::AnticipatedChoke);
	}
}

void CVisuals::DrawOnScreenConditions(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 4)) || !pLocal->IsAlive())
		return;

	int x = Vars::Menu::ConditionsDisplay.Value.x;
	int y = Vars::Menu::ConditionsDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}

	std::vector<std::wstring> conditionsVec = F::LocalConditions.GetPlayerConditions(pLocal);

	int offset = 0;
	for (const std::wstring& cond : conditionsVec)
	{
		H::Draw.String(fFont, x, y + offset, Vars::Menu::Theme::Active.Value, align, cond.data());
		offset += fFont.m_nTall + 1;
	}
}

void CVisuals::DrawSeedPrediction(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 5)) || !pLocal || !pLocal->IsAlive() || !Vars::Aimbot::General::NoSpread.Value)
		return;

	if (!Vars::Debug::Info.Value)
	{
		auto pWeapon = H::Entities.GetWeapon();
		if (!pWeapon || !F::NoSpreadHitscan.ShouldRun(pLocal, pWeapon))
			return;
	}

	int x = Vars::Menu::SeedPredictionDisplay.Value.x;
	int y = Vars::Menu::SeedPredictionDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}

	const auto& cColor = F::NoSpreadHitscan.bSynced ? Vars::Menu::Theme::Active.Value : Vars::Menu::Theme::Inactive.Value;

	H::Draw.String(fFont, x, y, cColor, align, std::format("Uptime {}", F::NoSpreadHitscan.GetFormat(F::NoSpreadHitscan.flServerTime)).c_str());
	H::Draw.String(fFont, x, y += fFont.m_nTall + 1, cColor, align, std::format("Mantissa step {}", F::NoSpreadHitscan.flMantissaStep).c_str());
	if (Vars::Debug::Info.Value)
	{
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, cColor, align, std::format("Seed {}", F::NoSpreadHitscan.iSeed).c_str());
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, cColor, align, std::format("{} ({})", F::NoSpreadHitscan.dTimeDelta, F::NoSpreadHitscan.vTimeDeltas.size()).c_str());
	}
}

void CVisuals::ProjectileTrace(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, const bool bQuick)
{
	F::CameraWindow.ShouldDraw = false;
	if ((bQuick ? !Vars::Visuals::Simulation::ProjectileTrajectory.Value : !Vars::Visuals::Simulation::TrajectoryOnShot.Value) || !pLocal || !pWeapon
		|| pWeapon->m_iWeaponID() == TF_WEAPON_FLAMETHROWER && !Vars::Debug::Info.Value)
		return;

	ProjectileInfo projInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, bQuick ? I::EngineClient->GetViewAngles() : G::CurrentUserCmd->viewangles, projInfo, true, bQuick, (bQuick && Vars::Aimbot::Projectile::AutoRelease.Value) ? Vars::Aimbot::Projectile::AutoRelease.Value / 100 : -1.f)
		|| !F::ProjSim.Initialize(projInfo))
		return;

	CGameTrace trace = {};
	CTraceFilterProjectile filter = {};
	filter.pSkip = pLocal;

	for (int n = -1; n < TIME_TO_TICKS(projInfo.m_flLifetime); n++)
	{
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(projInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		SDK::TraceHull(Old, New, projInfo.m_vHull * -1, projInfo.m_vHull, MASK_SOLID, &filter, &trace);
		if (trace.DidHit())
		{
			const float flSize = std::max(projInfo.m_vHull.x, 1.f);
			const Vec3 vSize = { 1.f, flSize, flSize };
			Vec3 vAngles; Math::VectorAngles(trace.plane.normal, vAngles);

			if (bQuick)
			{
				RenderBox(trace.endpos, vSize * -1, vSize, vAngles, Vars::Colors::ProjectileColor.Value, { 0, 0, 0, 0 });
				if (Vars::Colors::ClippedColor.Value.a)
					RenderBox(trace.endpos, vSize * -1, vSize, vAngles, Vars::Colors::ClippedColor.Value, { 0, 0, 0, 0 }, true);
			}
			else
			{
				G::BoxesStorage.clear();
				G::BoxesStorage.push_back({ trace.endpos, vSize * -1, vSize, vAngles, I::GlobalVars->curtime + TICKS_TO_TIME(n) + F::Backtrack.GetReal(), Vars::Colors::ProjectileColor.Value});
			}
			break;
		}
	}

	projInfo.PredictionLines.push_back({ trace.endpos, Math::GetRotatedPosition(trace.endpos, Math::VelocityToAngles(F::ProjSim.GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::Simulation::SeparatorLength.Value) });
	
	if (bQuick)
	{
		DrawSimLine(projInfo.PredictionLines, Vars::Colors::ProjectileColor.Value);
		if (Vars::Colors::ClippedColor.Value.a)
			DrawSimLine(projInfo.PredictionLines, Vars::Colors::ClippedColor.Value, false, true);

		if (!I::EngineVGui->IsGameUIVisible() && Vars::Visuals::Simulation::ProjectileCamera.Value && pLocal->m_vecOrigin().DistTo(trace.endpos) > 500.f)
		{
			auto vAngles = Math::CalcAngle(trace.startpos, trace.endpos);
			Vec3 vForward = {}; Math::AngleVectors(vAngles, &vForward);
			SDK::Trace(trace.endpos, trace.endpos - vForward * 500.f, MASK_SOLID, &filter, &trace);

			F::CameraWindow.ShouldDraw = true;
			F::CameraWindow.CameraOrigin = trace.endpos;
			F::CameraWindow.CameraAngles = vAngles;
		}
	}
	else
	{
		G::LinesStorage.clear();
		G::LinesStorage.push_back({ projInfo.PredictionLines, -float(projInfo.PredictionLines.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()), Vars::Colors::ProjectileColor.Value });
	}
}

void CVisuals::DrawAntiAim(CTFPlayer* pLocal)
{
	if (!pLocal->IsAlive() || pLocal->IsAGhost() || !I::Input->CAM_IsThirdPerson())
		return;

	if (F::AntiAim.AntiAimOn() && Vars::Debug::AntiAimLines.Value)
	{
		const auto& vOrigin = pLocal->GetAbsOrigin();

		Vec3 vScreen1, vScreen2;
		if (SDK::W2S(vOrigin, vScreen1))
		{
			constexpr auto distance = 50.f;
			if (SDK::W2S(Math::GetRotatedPosition(vOrigin, F::AntiAim.vRealAngles.y, distance), vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 0, 255, 0, 255 });

			if (SDK::W2S(Math::GetRotatedPosition(vOrigin, F::AntiAim.vFakeAngles.y, distance), vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 255, 0, 0, 255 });
		}

		for (auto& vPair : F::AntiAim.vEdgeTrace)
		{
			if (SDK::W2S(vPair.first, vScreen1) && SDK::W2S(vPair.second, vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 255, 255, 255, 255 });
		}
	}
}

void CVisuals::DrawDebugInfo(CTFPlayer* pLocal)
{
	// Debug info
	if (Vars::Debug::Info.Value)
	{
		int x = 10, y = 10;
		const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

		Vec3 vOrigin = pLocal->m_vecOrigin();
		H::Draw.String(fFont, x, y, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, "Origin: (%.3f, %.3f, %.3f)", vOrigin.x, vOrigin.y, vOrigin.z);

		Vec3 vVelocity = pLocal->m_vecVelocity();
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, "Velocity: %.3f (%.3f, %.3f, %.3f)", vVelocity.Length(), vVelocity.x, vVelocity.y, vVelocity.z);

		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, std::format("Attacking: {} ({}, {})", G::IsAttacking, G::CanPrimaryAttack, G::CanSecondaryAttack).c_str());

		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, "RoundState: %i", SDK::GetRoundState());

		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, std::format("Choke: {}, {}", G::Choking, I::ClientState->chokedcommands).c_str());
		
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, std::format("Ticks: {}, {}", G::ShiftedTicks, G::ShiftedGoal).c_str());

		if (auto pCmd = G::LastUserCmd)
		{
			H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, "pCmd move: (%.0f, %.0f)", pCmd->forwardmove, pCmd->sidemove);
			H::Draw.String(fFont, x, y += fFont.m_nTall + 1, { 255, 255, 255, 255 }, ALIGN_TOPLEFT, "pCmd buttons: %i", pCmd->buttons);
		}
	}
}



std::vector<DrawBox> CVisuals::GetHitboxes(matrix3x4 bones[128], CBaseAnimating* pEntity, const int iHitbox)
{
	std::vector<DrawBox> vBoxes = {};

	auto pModel = pEntity->GetModel();
	if (!pModel) return vBoxes;
	auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHDR) return vBoxes;
	auto pSet = pHDR->pHitboxSet(pEntity->m_nHitboxSet());
	if (!pSet) return vBoxes;

	for (int i = iHitbox != -1 ? iHitbox : 0; i < pSet->numhitboxes; ++i)
	{
		if (iHitbox != -1 && iHitbox != i)
			break;

		auto bbox = pSet->pHitbox(i);
		if (!bbox) continue;

		matrix3x4 rotMatrix;
		Math::AngleMatrix(bbox->angle, rotMatrix);

		matrix3x4 matrix;
		Math::ConcatTransforms(bones[bbox->bone], rotMatrix, matrix);

		Vec3 bboxAngle;
		Math::MatrixAngles(matrix, bboxAngle);

		Vec3 matrixOrigin;
		Math::GetMatrixOrigin(matrix, matrixOrigin);

		vBoxes.push_back({ matrixOrigin, bbox->bbmin, bbox->bbmax, bboxAngle, I::GlobalVars->curtime + 5.f, Vars::Colors::HitboxEdge.Value, Vars::Colors::HitboxFace.Value, true });
	}

	return vBoxes;
}

void CVisuals::DrawBulletLines()
{
	for (auto& Line : G::BulletsStorage)
	{
		if (Line.m_flTime < I::GlobalVars->curtime) continue;

		RenderLine(Line.m_line.first, Line.m_line.second, Line.m_color, Line.m_bZBuffer);
	}
}

void CVisuals::DrawSimLine(std::deque<std::pair<Vec3, Vec3>>& Line, Color_t Color, bool bSeparators, bool bZBuffer, float flTime)
{
	for (size_t i = 1; i < Line.size(); i++)
	{
		if (flTime < 0.f && Line.size() - i > -flTime)
			continue;

		if (!bSeparators)
			RenderLine(Line.at(i - 1).first, Line.at(i).first, Color, bZBuffer);
		else
		{
			const auto& vStart = Line[i - 1].first;
			const auto& vRotate = Line[i - 1].second;
			const auto& vEnd = Line[i].first;
			if (((Line.size() - i) % Vars::Visuals::Simulation::SeparatorSpacing.Value) == 0)
				RenderLine(vStart, vRotate, Color);
			RenderLine(vStart, vEnd, Color);
		}
	}
}

void CVisuals::DrawSimLines()
{
	for (auto& Line : G::LinesStorage)
	{
		if (Line.m_flTime >= 0.f && Line.m_flTime < I::GlobalVars->curtime)
			continue;

		DrawSimLine(Line.m_line, Line.m_color, Vars::Visuals::Simulation::Separators.Value, Line.m_bZBuffer, Line.m_flTime);
	}
}

void CVisuals::DrawBoxes()
{
	for (auto& Box : G::BoxesStorage)
	{
		if (Box.m_flTime < I::GlobalVars->curtime) continue;

		RenderBox(Box.m_vecPos, Box.m_vecMins, Box.m_vecMaxs, Box.m_vecOrientation, Box.m_colorEdge, Box.m_colorFace, Box.m_bZBuffer);
	}
}

void CVisuals::RevealBulletLines()
{
	for (auto& Line : G::BulletsStorage)
		Line.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::RevealSimLines()
{
	for (auto& PredictionLine : G::LinesStorage)
		PredictionLine.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::RevealBoxes()
{
	for (auto& Box : G::BoxesStorage)
		Box.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::DrawServerHitboxes(CTFPlayer* pLocal)
{
	static int iOldTick = I::GlobalVars->tickcount;
	if (iOldTick == I::GlobalVars->tickcount)
		return;
	iOldTick = I::GlobalVars->tickcount;

	if (I::Input->CAM_IsThirdPerson() && Vars::Debug::ServerHitbox.Value && pLocal->IsAlive())
	{
		using GetServerAnimating_t = void* (*)(int);
		static auto GetServerAnimating = S::GetServerAnimating.As<GetServerAnimating_t>();

		using DrawServerHitboxes_t = void(__fastcall*)(void*, float, bool); // C_BaseAnimating, Duration, MonoColour
		static auto DrawServerHitboxes = S::DrawServerHitboxes.As<DrawServerHitboxes_t>();

		void* server_animating = GetServerAnimating(pLocal->entindex());
		if (server_animating)
			DrawServerHitboxes(server_animating, TICK_INTERVAL, true);
	}
}

void CVisuals::RenderLine(const Vec3& vStart, const Vec3& vEnd, Color_t cLine, bool bZBuffer)
{
	if (cLine.a)
	{
		static auto fnRenderLine = S::RenderLine.As<void(__cdecl*)(const Vector&, const Vector&, Color_t, bool)>();
		fnRenderLine(vStart, vEnd, cLine, bZBuffer);
	}
}

void CVisuals::RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace, bool bZBuffer)
{
	if (cFace.a)
	{
		static auto fnRenderBox = S::RenderBox.As<void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool, bool)>();
		fnRenderBox(vPos, vOrientation, vMins, vMaxs, cFace, bZBuffer, false);
	}

	if (cEdge.a)
	{
		static auto fnRenderWireframeBox = S::RenderWireframeBox.As<void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool)>();
		fnRenderWireframeBox(vPos, vOrientation, vMins, vMaxs, cEdge, bZBuffer);
	}
}



void CVisuals::FOV(CTFPlayer* pLocal, CViewSetup* pView)
{
	pLocal->m_iFOV() = pView->fov;

	const int fov = pLocal->IsScoped() ? Vars::Visuals::UI::ZoomFieldOfView.Value : Vars::Visuals::UI::FieldOfView.Value;
	if (!fov)
		return;

	pView->fov = fov;
	pLocal->m_iFOV() = fov;
}

void CVisuals::ThirdPerson(CTFPlayer* pLocal, CViewSetup* pView)
{
	if (!pLocal->IsAlive())
		return I::Input->CAM_ToFirstPerson();
	
	const bool bNoZoom = (!Vars::Visuals::Removals::Scope.Value || Vars::Visuals::UI::ZoomFieldOfView.Value < 70) && pLocal->IsScoped();
	const bool bForce = pLocal->IsTaunting() || pLocal->IsAGhost() || pLocal->IsInBumperKart() || pLocal->InCond(TF_COND_HALLOWEEN_THRILLER);

	//if (bForce)
	//	return;

	if (Vars::Visuals::ThirdPerson::Enabled.Value && !bNoZoom || bForce)
		I::Input->CAM_ToThirdPerson();
	else
		I::Input->CAM_ToFirstPerson();
	pLocal->ThirdPersonSwitch();

	if (I::Input->CAM_IsThirdPerson())
	{	// Thirdperson offset
		Vec3 vForward, vRight, vUp;
		Math::AngleVectors(pView->angles, &vForward, &vRight, &vUp);

		Vec3 offset;
		offset += vRight * Vars::Visuals::ThirdPerson::Right.Value;
		offset += vUp * Vars::Visuals::ThirdPerson::Up.Value;
		offset -= vForward * Vars::Visuals::ThirdPerson::Distance.Value;

		const Vec3 viewDiff = pView->origin - pLocal->GetEyePosition();
		CGameTrace trace = {}; CTraceFilterWorldAndPropsOnly filter = {};
		SDK::TraceHull(pView->origin - viewDiff, pView->origin + offset - viewDiff, { -14.f, -14.f, -14.f }, { 14.f, 14.f, 14.f }, MASK_SOLID, &filter, &trace);

		pView->origin += offset * trace.fraction - viewDiff;
	}
}

bool CVisuals::RemoveScope(int nPanel)
{
	if (!Vars::Visuals::Removals::Scope.Value)
		return false;

	if (!m_nHudZoom && FNV1A::Hash(I::VGuiPanel->GetName(nPanel)) == FNV1A::HashConst("HudScope"))
		m_nHudZoom = nPanel;

	return nPanel == m_nHudZoom;
}

void CVisuals::DrawSightlines()
{
	if (Vars::Visuals::UI::SniperSightlines.Value)
	{
		if (!m_SightLines.empty())
		{
			for (const auto& sightline : m_SightLines)
			{
				if (sightline.m_bDraw)
					RenderLine(sightline.m_vStart, sightline.m_vEnd, sightline.m_Color);
			}
		}
	}
}

void CVisuals::StoreSightlines()
{
	auto pLocal = H::Entities.GetLocal();
	if (Vars::Visuals::UI::SniperSightlines.Value && pLocal)
	{
		m_SightLines = {}; // should get rid of residual lines

		std::unordered_map<IClientEntity*, Vec3> mDots = {};
		for (auto pEntity : H::Entities.GetGroup(EGroupType::MISC_DOTS))
		{
			if (auto pOwner = pEntity->m_hOwnerEntity().Get())
				mDots[pOwner] = pEntity->m_vecOrigin();
		}

		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			int iEntityIndex = pPlayer->entindex();
			auto pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
			if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant() || !pPlayer->InCond(TF_COND_AIMING) ||
				!pWeapon || pWeapon->m_iWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->m_iWeaponID() == TF_WEAPON_MINIGUN)
			{
				continue;
			}

			Vec3 vShootPos = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();
			Vec3 vForward; Math::AngleVectors(pPlayer->GetEyeAngles(), &vForward);
			Vec3 vShootEnd = mDots.contains(pPlayer) ? mDots[pPlayer] : vShootPos + (vForward * 8192.f);

			CTraceFilterHitscan filter{};
			CGameTrace trace{};
			filter.pSkip = pPlayer;
			SDK::Trace(vShootPos, vShootEnd, MASK_SHOT, &filter, &trace);

			m_SightLines[pPlayer->entindex()] = { vShootPos, trace.endpos, H::Color.GetEntityDrawColor(pLocal, pPlayer, Vars::Colors::Relative.Value), true };
		}
	}
}

void CVisuals::PickupTimers()
{
	if (!Vars::Visuals::UI::PickupTimers.Value)
		return;

	for (auto pickupData = PickupDatas.begin(); pickupData != PickupDatas.end();)
	{
		const float timeDiff = I::EngineClient->Time() - pickupData->Time;
		if (timeDiff > 10.f)
		{
			pickupData = PickupDatas.erase(pickupData);
			continue;
		}

		auto timerText = std::format("{:.1f}s", 10.f - timeDiff);
		auto color = pickupData->Type ? Vars::Colors::Health.Value : Vars::Colors::Ammo.Value;

		Vec3 vScreen;
		if (SDK::W2S(pickupData->Location, vScreen))
			H::Draw.String(H::Fonts.GetFont(FONT_ESP), vScreen.x, vScreen.y, color, ALIGN_CENTER, timerText.c_str());

		++pickupData;
	}
}

// Credits go to reestart entirely
void CVisuals::ManualNetwork(const StartSoundParams_t& params)
{
	if (params.soundsource <= 0)
		return;

	Vector vOrigin = params.origin;
	const int iEntIndex = params.soundsource;
	auto pEntity = I::ClientEntityList->GetClientEntity(iEntIndex);

	if (pEntity && iEntIndex != I::EngineClient->GetLocalPlayer() && pEntity->IsDormant() && pEntity->GetClassID() == ETFClassID::CTFPlayer)
		G::DormancyMap[iEntIndex] = { vOrigin, I::EngineClient->Time() };
}



void CVisuals::OverrideWorldTextures()
{
	KeyValues* kv = nullptr;

	auto uHash = FNV1A::Hash(Vars::Visuals::World::WorldTexture.Value.c_str());
	if (uHash == FNV1A::HashConst("Default"))
		return;

	kv = new KeyValues("LightmappedGeneric");
	if (uHash == FNV1A::HashConst("Dev"))
		kv->SetString("$basetexture", "dev/dev_measuregeneric01b");
	else if (uHash == FNV1A::HashConst("Camo"))
		kv->SetString("$basetexture", "patterns/paint_strokes");
	else if (uHash == FNV1A::HashConst("Black"))
		kv->SetString("$basetexture", "patterns/combat/black");
	else if (uHash == FNV1A::HashConst("White"))
		kv->SetString("$basetexture", "patterns/combat/white");
	else if (uHash == FNV1A::HashConst("Flat"))
	{
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetString("$color2", "[0.12 0.12 0.15]");
	}
	else
		kv->SetString("$basetexture", Vars::Visuals::World::WorldTexture.Value.c_str());

	if (!kv)
		return;

	for (auto h = I::MaterialSystem->FirstMaterial(); h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		auto pMaterial = I::MaterialSystem->GetMaterial(h);
		if (!pMaterial || pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached() || pMaterial->IsTranslucent() || pMaterial->IsSpriteCard())
			continue;

		auto sGroup = std::string_view(pMaterial->GetTextureGroupName());
		auto sName = std::string_view(pMaterial->GetName());

		if (!sGroup._Starts_with("World")
			|| sName.find("water") != std::string_view::npos || sName.find("glass") != std::string_view::npos
			|| sName.find("door") != std::string_view::npos || sName.find("tools") != std::string_view::npos
			|| sName.find("player") != std::string_view::npos || sName.find("chicken") != std::string_view::npos
			|| sName.find("wall28") != std::string_view::npos || sName.find("wall26") != std::string_view::npos
			|| sName.find("decal") != std::string_view::npos || sName.find("overlay") != std::string_view::npos
			|| sName.find("hay") != std::string_view::npos)
		{
			continue;
		}

		pMaterial->SetShaderAndParams(kv);
	}
}

void ApplyModulation(const Color_t& clr, bool bSky = false)
{
	for (auto h = I::MaterialSystem->FirstMaterial(); h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		auto pMaterial = I::MaterialSystem->GetMaterial(h);
		if (!pMaterial || pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
			continue;

		auto sGroup = std::string_view(pMaterial->GetTextureGroupName());
		if (!bSky ? !sGroup._Starts_with("World") : !sGroup._Starts_with("SkyBox"))
			continue;

		pMaterial->ColorModulate(float(clr.r) / 255.f, float(clr.g) / 255.f, float(clr.b) / 255.f);
	}
}

void CVisuals::Modulate()
{
	const bool bScreenshot = Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot();
	const bool bWorldModulation = Vars::Visuals::World::Modulations.Value & 1 << 0 && !bScreenshot;
	const bool bSkyModulation = Vars::Visuals::World::Modulations.Value & 1 << 1 && !bScreenshot;

	static bool bLastConnectionState = I::EngineClient->IsConnected() && I::EngineClient->IsInGame();
	const bool bCurrConnectionState = I::EngineClient->IsConnected() && I::EngineClient->IsInGame();
	const bool bUnchanged = bLastConnectionState == bCurrConnectionState;

	bool bSetChanged, bColorChanged, bSkyChanged;
	{
		static auto oldW = bWorldModulation;
		const auto curW = bWorldModulation;
		static auto oldS = bSkyModulation;
		const auto curS = bSkyModulation;

		bSetChanged = curS != oldS || curW != oldW;

		oldW = curW;
		oldS = curS;
	}
	{
		static auto oldW = Vars::Colors::WorldModulation.Value;
		static auto oldS = Vars::Colors::SkyModulation.Value;
		const auto curW = Vars::Colors::WorldModulation.Value;
		const auto curS = Vars::Colors::SkyModulation.Value;

		bColorChanged = curW != oldW || curS != oldS;
	
		oldW = curW;
		oldS = curS;
	}
	{
		static auto oldS = Vars::Visuals::World::SkyboxChanger.Value;
		const auto curS = Vars::Visuals::World::SkyboxChanger.Value;

		bSkyChanged = curS != oldS;
	
		oldS = curS;
	}

	if (bSetChanged || bColorChanged || bSkyChanged || !bUnchanged)
	{
		bWorldModulation ? ApplyModulation(Vars::Colors::WorldModulation.Value) : ApplyModulation({ 255, 255, 255, 255 });
		bSkyModulation ? ApplyModulation(Vars::Colors::SkyModulation.Value, true) : ApplyModulation({ 255, 255, 255, 255 }, true);
		bLastConnectionState = bCurrConnectionState;
	}
}

void CVisuals::RestoreWorldModulation() // keep this because its mentioned in @DLLMain.cpp if you find a better way to do this, remove it ig.
{
	ApplyModulation({ 255, 255, 255, 255 });
	ApplyModulation({ 255, 255, 255, 255 }, true);
}

void CVisuals::SkyboxChanger()
{
	using LoadNamedSkysFn = bool(_cdecl*)(const char*);
	static auto fnLoadSkys = S::LoadSkys.As<LoadNamedSkysFn>();
	const bool bScreenshot = Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot();

	static auto sv_skyname = U::ConVars.FindVar("sv_skyname");
	if (FNV1A::Hash(Vars::Visuals::World::SkyboxChanger.Value.c_str()) != FNV1A::HashConst("Off") && !bScreenshot)
		fnLoadSkys(Vars::Visuals::World::SkyboxChanger.Value.c_str());
	else if (sv_skyname)
		fnLoadSkys(sv_skyname->GetString());
}