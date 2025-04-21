#include "../SDK/SDK.h"
#include "../Features/Visuals/Materials/Materials.h"

MAKE_HOOK(IMatSystemSurface_OnScreenSizeChanged, U::Memory.GetVFunc(I::MatSystemSurface, 111), void,
	void* rcx, int nOldWidth, int nOldHeight)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IMatSystemSurface_OnScreenSizeChanged[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, nOldWidth, nOldHeight);
#endif

	int nNewWidth, nNewHeight; I::MatSystemSurface->GetScreenSize(nNewWidth, nNewHeight);
	SDK::Output("Size", std::format("{}, {} -> {}, {}", nOldWidth, nOldHeight, nNewWidth, nNewHeight).c_str(), {}, true, true);

	CALL_ORIGINAL(rcx, nOldWidth, nOldHeight);

	H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);
	F::Materials.ReloadMaterials();
}