#include "StickyCam.h"
#include "../Materials/Materials.h"

void CStickyCam::Initialize()
{
	if (!InitializeMaterials())
		return;
	
	m_bInitialized = true;
}

void CStickyCam::Unload()
{
	CleanupMaterials();
	m_bInitialized = false;
}

bool CStickyCam::InitializeMaterials()
{
	if (m_pCameraMaterial && m_pCameraTexture)
		return true;
	
	// Create camera material
	if (!m_pCameraMaterial)
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "m_pStickyCameraTexture");
		kv->SetString("$ignorez", "1");
		kv->SetString("$nofog", "1");
		m_pCameraMaterial = F::Materials.Create("StickyCameraMaterial", kv);
	}
	
	// Create render target texture
	if (!m_pCameraTexture && I::MaterialSystem)
	{
		try
		{
			m_pCameraTexture = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
				"m_pStickyCameraTexture",
				GetCameraWidth(),
				GetCameraHeight(),
				RT_SIZE_NO_CHANGE,
				IMAGE_FORMAT_RGB888,
				MATERIAL_RT_DEPTH_SHARED,
				TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
				CREATERENDERTARGETFLAGS_HDR
			);
			
			if (m_pCameraTexture)
				m_pCameraTexture->IncrementReferenceCount();
		}
		catch (...)
		{
			m_pCameraTexture = nullptr;
		}
	}
	
	// Update tracked dimensions after successful initialization
	if (m_pCameraMaterial && m_pCameraTexture)
	{
		m_iLastWidth = GetCameraWidth();
		m_iLastHeight = GetCameraHeight();
	}
	
	// Create invisible material for sticky
	if (!m_pInvisibleMaterial)
	{
		KeyValues* kv = new KeyValues("VertexLitGeneric");
		kv->SetString("$basetexture", "vgui/white");
		kv->SetString("$no_draw", "1");
		m_pInvisibleMaterial = F::Materials.Create("StickyCamInvisible", kv);
	}
	
	// Create chams material for targets
	if (!m_pChamsMaterial)
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetString("$color2", "[1 0 0]");
		m_pChamsMaterial = F::Materials.Create("StickyCamChams", kv);
	}
	
	return m_pCameraMaterial && m_pCameraTexture && m_pInvisibleMaterial && m_pChamsMaterial;
}

void CStickyCam::CleanupMaterials()
{
	if (m_pCameraMaterial)
	{
		m_pCameraMaterial->DecrementReferenceCount();
		m_pCameraMaterial->DeleteIfUnreferenced();
		m_pCameraMaterial = nullptr;
	}
	
	if (m_pCameraTexture)
	{
		m_pCameraTexture->DecrementReferenceCount();
		m_pCameraTexture->DeleteIfUnreferenced();
		m_pCameraTexture = nullptr;
	}
	
	if (m_pInvisibleMaterial)
	{
		m_pInvisibleMaterial->DecrementReferenceCount();
		m_pInvisibleMaterial->DeleteIfUnreferenced();
		m_pInvisibleMaterial = nullptr;
	}
	
	if (m_pChamsMaterial)
	{
		m_pChamsMaterial->DecrementReferenceCount();
		m_pChamsMaterial->DeleteIfUnreferenced();
		m_pChamsMaterial = nullptr;
	}
}

bool CStickyCam::IsDemoman()
{
	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return false;
	return pLocal->m_iClass() == TF_CLASS_DEMOMAN;
}

Vec3 CStickyCam::CalculateAngles(const Vec3& source, const Vec3& dest)
{
	Vec3 delta = SubtractVectors(dest, source);
	float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);
	
	// Calculate pitch (convert radians to degrees)
	float pitch = atan2f(delta.z, hyp) * 57.2957795f;
	
	// Calculate yaw (convert radians to degrees)
	float yaw = atan2f(delta.y, delta.x) * 57.2957795f;
	
	// Adjust yaw based on quadrant
	if (delta.x < 0)
		yaw += 180.0f;
	else if (delta.y < 0)
		yaw += 360.0f;
	
	// Handle NaN cases
	if (pitch != pitch) pitch = 0.0f;
	if (yaw != yaw) yaw = 0.0f;
	
	return Vec3(-pitch, yaw, 0);
}

bool CStickyCam::CheckPlayerVisibility(const Vec3& startPos, CTFPlayer* pPlayer)
{
	if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant())
		return false;
	
	Vec3 targetPos = pPlayer->GetAbsOrigin();
	targetPos.z += 50.0f; // Aim at chest level
	
	CGameTrace trace;
	CTraceFilterHitscan filter;
	filter.pSkip = nullptr;
	
	SDK::Trace(startPos, targetPos, MASK_SHOT, &filter, &trace);
	return trace.fraction > 0.99f || trace.m_pEnt == pPlayer;
}

Vec3 CStickyCam::AddVectors(const Vec3& vec1, const Vec3& vec2)
{
	return Vec3(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
}

Vec3 CStickyCam::SubtractVectors(const Vec3& vec1, const Vec3& vec2)
{
	return Vec3(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
}

Vec3 CStickyCam::MultiplyVector(const Vec3& vec, float scalar)
{
	return Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}

Vec3 CStickyCam::GetStickyNormal(CBaseEntity* pSticky)
{
	if (!pSticky)
		return Vec3(0, 0, 1);
	
	Vec3 pos = pSticky->GetAbsOrigin();
	
	// Check for ceiling attachment
	CGameTrace upTrace;
	CTraceFilterHitscan filter;
	filter.pSkip = pSticky;
	
	SDK::Trace(pos, AddVectors(pos, Vec3(0, 0, 5)), MASK_SOLID, &filter, &upTrace);
	
	if (upTrace.fraction < 0.2f)
		return Vec3(0, 0, 1); // Upward normal for ceiling
	
	// Check for floor
	CGameTrace downTrace;
	SDK::Trace(pos, AddVectors(pos, Vec3(0, 0, -5)), MASK_SOLID, &filter, &downTrace);
	
	if (downTrace.fraction < 1.0f && downTrace.plane.normal.Length() > 0)
		return downTrace.plane.normal;
	
	return Vec3(0, 0, 1);
}

Vec3 CStickyCam::CalculateCameraOffset(const Vec3& stickyPos, const Vec3& normal)
{
	// Check which direction is blocked
	CGameTrace upTrace, downTrace;
	CTraceFilterHitscan filter;
	filter.pSkip = nullptr;
	
	SDK::Trace(stickyPos, AddVectors(stickyPos, Vec3(0, 0, 15)), MASK_SOLID, &filter, &upTrace);
	SDK::Trace(stickyPos, AddVectors(stickyPos, Vec3(0, 0, -15)), MASK_SOLID, &filter, &downTrace);
	
	float cameraOffset = Vars::Competitive::StickyCam::CameraOffset.Value;
	
	// If up is blocked but down is open, force camera downward
	if (upTrace.fraction < 0.5f && downTrace.fraction > 0.5f)
		return AddVectors(stickyPos, Vec3(0, 0, -cameraOffset));
	
	// If down is blocked but up is open, force camera upward
	if (downTrace.fraction < 0.5f && upTrace.fraction > 0.5f)
		return AddVectors(stickyPos, Vec3(0, 0, cameraOffset));
	
	// Use normal-based offset
	return AddVectors(stickyPos, MultiplyVector(normal, cameraOffset));
}

CTFPlayer* CStickyCam::FindNearestVisiblePlayer(const Vec3& position)
{
	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return nullptr;
	
	// Check if current target is still valid and within lock duration
	if (m_pCurrentTarget && I::GlobalVars->curtime - m_flTargetLockTime < Vars::Competitive::StickyCam::TrackTime.Value)
	{
		if (m_pCurrentTarget->IsAlive() && !m_pCurrentTarget->IsDormant())
		{
			if (I::GlobalVars->curtime - m_flLastOcclusionCheck > OCCLUSION_CHECK_INTERVAL)
			{
				m_bTargetVisible = CheckPlayerVisibility(position, m_pCurrentTarget);
				m_flLastOcclusionCheck = I::GlobalVars->curtime;
			}
			
			if (m_bTargetVisible)
			{
				Vec3 targetPos = m_pCurrentTarget->GetAbsOrigin();
				Vec3 delta = SubtractVectors(targetPos, position);
				if (delta.Length() <= Vars::Competitive::StickyCam::SearchRadius.Value)
					return m_pCurrentTarget;
			}
		}
	}
	
	// Find new target
	CTFPlayer* pNearest = nullptr;
	float fNearestDist = Vars::Competitive::StickyCam::SearchRadius.Value;
	
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant())
			continue;
		
		if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;
		
		Vec3 playerPos = pPlayer->GetAbsOrigin();
		float dist = SubtractVectors(playerPos, position).Length();
		
		if (dist < fNearestDist && CheckPlayerVisibility(position, pPlayer))
		{
			pNearest = pPlayer;
			fNearestDist = dist;
		}
	}
	
	if (pNearest && pNearest != m_pCurrentTarget)
	{
		m_flTargetLockTime = I::GlobalVars->curtime;
		m_bTargetVisible = true;
		m_flLastOcclusionCheck = I::GlobalVars->curtime;
	}
	
	return pNearest;
}

Vec3 CStickyCam::LerpAngles(const Vec3& start, const Vec3& target, float factor)
{
	float dx = target.x - start.x;
	float dy = target.y - start.y;
	
	// Normalize yaw difference
	while (dy > 180.0f) dy -= 360.0f;
	while (dy < -180.0f) dy += 360.0f;
	
	return Vec3(
		start.x + dx * factor,
		start.y + dy * factor,
		0
	);
}

void CStickyCam::UpdateStickies()
{
	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return;
	
	// Clean up invalid stickies
	for (int i = static_cast<int>(m_Stickies.size()) - 1; i >= 0; i--)
	{
		if (!m_Stickies[i].pSticky || m_Stickies[i].pSticky->IsDormant())
		{
			// Remove from visited list too
			auto it = std::find(m_VisitedStickies.begin(), m_VisitedStickies.end(), m_Stickies[i].pSticky);
			if (it != m_VisitedStickies.end())
				m_VisitedStickies.erase(it);
			
			m_Stickies.erase(m_Stickies.begin() + i);
		}
	}
	
	// Find and add new stickies
	for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
	{
		if (!pEntity || pEntity->IsDormant())
			continue;
		
		if (pEntity->GetClassID() != ETFClassID::CTFGrenadePipebombProjectile)
			continue;
		
		auto pProjectile = pEntity->As<CTFGrenadePipebombProjectile>();
		if (!pProjectile)
			continue;
		
		// Check if it's a sticky (type 1) and belongs to local player
		if (pProjectile->m_iType() != 1)
			continue;
		
		auto pThrower = pProjectile->m_hThrower().Get();
		if (!pThrower || pThrower != pLocal)
			continue;
		
		// Check if already in our list
		bool found = false;
		for (const auto& sticky : m_Stickies)
		{
			if (sticky.pSticky == pEntity)
			{
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			StickyData data;
			data.pSticky = pEntity;
			data.flCreationTime = I::GlobalVars->curtime;
			data.flLastVisibilityCheck = 0.0f;
			data.bVisible = false;
			m_Stickies.push_back(data);
		}
	}
	
	// Clean up visited stickies list - remove any that are no longer in our stickies list
	for (int i = static_cast<int>(m_VisitedStickies.size()) - 1; i >= 0; i--)
	{
		bool found = false;
		for (const auto& sticky : m_Stickies)
		{
			if (sticky.pSticky == m_VisitedStickies[i])
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			m_VisitedStickies.erase(m_VisitedStickies.begin() + i);
		}
	}
	
	// Update current sticky if invalid
	if (!m_pCurrentSticky || m_pCurrentSticky->IsDormant())
	{
		m_pCurrentSticky = nullptr;
		m_pCurrentTarget = nullptr;
		m_bTargetVisible = false;
		
		// In manual mode, just find any valid sticky (don't require targets)
		// In follow latest mode, find sticky with targets for better experience
		bool needTargets = (GetMode() == EStickyMode::FOLLOW_LATEST);
		
		for (const auto& sticky : m_Stickies)
		{
			if (!sticky.pSticky)
				continue;
			
			// Check if sticky is stationary
			Vec3 velocity;
			try {
				sticky.pSticky->EstimateAbsVelocity(velocity);
			}
			catch (...) {
				continue;
			}
			if (velocity.Length() < 1.0f)
			{
				if (!needTargets)
				{
					// Manual mode: any stationary sticky is fine
					m_pCurrentSticky = sticky.pSticky;
					break;
				}
				else
				{
					// Follow latest mode: prefer stickies with targets
					Vec3 stickyPos = sticky.pSticky->GetAbsOrigin();
					if (FindNearestVisiblePlayer(stickyPos))
					{
						m_pCurrentSticky = sticky.pSticky;
						break;
					}
				}
			}
		}
		
		// If follow latest mode didn't find a sticky with targets, fall back to any sticky
		if (!m_pCurrentSticky && needTargets)
		{
			for (const auto& sticky : m_Stickies)
			{
				if (!sticky.pSticky)
					continue;
				
				Vec3 velocity;
				try {
					sticky.pSticky->EstimateAbsVelocity(velocity);
				}
				catch (...) {
					continue;
				}
				if (velocity.Length() < 1.0f)
				{
					m_pCurrentSticky = sticky.pSticky;
					break;
				}
			}
		}
	}
}

bool CStickyCam::UpdateUserInput()
{
	float currentTime = I::GlobalVars->curtime;
	
	if (GetAsyncKeyState(VK_TAB) & 0x8000 && currentTime - m_flLastKeyPress > KEY_DELAY)
	{
		CycleNextSticky();
		m_flLastKeyPress = currentTime;
		return true;
	}
	
	return false;
}

void CStickyCam::CycleNextSticky()
{
	// Find available stationary stickies
	std::vector<CBaseEntity*> availableStickies;
	for (const auto& sticky : m_Stickies)
	{
		if (!sticky.pSticky || sticky.pSticky->IsDormant())
			continue;
		
		Vec3 velocity;
		try {
			sticky.pSticky->EstimateAbsVelocity(velocity);
		}
		catch (...) {
			continue;
		}
		if (velocity.Length() < 1.0f)
			availableStickies.push_back(sticky.pSticky);
	}
	
	if (availableStickies.empty())
	{
		m_pCurrentSticky = nullptr;
		m_pCurrentTarget = nullptr;
		m_bTargetVisible = false;
		m_VisitedStickies.clear();
		return;
	}
	
	// Find unvisited stickies
	std::vector<CBaseEntity*> unvisitedStickies;
	for (auto pSticky : availableStickies)
	{
		if (std::find(m_VisitedStickies.begin(), m_VisitedStickies.end(), pSticky) == m_VisitedStickies.end())
			unvisitedStickies.push_back(pSticky);
	}
	
	// Reset if all visited
	if (unvisitedStickies.empty())
	{
		m_VisitedStickies.clear();
		unvisitedStickies = availableStickies;
	}
	
	if (!unvisitedStickies.empty())
	{
		m_pCurrentSticky = unvisitedStickies[0];
		m_VisitedStickies.push_back(m_pCurrentSticky);
		m_pCurrentTarget = nullptr;
		m_bTargetVisible = false;
		m_flTargetLockTime = 0.0f;
	}
	else
	{
		// Safety fallback: if no unvisited stickies but we have available ones,
		// this means the visited list might be corrupted, so reset it
		if (!availableStickies.empty())
		{
			m_VisitedStickies.clear();
			m_pCurrentSticky = availableStickies[0];
			m_VisitedStickies.push_back(m_pCurrentSticky);
			m_pCurrentTarget = nullptr;
			m_bTargetVisible = false;
			m_flTargetLockTime = 0.0f;
		}
	}
}

bool CStickyCam::IsCameraViewClear(const Vec3& origin, const Vec3& angles)
{
	Vec3 forward;
	Math::AngleVectors(angles, &forward, nullptr, nullptr);
	
	CGameTrace trace;
	CTraceFilterHitscan filter;
	filter.pSkip = nullptr;
	
	SDK::Trace(origin, AddVectors(origin, MultiplyVector(forward, 50.0f)), MASK_SOLID, &filter, &trace);
	return trace.fraction > 0.9f;
}

float CStickyCam::GetStickyPlayerDistance(CBaseEntity* pSticky)
{
	if (!pSticky)
		return 0.0f;
	
	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return 0.0f;
	
	Vec3 stickyPos = pSticky->GetAbsOrigin();
	Vec3 playerPos = pLocal->GetAbsOrigin();
	
	return SubtractVectors(stickyPos, playerPos).Length();
}

void CStickyCam::DrawStickyRangeWarning()
{
	if (!m_pCurrentSticky)
		return;
	
	float distance = GetStickyPlayerDistance(m_pCurrentSticky);
	float warningDistance = STICKY_DORMANT_DISTANCE * STICKY_WARNING_THRESHOLD;
	
	if (!Vars::Competitive::StickyCam::ShowWarnings.Value || distance <= warningDistance)
		return;
		
	float remainingDistance = STICKY_DORMANT_DISTANCE - distance;
	std::string warningText = std::format("WARNING: Camera will go dormant in {:.1f} units", remainingDistance);
	
	// Get camera position for positioning
	int camX, camY;
	GetCameraPosition(camX, camY);
	
	// Calculate text dimensions
	auto font = H::Fonts.GetFont(FONT_ESP);
	Vec2 textSize = H::Draw.GetTextSize(warningText.c_str(), font);
	int textW = static_cast<int>(textSize.x);
	int textH = static_cast<int>(textSize.y);
	
	int warningX = camX + (GetCameraWidth() - textW) / 2;
	int warningY = camY + 10;
	
	// Draw warning background
	H::Draw.FillRect(warningX - 5, warningY - 5, textW + 10, textH + 10, {0, 0, 0, 180});
	
	// Draw warning text
	H::Draw.StringOutlined(H::Fonts.GetFont(FONT_ESP), warningX, warningY, {255, 50, 50, 255}, {0, 0, 0, 255}, ALIGN_TOPLEFT, warningText.c_str());
	
	// Draw distance bar
	int barWidth = 200;
	int barHeight = 6;
	int barX = camX + (GetCameraWidth() - barWidth) / 2;
	int barY = warningY + textH + 8;
	
	// Background bar
	H::Draw.FillRect(barX, barY, barWidth, barHeight, {50, 50, 50, 180});
	
	// Progress bar
	float progress = std::max(0.0f, std::min(1.0f, 1.0f - (distance / STICKY_DORMANT_DISTANCE)));
	int progressWidth = static_cast<int>(barWidth * progress);
	
	// Color changes from green to red based on distance
	byte r = static_cast<byte>(std::min(255.0f, (1.0f - progress) * 510.0f));
	byte g = static_cast<byte>(std::min(255.0f, progress * 510.0f));
	H::Draw.FillRect(barX, barY, progressWidth, barHeight, {r, g, 0, 255});
}

bool CStickyCam::IsTargetWithinRange(CTFPlayer* pTarget, const Vec3& stickyPos)
{
	if (!pTarget || !pTarget->IsAlive() || pTarget->IsDormant())
		return false;
	
	Vec3 targetPos = pTarget->GetAbsOrigin();
	Vec3 delta = SubtractVectors(targetPos, stickyPos);
	return delta.Length() <= 166.0f; // STICKY_CHAMS_DISTANCE from Lua
}

void CStickyCam::DrawOverlay()
{
	if (!Vars::Competitive::StickyCam::ShowOverlay.Value || !m_pCurrentSticky)
		return;
	
	DrawStickyRangeWarning();
	
	// Get camera position
	int camX, camY;
	GetCameraPosition(camX, camY);
	
	// Get player name and status
	std::string playerName = "No Target";
	std::string targetStatus = "Searching";
	float timeRemaining = 0.0f;
	
	if (m_pCurrentTarget)
	{
		PlayerInfo_t pi{};
		try {
			if (I::EngineClient->GetPlayerInfo(m_pCurrentTarget->entindex(), &pi))
				playerName = pi.name;
			else
				playerName = "Unknown";
		}
		catch (...) {
			playerName = "Unknown";
		}
		
		timeRemaining = std::max(0.0f, Vars::Competitive::StickyCam::TrackTime.Value - (I::GlobalVars->curtime - m_flTargetLockTime));
		targetStatus = m_bTargetVisible ? "Tracking" : "Target Lost";
	}
	std::string modeText = (GetMode() == EStickyMode::FOLLOW_LATEST) ? "Latest" : "Manual";
	
	// Count available stickies
	int availableCount = 0;
	for (const auto& sticky : m_Stickies)
	{
		if (sticky.pSticky && !sticky.pSticky->IsDormant())
		{
			Vec3 velocity;
			try {
				sticky.pSticky->EstimateAbsVelocity(velocity);
			}
			catch (...) {
				continue;
			}
			if (velocity.Length() < 1.0f)
				availableCount++;
		}
	}
	
	std::string title = std::format("Sticky Camera [{}] - {}: {} ({:.1f}s) [{}/{}]",
		modeText, targetStatus, playerName, timeRemaining,
		std::min(static_cast<int>(m_VisitedStickies.size()), availableCount), availableCount);
	
	// Draw title
	auto titleFont = H::Fonts.GetFont(FONT_ESP);
	Vec2 titleSize = H::Draw.GetTextSize(title.c_str(), titleFont);
	int titleW = static_cast<int>(titleSize.x);
	int titleH = static_cast<int>(titleSize.y);
	int titleX = camX + GetCameraWidth() / 2 - titleW / 2;
	int titleY = camY - 16;
	
	H::Draw.StringOutlined(H::Fonts.GetFont(FONT_ESP), titleX, titleY, {255, 255, 255, 255}, {0, 0, 0, 255}, ALIGN_CENTER, title.c_str());
	
	// Draw controls (only show TAB in manual mode)
	if (GetMode() == EStickyMode::MANUAL)
	{
		H::Draw.StringOutlined(H::Fonts.GetFont(FONT_ESP), camX + 5, camY + GetCameraHeight() + 5,
			{255, 255, 255, 200}, {0, 0, 0, 255}, ALIGN_TOPLEFT, "TAB - Cycle stickies");
	}
	else
	{
		H::Draw.StringOutlined(H::Fonts.GetFont(FONT_ESP), camX + 5, camY + GetCameraHeight() + 5,
			{255, 255, 255, 200}, {0, 0, 0, 255}, ALIGN_TOPLEFT, "Auto-tracking latest sticky");
	}
	
	// Get team color for frame
	auto pLocal = H::Entities.GetLocal();
	Color_t frameColor = {235, 64, 52, 255}; // Default red
	Color_t fillColor = {130, 26, 17, 255}; // Default dark red
	
	if (pLocal)
	{
		frameColor = H::Color.GetTeamColor(pLocal->m_iTeamNum(), pLocal->m_iTeamNum(), false);
		// Make fill color darker by reducing RGB values
		fillColor = {
			static_cast<byte>(frameColor.r * 0.55f),
			static_cast<byte>(frameColor.g * 0.55f), 
			static_cast<byte>(frameColor.b * 0.55f),
			255
		};
	}
	
	// Draw team-colored camera outline
	H::Draw.LineRect(camX, camY, GetCameraWidth(), GetCameraHeight(), frameColor);
	H::Draw.LineRect(camX, camY - 20, GetCameraWidth(), 20, frameColor);
	H::Draw.FillRect(camX + 1, camY - 19, GetCameraWidth() - 2, 18, fillColor);
}

bool CStickyCam::CheckMaterialsNeedReload()
{
	int currentWidth = GetCameraWidth();
	int currentHeight = GetCameraHeight();
	
	// Check if size has changed since last initialization
	if (m_iLastWidth != currentWidth || m_iLastHeight != currentHeight)
	{
		// Update tracked dimensions
		m_iLastWidth = currentWidth;
		m_iLastHeight = currentHeight;
		return true;
	}
	
	return false;
}

void CStickyCam::GetCameraPosition(int& x, int& y) const
{
	// Get screen size
	int screenW, screenH;
	I::MatSystemSurface->GetScreenSize(screenW, screenH);
	
	// Check if user has set custom position (WindowX/WindowY >= 0)
	if (Vars::Competitive::StickyCam::WindowX.Value >= 0 && Vars::Competitive::StickyCam::WindowY.Value >= 0)
	{
		// Use custom position
		x = Vars::Competitive::StickyCam::WindowX.Value;
		y = Vars::Competitive::StickyCam::WindowY.Value;
		
		// Clamp to screen bounds
		x = std::max(0, std::min(x, screenW - GetCameraWidth()));
		y = std::max(20, std::min(y, screenH - GetCameraHeight())); // Leave space for title bar
	}
	else
	{
		// Use default position from Lua original
		x = 5;
		y = 300;
		
		// Still clamp to screen bounds
		x = std::max(0, std::min(x, screenW - GetCameraWidth()));
		y = std::max(20, std::min(y, screenH - GetCameraHeight()));
	}
}

CBaseEntity* CStickyCam::FindLatestSticky()
{
	CBaseEntity* pLatest = nullptr;
	float fLatestTime = 0.0f;
	
	for (const auto& sticky : m_Stickies)
	{
		if (!sticky.pSticky || sticky.pSticky->IsDormant())
			continue;
		
		// Check if sticky is stationary
		Vec3 velocity;
		try {
			sticky.pSticky->EstimateAbsVelocity(velocity);
		}
		catch (...) {
			continue;
		}
		if (velocity.Length() >= 1.0f)
			continue;
		
		if (sticky.flCreationTime > fLatestTime)
		{
			fLatestTime = sticky.flCreationTime;
			pLatest = sticky.pSticky;
		}
	}
	
	return pLatest;
}

void CStickyCam::Draw()
{
	if (!IsEnabled() || !IsDemoman() || !m_bInitialized || !I::EngineClient->IsInGame() || !I::EngineClient->IsConnected())
		return;
	
	if (I::EngineVGui->IsGameUIVisible())
		return;
	
	// Check if materials need reloading due to size changes
	if (CheckMaterialsNeedReload())
	{
		CleanupMaterials();
		InitializeMaterials();
	}
	
	// Only handle manual input in manual mode
	if (GetMode() == EStickyMode::MANUAL)
		UpdateUserInput();
		
	UpdateStickies();
	
	// Auto-select sticky based on mode
	if (GetMode() == EStickyMode::FOLLOW_LATEST)
	{
		CBaseEntity* pLatest = FindLatestSticky();
		if (pLatest && pLatest != m_pCurrentSticky)
		{
			m_pCurrentSticky = pLatest;
			m_pCurrentTarget = nullptr;
			m_bTargetVisible = false;
		}
	}
	
	if (!m_pCurrentSticky || !m_pCameraMaterial || !m_pCameraTexture)
		return;
	
	// Validate sticky entity before using it
	if (!m_pCurrentSticky || m_pCurrentSticky->IsDormant())
	{
		m_pCurrentSticky = nullptr;
		return;
	}
	
	// Additional entity validation to prevent crashes
	auto pEntityFromList = I::ClientEntityList->GetClientEntity(m_pCurrentSticky->entindex());
	if (!pEntityFromList || pEntityFromList != m_pCurrentSticky)
	{
		m_pCurrentSticky = nullptr;
		return;
	}
	
	Vec3 stickyPos, normal, cameraPos;
	try {
		stickyPos = m_pCurrentSticky->GetAbsOrigin();
		normal = GetStickyNormal(m_pCurrentSticky);
		cameraPos = CalculateCameraOffset(stickyPos, normal);
	}
	catch (...) {
		m_pCurrentSticky = nullptr;
		return;
	}
	
	m_pCurrentTarget = FindNearestVisiblePlayer(cameraPos);
	
	if (m_pCurrentTarget && m_bTargetVisible)
	{
		Vec3 targetPos = m_pCurrentTarget->GetAbsOrigin();
		targetPos.z += 50.0f;
		Vec3 targetAngles = CalculateAngles(cameraPos, targetPos);
		m_vSmoothedAngles = LerpAngles(m_vSmoothedAngles, targetAngles, Vars::Competitive::StickyCam::AngleSpeed.Value);
	}
	
	// Check if we should display the camera
	bool shouldDisplay = false;
	if (Vars::Competitive::StickyCam::AlwaysShow.Value)
	{
		// Always show if we have a sticky (even without targets)
		shouldDisplay = true;
	}
	else
	{
		// Only show when we have a visible target
		shouldDisplay = (m_pCurrentTarget && m_bTargetVisible);
	}
	
	if (!shouldDisplay)
		return;
	
	// Draw camera to screen
	if (!I::MaterialSystem)
		return;
		
	auto renderCtx = I::MaterialSystem->GetRenderContext();
	if (!renderCtx)
		return;
	
	// Get configured camera position
	int x, y;
	GetCameraPosition(x, y);
	
	try
	{
		if (m_pCameraTexture && m_pCameraMaterial)
		{
			renderCtx->DrawScreenSpaceRectangle(
				m_pCameraMaterial,
				x, y, GetCameraWidth(), GetCameraHeight(),
				0, 0, GetCameraWidth(), GetCameraHeight(),
				m_pCameraTexture->GetActualWidth(), m_pCameraTexture->GetActualHeight(),
				nullptr, 1, 1
			);
		}
		renderCtx->Release();
	}
	catch (...)
	{
		if (renderCtx)
			renderCtx->Release();
	}
	
	// Draw overlay
	DrawOverlay();
}

void CStickyCam::RenderView(void* ecx, const CViewSetup& view)
{
	if (!m_bInitialized || !m_pCurrentSticky || !m_pCameraTexture)
		return;
	
	// Additional entity validation to prevent crashes
	auto pEntityFromList = I::ClientEntityList->GetClientEntity(m_pCurrentSticky->entindex());
	if (!pEntityFromList || pEntityFromList != m_pCurrentSticky)
	{
		m_pCurrentSticky = nullptr;
		return;
	}
	
	// Check if we should render the camera view
	bool shouldRender = false;
	if (Vars::Competitive::StickyCam::AlwaysShow.Value)
	{
		// Always render if we have a sticky
		shouldRender = true;
	}
	else
	{
		// Only render when we have a visible target
		shouldRender = (m_pCurrentTarget && m_bTargetVisible);
	}
	
	if (!shouldRender)
		return;
	
	Vec3 stickyPos, normal, cameraOrigin;
	try {
		stickyPos = m_pCurrentSticky->GetAbsOrigin();
		normal = GetStickyNormal(m_pCurrentSticky);
		cameraOrigin = CalculateCameraOffset(stickyPos, normal);
	}
	catch (...) {
		m_pCurrentSticky = nullptr;
		return;
	}
	
	if (!IsCameraViewClear(cameraOrigin, m_vSmoothedAngles))
		return;
	
	// Apply offset if configured
	if (GetViewMode() == EStickyViewMode::OFFSET)
	{
		Vec3 forward, up;
		Math::AngleVectors(m_vSmoothedAngles, &forward, nullptr, &up);
		
		float offsetX = Vars::Competitive::StickyCam::OffsetX.Value;
		float offsetY = Vars::Competitive::StickyCam::OffsetY.Value;
		
		cameraOrigin += forward * offsetX;
		cameraOrigin += up * offsetY;
	}
	
	// Setup view
	CViewSetup stickyView = view;
	stickyView.x = 0;
	stickyView.y = 0;
	stickyView.width = GetCameraWidth();
	stickyView.height = GetCameraHeight();
	stickyView.m_flAspectRatio = static_cast<float>(GetCameraWidth()) / static_cast<float>(GetCameraHeight());
	stickyView.fov = 90;
	stickyView.origin = cameraOrigin;
	stickyView.angles = m_vSmoothedAngles;
	
	// Render to texture
	if (!I::MaterialSystem || !m_pCameraTexture)
		return;
		
	auto renderCtx = I::MaterialSystem->GetRenderContext();
	if (!renderCtx)
		return;
	
	try
	{
		renderCtx->PushRenderTargetAndViewport();
		renderCtx->SetRenderTarget(m_pCameraTexture);
		
		static auto ViewRender_RenderView = U::Hooks.m_mHooks["CViewRender_RenderView"];
		if (ViewRender_RenderView)
			ViewRender_RenderView->Call<void>(ecx, stickyView, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH, RENDERVIEW_UNSPECIFIED);
		
		renderCtx->PopRenderTargetAndViewport();
		renderCtx->Release();
	}
	catch (...)
	{
		if (renderCtx)
		{
			try { renderCtx->PopRenderTargetAndViewport(); } catch (...) {}
			renderCtx->Release();
		}
	}
}

void CStickyCam::Reset()
{
	m_pCurrentSticky = nullptr;
	m_pCurrentTarget = nullptr;
	m_vSmoothedAngles = Vec3(0, 0, 0);
	m_flTargetLockTime = 0.0f;
	m_bTargetVisible = false;
	m_flLastOcclusionCheck = 0.0f;
	m_flLastKeyPress = 0.0f;
	m_VisitedStickies.clear();
}

void CStickyCam::UpdateChamsEntities()
{
	// Clear previous chams entries
	m_mEntities.clear();
	
	if (!IsEnabled() || !IsDemoman() || !Vars::Competitive::StickyCam::ShowChams.Value)
		return;
	
	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return;
	
	// Apply chams to players within damage radius of all our stickies
	for (const auto& stickyData : m_Stickies)
	{
		if (!stickyData.pSticky || stickyData.pSticky->IsDormant())
			continue;
		
		// Additional entity validation to prevent crashes
		int stickyIndex = 0;
		try {
			stickyIndex = stickyData.pSticky->entindex();
		}
		catch (...) {
			continue;
		}
		
		if (stickyIndex <= 0 || stickyIndex >= 2048)
			continue;
		
		auto pEntity = I::ClientEntityList->GetClientEntity(stickyIndex);
		if (!pEntity || pEntity != stickyData.pSticky)
			continue;
		
		// Check if sticky is stationary
		Vec3 velocity;
		try {
			stickyData.pSticky->EstimateAbsVelocity(velocity);
		}
		catch (...) {
			continue;
		}
		if (velocity.Length() >= 1.0f)
			continue;
		
		// Get sticky position and damage radius  
		auto pGrenade = stickyData.pSticky->As<CBaseGrenade>();
		if (!pGrenade)
			continue;
		
		// Additional validation before accessing grenade properties
		Vec3 stickyPos;
		float damageRadius;
		
		try {
			stickyPos = stickyData.pSticky->GetAbsOrigin();
			damageRadius = pGrenade->m_DmgRadius();
		}
		catch (...) {
			// Skip this sticky if accessing its properties fails
			continue;
		}
		if (damageRadius <= 0.0f)
			damageRadius = 146.0f; // Default sticky damage radius
		
		// Check all players for proximity to this sticky
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant())
				continue;
			
			// Skip local player
			if (pPlayer == pLocal)
				continue;
			
			// Skip teammates
			if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;
			
			// Check distance to sticky
			Vec3 playerPos = pPlayer->GetAbsOrigin();
			Vec3 delta = SubtractVectors(playerPos, stickyPos);
			float distance = delta.Length();
			
			if (distance <= damageRadius)
			{
				// Player is within sticky damage radius - apply chams
				m_mEntities[pPlayer->entindex()] = true;
			}
		}
	}
}