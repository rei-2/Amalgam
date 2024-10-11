#include "../SDK/SDK.h"

MAKE_HOOK(IPanel_PaintTraverse, U::Memory.GetVFunc(I::Panel, 41), void, __fastcall,
	void* rcx, VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	if (!Vars::Visuals::UI::StreamerMode.Value)
		return CALL_ORIGINAL(rcx, vguiPanel, forceRepaint, allowForce);

	auto uHash = FNV1A::Hash32(I::Panel->GetName(vguiPanel));
	if (uHash == FNV1A::Hash32Const("SteamFriendsList") || uHash == FNV1A::Hash32Const("avatar") || uHash == FNV1A::Hash32Const("RankPanel"))
		return;

	CALL_ORIGINAL(rcx, vguiPanel, forceRepaint, allowForce);
}