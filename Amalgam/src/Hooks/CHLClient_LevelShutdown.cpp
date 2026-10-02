#include "../SDK/SDK.h"

#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Spectate/Spectate.h"

void ResetTeamStatusPointer(); // CTFPlayerPanel_GetTeam.cpp

MAKE_HOOK(CHLClient_LevelShutdown, U::Memory.GetVirtual(I::Client, 7), void,
	void* rcx)
{
	DEBUG_RETURN(CHLClient_LevelShutdown, rcx);

	H::Entities.Clear(true);
	F::EnginePrediction.Unload();
	F::Spectate.Reset();
	ResetTeamStatusPointer();

	CALL_ORIGINAL(rcx);
}