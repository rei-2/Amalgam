#include "Direct3DDevice9_EndScene.h"

#include "../SDK/SDK.h"
#include "../Features/ImGui/Render.h"
#include "../Features/ImGui/Menu/Menu.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

MAKE_HOOK(Direct3DDevice9_EndScene, U::Memory.GetVFunc(I::DirectXDevice, 42), HRESULT, __stdcall,
	LPDIRECT3DDEVICE9 pDevice)
{
	static void* fRegularAddr = 0, *fOverlayAddr = 0;
	if (!fRegularAddr || !fOverlayAddr)
	{
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(_ReturnAddress(), &info, sizeof(MEMORY_BASIC_INFORMATION));

		char mod[MAX_PATH];
		GetModuleFileNameA((HMODULE)info.AllocationBase, mod, MAX_PATH);

		if (strstr(mod, "\\shaderapi"))
			fRegularAddr = _ReturnAddress();
		else
			fOverlayAddr = _ReturnAddress();
	}

	// proof of concept, frankly would like to keep surface in use
	//if (!Vars::Visuals::AntiOBS.Value ? (fRegularAddr && fRegularAddr != _ReturnAddress()) : (fOverlayAddr && fOverlayAddr != _ReturnAddress()))
	if (G::Unload || fRegularAddr && fRegularAddr != _ReturnAddress())
		return CALL_ORIGINAL(pDevice);

	F::Render.Render(pDevice);

	return CALL_ORIGINAL(pDevice);
}

MAKE_HOOK(Direct3DDevice9_Reset, U::Memory.GetVFunc(I::DirectXDevice, 16), HRESULT, __stdcall,
	LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const HRESULT Original = CALL_ORIGINAL(pDevice, pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();
	return Original;
}

LONG __stdcall WndProc::Func(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (F::Menu.IsOpen)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		if (ImGui::GetIO().WantTextInput || F::Menu.InKeybind)
		{
			I::InputSystem->ResetInputState();
			return 1;
		}

		if (uMsg >= WM_MOUSEFIRST && WM_MOUSELAST >= uMsg)
			return 1;
	}

	return CallWindowProc(Original, hWnd, uMsg, wParam, lParam);
}

MAKE_HOOK(VGuiSurface_LockCursor, U::Memory.GetVFunc(I::MatSystemSurface, 62), void, __fastcall,
	void* ecx)
{
	if (F::Menu.IsOpen)
		return I::MatSystemSurface->UnlockCursor();

	CALL_ORIGINAL(ecx);
}

MAKE_HOOK(VGuiSurface_SetCursor, U::Memory.GetVFunc(I::MatSystemSurface, 51), void, __fastcall,
	void* ecx, HCursor cursor)
{
	if (F::Menu.IsOpen)
	{
		switch (F::Render.Cursor)
		{
		case 0: cursor = 2; break;
		case 1: cursor = 3; break;
		case 2: cursor = 12; break;
		case 3: cursor = 11; break;
		case 4: cursor = 10; break;
		case 5: cursor = 9; break;
		case 6: cursor = 8; break;
		case 7: cursor = 14; break;
		case 8: cursor = 13; break;
		}
		return CALL_ORIGINAL(ecx, cursor);
	}

	CALL_ORIGINAL(ecx, cursor);
}

void WndProc::Initialize()
{
	while (!hwWindow)
		hwWindow = SDK::GetTeamFortressWindow();

	Original = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Func)));
}

void WndProc::Unload()
{
	SetWindowLongPtr(hwWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Original));
}