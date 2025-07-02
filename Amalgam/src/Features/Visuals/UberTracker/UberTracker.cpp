#include "UberTracker.h"
#include "../../../SDK/SDK.h"

std::string CUberTracker::GetMedicID(CBaseEntity* pEntity)
{
	if (!pEntity)
		return "unknown";
	
	return std::to_string(pEntity->entindex()) + "_unnamed";
}

std::string CUberTracker::GetWeaponType(int iItemDefinitionIndex, bool* bIsKritzOverride)
{
	if (bIsKritzOverride)
		*bIsKritzOverride = false;
	
	if (IGNORE_KRITZ && iItemDefinitionIndex == 35)
	{
		if (bIsKritzOverride)
			*bIsKritzOverride = true;
		return "UBER";
	}
	
	switch (iItemDefinitionIndex)
	{
		case 29:
		case 211:
		case 663:
			return "UBER";
		case 35:
			return "KRITZ";
		case 411:
			return "QUICKFIX";
		case 998:
			return "VACCINATOR";
		default:
			return "UBER";
	}
}

Color_t CUberTracker::GetColorForUber(float flPercentage, bool bIsAlive)
{
	if (!bIsAlive || flPercentage == 0.0f)
		return { 170, 170, 170, 255 };
	
	if (flPercentage > HIGH_UBER_THRESHOLD)
		return { 0, 255, 0, 255 };
	
	if (flPercentage > MID_UBER_THRESHOLD)
		return { 255, 255, 0, 255 };
	
	return { 255, 0, 0, 255 };
}

bool CUberTracker::DetectKritzUsage(KritzTracker& tracker, float flActualKritzPercentage, float flCurrentTime)
{
	if (tracker.LastKritzValue > 0 && flActualKritzPercentage < tracker.LastKritzValue - KRITZ_DROP_THRESHOLD)
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
		tracker.LastUberValue = flActualKritzPercentage * (UBER_RATE / KRITZ_RATE);
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
		
		int iTeamNumber = pEntity->m_iTeamNum();
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
		float flUberIncrease = flKritzIncrease * (UBER_RATE / KRITZ_RATE);
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
	if (!SHOW_TOP_LEFT_BOX || vMedics.empty())
		return;
	
	int iBoxHeight = std::max(30, 25 + static_cast<int>(vMedics.size() * 15));
	H::Draw.FillRect(5, iStartY - 5, 24 * 15, iBoxHeight, { 0, 0, 0, 100 });
	
	for (size_t i = 0; i < vMedics.size(); ++i)
	{
		const auto& medic = vMedics[i];
		int iYPos = iStartY + 15 + static_cast<int>(i * 15);
		Color_t color = GetColorForUber(medic.UberPercentage, medic.IsAlive);
		
		std::string sNameWeapon = medic.Name + " -> " + medic.WeaponName;
		std::string sUberText = medic.IsAlive ? (std::to_string(static_cast<int>(medic.UberPercentage)) + "%") : "DEAD";
		
		H::Draw.String(H::Fonts.GetFont(FONT_ESP), 20, iYPos, color, ALIGN_TOPLEFT, sNameWeapon.c_str());
		H::Draw.String(H::Fonts.GetFont(FONT_ESP), 22 * 15, iYPos, color, ALIGN_TOPLEFT, sUberText.c_str());
	}
}

void CUberTracker::DrawAdvantageText(const std::vector<MedicInfo>& vRedMedics, const std::vector<MedicInfo>& vBluMedics, int iLocalTeam)
{
	if (!SHOW_ADV_TEXT || vRedMedics.empty() || vBluMedics.empty())
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
		if (flCurrentTime - flFriendlyDeathTime <= MED_DEATH_DURATION)
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
	else if (sEnemyStatus == "MED DIED" && flCurrentTime - flEnemyDeathTime <= MED_DEATH_DURATION)
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
	else if (std::abs(flDifference) <= EVEN_THRESHOLD_RANGE)
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
	int iX = (iScreenW / 2) - 250;
	int iY = (iScreenH / 2) + 250;
	
	if (SHOW_KRITZ && enemyMedic.IsAlive && enemyMedic.RealWeaponName == "KRITZ" && !IGNORE_KRITZ)
	{
		H::Draw.String(H::Fonts.GetFont(FONT_ESP), iX, iY - 25, { 128, 0, 128, 255 }, ALIGN_CENTER, "KRITZ");
	}
	
	H::Draw.String(H::Fonts.GetFont(FONT_ESP), iX, iY, textColor, ALIGN_CENTER, sDisplayText.c_str());
}

void CUberTracker::Draw()
{
	if (I::EngineVGui->IsGameUIVisible())
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
		if (!pPlayer || pPlayer->m_iClass() != TF_CLASS_MEDIC)
			continue;
		
		int iTeamNumber = pPlayer->m_iTeamNum();
		if (iTeamNumber != TF_TEAM_RED && iTeamNumber != TF_TEAM_BLUE)
			continue;
		
		bool bIsAlive = pPlayer->IsAlive();
		auto pMedigun = pPlayer->GetWeaponFromSlot(SLOT_SECONDARY);
		
		if (!pMedigun)
			continue;
		
		auto pMedigunWeapon = pMedigun->As<CWeaponMedigun>();
		if (!pMedigunWeapon)
			continue;
		
		float flChargeLevel = pMedigunWeapon->m_flChargeLevel();
		int iItemDefIndex = pMedigun->GetWeaponID();
		
		bool bIsKritzOverride = false;
		std::string sWeaponName = GetWeaponType(iItemDefIndex, &bIsKritzOverride);
		std::string sRealWeaponName = sWeaponName;
		
		float flPercentageValue = flChargeLevel * 100.0f;
		
		if (bIsKritzOverride)
		{
			std::string sMedicID = GetMedicID(pPlayer);
			float flActualKritzPercentage = flPercentageValue;
			flPercentageValue = TranslateKritzToUber(pPlayer, sMedicID, flActualKritzPercentage, flCurrentTime);
			sRealWeaponName = "KRITZ";
		}
		
		MedicInfo medicInfo;
		medicInfo.Name = "Medic " + std::to_string(pPlayer->entindex());
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
	
	DrawMedicInfo(vAllMedics, 115);
	DrawAdvantageText(vRedMedics, vBluMedics, iLocalTeam);
}