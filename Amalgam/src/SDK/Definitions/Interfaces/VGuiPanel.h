#pragma once
#include "Interface.h"
#include "../Types.h"
#include "../../../Utils/Memory/Memory.h"

class CPanel
{
public:
	const char* GetName(unsigned int vgui_panel)
	{
		return reinterpret_cast<const char* (__thiscall*)(void*, unsigned int)>(U::Memory.GetVFunc(this, 36))(this, vgui_panel);
	}

	void SetMouseInputEnabled(unsigned int panel, bool state)
	{
		reinterpret_cast<void(__thiscall*)(void*, int, bool)>(U::Memory.GetVFunc(this, 32))(this, panel, state);
	}

	void SetTopmostPopup(unsigned int panel, bool state)
	{
		reinterpret_cast<void(__thiscall*)(void*, int, bool)>(U::Memory.GetVFunc(this, 59))(this, panel, state);
	}
};

MAKE_INTERFACE_VERSION(CPanel, VGuiPanel, "vgui2.dll", "VGUI_Panel009");