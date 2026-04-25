#include "KeyHandler.h"

#include "../../SDK/SDK.h"
#include "../../Features/ImGui/Render.h"
#include <Windows.h>

void CKeyHandler::StoreKey(byte iKey, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_aKeyStorage[iKey];

	// down
	const bool bDown = iKey && GetAsyncKeyState(iKey) & 0x8000 && SDK::IsGameWindowInFocus();

	// pressed
	const bool bPressed = bDown && !pStorage->m_bIsDown;

	// double click
	const double dFloatTime = SDK::PlatFloatTime();
	const bool bDouble = bPressed && dFloatTime < pStorage->m_dPressTime + 0.25;

	// released
	const bool bReleased = !bDown && pStorage->m_bIsDown;

	pStorage->m_bIsDown = bDown;
	pStorage->m_bIsPressed = bPressed;
	pStorage->m_bIsDouble = bDouble;
	pStorage->m_bIsReleased = bReleased;
	if (bPressed)
		pStorage->m_dPressTime = dFloatTime;
}

bool CKeyHandler::Down(byte iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_aKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsDown;
}

bool CKeyHandler::Pressed(byte iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_aKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsPressed;
}

bool CKeyHandler::Double(byte iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_aKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsDouble;
}

bool CKeyHandler::Released(byte iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_aKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsReleased;
}

static inline bool VirtualKeyOverride(byte iKey, std::string& sKey)
{
	switch (iKey)
	{
	case 0x0: sKey = "none"; return true;
	case VK_LBUTTON: sKey = "mouse1"; return true;
	case VK_RBUTTON: sKey = "mouse2"; return true;
	case VK_MBUTTON: sKey = "mouse3"; return true;
	case VK_XBUTTON1: sKey = "mouse4"; return true;
	case VK_XBUTTON2: sKey = "mouse5"; return true;
	case VK_F13: sKey = "f13"; return true;
	case VK_F14: sKey = "f14"; return true;
	case VK_F15: sKey = "f15"; return true;
	case VK_F16: sKey = "f16"; return true;
	case VK_F17: sKey = "f17"; return true;
	case VK_F18: sKey = "f18"; return true;
	case VK_F19: sKey = "f19"; return true;
	case VK_F20: sKey = "f20"; return true;
	case VK_F21: sKey = "f21"; return true;
	case VK_F22: sKey = "f22"; return true;
	case VK_F23: sKey = "f23"; return true;
	case VK_F24: sKey = "f24"; return true;
	case VK_LWIN:
	case VK_RWIN: sKey = "windows"; return true;
	case VK_APPS: sKey = "contextmenu"; return true;
	case VK_VOLUME_MUTE: sKey = "mute"; return true;
	case VK_VOLUME_DOWN: sKey = "volumedown"; return true;
	case VK_VOLUME_UP: sKey = "volumeup"; return true;
	case VK_MEDIA_STOP: sKey = "stop"; return true;
	case VK_MEDIA_PLAY_PAUSE: sKey = "playpause"; return true;
	case VK_MEDIA_PREV_TRACK: sKey = "previous"; return true;
	case VK_MEDIA_NEXT_TRACK: sKey = "next"; return true;
	}

	return false;
}
static inline bool VirtualKeyUnicode(byte iKey, std::string& sKey, HKL pKeyboard = nullptr)
{
	if (!F::Render.FontRegular || VK_A > iKey || iKey > VK_Z)
		return false;

	byte uScanCode = MapVirtualKeyEx(iKey, MAPVK_VK_TO_VSC, pKeyboard);
	static byte aKeyState[256] = {};
	if (wchar_t buffer[8]; ToUnicodeEx(iKey, uScanCode, aKeyState, buffer, 8, 4, pKeyboard) > 0 && F::Render.FontRegular->IsGlyphInFont(buffer[0]))
	{
		sKey = SDK::ConvertWideToUTF8(buffer);
		sKey.erase(std::remove_if(sKey.begin(), sKey.end(), ::isspace), sKey.end());
		std::transform(sKey.begin(), sKey.end(), sKey.begin(), ::tolower);
		return true;
	}

	return false;
}
static inline bool VirtualKeyText(byte iKey, std::string& sKey, HKL pKeyboard = nullptr)
{
	unsigned uKey = iKey;
	switch (iKey)
	{
	case VK_PAUSE:
		uKey = VK_NUMLOCK; break;
	}
	uKey = MapVirtualKeyEx(uKey, MAPVK_VK_TO_VSC, pKeyboard) << 16 | 1 << 25;
	switch (iKey)
	{
	case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
	case VK_NUMLOCK: case VK_HOME: case VK_END: case VK_NEXT: case VK_PRIOR:
	case VK_INSERT: case VK_DELETE: case VK_DIVIDE:
		uKey |= 1 << 24; break;
	}
	if (char buffer[16]; GetKeyNameText(uKey, buffer, sizeof(buffer)))
	{
		switch (iKey)
		{
		case VK_LSHIFT: case VK_LCONTROL: case VK_LMENU:
			if (VirtualKeyText(VK_LEFT, sKey)) sKey += buffer; else sKey = buffer; break;
		case VK_RSHIFT: case VK_RCONTROL: case VK_RMENU:
			if (VirtualKeyText(VK_RIGHT, sKey)) sKey += buffer; else sKey = buffer; break;
		default:
			sKey = buffer;
		}
		sKey.erase(std::remove_if(sKey.begin(), sKey.end(), ::isspace), sKey.end());
		std::transform(sKey.begin(), sKey.end(), sKey.begin(), ::tolower);
		return true;
	}

	return false;
}
static inline std::string VirtualKey2Str(byte iKey)
{
	std::string sKey = "unknown";
	if (VirtualKeyOverride(iKey, sKey))
		return sKey;
	if (VirtualKeyUnicode(iKey, sKey, GetKeyboardLayout(0)))
		return sKey;
	if (VirtualKeyText(iKey, sKey, GetKeyboardLayout(0)))
		return sKey;
	return sKey + std::format("{:02x}", iKey);
}

std::string& CKeyHandler::String(byte iKey, const bool bStore)
{
	std::string& sString = m_aStringStorage[iKey];

	if (bStore)
		sString = VirtualKey2Str(iKey);

	return sString;
}