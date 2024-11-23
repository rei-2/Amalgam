#include "../SDK/SDK.h"

MAKE_HOOK(IPanel_PaintTraverse, U::Memory.GetVFunc(I::Panel, 41), void,
	void* rcx, VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	if (!Vars::Visuals::UI::StreamerMode.Value)
		return CALL_ORIGINAL(rcx, vguiPanel, forceRepaint, allowForce);

	switch (FNV1A::Hash32(I::Panel->GetName(vguiPanel)))
	{
	case FNV1A::Hash32Const("SteamFriendsList"):
	case FNV1A::Hash32Const("avatar"):
	case FNV1A::Hash32Const("RankPanel"):
	case FNV1A::Hash32Const("ModelContainer"):
	case FNV1A::Hash32Const("ServerLabelNew"):
		return;
	}

	CALL_ORIGINAL(rcx, vguiPanel, forceRepaint, allowForce);
}