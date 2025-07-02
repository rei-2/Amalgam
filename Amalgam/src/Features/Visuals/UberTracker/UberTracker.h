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
	static constexpr bool SHOW_TOP_LEFT_BOX = true;
	static constexpr bool SHOW_ADV_TEXT = true;
	static constexpr bool SHOW_KRITZ = true;
	static constexpr bool IGNORE_KRITZ = false;
	
	static constexpr float MED_DEATH_DURATION = 3.0f;
	static constexpr int EVEN_THRESHOLD_RANGE = 5;
	static constexpr int MID_UBER_THRESHOLD = 40;
	static constexpr int HIGH_UBER_THRESHOLD = 70;
	
	static constexpr float UBER_RATE = 2.5f;
	static constexpr float KRITZ_RATE = 3.125f;
	static constexpr float KRITZ_DROP_THRESHOLD = 10.0f;
	
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