#pragma once
#include "../../SDK/SDK.h"
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui.h>

class CRender
{
public:
	void Render(IDirect3DDevice9* pDevice);
	void Initialize(IDirect3DDevice9* pDevice);

	void LoadColors();
	void LoadStyle();

	int Cursor = 2;

	// Colors
	ImColor Accent = { 255, 101, 101 };
	ImColor AccentLight = { 255, 111, 111 };
	ImColor Background = { 23, 23, 23, 250 };
	ImColor Foreground = { 11, 11, 11, 250 };
	ImColor Foremost = { 23, 23, 23, 250 };
	ImColor ForemostLight = { 25, 25, 25, 250 };
	ImColor Inactive = { 150, 150, 150 };
	ImColor Active = { 255, 255, 255 };

	// Fonts
	ImFont* FontSmall = nullptr;
	ImFont* FontRegular = nullptr;
	ImFont* FontBold = nullptr;
	ImFont* FontLarge = nullptr;
	ImFont* FontBlack = nullptr;
	ImFont* FontTitle = nullptr;
	ImFont* FontMono = nullptr;

	ImFont* IconFontRegular = nullptr;
	ImFont* IconFontLarge = nullptr;
};

ADD_FEATURE(CRender, Render);