#include "AutoQueue.h"

void CAutoQueue::Run()
{
	if (Vars::Misc::Queueing::AutoCasualQueue.Value && !I::TFPartyClient->BInQueueForMatchGroup(k_eTFMatchGroup_Casual_Default))
	{
		static bool bHasLoaded = false;
		if (!bHasLoaded)
		{
			I::TFPartyClient->LoadSavedCasualCriteria();
			bHasLoaded = true;
		}
		I::TFPartyClient->RequestQueueForMatch(k_eTFMatchGroup_Casual_Default);
	}
}