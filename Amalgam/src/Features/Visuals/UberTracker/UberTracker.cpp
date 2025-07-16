#include "UberTracker.h"
#include "../../../SDK/SDK.h"
#include "../../Players/PlayerUtils.h"

std::string CUberTracker::GetMedicID(CBaseEntity* pEntity)
{
	if (!pEntity)
		return "unknown";
	
	try {
		int entityIndex = pEntity->entindex();
		if (entityIndex <= 0 || entityIndex >= 2048)
			return "unknown";
		
		auto pEntityFromList = I::ClientEntityList->GetClientEntity(entityIndex);
		if (!pEntityFromList || pEntityFromList != pEntity)
			return "unknown";
		
		return std::to_string(entityIndex) + "_unnamed";
	}
	catch (...) {
		return "unknown";
	}
}

std::string CUberTracker::GetWeaponType(int iItemDefinitionIndex, bool* bIsKritzOverride)
{
	if (bIsKritzOverride)
		*bIsKritzOverride = false;
	
	if (Vars::Competitive::UberTracker::IgnoreKritz.Value && iItemDefinitionIndex == 35)
	{
		if (bIsKritzOverride)
			*bIsKritzOverride = true;
		return "UBER";
	}
	
	switch (iItemDefinitionIndex)
	{
		case 29:   // Standard Medigun
		case 211:  // Festive Medigun
		case 663:  // Silver Botkiller Medigun
			return "UBER";
		case 35:   // Kritzkrieg
			return "KRITZ";
		case 411:  // Quick-Fix
			return "QUICKFIX";
		case 998:  // Vaccinator
			return "VACCINATOR";
		default:
			return "UBER";
	}
}

Color_t CUberTracker::GetColorForUber(float flPercentage, bool bIsAlive)
{
	if (!bIsAlive || flPercentage == 0.0f)
		return { 170, 170, 170, 255 };
	
	if (flPercentage > Vars::Competitive::UberTracker::HighUberThreshold.Value)
		return { 0, 255, 0, 255 };
	
	if (flPercentage > Vars::Competitive::UberTracker::MidUberThreshold.Value)
		return { 255, 255, 0, 255 };
	
	return { 255, 0, 0, 255 };
}

bool CUberTracker::DetectKritzUsage(KritzTracker& tracker, float flActualKritzPercentage, float flCurrentTime)
{
	if (tracker.LastKritzValue > 0 && flActualKritzPercentage < tracker.LastKritzValue - Vars::Competitive::UberTracker::KritzDropThreshold.Value)
		return true;
	
	tracker.WasBuilding = flActualKritzPercentage > tracker.LastKritzValue;
	return false;
}

float CUberTracker::TranslateKritzToUber(CBaseEntity* pEntity, const std::string& sMedicID, float flActualKritzPercentage, float flCurrentTime)
{
	if (m_KritzTrackers.find(sMedicID) == m_KritzTrackers.end())
	{
		KritzTracker& tracker = m_KritzTrackers[sMedicID];
		tracker.LastKritzValue = flActualKritzPercentage;
		tracker.LastUberValue = flActualKritzPercentage * (Vars::Competitive::UberTracker::UberRate.Value / Vars::Competitive::UberTracker::KritzRate.Value);
		tracker.LastUpdateTime = flCurrentTime;
		return static_cast<float>(static_cast<int>(tracker.LastUberValue));
	}
	
	KritzTracker& tracker = m_KritzTrackers[sMedicID];
	
	if (DetectKritzUsage(tracker, flActualKritzPercentage, flCurrentTime))
	{
		tracker.WasUsed = true;
		tracker.LastKritzValue = flActualKritzPercentage;
		tracker.LastUberValue = 0.0f;
		tracker.LastUpdateTime = flCurrentTime;
		
		int iTeamNumber = 0;
		try {
			iTeamNumber = pEntity->m_iTeamNum();
		}
		catch (...) {
			return flActualKritzPercentage;
		}
		
		auto pLocal = H::Entities.GetLocal();
		if (pLocal)
		{
			int iLocalTeam = pLocal->m_iTeamNum();
			m_Status[iTeamNumber] = (iTeamNumber == iLocalTeam) ? "WE USED" : "THEY USED";
			m_UberDecreasing[iTeamNumber] = true;
		}
		
		return tracker.LastUberValue;
	}
	
	if (flActualKritzPercentage > tracker.LastKritzValue)
	{
		float flKritzIncrease = flActualKritzPercentage - tracker.LastKritzValue;
		float flUberIncrease = flKritzIncrease * (Vars::Competitive::UberTracker::UberRate.Value / Vars::Competitive::UberTracker::KritzRate.Value);
		tracker.LastUberValue = std::min(100.0f, tracker.LastUberValue + flUberIncrease);
	}
	else if (flActualKritzPercentage < tracker.LastKritzValue)
	{
		float flKritzDecrease = tracker.LastKritzValue - flActualKritzPercentage;
		tracker.LastUberValue = std::max(0.0f, tracker.LastUberValue - flKritzDecrease);
	}
	
	tracker.LastKritzValue = flActualKritzPercentage;
	tracker.LastUpdateTime = flCurrentTime;
	
	return static_cast<float>(static_cast<int>(tracker.LastUberValue));
}

void CUberTracker::DrawMedicInfo(const std::vector<MedicInfo>& vMedics, int iStartY)
{
	if (!Vars::Competitive::UberTracker::ShowTopBox.Value || vMedics.empty())
		return;
	
	int boxX = Vars::Competitive::UberTracker::BoxX.Value;
	int boxY = Vars::Competitive::UberTracker::BoxY.Value;
	int boxWidth = Vars::Competitive::UberTracker::BoxWidth.Value;
	int boxFontSize = Vars::Competitive::UberTracker::BoxFontSize.Value;
	
	int lineHeight = boxFontSize + 2;
	int iBoxHeight = std::max(30, 25 + static_cast<int>(vMedics.size() * lineHeight));
	H::Draw.FillRect(boxX, boxY - 5, boxWidth, iBoxHeight, { 0, 0, 0, 100 });
	
	for (size_t i = 0; i < vMedics.size(); ++i)
	{
		const auto& medic = vMedics[i];
		int iYPos = boxY + 15 + static_cast<int>(i * lineHeight);
		Color_t color = GetColorForUber(medic.UberPercentage, medic.IsAlive);
		
		std::string sNameWeapon = medic.Name + " -> " + medic.WeaponName;
		std::string sUberText = medic.IsAlive ? (std::to_string(static_cast<int>(medic.UberPercentage)) + "%") : "DEAD";
		
		// Get appropriate font based on info box font size
		auto font = (boxFontSize <= 12) ? H::Fonts.GetFont(FONT_INDICATORS) : H::Fonts.GetFont(FONT_ESP);
		
		H::Draw.String(font, boxX + 15, iYPos, color, ALIGN_TOPLEFT, sNameWeapon.c_str());
		H::Draw.String(font, boxX + boxWidth - 80, iYPos, color, ALIGN_TOPLEFT, sUberText.c_str());
	}
}

void CUberTracker::DrawAdvantageText(const std::vector<MedicInfo>& vRedMedics, const std::vector<MedicInfo>& vBluMedics, int iLocalTeam)
{
	if (!Vars::Competitive::UberTracker::ShowAdvantage.Value || vRedMedics.empty() || vBluMedics.empty())
		return;
	
	const auto& redMedic = vRedMedics[0];
	const auto& bluMedic = vBluMedics[0];
	
	if (iLocalTeam != TF_TEAM_RED && iLocalTeam != TF_TEAM_BLUE)
		return;
	
	float flRedUber = redMedic.IsAlive ? redMedic.UberPercentage : 0.0f;
	float flBluUber = bluMedic.IsAlive ? bluMedic.UberPercentage : 0.0f;
	
	int iEnemyTeam = (iLocalTeam == TF_TEAM_RED) ? TF_TEAM_BLUE : TF_TEAM_RED;
	int iFriendlyTeam = iLocalTeam;
	
	float flDifference = (iLocalTeam == TF_TEAM_RED) ? (flRedUber - flBluUber) : (flBluUber - flRedUber);
	
	const auto& friendlyMedic = (iFriendlyTeam == TF_TEAM_RED) ? redMedic : bluMedic;
	const auto& enemyMedic = (iEnemyTeam == TF_TEAM_RED) ? redMedic : bluMedic;
	
	std::string sDisplayText;
	Color_t textColor;
	
	float flCurrentTime = I::GlobalVars->curtime;
	float flFriendlyDeathTime = m_MedDeathTime[iFriendlyTeam];
	float flEnemyDeathTime = m_MedDeathTime[iEnemyTeam];
	
	std::string sFriendlyStatus = m_Status[iFriendlyTeam];
	std::string sEnemyStatus = m_Status[iEnemyTeam];
	
	if (sFriendlyStatus == "MED DIED")
	{
		if (flCurrentTime - flFriendlyDeathTime <= Vars::Competitive::UberTracker::MedDeathDuration.Value)
		{
			sDisplayText = "OUR MED DIED";
			textColor = { 255, 0, 0, 255 };
		}
		else
		{
			sDisplayText = "FULL DISAD";
			textColor = { 255, 0, 0, 255 };
		}
	}
	else if (sEnemyStatus == "MED DIED" && flCurrentTime - flEnemyDeathTime <= Vars::Competitive::UberTracker::MedDeathDuration.Value)
	{
		sDisplayText = "THEIR MED DIED";
		textColor = { 0, 255, 0, 255 };
	}
	else if (!enemyMedic.IsAlive)
	{
		sDisplayText = "FULL AD";
		textColor = { 0, 255, 0, 255 };
	}
	else if (!friendlyMedic.IsAlive)
	{
		sDisplayText = "FULL DISAD";
		textColor = { 255, 0, 0, 255 };
	}
	else if (friendlyMedic.UberPercentage == 100.0f && enemyMedic.UberPercentage < 100.0f)
	{
		sDisplayText = "FULL AD: " + std::to_string(static_cast<int>(flDifference)) + "%";
		textColor = { 0, 255, 0, 255 };
	}
	else if (enemyMedic.UberPercentage == 100.0f && friendlyMedic.UberPercentage < 100.0f)
	{
		sDisplayText = "FULL DISAD: " + std::to_string(static_cast<int>(std::abs(flDifference))) + "%";
		textColor = { 255, 0, 0, 255 };
	}
	else if (sEnemyStatus == "THEY USED")
	{
		sDisplayText = "THEY USED";
		textColor = { 255, 165, 0, 255 };
	}
	else if (sFriendlyStatus == "WE USED")
	{
		sDisplayText = "WE USED";
		textColor = { 0, 191, 255, 255 };
	}
	else if (std::abs(flDifference) <= Vars::Competitive::UberTracker::EvenThresholdRange.Value)
	{
		sDisplayText = "EVEN";
		textColor = { 128, 128, 128, 255 };
	}
	else if (flDifference > 0)
	{
		sDisplayText = "AD: " + std::to_string(static_cast<int>(flDifference)) + "%";
		textColor = { 0, 255, 0, 255 };
	}
	else
	{
		sDisplayText = "DISAD: " + std::to_string(static_cast<int>(std::abs(flDifference))) + "%";
		textColor = { 255, 0, 0, 255 };
	}
	
	int iScreenW, iScreenH;
	I::MatSystemSurface->GetScreenSize(iScreenW, iScreenH);
	int iX = (iScreenW / 2) - 250 + Vars::Competitive::UberTracker::AdvantageX.Value;
	int iY = (iScreenH / 2) + 250 + Vars::Competitive::UberTracker::AdvantageY.Value;
	int advantageFontSize = Vars::Competitive::UberTracker::AdvantageFontSize.Value;
	
	// Get appropriate font based on advantage font size
	auto font = (advantageFontSize <= 12) ? H::Fonts.GetFont(FONT_INDICATORS) : H::Fonts.GetFont(FONT_ESP);
	
	if (Vars::Competitive::UberTracker::ShowKritz.Value && enemyMedic.IsAlive && enemyMedic.RealWeaponName == "KRITZ" && !Vars::Competitive::UberTracker::IgnoreKritz.Value)
	{
		H::Draw.String(font, iX, iY - 25, { 128, 0, 128, 255 }, ALIGN_CENTER, "KRITZ");
	}
	
	H::Draw.String(font, iX, iY, textColor, ALIGN_CENTER, sDisplayText.c_str());
}

void CUberTracker::Draw()
{
	if (!Vars::Competitive::Features::UberTracker.Value)
		return;
	
	if (I::EngineVGui->IsGameUIVisible())
		return;
	
	if (!I::EngineClient->IsConnected() || !I::EngineClient->IsInGame())
		return;
	
	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return;
	
	int iLocalTeam = pLocal->m_iTeamNum();
	if (iLocalTeam != TF_TEAM_RED && iLocalTeam != TF_TEAM_BLUE)
		return;
	
	float flCurrentTime = I::GlobalVars->curtime;
	
	std::vector<MedicInfo> vRedMedics, vBluMedics;
	
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer)
			continue;
		
		// Validate entity before accessing members
		int iTeamNumber = 0;
		bool bIsAlive = false;
		CTFWeaponBase* pMedigun = nullptr;
		
		try {
			int entityIndex = pPlayer->entindex();
			if (entityIndex <= 0 || entityIndex > 64)
				continue;
			
			auto pEntityFromList = I::ClientEntityList->GetClientEntity(entityIndex);
			if (!pEntityFromList || pEntityFromList != pPlayer)
				continue;
			
			if (pPlayer->m_iClass() != TF_CLASS_MEDIC)
				continue;
			
			iTeamNumber = pPlayer->m_iTeamNum();
			if (iTeamNumber != TF_TEAM_RED && iTeamNumber != TF_TEAM_BLUE)
				continue;
			
			bIsAlive = pPlayer->IsAlive();
			pMedigun = pPlayer->GetWeaponFromSlot(SLOT_SECONDARY);
		}
		catch (...) {
			continue;
		}
		
		if (!pMedigun)
			continue;
		
		auto pMedigunWeapon = pMedigun->As<CWeaponMedigun>();
		if (!pMedigunWeapon)
			continue;
		
		float flChargeLevel = 0.0f;
		int iItemDefIndex = 0;
		
		try {
			flChargeLevel = pMedigunWeapon->m_flChargeLevel();
			iItemDefIndex = pMedigun->m_iItemDefinitionIndex();
		}
		catch (...) {
			continue;
		}
		
		bool bIsKritzOverride = false;
		std::string sWeaponName = GetWeaponType(iItemDefIndex, &bIsKritzOverride);
		std::string sRealWeaponName = sWeaponName; // Start with same value
		
		float flPercentageValue = flChargeLevel * 100.0f;
		
		// If this is a Kritzkrieg but we're showing it as Uber (when IGNORE_KRITZ=true)
		if (bIsKritzOverride)
		{
			std::string sMedicID = GetMedicID(pPlayer);
			float flActualKritzPercentage = flPercentageValue;
			
			// Translate Kritz percentage to Uber percentage
			flPercentageValue = TranslateKritzToUber(pPlayer, sMedicID, flActualKritzPercentage, flCurrentTime);
			
			// Store real weapon name for kritz warning
			sRealWeaponName = "KRITZ";
		}
		
		MedicInfo medicInfo;
		// Get player name properly
		PlayerInfo_t pi{};
		try {
			if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi))
				medicInfo.Name = F::PlayerUtils.GetPlayerName(pPlayer->entindex(), pi.name);
			else
				medicInfo.Name = "Medic " + std::to_string(pPlayer->entindex());
		}
		catch (...) {
			medicInfo.Name = "Unknown Medic";
		}
		medicInfo.WeaponName = sWeaponName;
		medicInfo.RealWeaponName = sRealWeaponName;
		medicInfo.UberPercentage = flPercentageValue;
		medicInfo.IsAlive = bIsAlive;
		medicInfo.TeamNumber = iTeamNumber;
		
		if (iTeamNumber == TF_TEAM_RED)
			vRedMedics.push_back(medicInfo);
		else
			vBluMedics.push_back(medicInfo);
		
		if (!bIsAlive && m_Status[iTeamNumber] != "MED DIED")
		{
			m_Status[iTeamNumber] = "MED DIED";
			m_MedDeathTime[iTeamNumber] = flCurrentTime;
			m_UberDecreasing[iTeamNumber] = false;
		}
		else if (m_Status[iTeamNumber] == "MED DIED" && bIsAlive)
		{
			m_Status[iTeamNumber] = "";
		}
		else if (bIsAlive && flPercentageValue < m_PrevUber[iTeamNumber] - 10.0f)
		{
			if (m_Status[iTeamNumber] != "THEY USED" && m_Status[iTeamNumber] != "WE USED")
			{
				m_Status[iTeamNumber] = (iTeamNumber == iLocalTeam) ? "WE USED" : "THEY USED";
			}
			m_UberDecreasing[iTeamNumber] = true;
		}
		else if (bIsAlive && flPercentageValue > m_PrevUber[iTeamNumber] && m_UberDecreasing[iTeamNumber])
		{
			m_Status[iTeamNumber] = "";
			m_UberDecreasing[iTeamNumber] = false;
		}
		
		m_PrevUber[iTeamNumber] = flPercentageValue;
	}
	
	std::vector<MedicInfo> vAllMedics;
	vAllMedics.insert(vAllMedics.end(), vRedMedics.begin(), vRedMedics.end());
	vAllMedics.insert(vAllMedics.end(), vBluMedics.begin(), vBluMedics.end());
	
	if (Vars::Competitive::UberTracker::ShowTopBox.Value)
		DrawMedicInfo(vAllMedics, 0); // Y position is now handled inside DrawMedicInfo
	
	if (Vars::Competitive::UberTracker::ShowAdvantage.Value)
		DrawAdvantageText(vRedMedics, vBluMedics, iLocalTeam);
}