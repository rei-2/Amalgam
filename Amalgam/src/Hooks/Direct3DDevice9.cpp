#include "Direct3DDevice9.h"

#include "../SDK/SDK.h"
#include "../Features/ImGui/Render.h"
#include "../Features/ImGui/Menu/Menu.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

MAKE_HOOK(Direct3DDevice9_Present, U::Memory.GetVirtual(I::DirectXDevice, 17), HRESULT,
	IDirect3DDevice9* pDevice, const RECT* pSource, const RECT* pDestination, const RGNDATA* pDirtyRegion)
{
	if (!G::Unload)
		F::Render.Render(pDevice);

	return CALL_ORIGINAL(pDevice, pSource, pDestination, pDirtyRegion);
}

MAKE_HOOK(Direct3DDevice9_Reset, U::Memory.GetVirtual(I::DirectXDevice, 16), HRESULT,
	LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const HRESULT Original = CALL_ORIGINAL(pDevice, pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();
	return Original;
}

LONG __stdcall WndProc::Func(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (F::Menu.m_bIsOpen)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

		if ((ImGui::GetIO().WantTextInput || F::Menu.m_bInKeybind) && WM_KEYFIRST <= uMsg && uMsg <= WM_KEYLAST)
		{
			I::InputSystem->ResetInputState();
			return 1;
		}

		if (WM_MOUSEFIRST <= uMsg && uMsg <= WM_MOUSELAST)
			return 1;
	}

	return CallWindowProc(Original, hWnd, uMsg, wParam, lParam);
}

MAKE_HOOK(VGuiSurface_LockCursor, U::Memory.GetVirtual(I::MatSystemSurface, 62), void,
	void* rcx)
{
	if (F::Menu.m_bIsOpen)
		return I::MatSystemSurface->UnlockCursor();

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(VGuiSurface_SetCursor, U::Memory.GetVirtual(I::MatSystemSurface, 51), void,
	void* rcx, HCursor cursor)
{
	if (F::Menu.m_bIsOpen)
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
	}

	CALL_ORIGINAL(rcx, cursor);
}

void WndProc::Initialize()
{
	hwWindow = SDK::GetTeamFortressWindow();

	Original = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Func)));
}

void WndProc::Unload()
{
	SetWindowLongPtr(hwWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Original));
}