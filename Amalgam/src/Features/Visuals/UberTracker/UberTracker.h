#pragma once
#include "../../../SDK/SDK.h"
#include <map>

struct MedicInfo
{
	std::string Name;
	std::string WeaponName;
	std::string RealWeaponName;
	float UberPercentage;
	bool IsAlive;
	int TeamNumber;
};

struct KritzTracker
{
	float LastKritzValue = 0.0f;
	float LastUberValue = 0.0f;
	float LastUpdateTime = 0.0f;
	bool WasUsed = false;
	bool WasBuilding = false;
};

class CUberTracker
{
private:
	// Configuration values are now read from Vars::Competitive::UberTracker::* variables
	
	std::map<int, float> m_PrevUber;
	std::map<int, std::string> m_Status;
	std::map<int, bool> m_UberDecreasing;
	std::map<int, float> m_MedDeathTime;
	std::map<std::string, KritzTracker> m_KritzTrackers;
	
	std::string GetMedicID(CBaseEntity* pEntity);
	std::string GetWeaponType(int iItemDefinitionIndex, bool* bIsKritzOverride = nullptr);
	Color_t GetColorForUber(float flPercentage, bool bIsAlive);
	bool DetectKritzUsage(KritzTracker& tracker, float flActualKritzPercentage, float flCurrentTime);
	float TranslateKritzToUber(CBaseEntity* pEntity, const std::string& sMedicID, float flActualKritzPercentage, float flCurrentTime);
	void DrawMedicInfo(const std::vector<MedicInfo>& vMedics, int iStartY);
	void DrawAdvantageText(const std::vector<MedicInfo>& vRedMedics, const std::vector<MedicInfo>& vBluMedics, int iLocalTeam);
	
public:
	void Draw();
};

ADD_FEATURE(CUberTracker, UberTracker)