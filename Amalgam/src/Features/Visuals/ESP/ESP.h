#pragma once
#include "../../../SDK/SDK.h"

class CESP
{
private:
	static const wchar_t* GetPlayerClass(int nClassNum);

	void DrawPlayers(CTFPlayer* pLocal);
	void DrawBuildings(CTFPlayer* pLocal);
	void DrawWorld();
	static void DrawBones(CTFPlayer* pPlayer, std::vector<int> vecBones, Color_t clr);

public:
	void Run(CTFPlayer* pLocal);
	bool GetDrawBounds(CBaseEntity* pEntity, int& x, int& y, int& w, int& h);
};

ADD_FEATURE(CESP, ESP)