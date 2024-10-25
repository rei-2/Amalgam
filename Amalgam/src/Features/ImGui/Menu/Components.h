#pragma once
#include "../Render.h"
#include "Menu.h"
#include "../MaterialDesign/IconDefinitions.h"
#include "../../Binds/Binds.h"
#include "../../Visuals/Materials/Materials.h"
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui_stdlib.h>

enum FText_
{
	FText_None = 0,
	FText_Middle = 1 << 0,
	FText_Right = 1 << 1
};

enum FButton_
{
	FButton_None = 0,
	FButton_Left = 1 << 0,
	FButton_Right = 1 << 1,
	FButton_Fit = 1 << 2,
	FButton_SameLine = 1 << 3,
	FButton_Large = 1 << 4,
	FButton_NoUpper = 1 << 5
};

enum FKeybind_
{
	FKeybind_None = 0,
	FKeybind_AllowNone = 1 << 6,
	FKeybind_AllowMenu = 1 << 7
};

enum FToggle_
{
	FToggle_None = 0,
	FToggle_Left = 1 << 0,
	FToggle_Right = 1 << 1,
	FToggle_PlainColor = 1 << 2
};

enum FSlider_
{
	FSlider_None = 0,
	FSlider_Left = 1 << 0,
	FSlider_Right = 1 << 1,
	FSlider_Clamp = 1 << 2, // will keep within bounds when using text input
	FSlider_Min = 1 << 3, // will keep above minimum when using text input
	FSlider_Max = 1 << 4, // will keep below maximum when using text input
	FSlider_Precision = 1 << 5, // allow more precise values outside of step when using text input
};

enum FDropdown_
{
	FDropdown_None = 0,
	FDropdown_Left = 1 << 0,
	FDropdown_Right = 1 << 1,
	FDropdown_Multi = 1 << 2,
	FDropdown_Modifiable = 1 << 3
};

enum FSDropdown_
{
	FSDropdown_None = 0,
	FSDropdown_Custom = 1 << 2,
	FSDropdown_AutoUpdate = 1 << 3
};

enum FColorPicker_
{
	FColorPicker_None = 0,
	FColorPicker_Left = 1 << 0,
	FColorPicker_Middle = 1 << 1,
	FColorPicker_SameLine = 1 << 2,
	FColorPicker_Dropdown = 1 << 3,
	FColorPicker_Tooltip = 1 << 4
};

static inline ImVec2  operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2  operator/(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static inline ImVec2  operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2  operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2  operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2  operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
static inline ImVec2  operator-(const ImVec2& lhs) { return ImVec2(-lhs.x, -lhs.y); }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs) { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const float rhs) { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

namespace ImGui
{
	std::unordered_map<uint32_t, int> mActiveMap;

	bool Disabled = false, Transparent = false;
	int CurrentBind = DEFAULT_BIND;

	std::vector<bool> vDisabled = {}, vTransparent = {};
	inline void PushDisabled(bool bDisabled)
	{
		vDisabled.push_back(Disabled = bDisabled);
	}
	inline void PopDisabled()
	{
		vDisabled.pop_back();
		Disabled = !vDisabled.empty() ? vDisabled.back() : false;
	}
	inline void PushTransparent(bool bTransparent)
	{
		vTransparent.push_back(Transparent = bTransparent);
	}
	inline void PopTransparent()
	{
		vTransparent.pop_back();
		Transparent = !vTransparent.empty() ? vTransparent.back() : false;
	}

	inline float fnmodf(float flX, float flY)
	{
		// silly fix for negative values
		return fmodf(flX, flY) + (flX < 0 ? flY : 0);
	}

	inline bool IsColorBright(Color_t tColor)
	{
		return tColor.r + tColor.g + tColor.b > 510;
	}

	inline bool IsColorBright(ImColor tColor)
	{
		return tColor.Value.x + tColor.Value.y + tColor.Value.z > 2.f;
	}

	inline ImVec4 ColorToVec(Color_t tColor)
	{
		return { float(tColor.r) / 255.f, float(tColor.g) / 255.f, float(tColor.b) / 255.f, float(tColor.a) / 255.f };
	}

	inline Color_t VecToColor(ImVec4 tColor)
	{
		return {
			static_cast<byte>(tColor.x * 256.0f > 255 ? 255 : tColor.x * 256.0f),
			static_cast<byte>(tColor.y * 256.0f > 255 ? 255 : tColor.y * 256.0f),
			static_cast<byte>(tColor.z * 256.0f > 255 ? 255 : tColor.z * 256.0f),
			static_cast<byte>(tColor.w * 256.0f > 255 ? 255 : tColor.w * 256.0f)
		};
	}



	inline ImVec2 GetDrawPos()
	{
		return GetWindowPos() - GetCurrentWindow()->Scroll;
	}

	inline float GetDrawPosX()
	{
		return GetWindowPos().x - GetCurrentWindow()->Scroll.x;
	}

	inline float GetDrawPosY()
	{
		return GetWindowPos().y - GetCurrentWindow()->Scroll.y;
	}

	inline bool IsMouseWithin(float x, float y, float w, float h)
	{
		ImVec2 vMouse = GetMousePos();
		bool bWithin = x <= vMouse.x && vMouse.x < x + w
					&& y <= vMouse.y && vMouse.y < y + h;

		//ImDrawList* pDrawList = GetForegroundDrawList();
		//ImVec2 vDrawPos = GetDrawPos() - GetWindowPos();
		//pDrawList->AddRectFilled({ vDrawPos.x + x, vDrawPos.y + y }, { vDrawPos.x + x + w, vDrawPos.y + y + h }, bWithin ? ImColor(1.f, 0.f, 0.f, 0.5f) : ImColor(0.f, 1.f, 1.f, 0.5f));

		return bWithin;
	}

	inline std::string StripDoubleHash(const char* sText)
	{
		std::string sBegin = sText, sEnd = FindRenderedTextEnd(sText);
		return sBegin.replace(sBegin.end() - sEnd.size(), sBegin.end(), "");
	}

	inline std::string TruncateText(const char* sText, int iPixels)
	{
		std::string sOriginal = sText;
		if (sOriginal.empty())
			return "";

		std::string sTruncated = "";
		int i = 0; while (CalcTextSize(sTruncated.c_str()).x < iPixels)
		{
			i++; sTruncated = sOriginal.substr(0, i);
			if (i == sOriginal.size())
			{
				i = 0; break;
			}
		}
		if (i)
			sTruncated += "...";

		return sTruncated;
	}

	inline const char* FormatText(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		const char* sText;
		ImFormatStringToTempBufferV(&sText, nullptr, fmt, args);
		va_end(args);

		return sText;
	}



	inline void DebugDummy(ImVec2 vSize)
	{
		ImVec2 vOriginalPos = GetCursorPos();

		//PushStyleColor(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.5f });
		//Button("##", { std::max(vSize.x, 2.f), std::max(vSize.y, 2.f) });
		//PopStyleColor();

		SetCursorPos(vOriginalPos); Dummy(vSize);
	}
	inline void DebugShift(ImVec2 vSize)
	{
		ImVec2 vOriginalPos = GetCursorPos();

		//PushStyleColor(ImGuiCol_Button, { 1.f, 1.f, 1.f, 0.5f });
		//Button("##", { std::max(vSize.x, 2.f), std::max(vSize.y, 2.f) });
		//PopStyleColor();

		SetCursorPos({ vOriginalPos.x + vSize.x, vOriginalPos.y + vSize.y });
	}

	inline void AddSteppedRect(ImVec2 vPos, ImVec2 vPosMin, ImVec2 vPosMax, ImVec2 vClipMin, ImVec2 vClipMax, float flVMin, float flVMax, float flStep, ImU32 uPrimary, ImU32 uSecondary)
	{
		ImDrawList* pDrawList = GetWindowDrawList();
		pDrawList->PushClipRect({ vPos.x + vClipMin.x, vPos.y + vClipMin.y }, { vPos.x + vClipMax.x, vPos.y + vClipMax.y }, true);

		int iSteps = (flVMax - flVMin) / flStep;
		if (iSteps < 21)
		{
			std::vector<std::pair<int, int>> vSteps;

			float flMin = flVMin - fnmodf(flVMin + flStep / 2, flStep) + flStep / 2, max = flVMax - fnmodf(flVMax + flStep / 2, flStep) + flStep / 2;

			if (fabsf(flVMin - flMin) < 0.001f)
				vSteps.push_back({ vPosMin.x, vPosMin.x + 2 });
			while (true)
			{
				flMin += flStep;
				if (flMin + flStep / 2 > flVMax)
					break;

				float flPercent = std::clamp((flMin - flVMin) / (flVMax - flVMin), 0.f, 1.f);
				float flPosition = vPosMin.x + (vPosMax.x - vPosMin.x) * flPercent;
				vSteps.push_back({ flPosition - 1, flPosition + 1 });
			}
			if (fabsf(flVMax - max) < 0.001f)
				vSteps.push_back({ vPosMax.x - 2, vPosMax.x });

			if (vSteps.size())
			{
				for (size_t i = 0; i < vSteps.size(); i++)
				{
					if (!i)
						pDrawList->AddRectFilled({ vPos.x + vPosMin.x, vPos.y + vPosMin.y }, { vPos.x + vSteps.front().first, vPos.y + vPosMax.y }, uPrimary);
					else
						pDrawList->AddRectFilled({ vPos.x + vSteps[i - 1].second, vPos.y + vPosMin.y }, { vPos.x + vSteps[i].first, vPos.y + vPosMax.y }, uPrimary);
					pDrawList->AddRectFilled({ vPos.x + vSteps[i].first, vPos.y + vPosMin.y }, { vPos.x + vSteps[i].second, vPos.y + vPosMax.y }, uSecondary);
				}
				pDrawList->AddRectFilled({ vPos.x + vSteps.back().second, vPos.y + vPosMin.y }, { vPos.x + vPosMax.x, vPos.y + vPosMax.y }, uPrimary);

				return pDrawList->PopClipRect();
			}
		}
		pDrawList->AddRectFilled({ vPos.x + vPosMin.x, vPos.y + vPosMin.y }, { vPos.x + vPosMax.x, vPos.y + vPosMax.y }, uPrimary);

		pDrawList->PopClipRect();
	}

	inline void FTooltip(const char* sDescription, bool bCondition = IsItemHovered())
	{
		if (bCondition)
			SetTooltip(sDescription);
	}

	inline void IconImage(const char* sIcon, bool bLarge = false, ImVec4 tColor = { 1, 1, 1, -1 })
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		if (tColor.w >= 0.f)
			PushStyleColor(ImGuiCol_Text, tColor);
		PushFont(bLarge ? F::Render.IconFontLarge : F::Render.IconFontRegular);
		TextUnformatted(sIcon);
		PopFont();
		if (tColor.w >= 0.f)
			PopStyleColor();

		if (Transparent || Disabled)
			PopStyleVar();
	}

	inline bool IconButton(const char* sIcon, bool bLarge = false, ImVec4 tColor = { 1, 1, 1, -1 })
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		ImVec2 vOriginalPos = GetCursorPos();

		if (tColor.w >= 0.f)
			PushStyleColor(ImGuiCol_Text, tColor);
		PushFont(bLarge ? F::Render.IconFontLarge : F::Render.IconFontRegular);
		TextUnformatted(sIcon);
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		const bool bReturn = IsItemClicked();
		PopFont();
		if (tColor.w >= 0.f)
			PopStyleColor();

		// prevent accidental dragging
		SetCursorPos(vOriginalPos);
		Button("##", GetItemRectSize());

		if (Transparent || Disabled)
			PopStyleVar();

		return Disabled ? false : bReturn;
	}

	std::unordered_map<const char*, float> mLastHeights;
	std::vector<const char*> vStoredLabels;
	inline bool Section(const char* sLabel, float flMinHeight = 1.f, bool bForceHeight = false)
	{
		vStoredLabels.push_back(sLabel);
		if (!bForceHeight && mLastHeights.contains(sLabel) && mLastHeights[sLabel] > flMinHeight)
			flMinHeight = mLastHeights[sLabel];
		PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
		const bool bReturn = BeginChild(sLabel, { GetColumnWidth(), flMinHeight + 8 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding);

		PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8, 0 });
		if (sLabel[0] != '#')
		{
			ImVec2 vOriginalPos = GetCursorPos();

			PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
			PushFont(F::Render.FontBold);
			SetCursorPosY(vOriginalPos.y + 1);
			TextUnformatted(StripDoubleHash(sLabel).c_str());
			PopFont();
			PopStyleColor();

			SetCursorPos(vOriginalPos); DebugDummy({ 0, 16 });
		}

		return bReturn;
	}
	inline void EndSection()
	{
		const char* sLabel = vStoredLabels.back();
		vStoredLabels.pop_back();
		if (GetItemRectMax().y - GetWindowPos().y > 0.f)
			mLastHeights[sLabel] = GetItemRectMax().y - GetWindowPos().y;

		PopStyleVar();
		EndChild();
		PopStyleVar();
	}

	// widgets
	inline bool FTabs(std::vector<const char*> vEntries, int* pVar, const ImVec2 vSize, const ImVec2 vPos, bool bVertical = false, std::vector<const char*> vIcons = {})
	{
		if (vIcons.size() && vIcons.size() != vEntries.size())
			return false;

		ImDrawList* pDrawList = GetWindowDrawList();
		const int iOriginalTab = pVar ? *pVar : 0;

		for (size_t i = 0; i < vEntries.size(); i++)
		{
			ImVec2 vNewPos = bVertical ? ImVec2(vPos.x, vPos.y + vSize.y * i) : ImVec2(vPos.x + vSize.x * i, vPos.y);
			ImVec2 vDrawPos = GetDrawPos() + vNewPos;

			if (i != iOriginalTab)
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			else
			{
				if (!bVertical)
					pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y + vSize.y - 2 }, { vDrawPos.x + vSize.x, vDrawPos.y + vSize.y }, F::Render.Accent);
				else
					pDrawList->AddRectFilled({ vDrawPos.x + vSize.x - 2, vDrawPos.y }, { vDrawPos.x + vSize.x, vDrawPos.y + vSize.y }, F::Render.Accent);
			}
			SetCursorPos(vNewPos);
			if (Button(std::format("##{}", vEntries[i]).c_str(), vSize) && i != iOriginalTab && pVar)
			{
				if (vStoredLabels.size() == 0)
					mLastHeights.clear();
				*pVar = int(i);
			}
			if (!Disabled && IsItemHovered())
				SetMouseCursor(ImGuiMouseCursor_Hand);

			ImVec2 vOriginalPos = GetCursorPos();
			std::string sStripped = StripDoubleHash(vEntries[i]);
			ImVec2 vTextSize = CalcTextSize(sStripped.c_str());
			ImVec2 vTextPos = { vNewPos.x + (vSize.x - vTextSize.x) / 2, vNewPos.y + (vSize.y - vTextSize.y) / 2 + (vIcons.empty() ? 0 : 10) };
			SetCursorPos(vTextPos);
			TextUnformatted(sStripped.c_str());
			if (!vIcons.empty())
			{
				SetCursorPos({ vNewPos.x + vSize.x / 2 - 8, vNewPos.y + vSize.x / 2 - 14 });
				IconImage(vIcons[i]);
			}
			SetCursorPos(vOriginalPos);

			if (i != iOriginalTab)
				PopStyleColor();
		}
		return pVar ? *pVar != iOriginalTab : false;
	}

	inline bool FSelectable(const char* sLabel, ImVec4 tColor = { 0.2f, 0.6f, 0.85f, 1.f }, bool bSelected = false, int iFlags = 0, const ImVec2& vSize = {})
	{
		PushStyleVar(ImGuiStyleVar_SelectableRounding, 3.f);
		PushStyleColor(ImGuiCol_HeaderHovered, tColor);
		tColor.x /= 1.1f; tColor.y /= 1.1f; tColor.z /= 1.1f;
		PushStyleColor(ImGuiCol_HeaderActive, tColor);
		const bool bReturn = Selectable(sLabel, bSelected, iFlags, vSize);
		PopStyleColor(2);
		PopStyleVar();
		return bReturn;
	}

	inline void FText(const char* sText, int iFlags = 0)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		float flWindowWidth = GetWindowSize().x;
		float flTextWidth = CalcTextSize(sText).x;
		if (iFlags & FText_Middle)
			SetCursorPosX((flWindowWidth - flTextWidth) * 0.5f);
		else if (iFlags & FText_Right)
			SetCursorPosX(flWindowWidth - flTextWidth - 8);
		TextUnformatted(sText);

		if (Transparent || Disabled)
			PopStyleVar();
	}

	inline bool FButton(const char* sLabel, int iFlags = 0, int iSizeOffset = 0)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		std::string sLabel2 = sLabel;
		if (!(iFlags & FButton_NoUpper))
		{
			std::transform(sLabel2.begin(), sLabel2.end(), sLabel2.begin(), ::toupper);
			sLabel = sLabel2.c_str();
		}

		float flSizeX = GetWindowSize().x - 2 * GetStyle().WindowPadding.x;
		if (iFlags & FButton_Left || iFlags & FButton_Right)
			flSizeX = GetWindowSize().x / 2 - GetStyle().WindowPadding.x - 4;
		else if (iFlags & FButton_Fit)
			flSizeX = CalcTextSize(sLabel).x + (iFlags & FButton_Large ? 28 : 18);
		if (iFlags & FButton_SameLine)
			SameLine();
		else if (iFlags & FButton_Right)
			SetCursorPosX(flSizeX + 20);

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, 8 });

		PushStyleColor(ImGuiCol_Border, F::Render.Accent.Value);
		PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		const bool bReturn = Button(sLabel, { flSizeX + iSizeOffset, iFlags & FButton_Large ? 40.f : 30.f });
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		PopStyleVar();
		PopStyleColor();

		SetCursorPos(vOriginalPos); DebugDummy({ flSizeX + iSizeOffset, iFlags & FButton_Large ? 48.f : 38.f });

		if (Transparent || Disabled)
			PopStyleVar();

		return Disabled ? false : bReturn;
	}

	inline bool FToggle(const char* sLabel, bool* pVar, int iFlags = 0, bool* pHovered = nullptr)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		float flSizeX = GetWindowSize().x;
		if (iFlags & (FToggle_Left | FToggle_Right))
			flSizeX = flSizeX / 2 + 4;
		if (iFlags & FToggle_Right)
			SameLine(flSizeX);
		flSizeX = flSizeX - 2 * GetStyle().WindowPadding.x;

		ImVec2 vOriginalPos = GetCursorPos();

		bool bReturn = Button(std::format("##{}", sLabel).c_str(), { flSizeX, 24 });
		if (Disabled)
			bReturn = false;
		if (bReturn)
			*pVar = !*pVar;
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);

		SetCursorPos({ vOriginalPos.x + 4, vOriginalPos.y + 3 });
		IconImage(*pVar ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, true, *pVar ? (iFlags & FToggle_PlainColor ? F::Render.Active.Value : F::Render.Accent.Value) : F::Render.Inactive.Value);

		SetCursorPos({ vOriginalPos.x + 24, vOriginalPos.y + 5 });
		if (*pVar)
			PushStyleColor(ImGuiCol_Text, F::Render.Active.Value);
		else
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
		TextUnformatted(StripDoubleHash(sLabel).c_str());
		PopStyleColor();

		SetCursorPos(vOriginalPos); DebugDummy({ flSizeX, 24 });

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y, flSizeX, 24);
		}

		return bReturn;
	}

	inline bool FSlider(const char* sLabel, float* pVar1, float* pVar2, float flMin, float flMax, float flStep = 1.f, const char* fmt = "%.0f", int iFlags = 0, bool* pHovered = nullptr)
	{
		auto uHash = FNV1A::Hash32Const(sLabel);
		ImDrawList* pDrawList = GetWindowDrawList();
		float flOriginal1 = *pVar1, flOriginal2 = pVar2 ? *pVar2 : 0.f;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		float flSizeX = GetWindowSize().x, flSizeXHalf = flSizeX / 2 + 4;
		if (iFlags & (FSlider_Left | FSlider_Right))
		{
			flSizeX = flSizeXHalf;
			if (iFlags & FSlider_Right)
				SameLine(flSizeX);
		}
		flSizeX = flSizeX - 2 * GetStyle().WindowPadding.x;
		flSizeXHalf -= 2 * GetStyle().WindowPadding.x;

		ImVec2 vOriginalPos = GetCursorPos(), vDrawPos = GetDrawPos();
		PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);

		ImVec2 vTextPos = { vOriginalPos.x + 6, vOriginalPos.y + (iFlags & (FSlider_Left | FSlider_Right) ? 3 : 5) };
		SetCursorPos(vTextPos);
		TextUnformatted(StripDoubleHash(sLabel).c_str());

		{
			auto uHash2 = FNV1A::Hash32Const(std::format("{}## Text", sLabel).c_str());

			static std::string sText, sInput;
			if (!mActiveMap[uHash2])
				sText = pVar2 ? FormatText(fmt, *pVar1, *pVar2) : FormatText(fmt, *pVar1);
			else
			{
				SetCursorPos({ -1000, vTextPos.y }); // lol
				SetKeyboardFocusHere();
				InputText("##SliderText", &sInput, ImGuiInputTextFlags_CharsDecimal); sText = sInput;

				bool bEnter = U::KeyHandler.Pressed(VK_RETURN);
				if (bEnter)
				{
					try // prevent the user from being a retard with invalid inputs
					{
						float* pVar = mActiveMap[uHash2] == 1 ? pVar1 : pVar2;

						*pVar = sText.length() ? std::stof(sText) : 0.f;
						if (pVar2)
							*pVar = std::min(*pVar, pVar1 == pVar ? *pVar2 - flStep : *pVar1 + flStep);
						if (!(iFlags & FSlider_Precision))
							*pVar = *pVar - fnmodf(*pVar - flStep / 2, flStep) + flStep / 2;
						if (iFlags & FSlider_Clamp)
							*pVar = std::clamp(*pVar, flMin, flMax);
						else if (iFlags & FSlider_Min)
							*pVar = std::max(*pVar, flMin);
						else if (iFlags & FSlider_Max)
							*pVar = std::min(*pVar, flMax);
					}
					catch (...) {}
				}
				if (bEnter || IsMouseClicked(ImGuiMouseButton_Left) || U::KeyHandler.Pressed(VK_ESCAPE))
					mActiveMap[uHash2] = false;
			}
			float flWidth = CalcTextSize(sText.c_str()).x;
			if (iFlags & (FSlider_Left | FSlider_Right))
				SetCursorPos({ vOriginalPos.x + flSizeX - flWidth - 6, vTextPos.y });
			else
				SetCursorPos({ vOriginalPos.x + flSizeX - 40, vTextPos.y });

			ImVec2 vOriginalPos2 = GetCursorPos();
			TextUnformatted(sText.c_str());
			if (!Disabled)
			{
				if (!Disabled && IsItemHovered() && IsWindowHovered())
					SetMouseCursor(ImGuiMouseCursor_TextInput);
				if (mActiveMap[uHash2])
					pDrawList->AddRectFilled({ vDrawPos.x + vOriginalPos2.x, vDrawPos.y + vOriginalPos2.y + 14 }, { vDrawPos.x + vOriginalPos2.x + flWidth, vDrawPos.y + vOriginalPos2.y + 15 }, F::Render.Active);
				else if (IsItemClicked())
				{
					float* pVar = !pVar2 || GetMousePos().x - vDrawPos.x - vOriginalPos2.x < flWidth / 2 ? pVar1 : pVar2;
					sInput = std::format("{}", *pVar); // would use to_string but i don't like its formatting
					mActiveMap[uHash2] = pVar == pVar1 ? 1 : 2;
				}
			}
		}

		vDrawPos += vOriginalPos;
		ImVec2 vMins = { flSizeX - flSizeXHalf - 16, 11 }, vMaxs = { flSizeX - 54, 13 };
		ImColor vAccent = F::Render.Accent, vMuted = vAccent, vWashed = vAccent, vTransparent = vAccent;
		{
			float flA = GetStyle().Alpha;
			vAccent.Value.w *= flA, vMuted.Value.w *= 0.8f * flA, vWashed.Value.w *= 0.4f * flA, vTransparent.Value.w *= 0.2f * flA;
		}
		if (iFlags & (FSlider_Left | FSlider_Right))
			vMins = { 6, 24 }, vMaxs = { flSizeX - 6, 26 };

		bool bWithin = IsMouseWithin(vDrawPos.x + vMins.x - 6, vDrawPos.y + vMins.y - 6, (vMaxs.x - vMins.x) + 12, (vMaxs.y - vMins.y) + 12);
		if (!Disabled && bWithin && IsWindowHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		ImVec2 vMouse = GetMousePos();
		float flMousePerc = (vMouse.x - (vDrawPos.x + vMins.x)) / ((vDrawPos.x + vMaxs.x) - (vDrawPos.x + vMins.x)) + (flStep / 2) / (flMax - flMin);
		if (pVar2)
		{
			float flLowerPerc = std::clamp((*pVar1 - flMin) / (flMax - flMin), 0.f, 1.f), flUpperPerc = std::clamp((*pVar2 - flMin) / (flMax - flMin), 0.f, 1.f);
			float flLowerPos = vMins.x + (vMaxs.x - vMins.x) * flLowerPerc, flUpperPos = vMins.x + (vMaxs.x - vMins.x) * flUpperPerc;

			AddSteppedRect(vDrawPos, vMins, vMaxs, vMins, { flLowerPos, vMaxs.y }, flMin, flMax, flStep, vWashed, vMuted);
			AddSteppedRect(vDrawPos, vMins, vMaxs, { flLowerPos, vMins.y }, { flUpperPos, vMaxs.y }, flMin, flMax, flStep, vAccent, vWashed);
			AddSteppedRect(vDrawPos, vMins, vMaxs, { flUpperPos, vMins.y }, vMaxs, flMin, flMax, flStep, vWashed, vMuted);
			pDrawList->AddCircleFilled({ vDrawPos.x + flLowerPos, vDrawPos.y + vMins.y + 1 }, 3.f, vAccent);
			pDrawList->AddCircleFilled({ vDrawPos.x + flUpperPos, vDrawPos.y + vMins.y + 1 }, 3.f, vAccent);

			if (!Disabled)
			{
				if (bWithin && !mActiveMap[uHash] && IsWindowHovered())
				{
					int iVar = fabsf(vMouse.x - (vDrawPos.x + flLowerPos)) < fabsf(vMouse.x - (vDrawPos.x + flUpperPos)) ? 1 : 2;
					if (IsMouseClicked(ImGuiMouseButton_Left))
						mActiveMap[uHash] = iVar;
					pDrawList->AddCircleFilled({ vDrawPos.x + (iVar == 1 ? flLowerPos : flUpperPos), vDrawPos.y + vMins.y + 1 }, 11.f, vTransparent);
				}
				else if (mActiveMap[uHash] && IsMouseDown(ImGuiMouseButton_Left))
				{
					float* pVar = mActiveMap[uHash] == 1 ? pVar1 : pVar2;
					*pVar = flMin + (flMax - flMin) * flMousePerc;
					*pVar = std::clamp(*pVar - fnmodf(*pVar, flStep), pVar == pVar2 ? *pVar1 + flStep : flMin, pVar == pVar1 ? *pVar2 - flStep : flMax);
					pDrawList->AddCircleFilled({ vDrawPos.x + (pVar == pVar1 ? flLowerPos : flUpperPos), vDrawPos.y + vMins.y + 1 }, 11.f, vWashed);
				}
				else
					mActiveMap[uHash] = false;
			}
		}
		else
		{
			float flPercent = std::clamp((*pVar1 - flMin) / (flMax - flMin), 0.f, 1.f);
			float flPos = vMins.x + (vMaxs.x - vMins.x) * flPercent;

			AddSteppedRect(vDrawPos, vMins, vMaxs, vMins, { flPos, vMaxs.y }, flMin, flMax, flStep, vAccent, vWashed);
			AddSteppedRect(vDrawPos, vMins, vMaxs, { flPos, vMins.y }, vMaxs, flMin, flMax, flStep, vWashed, vMuted);
			pDrawList->AddCircleFilled({ vDrawPos.x + flPos, vDrawPos.y + vMins.y + 1 }, 3.f, vAccent);

			if (!Disabled)
			{
				if (bWithin && !mActiveMap[uHash] && IsWindowHovered())
				{
					if (IsMouseClicked(ImGuiMouseButton_Left))
						mActiveMap[uHash] = 1;
					pDrawList->AddCircleFilled({ vDrawPos.x + flPos, vDrawPos.y + vMins.y + 1 }, 11.f, vTransparent);
				}
				else if (mActiveMap[uHash] && IsMouseDown(ImGuiMouseButton_Left))
				{
					*pVar1 = flMin + (flMax - flMin) * flMousePerc;
					*pVar1 = std::clamp(*pVar1 - fnmodf(*pVar1, flStep), flMin, flMax);
					pDrawList->AddCircleFilled({ vDrawPos.x + flPos, vDrawPos.y + vMins.y + 1 }, 11.f, vWashed);
				}
				else
					mActiveMap[uHash] = false;
			}
		}

		PopStyleColor();
		SetCursorPos({ vOriginalPos.x + vMins.x - 6, vOriginalPos.y + vMins.y - 6 });
		Button("##", { vMaxs.x - vMins.x + 12, 14 }); // don't drag it around
		SetCursorPos(vOriginalPos); Dummy({ 0, iFlags & (FSlider_Left | FSlider_Right) ? 32.f : 24.f });

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			float w = (iFlags & (FSlider_Left | FSlider_Right) ? GetWindowSize().x / 2 + 4 : GetWindowSize().x) - 2 * GetStyle().WindowPadding.x;
			float h = iFlags & (FSlider_Left | FSlider_Right) ? 32.f : 24.f;
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y, w, h);
		}

		return *pVar1 != flOriginal1 || pVar2 && *pVar2 != flOriginal2;
	}

	inline bool FSlider(const char* sLabel, int* pVar1, int* pVar2, int iMin, int iMax, int iStep = 1, const char* fmt = "%d", int iFlags = 0, bool* pHovered = nullptr)
	{
		// replace incorrect formats as it will be converted to float
		std::string sReplace = fmt;

		std::string sFrom = "%d", sTo = "%.0f";
		size_t find = sReplace.find(sFrom);
		while (find != std::string::npos)
		{
			sReplace.replace(find, sFrom.length(), sTo);
			find = sReplace.find(sFrom);
		}
		sFrom = "%i";
		find = sReplace.find(sFrom);
		while (find != std::string::npos)
		{
			sReplace.replace(find, sFrom.length(), sTo);
			find = sReplace.find(sFrom);
		}

		fmt = sReplace.c_str();

		float flRedir1 = *pVar1; float flRedir2 = pVar2 ? *pVar2 : 0;
		const bool bReturn = FSlider(sLabel, &flRedir1, pVar2 ? &flRedir2 : nullptr, iMin, iMax, iStep, fmt, iFlags, pHovered);
		*pVar1 = flRedir1; if (pVar2) *pVar2 = flRedir2;
		return bReturn;
	}

	inline bool FSlider(const char* sLabel, float* pVar, float flMin, float flMax, float flStep = 1.f, const char* fmt = "%.0f", int iFlags = 0, bool* pHovered = nullptr)
	{
		return FSlider(sLabel, pVar, nullptr, flMin, flMax, flStep, fmt, iFlags, pHovered);
	}

	inline bool FSlider(const char* sLabel, int* pVar, int iMin, int iMax, int iStep = 1, const char* fmt = "%d", int iFlags = 0, bool* pHovered = nullptr)
	{
		return FSlider(sLabel, pVar, nullptr, iMin, iMax, iStep, fmt, iFlags, pHovered);
	}

	inline bool FDropdown(const char* sLabel, int* pVar, std::vector<const char*> vEntries, std::vector<int> vValues = {}, int iFlags = 0, int iSizeOffset = 0, bool* pHovered = nullptr, int* pModified = nullptr)
	{
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		if (vValues.empty())
		{
			int i = 0; for (auto& sEntry : vEntries)
			{
				if (FNV1A::Hash32(sEntry) == FNV1A::Hash32Const("##Divider"))
					continue;

				vValues.push_back(iFlags & FDropdown_Multi ? 1 << i : i);
				i++;
			}
		}

		std::string sPreview = "";
		if (iFlags & FDropdown_Multi && !*pVar)
			sPreview = "None";
		else
		{
			int i = 0; for (auto& iValue : vValues)
			{
				while (FNV1A::Hash32(vEntries[i]) == FNV1A::Hash32Const("##Divider"))
					i++;

				if (iFlags & FDropdown_Multi && *pVar & iValue)
					sPreview += std::format("{}, ", StripDoubleHash(vEntries[i]).c_str());
				else if (!(iFlags & FDropdown_Multi) && *pVar == iValue)
					sPreview = std::format("{}##", StripDoubleHash(vEntries[i]).c_str());
				i++;
			}
			sPreview.pop_back(); sPreview.pop_back();
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 13.5f });
		float flSizeX = GetWindowSize().x;
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			flSizeX = flSizeX / 2 + 4;
		if (iFlags & FDropdown_Right)
			SameLine(flSizeX);
		if (strstr(sLabel, "## Bind"))
			iSizeOffset = 0;
		flSizeX = flSizeX - 2 * GetStyle().WindowPadding.x + iSizeOffset;
		PushItemWidth(flSizeX);

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, 8 });

		if (Disabled)
		{	// lol
			Button("##", { flSizeX, 40 });
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, 8 });
		}

		bool bActive = BeginCombo(std::format("##{}", sLabel).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge);
		if (bActive)
		{
			DebugDummy({ 0, 8 });

			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 19 });
			int i = 0; for (auto& sEntry : vEntries)
			{
				if (FNV1A::Hash32(sEntry) == FNV1A::Hash32Const("##Divider"))
				{
					ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
					ImColor tInactive = F::Render.Inactive; tInactive.Value.w *= GetStyle().Alpha;
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x + 18, vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowSize().x - 18, vDrawPos.y + flPosY + 1 }, tInactive);
					Dummy({});
					continue;
				}

				std::string sStripped = StripDoubleHash(sEntry);
				if (iFlags & FDropdown_Multi)
				{
					bool bFlagActive = *pVar & vValues[i];

					if (Selectable(std::format("##{}", sEntry).c_str(), bFlagActive, ImGuiSelectableFlags_DontClosePopups))
					{
						if (bFlagActive)
							*pVar &= ~vValues[i];
						else
							*pVar |= vValues[i];
						bReturn = true;
					}

					ImVec2 vOriginalPos2 = GetCursorPos();
					SetCursorPos({ vOriginalPos2.x + 40, vOriginalPos2.y - 31 });
					PushStyleColor(ImGuiCol_Text, bFlagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(sStripped.c_str());
					PopStyleColor();

					SetCursorPos({ vOriginalPos2.x + 16, vOriginalPos2.y - 33 });
					IconImage(bFlagActive ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, true, bFlagActive ? F::Render.Accent.Value : F::Render.Inactive.Value);
					SetCursorPos(vOriginalPos2);
				}
				else
				{
					if (iFlags & FDropdown_Modifiable)
					{
						ImVec2 vOriginalPos2 = GetCursorPos();
						SetCursorPos({ vOriginalPos2.x + 16, vOriginalPos2.y - 1 });
						if (IconButton(vValues[i] == -1 ? ICON_MD_ADD_CIRCLE : ICON_MD_REMOVE_CIRCLE, false, { 0, 0, 0, 0 }) && pModified)
							*pModified = vValues[i];
						SetCursorPos(vOriginalPos2);
					}

					if (Selectable(std::format("##{}", sEntry).c_str(), *pVar == vValues[i]))
						*pVar = vValues[i], bReturn = true;

					ImVec2 vOriginalPos2 = GetCursorPos();
					SetCursorPos({ vOriginalPos2.x + (iFlags & FDropdown_Modifiable ? 40 : 20), vOriginalPos2.y - 31 });
					PushStyleColor(ImGuiCol_Text, *pVar == vValues[i] ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(sStripped.c_str());
					PopStyleColor();

					if (iFlags & FDropdown_Modifiable) // do second image here so as to not cover
					{
						SetCursorPos({ vOriginalPos2.x + 16, vOriginalPos2.y - 33 });
						IconImage(vValues[i] == -1 ? ICON_MD_ADD_CIRCLE : ICON_MD_REMOVE_CIRCLE);
					}
					SetCursorPos(vOriginalPos2);
				}
				i++;
			}
			PopStyleVar();

			SetCursorPosY(GetCursorPosY() - 10); Dummy({});

			EndCombo();
		}
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			ImVec2 vOriginalPos2 = GetCursorPos();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y - 5 });
			PushFont(F::Render.FontSmall);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			TextUnformatted(StripDoubleHash(sLabel).c_str());
			PopStyleColor();
			PopFont();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y + 8 });
			TextUnformatted(TruncateText(sPreview.c_str(), flSizeX - 55).c_str());

			SetCursorPos({ vOriginalPos2.x + flSizeX - 25, vOriginalPos2.y - 2 });
			IconImage(bActive ? ICON_MD_ARROW_DROP_UP : ICON_MD_ARROW_DROP_DOWN, true);

			EndComboPreview();
		}
		SetCursorPos(vOriginalPos); DebugDummy({ flSizeX, 48 });

		PopItemWidth();
		PopStyleVar();

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			float w = (iFlags & (FDropdown_Left | FDropdown_Right) ? GetWindowSize().x / 2 + 4 : GetWindowSize().x) - 2 * GetStyle().WindowPadding.x + iSizeOffset;
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + 8, w, 40.f);
		}

		return bReturn;
	}

	inline bool FSDropdown(const char* sLabel, std::string* pVar, std::vector<const char*> vEntries = {}, int iFlags = 0, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		auto uHash = FNV1A::Hash32Const(sLabel);
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 13.5f });
		if (vEntries.empty())
		{
			PushStyleColor(ImGuiCol_PopupBg, {});
			PushStyleVar(ImGuiStyleVar_WindowPadding, { GetStyle().WindowPadding.x, 0 });
		}
		float flSizeX = GetWindowSize().x;
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			flSizeX = flSizeX / 2 + 4;
		if (iFlags & FDropdown_Right)
			SameLine(flSizeX);
		if (strstr(sLabel, "## Bind"))
			iSizeOffset = 0;
		flSizeX = flSizeX - 2 * GetStyle().WindowPadding.x + iSizeOffset;
		PushItemWidth(flSizeX);

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, 8 });

		if (Disabled)
		{	// lol
			Button("##", { flSizeX, 40 });
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, 8 });
		}

		static std::string sPreview = "", sInput = "", sTab = "\n";
		if (BeginCombo(std::format("##{}", sLabel).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			if (!mActiveMap[uHash])
				sPreview = sInput = "";

			mActiveMap[uHash] = true;

			// this textinput is being used as a temporary measure to prevent the main window drawing over the popup
			ImVec2 vOriginalPos2 = GetCursorPos();
			SetCursorPos({ -1000, vEntries.empty() ? -100 : GetScrollY() }); // lol
			if (!IsAnyItemActive())
				SetKeyboardFocusHere();
			bool bEnter = InputText("##FSDropdown", &sInput, ImGuiInputTextFlags_EnterReturnsTrue);
			if (sInput != sTab)
			{
				sPreview = sInput;
				sTab = "\n";
			}
			SetCursorPos(vOriginalPos2);

			auto uPreviewHash = FNV1A::Hash32(sPreview.c_str());
			std::deque<std::string> vValid = {};
			{
				std::string sSearch = sInput;
				std::transform(sSearch.begin(), sSearch.end(), sSearch.begin(), ::tolower);
				for (auto& sEntry : vEntries)
				{
					if (FNV1A::Hash32(sEntry) == FNV1A::Hash32Const("##Divider"))
					{
						vValid.push_back(sEntry);
						continue;
					}

					std::string sEntryLower = sEntry;
					std::transform(sEntryLower.begin(), sEntryLower.end(), sEntryLower.begin(), ::tolower);
					if (sEntryLower.find(sSearch) != std::string::npos)
						vValid.push_back(sEntry);
				}
			}

			if (!vValid.empty() && FNV1A::Hash32(vValid.front().c_str()) == FNV1A::Hash32Const("##Divider"))
				vValid.pop_front();
			if (!vValid.empty() && FNV1A::Hash32(vValid.back().c_str()) == FNV1A::Hash32Const("##Divider"))
				vValid.pop_back();
			if (!vValid.empty())
			{
				int iTab = U::KeyHandler.Pressed(VK_TAB) ? 1 : 0;

				DebugDummy({ 0, 8 });
				PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 19 });

				bool bDivider = false;
				for (auto& sEntry : vValid)
				{
					if (FNV1A::Hash32(sEntry.c_str()) == FNV1A::Hash32Const("##Divider"))
					{
						if (!bDivider)
						{
							ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
							ImColor vInactive = F::Render.Inactive; vInactive.Value.w *= GetStyle().Alpha;
							GetWindowDrawList()->AddRectFilled({ vDrawPos.x + 18, vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowSize().x - 18, vDrawPos.y + flPosY + 1 }, vInactive);
							Dummy({});
						}
						bDivider = true;
						continue;
					}
					else
						bDivider = false;

					if (bEnter && !(iFlags & FSDropdown_Custom))
					{
						*pVar = sEntry; bReturn = true;
						CloseCurrentPopup(); break;
					}
					if (iTab)
					{
						if (uPreviewHash == FNV1A::Hash32(sEntry.c_str()))
							iTab = 1;
						else if (iTab == 1)
						{
							sPreview = sEntry;
							sTab = sInput;
							iTab = 2;
						}
					}

					bool bActive = FNV1A::Hash32(pVar->c_str()) == FNV1A::Hash32(sEntry.c_str());
					if (Selectable(std::format("##{}", sEntry).c_str(), bActive))
						*pVar = sEntry, bReturn = true;

					ImVec2 vOriginalPos3 = GetCursorPos();
					SetCursorPos({ vOriginalPos3.x + 20, vOriginalPos3.y - 31 });
					PushStyleColor(ImGuiCol_Text, bActive ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(sEntry.c_str());
					PopStyleColor();
					SetCursorPos(vOriginalPos3);
				}

				PopStyleVar();
				SetCursorPosY(GetCursorPosY() - 10); Dummy({});
			}

			if ((bEnter || iFlags & FSDropdown_AutoUpdate) && (iFlags & FSDropdown_Custom || vEntries.empty()))
				*pVar = sPreview; bReturn = true;
			if (bEnter)
				CloseCurrentPopup();

			EndCombo();
		}
		else
			mActiveMap[uHash] = false;
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_TextInput);
		if (BeginComboPreview())
		{
			ImVec2 vOriginalPos2 = GetCursorPos();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y - 5 });
			PushFont(F::Render.FontSmall);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			TextUnformatted(StripDoubleHash(sLabel).c_str());
			PopStyleColor();
			PopFont();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y + 8 });
			// would like this to work properly, text looks nicer but overrides popup window
			/*
			if (mActiveMap[uHash])
			{
				PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
				PushStyleColor(ImGuiCol_FrameBg, {});
				PushItemWidth(flSizeX - 12);
				if (!IsAnyItemActive()) // silly, but afaik no way to have a one time focus
					SetKeyboardFocusHere();
				enter = FInputText("##FSDropdown", &preview, ImGuiInputTextFlags_EnterReturnsTrue);
				PopItemWidth();
				PopStyleColor();
				PopStyleVar();
			}
			else
				TextUnformatted(TruncateText(var->c_str(), flSizeX - (entries.size() ? 55 : 15)).c_str());
			*/
			TextUnformatted(TruncateText(mActiveMap[uHash] ? sPreview.c_str() : pVar->c_str(), flSizeX - (vEntries.empty() ? 35 : 55)).c_str());

			if (!vEntries.empty())
			{
				SetCursorPos({ vOriginalPos2.x + flSizeX - 25, vOriginalPos2.y - 2 });
				IconImage(mActiveMap[uHash] ? ICON_MD_ARROW_DROP_UP : ICON_MD_ARROW_DROP_DOWN, true);
			}

			if (mActiveMap[uHash] || iFlags & FSDropdown_Custom || vEntries.empty())
			{
				ImVec2 vDrawPos = GetDrawPos(); vDrawPos += vOriginalPos2;
				GetWindowDrawList()->AddRectFilled({ vDrawPos.x + 12, vDrawPos.y + 22 }, { vDrawPos.x + flSizeX - (vEntries.empty() ? 13 : 33), vDrawPos.y + 23 }, mActiveMap[uHash] ? F::Render.Active : F::Render.Inactive);
			}

			EndComboPreview();
		}
		PopItemWidth();
		PopStyleVar();
		if (vEntries.empty())
		{
			PopStyleColor();
			PopStyleVar();
		}

		SetCursorPos(vOriginalPos); DebugDummy({ flSizeX, 48 });

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			float w = (iFlags & (FDropdown_Left | FDropdown_Right) ? GetWindowSize().x / 2 + 4 : GetWindowSize().x) - 2 * GetStyle().WindowPadding.x + iSizeOffset;
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + 8, w, 40.f);
		}

		return bReturn;
	}

	/*
	// in it's current state it sees no use, so i'm commenting it out for now
	inline bool FVDropdown(const char* sLabel, std::vector<std::string>* pVar, std::vector<std::string> vEntries, int iFlags = 0, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		std::unordered_map<std::string, std::vector<std::string>::iterator> mIts = {};
		for (auto it = pVar->begin(); it != pVar->end(); it++)
			mIts[*it] = it;

		std::string sPreview = "";
		if (pVar->empty())
			sPreview = "None";
		else
		{
			for (size_t i = 0; i < pVar->size(); i++)
				sPreview += std::format("{}, ", (*pVar)[i].c_str());
			sPreview.pop_back(); sPreview.pop_back();
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 13.5f });
		float flSizeX = GetWindowSize().x;
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			flSizeX = flSizeX / 2 + 4;
		if (iFlags & FDropdown_Right)
			SameLine(flSizeX);
		if (strstr(sLabel, "## Bind"))
			iSizeOffset = 0;
		flSizeX = flSizeX - 2 * GetStyle().WindowPadding.x + iSizeOffset;
		PushItemWidth(flSizeX);

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, 8 });

		if (Disabled)
		{	// lol
			Button("##", { flSizeX, 40 });
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, 8 });
		}

		bool bActive = false;
		if (BeginCombo(std::format("##{}", sLabel).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			bActive = true;

			DebugDummy({ 0, 8 });
			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 19 });
			for (auto& sEntry : vEntries)
			{
				if (FNV1A::Hash32(sEntry.c_str()) == FNV1A::Hash32Const("##Divider"))
				{
					ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
					ImColor tInactive = F::Render.Inactive; tInactive.Value.w *= GetStyle().Alpha;
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x + 18, vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowSize().x - 18, vDrawPos.y + flPosY + 1 }, tInactive);
					Dummy({});
					continue;
				}

				auto cFind = mIts.find(sEntry);
				bool bFlagActive = cFind != mIts.end();

				if (Selectable(std::format("##{}", sEntry).c_str(), bFlagActive, ImGuiSelectableFlags_DontClosePopups))
				{
					if (bFlagActive)
						pVar->erase(cFind->second);
					else
						pVar->push_back(sEntry);
					bReturn = true;
				}

				// shift based on number of digits in var size
				ImVec2 vOriginalPos2 = GetCursorPos();
				SetCursorPos({ vOriginalPos2.x + 40 + 6 * int(log10(std::max(pVar->size(), 1ui64))), vOriginalPos2.y - 31 });
				PushStyleColor(ImGuiCol_Text, bFlagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
				TextUnformatted(sEntry.c_str());
				PopStyleColor();

				if (bFlagActive)
				{
					SetCursorPos({ vOriginalPos2.x + 18, vOriginalPos2.y - 31 });
					PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
					Text("%i", std::distance(pVar->begin(), mIts[sEntry]) + 1);
					PopStyleColor();
				}
				SetCursorPos(vOriginalPos2);
			}
			PopStyleVar();
			SetCursorPosY(GetCursorPosY() - 10); Dummy({});

			EndCombo();
		}
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			ImVec2 vOriginalPos2 = GetCursorPos();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y - 5 });
			PushFont(F::Render.FontSmall);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			TextUnformatted(StripDoubleHash(sLabel).c_str());
			PopStyleColor();
			PopFont();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y + 8 });
			TextUnformatted(TruncateText(sPreview.c_str(), flSizeX - 55).c_str());

			SetCursorPos({ vOriginalPos2.x + flSizeX - 25, vOriginalPos2.y - 2 });
			IconImage(bActive ? ICON_MD_ARROW_DROP_UP : ICON_MD_ARROW_DROP_DOWN, true);

			EndComboPreview();
		}
		SetCursorPos(vOriginalPos); DebugDummy({ flSizeX, 48 });

		PopItemWidth();
		PopStyleVar();

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			float w = (iFlags & (FDropdown_Left | FDropdown_Right) ? GetWindowSize().x / 2 + 4 : GetWindowSize().x) - 2 * GetStyle().WindowPadding.x + iSizeOffset;
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + 8, w, 40.f);
		}

		return bReturn;
	}
	*/

	inline bool ColorPicker(const char* sLabel, Color_t* tColor, bool bTooltip = true, int iFlags = 0)
	{
		ImVec2 vOriginalPos = GetCursorPos();
		if (Disabled)
		{	// lol
			Button("##", iFlags & FColorPicker_Dropdown ? ImVec2(10, 40) : ImVec2(12, 12));
			SetCursorPos(vOriginalPos);
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 2, 2 });
		PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 4 });
		PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 4, 0 });
		PushStyleColor(ImGuiCol_PopupBg, F::Render.Foreground.Value);
		ImVec4 tempColor = ColorToVec(*tColor);
		bool bReturn = ColorEdit4(std::format("##{}", sLabel).c_str(), &tempColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Round, iFlags & FColorPicker_Dropdown ? ImVec2(10, 40) : ImVec2(12, 12));
		if (bReturn)
			*tColor = VecToColor(tempColor);
		PopStyleColor();
		PopStyleVar(3);
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (bTooltip)
			FTooltip(sLabel);

		return bReturn;
	}

	// if items overlap, use before to have working input, e.g. a middle toggle and a color picker
	inline bool FColorPicker(const char* sLabel, Color_t* tColor, int iOffset = 0, int iFlags = 0, bool* pHovered = nullptr)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool bReturn = false;
		if (!(iFlags & FColorPicker_Dropdown))
		{
			int iPos = 14;
			if (iFlags & FColorPicker_Left)
				iPos += (iOffset * 12);
			else if (iFlags & FColorPicker_Middle)
				iPos += GetContentRegionMax().x / 2 - (iOffset * 12);
			else
				iPos = GetContentRegionMax().x - 18 - (iOffset * 12);
			if (iFlags & FColorPicker_SameLine)
				SameLine(iPos);
			else
				SetCursorPosX(iPos);

			ImVec2 vOriginalPos = GetCursorPos();
			DebugShift({ 0, 6 });

			bReturn = ColorPicker(sLabel, tColor, !(iFlags & (FColorPicker_Left | FColorPicker_Middle)) || iFlags & FColorPicker_Tooltip);
			if (iFlags & (FColorPicker_Left | FColorPicker_Middle))
			{
				SetCursorPos({ vOriginalPos.x + 18, vOriginalPos.y + 5 });
				TextUnformatted(StripDoubleHash(sLabel).c_str());
				SetCursorPos(vOriginalPos); DebugDummy({ 0, 24 });
			}
			else
			{
				SetCursorPos(vOriginalPos); Dummy({ 0, 0 });
			}

			if (pHovered && IsWindowHovered())
			{
				vOriginalPos += GetDrawPos();
				ImVec2 vSize = { 12, 12 };
				if (iFlags & (FColorPicker_Left | FColorPicker_Middle))
				{
					vOriginalPos.x -= 6;
					vSize = { GetWindowSize().x / 2 + 4 - 2 * GetStyle().WindowPadding.x, 24 };
				}
				else
					vOriginalPos.y += 6;
				*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y, vSize.x, vSize.y);
			}
		}
		else
		{
			SameLine(); DebugShift({ -8, 0 });
			ImVec2 vOriginalPos = GetCursorPos(); DebugShift({ 0, 8 });
			bReturn = ColorPicker(sLabel, tColor, iFlags & FColorPicker_Tooltip, iFlags);
			SetCursorPos(vOriginalPos); DebugDummy({ 10, 48 });

			if (pHovered && IsWindowHovered())
			{
				vOriginalPos += GetDrawPos();
				*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + 8, 10.f, 40.f);
			}
		}

		if (Transparent || Disabled)
			PopStyleVar();

		return bReturn;
	}

	inline std::string VK2STR(short key)
	{
		switch (key)
		{
		case 0x0: return "none";
		case VK_LBUTTON: return "mouse1";
		case VK_RBUTTON: return "mouse2";
		case VK_MBUTTON: return "mouse3";
		case VK_XBUTTON1: return "mouse4";
		case VK_XBUTTON2: return "mouse5";
		case VK_CONTROL:
		case VK_LCONTROL:
		case VK_RCONTROL: return "control";
		case VK_NUMPAD0: return "num0";
		case VK_NUMPAD1: return "num1";
		case VK_NUMPAD2: return "num2";
		case VK_NUMPAD3: return "num3";
		case VK_NUMPAD4: return "num4";
		case VK_NUMPAD5: return "num5";
		case VK_NUMPAD6: return "num6";
		case VK_NUMPAD7: return "num7";
		case VK_NUMPAD8: return "num8";
		case VK_NUMPAD9: return "num9";
		case VK_DIVIDE: return "num/";
		case VK_INSERT: return "insert";
		case VK_DELETE: return "delete";
		case VK_PRIOR: return "pgup";
		case VK_NEXT: return "pgdown";
		case VK_HOME: return "home";
		case VK_END: return "end";
		case VK_CLEAR: return "clear";
		case VK_UP: return "up";
		case VK_DOWN: return "down";
		case VK_LEFT: return "left";
		case VK_RIGHT: return "right";
		case VK_ESCAPE: return "escape";
		case VK_F13: return "f13";
		case VK_F14: return "f14";
		case VK_F15: return "f15";
		case VK_F16: return "f16";
		case VK_F17: return "f17";
		case VK_F18: return "f18";
		case VK_F19: return "f19";
		case VK_F20: return "f20";
		case VK_F21: return "f21";
		case VK_F22: return "f22";
		case VK_F23: return "f23";
		case VK_F24: return "f24";
		case VK_LWIN:
		case VK_RWIN: return "windows";
		case VK_PAUSE: return "pause";
		case VK_APPS: return "apps";
		}

		std::string str = "unknown";

		CHAR output[16] = { "\0" };
		if (GetKeyNameTextA(MapVirtualKeyW(key, MAPVK_VK_TO_VSC) << 16, output, 16))
			str = output;

		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());

		if (Vars::Debug::Info.Value && FNV1A::Hash32(str.c_str()) == FNV1A::Hash32Const("unknown"))
			str = std::format("{:#x}", key);

		return str;
	}
	inline void FKeybind(const char* sLabel, int& iOutput, int iFlags = 0, int iSizeOffset = 0)
	{
		static bool bCanceled = false;

		ImGuiID uId = GetID(sLabel);
		PushID(sLabel);

		if (GetActiveID() == uId)
		{
			F::Menu.InKeybind = true;

			//FButton("...", flags | FButton_NoUpper, sizeOffset);
			FButton(std::format("{}: ...", sLabel).c_str(), iFlags | FButton_NoUpper, iSizeOffset);
			bool bHovered = IsItemHovered();

			if (bHovered && IsMouseClicked(ImGuiMouseButton_Left))
			{
				bCanceled = true;
				ClearActiveID();
			}
			else
			{
				SetActiveID(uId, GetCurrentWindow());

				int iKeyPressed = 0;
				for (short iKey = 0; iKey < 255; iKey++)
				{
					if (U::KeyHandler.Pressed(iKey))
					{
						iKeyPressed = iKey;
						break;
					}
				}

				if (iKeyPressed)
				{
					switch (iKeyPressed)
					{
					case VK_LBUTTON:
						iOutput = bHovered ? iOutput : iKeyPressed;
						break;
					case VK_ESCAPE:
						if (iFlags & FKeybind_AllowNone)
						{
							iOutput = 0x0;
							break;
						}
						[[fallthrough]];
					default:
						if (iFlags & FKeybind_AllowMenu || iKeyPressed != Vars::Menu::MenuPrimaryKey.Value && iKeyPressed != Vars::Menu::MenuSecondaryKey.Value)
							iOutput = iKeyPressed;
					}
					ClearActiveID();
				}
			}

			GetCurrentContext()->ActiveIdAllowOverlap = true;
		}
		//else if (FButton(VK2STR(output).c_str(), flags | FButton_NoUpper) && !bCanceled)
		else if (FButton(std::format("{}: {}", sLabel, VK2STR(iOutput)).c_str(), iFlags | FButton_NoUpper, iSizeOffset) && !bCanceled)
			SetActiveID(uId, GetCurrentWindow());

		if (bCanceled && !IsMouseDown(ImGuiMouseButton_Left) && !IsMouseReleased(ImGuiMouseButton_Left))
			bCanceled = false;

		PopID();
	}

	// dropdown for materials
	inline bool FMDropdown(const char* sLabel, std::vector<std::pair<std::string, Color_t>>* pVar, int iFlags = 0, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		// material stuff
		std::vector<std::pair<std::string, Material_t>> vMaterials;
		for (auto& [sName, mat] : F::Materials.mChamMaterials)
		{
			if (FNV1A::Hash32(sName.c_str()) != FNV1A::Hash32Const("None"))
				vMaterials.push_back({ sName, mat });
		}

		std::sort(vMaterials.begin(), vMaterials.end(), [&](const auto& a, const auto& b) -> bool
			{
				// keep locked materials higher
				if (a.second.bLocked && !b.second.bLocked)
					return true;
				if (!a.second.bLocked && b.second.bLocked)
					return false;

				return a.first < b.first;
			});

		std::vector<std::string> vEntries = { "Original" };
		for (auto& pair : vMaterials)
			vEntries.push_back(pair.first.c_str());

		// actual dropdown
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		std::unordered_map<std::string, std::vector<std::pair<std::string, Color_t>>::iterator> mIts = {};
		for (auto it = pVar->begin(); it != pVar->end(); it++)
			mIts[it->first] = it;

		std::string sPreview = "";
		if (pVar->empty())
			sPreview = "None";
		else
		{
			for (auto& pair : *pVar)
				sPreview += std::format("{}, ", pair.first.c_str());
			sPreview.pop_back(); sPreview.pop_back();
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 13.5f });
		float flSizeX = GetWindowSize().x;
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			flSizeX = flSizeX / 2 + 4;
		if (iFlags & FDropdown_Right)
			SameLine(flSizeX);
		if (strstr(sLabel, "## Bind"))
			iSizeOffset = 0;
		flSizeX = flSizeX - 2 * GetStyle().WindowPadding.x + iSizeOffset;
		PushItemWidth(flSizeX);

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, 8 });

		if (Disabled)
		{	// lol
			Button("##", { flSizeX, 40 });
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, 8 });
		}

		bool bActive = false;
		if (BeginCombo(std::format("##{}", sLabel).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			bActive = true;

			DebugDummy({ 0, 8 });
			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 19 });
			for (auto& sEntry : vEntries)
			{
				if (FNV1A::Hash32(sEntry.c_str()) == FNV1A::Hash32Const("##Divider"))
				{
					ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
					ImColor tInactive = F::Render.Inactive; tInactive.Value.w *= GetStyle().Alpha;
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x + 18, vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowSize().x - 18, vDrawPos.y + flPosY + 1 }, tInactive);
					Dummy({});
					continue;
				}

				auto cFind = mIts.find(sEntry);
				bool bFlagActive = cFind != mIts.end();
				int iEntry = bFlagActive ? std::distance(pVar->begin(), mIts[sEntry]) + 1 : 0;

				if (bFlagActive) // do here so as to not sink input
				{
					ImVec2 vOriginalPos2 = GetCursorPos();
					SetCursorPos({ vOriginalPos2.x + flSizeX - 31, vOriginalPos2.y + 1 });
					ColorPicker(std::format("MaterialColor{}", iEntry).c_str(), &cFind->second->second, false);
					SetCursorPos(vOriginalPos2);
				}
				bool bHovered = bFlagActive ? IsItemHovered() : false;

				if (Selectable(std::format("##{}", sEntry).c_str(), bFlagActive, ImGuiSelectableFlags_DontClosePopups))
				{
					if (bFlagActive)
						pVar->erase(cFind->second);
					else
						pVar->push_back({ sEntry, {} });
					bReturn = true;
				}
				bHovered = !bHovered && IsItemHovered();

				// shift based on number of digits in var size
				ImVec2 vOriginalPos2 = GetCursorPos();
				SetCursorPos({ vOriginalPos2.x + 40 + 6 * int(log10(std::max(pVar->size(), 1ui64))), vOriginalPos2.y - 31 });
				PushStyleColor(ImGuiCol_Text, bFlagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
				TextUnformatted(bHovered ? sEntry.c_str() : TruncateText(sEntry.c_str(), flSizeX - (bFlagActive ? 92 : 70)).c_str());
				PopStyleColor();

				if (bFlagActive)
				{
					SetCursorPos({ vOriginalPos2.x + 18, vOriginalPos2.y - 31 });
					PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
					Text("%i", iEntry);
					PopStyleColor();
				}
				SetCursorPos(vOriginalPos2);
			}
			PopStyleVar();
			SetCursorPosY(GetCursorPosY() - 10); Dummy({});

			EndCombo();
		}
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			ImVec2 vOriginalPos2 = GetCursorPos();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y - 5 });
			PushFont(F::Render.FontSmall);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			TextUnformatted(StripDoubleHash(sLabel).c_str());
			PopStyleColor();
			PopFont();

			SetCursorPos({ vOriginalPos2.x + 12, vOriginalPos2.y + 8 });
			TextUnformatted(TruncateText(sPreview.c_str(), flSizeX - 55).c_str());

			SetCursorPos({ vOriginalPos2.x + flSizeX - 25, vOriginalPos2.y - 2 });
			IconImage(bActive ? ICON_MD_ARROW_DROP_UP : ICON_MD_ARROW_DROP_DOWN, true);

			EndComboPreview();
		}
		SetCursorPos(vOriginalPos); DebugDummy({ flSizeX, 48 });

		PopItemWidth();
		PopStyleVar();

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			float w = (iFlags & (FDropdown_Left | FDropdown_Right) ? GetWindowSize().x / 2 + 4 : GetWindowSize().x) - 2 * GetStyle().WindowPadding.x + iSizeOffset;
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + 8, w, 40.f);
		}

		return bReturn;
	}

	// convar wrappers
	bool OldDisabled, OldTransparent;

	template <class T>
	inline int GetBind(ConfigVar<T>& var, bool bForce = false)
	{
		if (var.m_iFlags & (NOSAVE | NOBIND))
			return DEFAULT_BIND;

		if (bForce)
			return CurrentBind;

		int iParent = CurrentBind;
		while (true)
		{
			if (iParent == DEFAULT_BIND || var.Map.contains(iParent))
				break;
			iParent = F::Binds.GetParent(iParent);
		}
		return iParent;
	}

	template <class T>
	inline T GetParentValue(ConfigVar<T>& var, int iBind) // oh my god
	{
		int iParent = iBind;
		while (true)
		{
			iParent = F::Binds.GetParent(iParent);
			if (iParent == DEFAULT_BIND || var.Map.contains(iParent))
				break;
		}
		return var.Map[iParent];
	}

	template <class T>
	inline T FGet(ConfigVar<T>& var, bool bDisable = false)
	{
		OldDisabled = Disabled, OldTransparent = Transparent;

		int iBind = GetBind(var);
		if (bDisable)
		{
			if (CurrentBind == DEFAULT_BIND)
			{
				if (Vars::Menu::MenuShowsBinds.Value && var.Map[DEFAULT_BIND] != var.Value)
				{
					for (auto& [_iBind, tVal] : var.Map)
					{
						if (_iBind == DEFAULT_BIND)
							continue;

						if (tVal == var.Value)
						{
							Disabled = true;
							return tVal;
						}
					}
				}
			}
			else
				Transparent = CurrentBind != iBind && !(var.m_iFlags & (NOSAVE | NOBIND));
		}
		return var.Map[iBind];
	}

	template <class T>
	inline void FSet(ConfigVar<T>& var, T val)
	{
		if (!Disabled)
		{
			int iBind = GetBind(var, true);
			auto tVal = GetParentValue(var, iBind);

			if (tVal != val)
				var.Map[iBind] = val;
			else if (iBind != DEFAULT_BIND)
			{
				for (auto it = var.Map.begin(); it != var.Map.end();)
				{
					if (it->first == iBind)
						it = var.Map.erase(it);
					else
						++it;
				}
			}
		}

		Disabled = OldDisabled, Transparent = OldTransparent;
	}

	template <class T>
	inline void DrawBindInfo(ConfigVar<T>& var, T& val, std::string sBind, bool bNewPopup)
	{
		FText(std::format("Bind '{}'", sBind).c_str());

		static int iBind = DEFAULT_BIND;
		static Bind_t tBind = {};
		if (bNewPopup)
		{
			iBind = DEFAULT_BIND;
			tBind = { sBind };
		}

		std::vector<const char*> vEntries = {};
		std::vector<int> vValues = {};

		std::vector<std::string> vStrings = {}; // prevent dangling pointers
		for (auto& [_iBind, _] : var.Map)
		{
			if (_iBind != DEFAULT_BIND)
				vValues.push_back(_iBind);
		}
		std::sort(vValues.begin(), vValues.end(), [&](const int a, const int b) -> bool
			{
				return a < b;
			});
		for (auto _iBind : vValues)
			vStrings.push_back(std::format("{}## Bind{}", _iBind != DEFAULT_BIND && _iBind < F::Binds.vBinds.size() ? F::Binds.vBinds[_iBind].Name : sBind, _iBind));
		for (auto& sEntry : vStrings)
			vEntries.push_back(sEntry.c_str());

		vEntries.push_back("new bind");
		vValues.push_back(-1);

		int iModified = -2;
		if (FDropdown("Bind", &iBind, vEntries, vValues, FDropdown_Modifiable, -60, nullptr, &iModified))
		{
			if (iBind != DEFAULT_BIND && iBind < F::Binds.vBinds.size())
				tBind = F::Binds.vBinds[iBind];
			else
				tBind = { sBind };
			if (var.Map.contains(iBind))
				val = var.Map[iBind];
		}
		if (iModified != -2)
		{
			if (iModified == -1)
			{
				iBind = int(F::Binds.vBinds.size());
				tBind = { sBind };
				F::Binds.AddBind(iBind, tBind);
			}
			else
			{
				auto cFind = var.Map.find(iModified);
				if (cFind != var.Map.end())
					var.Map.erase(cFind);

				F::Binds.RemoveBind(iModified, false);
				iBind = -1;
			}
		}

		PushDisabled(iBind == DEFAULT_BIND);

		{
			ImVec2 vOriginalPos = GetCursorPos();

			SetCursorPos({ GetWindowSize().x - 34, 40 });
			if (IconButton(tBind.Visible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF))
				tBind.Visible = !tBind.Visible;

			SetCursorPos({ GetWindowSize().x - 59, 40 });
			if (IconButton(!tBind.Not ? ICON_MD_CODE : ICON_MD_CODE_OFF))
				tBind.Not = !tBind.Not;

			SetCursorPos(vOriginalPos);
		}

		FDropdown("Type", &tBind.Type, { "Key", "Class", "Weapon type" }, {}, FDropdown_Left);
		switch (tBind.Type)
		{
		case 0: tBind.Info = std::min(tBind.Info, 2); FDropdown("Behavior", &tBind.Info, { "Hold", "Toggle", "Double click" }, {}, FDropdown_Right); break;
		case 1: tBind.Info = std::min(tBind.Info, 8); FDropdown("Class", &tBind.Info, { "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdown_Right); break;
		case 2: tBind.Info = std::min(tBind.Info, 2); FDropdown("Weapon type", &tBind.Info, { "Hitscan", "Projectile", "Melee" }, {}, FDropdown_Right); break;
		}

		if (tBind.Type == 0)
			FKeybind("Key", tBind.Key);

		if (!Disabled && iBind != DEFAULT_BIND && iBind < F::Binds.vBinds.size())
		{
			var.Map[iBind] = val;

			// don't completely override to retain misc info
			auto& _tBind = F::Binds.vBinds[iBind];
			_tBind.Type = tBind.Type;
			_tBind.Info = tBind.Info;
			_tBind.Key = tBind.Key;
			_tBind.Visible = tBind.Visible;
			_tBind.Not = tBind.Not;
		}

		Dummy({ 0, 8 });
	}

	#define WRAPPER(function, type, parameters, arguments)\
	inline bool function(const char* sLabel, ConfigVar<type>& var, parameters, bool* pHovered = nullptr)\
	{\
		auto val = FGet(var, true);\
		bool bHovered = false;\
		const bool bReturn = function(sLabel, arguments, &bHovered);\
		FSet(var, val);\
		if (!(var.m_iFlags & NOBIND) && !Disabled && CurrentBind == DEFAULT_BIND)\
		{	/*probably a better way to do this*/\
			static auto staticVal = val;\
			bool bNewPopup = bHovered && IsMouseReleased(ImGuiMouseButton_Right) && !IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId);\
			if (bNewPopup)\
			{\
				OpenPopup(var.m_sName.c_str());\
				staticVal = val;\
			}\
			SetNextWindowSize({ 300, 0 });\
			PushStyleColor(ImGuiCol_PopupBg, F::Render.Foreground.Value);\
			bool bPopup = BeginPopup(var.m_sName.c_str());\
			PopStyleColor();\
			if (bPopup)\
			{\
				std::string sLower = StripDoubleHash(sLabel);\
				std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);\
				switch (FNV1A::Hash32Const(#function)) /*get rid of any visual flags*/\
				{\
				case FNV1A::Hash32Const("FToggle"):\
					iFlags &= ~(FToggle_Left | FToggle_Right); break;\
				case FNV1A::Hash32Const("FSlider"):\
					iFlags &= ~(FSlider_Left | FSlider_Right); break;\
				case FNV1A::Hash32Const("FDropdown"):\
				case FNV1A::Hash32Const("FSDropdown"):\
				/*case FNV1A::Hash32Const("FVDropdown"):*/\
				case FNV1A::Hash32Const("FMDropdown"):\
					iFlags &= ~(FDropdown_Left | FDropdown_Right); break;\
				case FNV1A::Hash32Const("FColorPicker"):\
					iFlags &= ~(FColorPicker_Middle | FColorPicker_SameLine | FColorPicker_Dropdown | FColorPicker_Tooltip);\
					iFlags |= FColorPicker_Left; /*add left flag for color pickers*/\
				}\
				PushTransparent(false);\
				DrawBindInfo(var, staticVal, sLower, bNewPopup);\
				val = staticVal;\
				function(std::format("{}## Bind", sLabel).c_str(), arguments);\
				staticVal = val;\
				PopDisabled();\
				PopTransparent();\
				EndPopup();\
			}\
			SetNextWindowSize({ 0, 0 });\
		}\
		if (pHovered) *pHovered = bHovered;\
		return bReturn;\
	}

	WRAPPER(FToggle, bool, VA_LIST(int iFlags = 0), VA_LIST(&val, iFlags))
	WRAPPER(FSlider, IntRange_t, VA_LIST(int iMin, int iMax, int iStep = 1, const char* fmt = "%d", int iFlags = 0), VA_LIST(&val.Min, &val.Max, iMin, iMax, iStep, fmt, iFlags))
	WRAPPER(FSlider, FloatRange_t, VA_LIST(float flMin, float flMax, float flStep = 1.f, const char* fmt = "%.0f", int iFlags = 0), VA_LIST(&val.Min, &val.Max, flMin, flMax, flStep, fmt, iFlags))
	WRAPPER(FSlider, float, VA_LIST(float flMin, float flMax, float flStep = 1.f, const char* fmt = "%.0f", int iFlags = 0), VA_LIST(&val, flMin, flMax, flStep, fmt, iFlags))
	WRAPPER(FSlider, int, VA_LIST(int iMin, int iMax, int iStep = 1, const char* fmt = "%d", int iFlags = 0), VA_LIST(&val, iMin, iMax, iStep, fmt, iFlags))
	WRAPPER(FDropdown, int, VA_LIST(std::vector<const char*> vEntries, std::vector<int> vValues = {}, int iFlags = 0, int iSizeOffset = 0), VA_LIST(&val, vEntries, vValues, iFlags, iSizeOffset))
	WRAPPER(FSDropdown, std::string, VA_LIST(std::vector<const char*> vEntries = {}, int iFlags = 0, int iSizeOffset = 0), VA_LIST(&val, vEntries, iFlags, iSizeOffset))
	//WRAPPER(FVDropdown, std::vector<std::string>, VA_LIST(std::vector<std::string> vEntries, int iFlags = 0, int iSizeOffset = 0), VA_LIST(&val, vEntries, iFlags, iSizeOffset))
	WRAPPER(FMDropdown, VA_LIST(std::vector<std::pair<std::string, Color_t>>), VA_LIST(int iFlags = 0, int iSizeOffset = 0), VA_LIST(&val, iFlags, iSizeOffset))
	WRAPPER(FColorPicker, Color_t, VA_LIST(int iOffset = 0, int iFlags = 0), VA_LIST(&val, iOffset, iFlags))
	WRAPPER(FColorPicker, Gradient_t, VA_LIST(bool bStart = true, int iOffset = 0, int iFlags = 0), VA_LIST(bStart ? &val.StartColor : &val.EndColor, iOffset, iFlags))
}