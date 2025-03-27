#pragma once
#include "../Render.h"
#include "../../../SDK/Helpers/Draw/Draw.h"
#include "Menu.h"
#include "../Fonts/MaterialDesign/IconDefinitions.h"
#include "../../Binds/Binds.h"
#include "../../Visuals/Materials/Materials.h"
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui_stdlib.h>

enum FTabs_
{
	FTabs_None = 0,
	FTabs_Horizontal = 0,
	FTabs_Vertical = 1 << 0,
	FTabs_HorizontalIcons = 0,
	FTabs_VerticalIcons = 1 << 1,
	FTabs_AlignCenter = 0,
	FTabs_AlignLeft = 1 << 2,
	FTabs_AlignRight = 1 << 3,
	FTabs_AlignTop = 1 << 4,
	FTabs_AlignBottom = 1 << 5,
	FTabs_AlignForward = 0,
	FTabs_AlignReverse = 1 << 6,
	FTabs_BarLeft = 1 << 7,
	FTabs_BarRight = 1 << 8,
	FTabs_BarTop = 1 << 9,
	FTabs_BarBottom = 1 << 10,
	FTabs_Fit = 1 << 11
};

enum FText_
{
	FText_None = 0,
	FText_Middle = 1 << 0,
	FText_Right = 1 << 1,
	FText_SameLine = 1 << 2
};

enum FButton_
{
	FButton_None = 0,
	FButton_Left = 1 << 0,
	FButton_Right = 1 << 1,
	FButton_Fit = 1 << 2,
	FButton_SameLine = 1 << 3,
	FButton_NoUpper = 1 << 4
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
	FSlider_NoAutoUpdate = 1 << 6
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

//#define ALTERNATE_FULL_SLIDER

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
	inline void PopDisabled(int count = 1)
	{
		int iSize = int(vDisabled.size());
		if (iSize < count)
		{
			IM_ASSERT_USER_ERROR(0, "Calling PopDisabled() too many times: stack underflow.");
			count = iSize;
		}
		for (int i = 0; i < count; i++)
			vDisabled.pop_back();
		Disabled = !vDisabled.empty() ? vDisabled.back() : false;
	}
	inline void PushTransparent(bool bTransparent)
	{
		vTransparent.push_back(Transparent = bTransparent);
	}
	inline void PopTransparent(int count = 1)
	{
		int iSize = int(vTransparent.size());
		if (iSize < count)
		{
			IM_ASSERT_USER_ERROR(0, "Calling PopTransparent() too many times: stack underflow.");
			count = iSize;
		}
		for (int i = 0; i < count; i++)
			vTransparent.pop_back();
		Transparent = !vTransparent.empty() ? vTransparent.back() : false;
	}

	inline float fnmodf(float flX, float flY)
	{
		// silly fix for negative values
		return fmodf(flX, flY) + (flX < 0 ? flY : 0);
	}

	inline bool IsColorBright(ImColor tColor)
	{
		return tColor.Value.x + tColor.Value.y + tColor.Value.z > 2.f;
	}
	inline bool IsColorDark(ImColor tColor)
	{
		return tColor.Value.x + tColor.Value.y + tColor.Value.z < 0.788f;
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

		/*
		ImDrawList* pDrawList = GetForegroundDrawList();
		ImVec2 vDrawPos = GetDrawPos() - GetWindowPos();
		pDrawList->AddRectFilled({ vDrawPos.x + x, vDrawPos.y + y }, { vDrawPos.x + x + w, vDrawPos.y + y + h }, bWithin ? ImColor(1.f, 0.f, 0.f, 0.5f) : ImColor(0.f, 1.f, 1.f, 0.5f));
		*/

		return bWithin;
	}

	inline std::string StripDoubleHash(const char* sText)
	{
		std::string sBegin = sText, sEnd = FindRenderedTextEnd(sText);
		return sBegin.replace(sBegin.end() - sEnd.size(), sBegin.end(), "");
	}



	inline void DebugDummy(ImVec2 vSize)
	{
		/*
		ImVec2 vOriginalPos = GetCursorPos();
		PushStyleColor(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.5f });
		Button("##", { std::max(vSize.x, 2.f), std::max(vSize.y, 2.f) });
		PopStyleColor();
		SetCursorPos(vOriginalPos);
		*/

		Dummy(vSize);
	}
	inline void DebugShift(ImVec2 vSize)
	{
		ImVec2 vOriginalPos = GetCursorPos();

		/*
		PushStyleColor(ImGuiCol_Button, { 1.f, 1.f, 1.f, 0.5f });
		Button("##", { std::max(vSize.x, 2.f), std::max(vSize.y, 2.f) });
		PopStyleColor();
		*/

		SetCursorPos({ vOriginalPos.x + vSize.x, vOriginalPos.y + vSize.y });
	}

	inline void RenderBackground(ImU32 uBackground)
	{
		ImVec2 vSize = GetWindowSize();
		ImVec2 vDrawPos = GetDrawPos();
		ImDrawList* pDrawList = GetWindowDrawList();

		pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y }, { vDrawPos.x + vSize.x, vDrawPos.y + vSize.y }, uBackground, H::Draw.Scale(3));
	}

	inline void RenderBackground(ImU32 uBackground, ImU32 uBorder, float flInset = H::Draw.Scale())
	{
		ImVec2 vSize = GetWindowSize();
		ImVec2 vDrawPos = GetDrawPos();
		ImDrawList* pDrawList = GetWindowDrawList();

		pDrawList->AddRectFilled({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + vSize.x, vDrawPos.y - flInset + vSize.y }, uBackground, H::Draw.Scale(3));
		
		flInset += H::Draw.Scale(0.5f) - 0.5f - H::Draw.Scale();
		pDrawList->AddRect({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + vSize.x, vDrawPos.y - flInset + vSize.y }, uBorder, H::Draw.Scale(4), ImDrawFlags_None, H::Draw.Scale());
	}

	inline void RenderTwoToneBackground(float flSize, ImU32 uTitle, ImU32 uBackground, bool bHorizontal = false)
	{
		ImVec2 vSize = GetWindowSize();
		ImVec2 vDrawPos = GetDrawPos();
		ImDrawList* pDrawList = GetWindowDrawList();

		if (!bHorizontal)
		{
			pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y }, { vDrawPos.x + vSize.x, vDrawPos.y + flSize }, uTitle, H::Draw.Scale(3), ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);
			pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y + flSize }, { vDrawPos.x + vSize.x, vDrawPos.y + vSize.y }, uBackground, H::Draw.Scale(3), ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight);
		}
		else
		{
			pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y }, { vDrawPos.x + flSize, vDrawPos.y + vSize.y }, uTitle, H::Draw.Scale(3), ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomLeft);
			pDrawList->AddRectFilled({ vDrawPos.x + flSize, vDrawPos.y }, { vDrawPos.x + vSize.x, vDrawPos.y + vSize.y }, uBackground, H::Draw.Scale(3), ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight);
		}
	}

	inline void RenderTwoToneBackground(float flSize, ImU32 uTitle, ImU32 uBackground, ImU32 uBorder, float flInset = H::Draw.Scale(), bool bVertical = true, bool bTwoToneBorder = true)
	{
		ImVec2 vSize = GetWindowSize();
		ImVec2 vDrawPos = GetDrawPos();
		ImDrawList* pDrawList = GetWindowDrawList();

		if (bVertical)
		{
			pDrawList->AddRectFilled({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + vSize.x, vDrawPos.y + flSize }, uTitle, H::Draw.Scale(3), ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);
			pDrawList->AddRectFilled({ vDrawPos.x + flInset, vDrawPos.y + flSize }, { vDrawPos.x - flInset + vSize.x, vDrawPos.y - flInset + vSize.y }, uBackground, H::Draw.Scale(3), ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight);
			if (bTwoToneBorder)
				pDrawList->AddRectFilled({ vDrawPos.x + flInset, vDrawPos.y + flSize - H::Draw.Scale() }, {vDrawPos.x - flInset + vSize.x, vDrawPos.y + flSize }, uBorder);
		}
		else
		{
			pDrawList->AddRectFilled({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x + flSize, vDrawPos.y - flInset + vSize.y }, uTitle, H::Draw.Scale(3), ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomLeft);
			pDrawList->AddRectFilled({ vDrawPos.x + flSize, vDrawPos.y + flInset }, { vDrawPos.x - flInset + vSize.x, vDrawPos.y - flInset + vSize.y }, uBackground, H::Draw.Scale(3), ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight);
			if (bTwoToneBorder)
				pDrawList->AddRectFilled({ vDrawPos.x + flSize - H::Draw.Scale(), vDrawPos.y + flInset }, { vDrawPos.x + flSize, vDrawPos.y - flInset + vSize.y }, uBorder);
		}
		
		flInset += H::Draw.Scale(0.5f) - 0.5f - H::Draw.Scale();
		pDrawList->AddRect({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + vSize.x, vDrawPos.y - flInset + vSize.y }, uBorder, H::Draw.Scale(4), ImDrawFlags_None, H::Draw.Scale());
	}

	inline void AddSteppedRect(ImVec2 vPos, ImVec2 vPosMin, ImVec2 vPosMax, ImVec2 vClipMin, ImVec2 vClipMax, float flVMin, float flVMax, float flStep, ImU32 uPrimary, ImU32 uSecondary, float flStepWidth)
	{
		ImDrawList* pDrawList = GetWindowDrawList();
		pDrawList->PushClipRect({ vPos.x + vClipMin.x, vPos.y + vClipMin.y }, { vPos.x + vClipMax.x, vPos.y + vClipMax.y }, true);

		int iSteps = (flVMax - flVMin) / flStep;
		if (iSteps < 21)
		{
			std::vector<std::pair<float, float>> vSteps;

			float flMin = flVMin - fnmodf(flVMin + flStep / 2, flStep) + flStep / 2, max = flVMax - fnmodf(flVMax + flStep / 2, flStep) + flStep / 2;

			if (fabsf(flVMin - flMin) < 0.001f)
				vSteps.emplace_back(vPosMin.x, vPosMin.x + flStepWidth);
			while (true)
			{
				flMin += flStep;
				if (flMin + flStep / 2 > flVMax)
					break;

				float flPercent = std::clamp((flMin - flVMin) / (flVMax - flVMin), 0.f, 1.f);
				float flPosition = vPosMin.x + (vPosMax.x - vPosMin.x) * flPercent;
				vSteps.emplace_back(roundf(flPosition - flStepWidth / 2), roundf(flPosition + flStepWidth / 2));
			}
			if (fabsf(flVMax - max) < 0.001f)
				vSteps.emplace_back(vPosMax.x - flStepWidth, vPosMax.x);

			if (!vSteps.empty())
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

	inline ImVec2 FCalcTextSize(const char* sText, ImFont* pFont = nullptr)
	{
		if (pFont) PushFont(pFont);
		auto vTextSize = CalcTextSize(sText);
		if (pFont) PopFont();
		return vTextSize;
	}

	inline void FText(const char* sText, int iFlags = FText_None, ImFont* pFont = nullptr)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		if (pFont)
			PushFont(pFont);

		if (iFlags & FText_SameLine)
			SameLine();
		if (iFlags & (FText_Middle | FText_Right))
		{
			float flWindowWidth = GetWindowWidth();
			float flTextWidth = CalcTextSize(sText).x;
			if (iFlags & FText_Middle)
				SetCursorPosX((flWindowWidth - flTextWidth) * 0.5f);
			else if (iFlags & FText_Right)
				SetCursorPosX(flWindowWidth - flTextWidth - GetStyle().WindowPadding.x);
		}
		auto vDrawPos = GetDrawPos();
		auto vCursorPos = GetCursorPos();
		TextUnformatted(sText);

		if (pFont)
			PopFont();

		if (Transparent || Disabled)
			PopStyleVar();
	}

	inline std::string TruncateText(std::string sText, int iPixels, ImFont* pFont = nullptr, std::string sEnd = "...", size_t* pLength = nullptr)
	{
		if (sText.empty())
			return "";
		else if (FCalcTextSize(sText.c_str(), pFont).x < iPixels) // no unnecessary truncation
			return sText;

		std::string sTruncated = "";
		size_t i = 0;
		while (FCalcTextSize(std::format("{}{}{}", sTruncated, sText[i + 1], sEnd).c_str(), pFont).x < iPixels)
		{
			sTruncated += sText[i++];
			if (i >= sText.length())
			{
				i = 0;
				break;
			}
		}
		if (i)
			sTruncated += sEnd;
		if (pLength)
			*pLength = i;

		return sTruncated;
	}

	inline std::deque<std::string> WrapText(std::string sText, int iPixels, ImFont* pFont = nullptr)
	{
		if (sText.empty())
			return { "" };
		else if (FCalcTextSize(sText.c_str(), pFont).x < iPixels) // no unnecessary wrapping
			return { sText };

		std::vector<std::string> vWords = { "" };
		for (char iChar : sText)
		{
			if (iChar == ' ' || iChar == '\n' || iChar == '\t')
				vWords.push_back(std::string(1, iChar));
			else
				vWords.back().push_back(iChar);
		}

		std::deque<std::string> vWrapped = { "" };
		int iWord = 0;
		for (auto& sWord : vWords)
		{
			begin:
			if (sWord.empty())
				continue;

			auto sSeparator = iWord ? std::string(1, sWord[0]) : "";
			if (sWord[0] == ' ' || sWord[0] == '\n' || sWord[0] == '\t')
				sWord.erase(0, 1);
			if (sSeparator != "\n" && FCalcTextSize(std::format("{}{}{}", vWrapped.back(), sSeparator, sWord).c_str(), pFont).x < iPixels)
			{
				vWrapped.back() += std::format("{}{}", sSeparator, sWord);
				iWord++;
			}
			else
			{
				bool bContinue = false, bFirstWord = !iWord; iWord = 0;
				if (bFirstWord)
				{
					size_t i = 0;
					vWrapped.back() += TruncateText(sWord, iPixels, pFont, "-", &i);
					if (!i) // i don't know why it fucking does this, but i don't care enough to fix it
						bContinue = true;
					else
						sWord = sWord.substr(i, sWord.length() - i);
				}
				if (vWrapped.back().empty())
					break; // don't do the same thing over and over
				vWrapped.push_back("");
				if (!bContinue)
					goto begin;
			}
		}

		return vWrapped;
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

	inline ImVec2 IconSize(const char* sIcon)
	{
		PushFont(F::Render.IconFont);
		ImVec2 vTextSize = CalcTextSize(sIcon);
		PopFont();
		return vTextSize;
	}

	inline void IconImage(const char* sIcon, ImVec4 tColor = { 1, 1, 1, -1 })
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		if (tColor.w >= 0.f)
			PushStyleColor(ImGuiCol_Text, tColor);
		PushFont(F::Render.IconFont);
		TextUnformatted(sIcon);
		PopFont();
		if (tColor.w >= 0.f)
			PopStyleColor();

		if (Transparent || Disabled)
			PopStyleVar();
	}

	inline bool IconButton(const char* sIcon, ImVec4 tColor = { 1, 1, 1, -1 }, bool* pHovered = nullptr)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		ImVec2 vOriginalPos = GetCursorPos();

		if (tColor.w >= 0.f)
			PushStyleColor(ImGuiCol_Text, tColor);
		PushFont(F::Render.IconFont);
		TextUnformatted(sIcon);
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (pHovered)
			*pHovered = IsItemHovered();
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

	inline bool FBeginPopup(const char* str_id, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
	{
		PushStyleVar(ImGuiStyleVar_PopupBorderSize, H::Draw.Scale(1));
		PushStyleColor(ImGuiCol_PopupBg, F::Render.Background0p5.Value);
		bool bReturn = BeginPopup(str_id, flags);
		PopStyleColor();
		PopStyleVar();
		return bReturn;
	}

	inline bool FBeginPopupModal(const char* name, bool* p_open, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
	{
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, H::Draw.Scale(1));
		PushStyleColor(ImGuiCol_PopupBg, F::Render.Background0p5.Value);
		bool bReturn = BeginPopupModal(name, p_open, flags);
		PopStyleColor();
		PopStyleVar();
		return bReturn;
	}

	static std::unordered_map<const char*, float> mLastHeights;
	static std::vector<const char*> vStoredLabels;
	inline bool Section(const char* sLabel, bool bExtraPadding = false, float flMinHeight = 28.f, bool bForceHeight = false)
	{
		vStoredLabels.push_back(sLabel);
		if (!bForceHeight && mLastHeights.contains(sLabel) && mLastHeights[sLabel] > flMinHeight)
			flMinHeight = mLastHeights[sLabel];
		PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
		const bool bReturn = BeginChild(sLabel, { GetColumnWidth(), flMinHeight }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding);
		if (bReturn)
		{
			if (sLabel[0] != '#')
				RenderTwoToneBackground(H::Draw.Scale(28), F::Render.Background0, F::Render.Background0p5, F::Render.Background2);
			else
				RenderBackground(F::Render.Background0p5, F::Render.Background2);
		}

		PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), 0 });
		if (sLabel[0] != '#')
		{
			ImVec2 vOriginalPos = GetCursorPos();

			PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
			PushFont(F::Render.FontBold);
			TextUnformatted(StripDoubleHash(sLabel).c_str());
			PopFont();
			PopStyleColor();

			SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(bExtraPadding ? 27 : 19) });
		}

		return bReturn;
	}
	inline void EndSection()
	{
		const char* sLabel = vStoredLabels.back();
		vStoredLabels.pop_back();
		float flHeight = GetItemRectMax().y - GetWindowPos().y;
		if (flHeight > 0.f)
			mLastHeights[sLabel] = flHeight + GetStyle().WindowPadding.y;

		PopStyleVar();
		EndChild();
		PopStyleVar();
	}

	// widgets
	inline bool FTabs(std::vector<std::vector<const char*>> vEntries, std::vector<int*> vVars, const ImVec2 vSize, const ImVec2 vPos, int iFlags = FTabs_None, std::vector<std::vector<const char*>> vIcons = {}, ImVec2 vAllTextOffset = {}, ImVec2 vAllBarOffset = {}, ImVec2 vSubTextOffset = {}, ImVec2 vSubBarOffset = {}, float flBarSizeMod = 0.f, float flBarThickness = 1.f)
	{	// WOW fuck this
		if (!vIcons.empty() && vIcons.size() != vEntries.size())
		{
			IM_ASSERT_USER_ERROR(0, "FTabs() vIcons size mismatch to vEntries.");
			return false;
		}

		ImDrawList* pDrawList = GetWindowDrawList();
		bool bChanged = false;
		std::pair<int, int> pOriginal;
		{
			auto pVar1 = !vVars.empty() ? vVars[0] : nullptr;
			auto pVar2 = pVar1 && vEntries[*pVar1].size() > 1 && *pVar1 + 1 < vVars.size() ? vVars[*pVar1 + 1] : nullptr;
			pOriginal = { pVar1 ? *pVar1 : -1, pVar2 ? *pVar2 : -1 };
		}

		bool bIcons = !vIcons.empty();
		bool bVertical = iFlags & FTabs_Vertical;
		bool bVerticalIcons = iFlags & FTabs_VerticalIcons;
		bool bFit = iFlags & FTabs_Fit;
		bool bReverse = iFlags & FTabs_AlignReverse;
		int iAlign = iFlags & FTabs_AlignLeft ? 1 : iFlags & FTabs_AlignRight ? 2 : iFlags & FTabs_AlignTop ? 3 : iFlags & FTabs_AlignBottom ? 4 : 0;
		int iBar = iFlags & FTabs_BarLeft ? 1 : iFlags & FTabs_BarRight ? 2 : iFlags & FTabs_BarTop ? 3 : iFlags & FTabs_BarBottom ? 4 : bVertical ? 2 : 4;

		ImVec2 vOffset = vPos;
		for (size_t i = bReverse ? vEntries.size() - 1 : 0; i < vEntries.size(); (bReverse ? i-- : i++))
		{
			for (size_t j = bReverse ? vEntries[i].size() - 1 : 0; j < vEntries[i].size(); (bReverse ? j-- : j++))
			{
				if (j && i != pOriginal.first)
					continue;

				int iTabState = 0;
				if (vEntries[i].size() <= 1)
				{
					if (i == pOriginal.first)
						iTabState = 2;
				}
				else if (!j)
				{
					if (i == pOriginal.first)
						iTabState = 1;
				}
				else
				{
					if (i == pOriginal.first && j - 1 == pOriginal.second)
						iTabState = 2;
				}

				auto sEntry = vEntries[i][j];
				auto sIcon = i < vIcons.size() && j < vIcons[i].size() ? vIcons[i][j] : nullptr;
				int* pVar = nullptr;
				if (!vVars.empty())
				{
					if (!j)
						pVar = vVars[0];
					else if (i + 1 < vVars.size())
						pVar = vVars[i + 1];
				}

				ImVec2 vNewSize = vSize;
				if (bFit)
				{
					if (!bVertical)
						vNewSize.x += FCalcTextSize(StripDoubleHash(sEntry).c_str()).x + (sIcon && !bVerticalIcons ? IconSize(sIcon).x : 0.f);
					else
						vNewSize.y += FCalcTextSize(StripDoubleHash(sEntry).c_str()).y + (sIcon && bVerticalIcons ? IconSize(sIcon).x : 0.f);
				}
				if (bReverse)
				{
					if (!bVertical)
						vOffset.x -= vNewSize.x;
					else
						vOffset.y -= vNewSize.y;
				}
				ImVec2 vDrawPos = GetDrawPos() + vOffset;
				ImVec2 vCurTextOffset = vAllTextOffset + (j ? vSubTextOffset : ImVec2());
				ImVec2 vCurBarOffset = vAllBarOffset + (j ? vSubBarOffset : ImVec2());

				if (!iTabState)
					PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				else if (iTabState == 2)
				{
					switch (iBar)
					{
					case 1: // left
						pDrawList->AddRectFilled(
							{ vDrawPos.x + vCurBarOffset.x, vDrawPos.y + vCurBarOffset.y + flBarSizeMod },
							{ vDrawPos.x + vCurBarOffset.x + H::Draw.Scale(flBarThickness), vDrawPos.y + vCurBarOffset.y - flBarSizeMod + vNewSize.y },
							F::Render.Accent
						);
						break;
					case 2: // right
						pDrawList->AddRectFilled(
							{ vDrawPos.x + vCurBarOffset.x + vNewSize.x - H::Draw.Scale(flBarThickness), vDrawPos.y + vCurBarOffset.y + flBarSizeMod },
							{ vDrawPos.x + vCurBarOffset.x + vNewSize.x, vDrawPos.y + vCurBarOffset.y - flBarSizeMod + vNewSize.y },
							F::Render.Accent
						);
						break;
					case 3: // top
						pDrawList->AddRectFilled(
							{ vDrawPos.x + vCurBarOffset.x + flBarSizeMod, vDrawPos.y + vCurBarOffset.y },
							{ vDrawPos.x + vCurBarOffset.x - flBarSizeMod + vNewSize.x, vDrawPos.y + vCurBarOffset.y + H::Draw.Scale(flBarThickness) },
							F::Render.Accent
						);
						break;
					case 4: // bottom
						pDrawList->AddRectFilled(
							{ vDrawPos.x + vCurBarOffset.x + flBarSizeMod, vDrawPos.y + vCurBarOffset.y + vNewSize.y - H::Draw.Scale(flBarThickness) },
							{ vDrawPos.x + vCurBarOffset.x - flBarSizeMod + vNewSize.x, vDrawPos.y + vCurBarOffset.y + vNewSize.y },
							F::Render.Accent
						);
						break;
					}
				}

				SetCursorPos(vOffset);
				if (Button(std::format("##{}", sEntry).c_str(), vNewSize) && !iTabState && pVar)
				{
					*pVar = int(!j ? i : j - 1);
					bChanged = true;
					if (vStoredLabels.size() == 0)
						mLastHeights.clear();
				}
				if (!Disabled && IsItemHovered())
					SetMouseCursor(ImGuiMouseCursor_Hand);

				ImVec2 vOriginalPos = GetCursorPos();
				std::string sStripped = StripDoubleHash(sEntry);

				ImVec2 vTextSize = FCalcTextSize(sStripped.c_str());
				ImVec2 vIconSize = { H::Draw.Scale(16), H::Draw.Scale(16) }; //IconSize(vIcons[i]);

				ImVec2 vTextOffset, vIconOffset;
				if (bIcons)
				{
					if (!bVerticalIcons)
					{
						switch (iAlign)
						{
						case 0: // center
							vTextOffset = { (vNewSize.x - vTextSize.x) / 2 + vIconSize.x / 1.5f, (vNewSize.y - vTextSize.y) / 2 };
							vIconOffset = { (vNewSize.x - vTextSize.x) / 2 - vIconSize.x / 1.5f, (vNewSize.y - vIconSize.y) / 2 };
							break;
						case 1: // left
							vTextOffset = { vIconSize.x * 1.5f, (vNewSize.y - vTextSize.y) / 2 };
							vIconOffset = { 0.f, (vNewSize.y - vIconSize.y) / 2 };
							break;
						case 2: // right
							vTextOffset = { vNewSize.x - vTextSize.x, (vNewSize.y - vTextSize.y) / 2 };
							vIconOffset = { vNewSize.x - vTextSize.x - vIconSize.x * 1.5f, (vNewSize.y - vIconSize.y) / 2 };
							break;
						case 3: // top
							vTextOffset = { (vNewSize.x - vTextSize.x) / 2 + vIconSize.x / 1.5f, { (vIconSize.y - vTextSize.y) / 2 } };
							vIconOffset = { (vNewSize.x - vTextSize.x) / 2 - vIconSize.x / 1.5f, 0.f };
							break;
						case 4: // bottom
							vTextOffset = { (vNewSize.x - vTextSize.x) / 2 + vIconSize.x / 1.5f, vNewSize.y - vTextSize.y - (vIconSize.y - vTextSize.y) / 2 };
							vIconOffset = { (vNewSize.x - vTextSize.x) / 2 - vIconSize.x / 1.5f, vNewSize.y - vIconSize.y };
							break;
						}
					}
					else
					{
						switch (iAlign)
						{
						case 0: // center
							vTextOffset = { (vNewSize.x - vTextSize.x) / 2, (vNewSize.y - vTextSize.y) / 2 + vIconSize.y / 1.5f };
							vIconOffset = { (vNewSize.x - vIconSize.x) / 2, (vNewSize.y - vTextSize.y) / 2 - vIconSize.y / 1.5f };
							break;
						case 1: // left
							vTextOffset = { 0.f, (vNewSize.y - vTextSize.y) / 2 + vIconSize.y / 1.5f };
							vIconOffset = { (vTextSize.x - vIconSize.x) / 2, (vNewSize.y - vTextSize.y) / 2 - vIconSize.y / 1.5f };
							break;
						case 2: // right
							vTextOffset = { vNewSize.x - vTextSize.x, (vNewSize.y - vTextSize.y) / 2 + vIconSize.y / 1.5f };
							vIconOffset = { vNewSize.x - (vTextSize.x + vIconSize.x) / 2, (vNewSize.y - vTextSize.y) / 2 - vIconSize.y / 1.5f };
							break;
						case 3: // top
							vTextOffset = { (vNewSize.x - vTextSize.x) / 2, vIconSize.y * 1.5f };
							vIconOffset = { (vNewSize.x - vIconSize.x) / 2, 0.f };
							break;
						case 4: // bottom
							vTextOffset = { (vNewSize.x - vTextSize.x) / 2, vNewSize.y - vTextSize.y };
							vIconOffset = { (vNewSize.x - vIconSize.x) / 2, vNewSize.y - vTextSize.y - vIconSize.y * 1.5f };
							break;
						}
					}
				}
				else
				{
					switch (iAlign)
					{
					case 0: // center
						vTextOffset = { (vNewSize.x - vTextSize.x) / 2, (vNewSize.y - vTextSize.y) / 2 };
						break;
					case 1: // left
						vTextOffset = { 0.f, (vNewSize.y - vTextSize.y) / 2 };
						break;
					case 2: // right
						vTextOffset = { vNewSize.x - vTextSize.x, (vNewSize.y - vTextSize.y) / 2 };
						break;
					case 3: // top
						vTextOffset = { (vNewSize.x - vTextSize.x) / 2, 0.f };
						break;
					case 4: // bottom
						vTextOffset = { (vNewSize.x - vTextSize.x) / 2, vNewSize.y - vTextSize.y };
						break;
					}
				}

				SetCursorPos(vOffset + vCurTextOffset + vTextOffset);
				TextUnformatted(sStripped.c_str());

				if (sIcon)
				{
					SetCursorPos(vOffset + vCurTextOffset + vIconOffset);
					IconImage(sIcon);
				}

				SetCursorPos(vOriginalPos);

				if (!iTabState)
					PopStyleColor();

				if (!bReverse)
				{
					if (!bVertical)
						vOffset.x += vNewSize.x;
					else
						vOffset.y += vNewSize.y;
				}
			}
		}

		return bChanged;
	}

	inline bool FTabs(std::vector<const char*> vEntries, int* pVar, const ImVec2 vSize, const ImVec2 vPos, int iFlags = FTabs_None, std::vector<const char*> vIcons = {}, ImVec2 vAllTextOffset = {}, ImVec2 vAllBarOffset = {}, ImVec2 vSubTextOffset = {}, ImVec2 vSubBarOffset = {}, float flBarSizeMod = 0.f, float flBarThickness = 1.f)
	{
		std::vector<std::vector<const char*>> vNewEntries;
		for (auto& sEntry : vEntries)
			vNewEntries.push_back({ sEntry });
		std::vector<std::vector<const char*>> vNewIcons;
		for (auto& sIcon : vIcons)
			vNewIcons.push_back({ sIcon });

		return FTabs(vNewEntries, { pVar }, vSize, vPos, iFlags, vNewIcons, vAllTextOffset, vAllBarOffset, vSubTextOffset, vSubBarOffset, flBarSizeMod, flBarThickness);
	}

	inline bool FSelectable(const char* sLabel, ImVec4* pColor, float flRounding = H::Draw.Scale(3), bool bSelected = false, int iFlags = ImGuiSelectableFlags_None, const ImVec2& vSize = {})
	{
		PushStyleVar(ImGuiStyleVar_FrameRounding, flRounding);
		if (pColor)
		{
			auto tColor = *pColor;
			PushStyleColor(ImGuiCol_HeaderHovered, tColor);
			tColor.x /= 1.1f; tColor.y /= 1.1f; tColor.z /= 1.1f;
			PushStyleColor(ImGuiCol_HeaderActive, tColor);
		}
		const bool bReturn = Selectable(sLabel, bSelected, iFlags, vSize);
		if (pColor)
			PopStyleColor(2);
		PopStyleVar();
		return bReturn;
	}

	inline bool FSelectable(const char* sLabel, ImVec4 tColor = { 0.2f, 0.6f, 0.85f, 1.f }, float flRounding = H::Draw.Scale(3), bool bSelected = false, int iFlags = ImGuiSelectableFlags_None, const ImVec2& vSize = {})
	{
		return FSelectable(sLabel, &tColor, flRounding, bSelected, iFlags, vSize);
	}

	struct Row_t
	{
		float m_flPosY;
		float m_flSizeY;
	};
	static std::vector<Row_t> vRowSizes;
	inline void AddRowSize(float flPosY, float flSizeY)
	{
		if (GetCurrentWindow()->Flags & ImGuiWindowFlags_Popup)
			return;

		flPosY += GetDrawPos().y;
		if (!vRowSizes.empty() && vRowSizes.back().m_flPosY != flPosY)
			vRowSizes.clear();
		vRowSizes.emplace_back(flPosY, flSizeY);
	}
	inline float GetRowPos(bool bDrawPos = false)
	{
		if (!vRowSizes.empty())
			return vRowSizes.front().m_flPosY - (!bDrawPos ? GetDrawPos().y : 0.f);
		else
			return GetCursorPosY() + (bDrawPos ? GetDrawPos().y : 0.f);
	}
	inline float GetRowSize(float flDefault = 0.f)
	{
		if (GetCurrentWindow()->Flags & ImGuiWindowFlags_Popup)
			return flDefault;

		float flMax = 0.f;
		for (auto& [_, flSize] : vRowSizes)
			flMax = std::max(flMax, flSize);
		return flMax;
	}

	inline bool FButton(const char* sLabel, int iFlags = FButton_None, ImVec2 vSize = { 0, 30 }, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		std::string sLabel2 = sLabel;
		if (!(iFlags & FButton_NoUpper))
		{
			std::transform(sLabel2.begin(), sLabel2.end(), sLabel2.begin(), ::toupper);
			sLabel = sLabel2.c_str();
		}

		if (!vSize.x)
		{
			vSize.x = GetWindowWidth();
			if (iFlags & (FButton_Left | FButton_Right))
				vSize.x = vSize.x / 2 - GetStyle().WindowPadding.x * 1.5f;
			else if (iFlags & FButton_Fit)
				vSize.x = FCalcTextSize(sLabel).x + H::Draw.Scale(vSize.y - 12);
			else
				vSize.x -= GetStyle().WindowPadding.x * 2;
			if (iFlags & FButton_SameLine)
				SameLine();
			else if (iFlags & FButton_Right)
				SetCursorPosX(vSize.x + 20);
		}
		else
			vSize.x = H::Draw.Scale(vSize.x);
		vSize.x += H::Draw.Scale(iSizeOffset); vSize.y = H::Draw.Scale(vSize.y);

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, GetStyle().WindowPadding.y });

		ImVec4 vAccent = F::Render.Accent.Value;
		ImVec2 vDrawPos = GetCursorPos() + GetDrawPos();
		bool bClicked = IsMouseDown(ImGuiMouseButton_Left);
		bool bWithin = !Disabled && (IsWindowHovered() || bClicked) && IsMouseWithin(vDrawPos.x, vDrawPos.y, vSize.x, vSize.y);
		if (bWithin && bClicked)
			vAccent.x /= 1.5f, vAccent.y /= 1.5f, vAccent.z /= 1.5f;
		PushStyleColor(ImGuiCol_Border, vAccent);
		PushStyleVar(ImGuiStyleVar_FrameBorderSize, H::Draw.Scale(bWithin ? 2 : 1));
		const bool bReturn = Button(sLabel, vSize);
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (pHovered)
			*pHovered = IsItemHovered();
		PopStyleVar();
		PopStyleColor();

		SetCursorPos(vOriginalPos);
		AddRowSize(vOriginalPos.y, vSize.y + GetStyle().WindowPadding.y);
		DebugDummy({ vSize.x, GetRowSize(vSize.y + GetStyle().WindowPadding.y) });

		if (Transparent || Disabled)
			PopStyleVar();

		return Disabled ? false : bReturn;
	}

	inline bool FToggle(const char* sLabel, bool* pVar, int iFlags = FToggle_None, bool* pHovered = nullptr)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		ImVec2 vSize;

		vSize.x = GetWindowWidth();
		if (iFlags & (FToggle_Left | FToggle_Right))
			vSize.x = vSize.x / 2 - GetStyle().WindowPadding.x * 1.5f;
		else
			vSize.x -= GetStyle().WindowPadding.x * 2;
		if (iFlags & FToggle_Right)
			SameLine(vSize.x + GetStyle().WindowPadding.x * 2.f);

		ImVec2 vOriginalPos = GetCursorPos();

		auto vWrapped = WrapText(StripDoubleHash(sLabel), vSize.x - H::Draw.Scale(24));
		int iWraps = std::min(int(vWrapped.size()), 2); // prevent too many wraps
		vSize.y = H::Draw.Scale(6 + 18 * iWraps);

		bool bReturn = Button(std::format("##{}", sLabel).c_str(), vSize);
		if (Disabled)
			bReturn = false;
		if (bReturn)
			*pVar = !*pVar;
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);

		SetCursorPos({ vOriginalPos.x + H::Draw.Scale(4), vOriginalPos.y + H::Draw.Scale(-5 + 9 * iWraps) });
		IconImage(*pVar ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, *pVar ? (iFlags & FToggle_PlainColor ? F::Render.Active.Value : F::Render.Accent.Value) : F::Render.Inactive.Value);

		PushStyleColor(ImGuiCol_Text, *pVar ? F::Render.Active.Value : F::Render.Inactive.Value);
		for (size_t i = 0; i < iWraps; i++)
		{
			SetCursorPos({ vOriginalPos.x + H::Draw.Scale(24), vOriginalPos.y + H::Draw.Scale(5 + 18 * i) });
			TextUnformatted(vWrapped[i].c_str());
		}
		PopStyleColor();

		SetCursorPos(vOriginalPos);
		AddRowSize(vOriginalPos.y, vSize.y);
		DebugDummy({ vSize.x, GetRowSize(vSize.y) });

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y, vSize.x, vSize.y);
		}

		return bReturn;
	}

	inline bool FSlider(const char* sLabel, float* pVar1, float* pVar2, float flMin, float flMax, float flStep = 1.f, const char* fmt = "%.0f", int iFlags = FSlider_None, bool* pHovered = nullptr)
	{
		auto uHash = FNV1A::Hash32Const(sLabel);

		ImDrawList* pDrawList = GetWindowDrawList();
		float flOriginal1 = *pVar1, flOriginal2 = pVar2 ? *pVar2 : 0.f;

		static std::unordered_map<uint32_t, std::pair<float, float>> mStaticVars;
		if (!mActiveMap[uHash])
			mStaticVars[uHash] = { flOriginal1, flOriginal2 };
		float& flSVar1 = mStaticVars[uHash].first, &flSVar2 = mStaticVars[uHash].second;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		ImVec2 vSize;
		bool bFull = !(iFlags & (FSlider_Left | FSlider_Right));

		vSize.x = GetWindowWidth();
		if (!bFull)
			vSize.x = vSize.x / 2 - GetStyle().WindowPadding.x * 1.5f;
		else
			vSize.x -= GetStyle().WindowPadding.x * 2;
		if (iFlags & FSlider_Right)
			SameLine(vSize.x + GetStyle().WindowPadding.x * 2);

		ImVec2 vOriginalPos = GetCursorPos(), vDrawPos = GetDrawPos();
		PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);

		static std::unordered_map<uint32_t, float> mEntryWidth = {};
		float& flEntryWidth = mEntryWidth[FNV1A::Hash32(sLabel)];
#ifdef ALTERNATE_FULL_SLIDER
		auto vWrapped = WrapText(StripDoubleHash(sLabel), bFull ? vSize.x / 2 - H::Draw.Scale(24) : vSize.x - flEntryWidth - H::Draw.Scale(20));
#else
		auto vWrapped = WrapText(StripDoubleHash(sLabel), vSize.x - H::Draw.Scale(20) - flEntryWidth);
#endif
		int iWraps = std::min(int(vWrapped.size()), 2); // prevent too many wraps
#ifdef ALTERNATE_FULL_SLIDER
		vSize.y = H::Draw.Scale(H::Draw.Scale(bFull ? 6 : 14) + 18 * iWraps);
#else
		vSize.y = H::Draw.Scale(14 + 18 * iWraps);
#endif

		for (size_t i = 0; i < iWraps; i++)
		{
#ifdef ALTERNATE_FULL_SLIDER
			SetCursorPos({ vOriginalPos.x + H::Draw.Scale(6), vOriginalPos.y + H::Draw.Scale((bFull ? 5 : 3) + 18 * i) });
#else
			SetCursorPos({ vOriginalPos.x + H::Draw.Scale(6), vOriginalPos.y + H::Draw.Scale(3 + 18 * i) });
#endif
			TextUnformatted(vWrapped[i].c_str());
		}

#ifdef ALTERNATE_FULL_SLIDER
		float flTextY = vOriginalPos.y + H::Draw.Scale(bFull ? -4 + 9 * iWraps : -15 + 18 * iWraps);
#else
		float flTextY = vOriginalPos.y + H::Draw.Scale(-15 + 18 * iWraps);
#endif
		{
			auto uHash2 = FNV1A::Hash32Const(std::format("{}## Text", sLabel).c_str());

			static std::string sText, sInput;
			if (!mActiveMap[uHash2])
				sText = pVar2 ? FormatText(fmt, flSVar1, flSVar2) : FormatText(fmt, flSVar1);
			else
			{
				SetCursorPos({ -1000, flTextY }); // lol
				SetKeyboardFocusHere();
				InputText("##SliderText", &sInput, ImGuiInputTextFlags_CharsDecimal); sText = sInput;

				bool bEnter = U::KeyHandler.Pressed(VK_RETURN);
				if (bEnter)
				{
					try // prevent the user from being a retard with invalid inputs
					{
						bool bVar1 = mActiveMap[uHash2] == 1;
						float& pVar = bVar1 ? flSVar1 : flSVar2;

						pVar = sText.length() ? std::stof(sText) : 0.f;
						if (pVar2)
							pVar = std::min(pVar, bVar1 ? *pVar2 - flStep : *pVar1 + flStep);
						if (!(iFlags & FSlider_Precision))
							pVar = pVar - fnmodf(pVar - flStep / 2, flStep) + flStep / 2;
						if (iFlags & FSlider_Clamp)
							pVar = std::clamp(pVar, flMin, flMax);
						else if (iFlags & FSlider_Min)
							pVar = std::max(pVar, flMin);
						else if (iFlags & FSlider_Max)
							pVar = std::min(pVar, flMax);

						*pVar1 = flSVar1;
						if (pVar2)
							*pVar2 = flSVar2;
					}
					catch (...) {}
				}
				if (bEnter || IsMouseClicked(ImGuiMouseButton_Left) || U::KeyHandler.Pressed(VK_ESCAPE))
					mActiveMap[uHash2] = false;
			}
			float flWidth = FCalcTextSize(sText.c_str()).x;
#ifdef ALTERNATE_FULL_SLIDER
			if (bFull)
				SetCursorPos({ vOriginalPos.x + vSize.x - H::Draw.Scale(36), flTextY });
			else
				SetCursorPos({ vOriginalPos.x + vSize.x - flWidth - H::Draw.Scale(6), flTextY });
#else
				SetCursorPos({ vOriginalPos.x + vSize.x - flWidth - H::Draw.Scale(6), flTextY });
#endif

			ImVec2 vOriginalPos2 = GetCursorPos();
			flEntryWidth = FCalcTextSize(sText.c_str()).x;
			TextUnformatted(sText.c_str());
			if (!Disabled)
			{
				if (!Disabled && IsItemHovered() && IsWindowHovered())
					SetMouseCursor(ImGuiMouseCursor_TextInput);
				if (mActiveMap[uHash2])
					pDrawList->AddRectFilled({ vDrawPos.x + vOriginalPos2.x, vDrawPos.y + vOriginalPos2.y + H::Draw.Scale(14) }, { vDrawPos.x + vOriginalPos2.x + flWidth, vDrawPos.y + vOriginalPos2.y + H::Draw.Scale(15) }, F::Render.Active);
				else if (IsItemClicked())
				{
					float* pVar = !pVar2 || GetMousePos().x - vDrawPos.x - vOriginalPos2.x < flWidth / 2 ? pVar1 : pVar2;
					sInput = std::format("{}", *pVar); // would use to_string but i don't like its formatting
					mActiveMap[uHash2] = pVar == pVar1 ? 1 : 2;
				}
			}
		}

		vDrawPos += vOriginalPos;
#ifdef ALTERNATE_FULL_SLIDER
		ImVec2 vMins = { vSize.x / 2 - H::Draw.Scale(10), vSize.y / 2 - H::Draw.Scale(1) }, vMaxs = { vSize.x - H::Draw.Scale(50), vSize.y / 2 + H::Draw.Scale(1) };
		if (!bFull)
			vMins = { H::Draw.Scale(6), vSize.y - H::Draw.Scale(8) }, vMaxs = { vSize.x - H::Draw.Scale(6), vSize.y - H::Draw.Scale(6) };
#else
		ImVec2 vMins = { H::Draw.Scale(6), vSize.y - H::Draw.Scale(8) }, vMaxs = { vSize.x - H::Draw.Scale(6), vSize.y - H::Draw.Scale(6) };
#endif
		ImColor vAccent = F::Render.Accent, vMuted = vAccent, vWashed = vAccent, vTransparent = vAccent;
		{
			float flA = GetStyle().Alpha;
			vAccent.Value.w *= flA, vMuted.Value.w *= 0.8f * flA, vWashed.Value.w *= 0.4f * flA, vTransparent.Value.w *= 0.2f * flA;
		}

		bool bWithin = IsWindowHovered() && IsMouseWithin(vDrawPos.x + vMins.x - H::Draw.Scale(6), vDrawPos.y + vMins.y - H::Draw.Scale(6), (vMaxs.x - vMins.x) + H::Draw.Scale(12), (vMaxs.y - vMins.y) + H::Draw.Scale(12));
		if (!Disabled && bWithin)
			SetMouseCursor(ImGuiMouseCursor_Hand);
		ImVec2 vMouse = GetMousePos();
		float flMousePerc = (vMouse.x - (vDrawPos.x + vMins.x)) / ((vDrawPos.x + vMaxs.x) - (vDrawPos.x + vMins.x)) + (flStep / 2) / (flMax - flMin);
		if (pVar2)
		{
			float flLowerPerc = std::clamp((flSVar1 - flMin) / (flMax - flMin), 0.f, 1.f), flUpperPerc = std::clamp((flSVar2 - flMin) / (flMax - flMin), 0.f, 1.f);
			float flLowerPos = vMins.x + (vMaxs.x - vMins.x) * flLowerPerc, flUpperPos = vMins.x + (vMaxs.x - vMins.x) * flUpperPerc;

			AddSteppedRect(vDrawPos, vMins, vMaxs, vMins, { flLowerPos, vMaxs.y }, flMin, flMax, flStep, vWashed, vMuted, H::Draw.Scale(2));
			AddSteppedRect(vDrawPos, vMins, vMaxs, { flLowerPos, vMins.y }, { flUpperPos, vMaxs.y }, flMin, flMax, flStep, vAccent, vWashed, H::Draw.Scale(2));
			AddSteppedRect(vDrawPos, vMins, vMaxs, { flUpperPos, vMins.y }, vMaxs, flMin, flMax, flStep, vWashed, vMuted, H::Draw.Scale(2));
			pDrawList->AddCircleFilled({ vDrawPos.x + flLowerPos, vDrawPos.y + vMins.y + H::Draw.Scale(1) }, H::Draw.Scale(3), vAccent);
			pDrawList->AddCircleFilled({ vDrawPos.x + flUpperPos, vDrawPos.y + vMins.y + H::Draw.Scale(1) }, H::Draw.Scale(3), vAccent);

			if (!Disabled)
			{
				if (bWithin && !mActiveMap[uHash])
				{
					int iVar = fabsf(vMouse.x - (vDrawPos.x + flLowerPos)) < fabsf(vMouse.x - (vDrawPos.x + flUpperPos)) ? 1 : 2;
					if (IsMouseClicked(ImGuiMouseButton_Left))
						mActiveMap[uHash] = iVar;
					pDrawList->AddCircleFilled({ vDrawPos.x + (iVar == 1 ? flLowerPos : flUpperPos), vDrawPos.y + vMins.y + H::Draw.Scale(1) }, H::Draw.Scale(11), vTransparent);
				}
				else if (mActiveMap[uHash] && IsMouseDown(ImGuiMouseButton_Left))
				{
					bool bVar1 = mActiveMap[uHash] == 1;
					float& flVar = bVar1 ? flSVar1 : flSVar2;
					flVar = flMin + (flMax - flMin) * flMousePerc;
					flVar = std::clamp(flVar - fnmodf(flVar, flStep), !bVar1 ? flSVar1 + flStep : flMin, bVar1 ? flSVar2 - flStep : flMax);
					pDrawList->AddCircleFilled({ vDrawPos.x + (bVar1 ? flLowerPos : flUpperPos), vDrawPos.y + vMins.y + H::Draw.Scale(1) }, H::Draw.Scale(11), vWashed);
				}
				else
					mActiveMap[uHash] = false;

				if (iFlags & FSlider_NoAutoUpdate ? !mActiveMap[uHash] : true)
				{
					*pVar1 = flSVar1;
					*pVar2 = flSVar2;
				}
			}
		}
		else
		{
			float flPercent = std::clamp((flSVar1 - flMin) / (flMax - flMin), 0.f, 1.f);
			float flPos = vMins.x + (vMaxs.x - vMins.x) * flPercent;

			AddSteppedRect(vDrawPos, vMins, vMaxs, vMins, { flPos, vMaxs.y }, flMin, flMax, flStep, vAccent, vWashed, H::Draw.Scale(2));
			AddSteppedRect(vDrawPos, vMins, vMaxs, { flPos, vMins.y }, vMaxs, flMin, flMax, flStep, vWashed, vMuted, H::Draw.Scale(2));
			pDrawList->AddCircleFilled({ vDrawPos.x + flPos, vDrawPos.y + vMins.y + H::Draw.Scale(1) }, H::Draw.Scale(3), vAccent);

			if (!Disabled)
			{
				if (bWithin && !mActiveMap[uHash])
				{
					if (IsMouseClicked(ImGuiMouseButton_Left))
						mActiveMap[uHash] = 1;
					pDrawList->AddCircleFilled({ vDrawPos.x + flPos, vDrawPos.y + vMins.y + H::Draw.Scale(1) }, H::Draw.Scale(11), vTransparent);
				}
				else if (mActiveMap[uHash] && IsMouseDown(ImGuiMouseButton_Left))
				{
					flSVar1 = flMin + (flMax - flMin) * flMousePerc;
					flSVar1 = std::clamp(flSVar1 - fnmodf(flSVar1, flStep), flMin, flMax);
					pDrawList->AddCircleFilled({ vDrawPos.x + flPos, vDrawPos.y + vMins.y + H::Draw.Scale(1) }, H::Draw.Scale(11), vWashed);
				}
				else
					mActiveMap[uHash] = false;

				if (iFlags & FSlider_NoAutoUpdate ? !mActiveMap[uHash] : true)
					*pVar1 = flSVar1;
			}
		}

		PopStyleColor();
		SetCursorPos({ vOriginalPos.x + vMins.x - 6, vOriginalPos.y + vMins.y - 6 });
		Button("##", { vMaxs.x - vMins.x + 12, 14 }); // don't drag it around
		SetCursorPos(vOriginalPos);
		AddRowSize(vOriginalPos.y, vSize.y);
		DebugDummy({ vSize.x, GetRowSize(vSize.y) });

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y, vSize.x, vSize.y);
		}

		return *pVar1 != flOriginal1 || pVar2 && *pVar2 != flOriginal2;
	}

	inline bool FSlider(const char* sLabel, int* pVar1, int* pVar2, int iMin, int iMax, int iStep = 1, const char* fmt = "%d", int iFlags = FSlider_None, bool* pHovered = nullptr)
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

	inline bool FSlider(const char* sLabel, float* pVar, float flMin, float flMax, float flStep = 1.f, const char* fmt = "%.0f", int iFlags = FSlider_None, bool* pHovered = nullptr)
	{
		return FSlider(sLabel, pVar, nullptr, flMin, flMax, flStep, fmt, iFlags, pHovered);
	}

	inline bool FSlider(const char* sLabel, int* pVar, int iMin, int iMax, int iStep = 1, const char* fmt = "%d", int iFlags = FSlider_None, bool* pHovered = nullptr)
	{
		return FSlider(sLabel, pVar, nullptr, iMin, iMax, iStep, fmt, iFlags, pHovered);
	}

	inline bool FDropdown(const char* sLabel, int* pVar, std::vector<const char*> vEntries, std::vector<int> vValues = {}, int iFlags = FDropdown_None, int iSizeOffset = 0, bool* pHovered = nullptr, int* pModified = nullptr)
	{
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool bTitle = sLabel[0] != '#';

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
		if (!(iFlags & FDropdown_Multi) || *pVar)
		{
			size_t i = 0; for (auto& iValue : vValues)
			{
				while (FNV1A::Hash32(vEntries[i]) == FNV1A::Hash32Const("##Divider"))
					i++;

				if (iFlags & FDropdown_Multi && *pVar & iValue)
					sPreview += std::format("{}, ", StripDoubleHash(vEntries[i]).c_str());
				else if (!(iFlags & FDropdown_Multi) && *pVar == iValue)
					sPreview = std::format("{}##", StripDoubleHash(vEntries[i]).c_str());
				i++;
			}
			if (sPreview.length() > 1)
			{
				sPreview.pop_back(); sPreview.pop_back();
			}
		}
		if (sPreview.empty())
			sPreview = "None";

		ImVec2 vSize = { GetWindowWidth(), H::Draw.Scale(bTitle ? 40 : 24) };
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			vSize.x = vSize.x / 2 - GetStyle().WindowPadding.x * 1.5f;
		else
			vSize.x -= GetStyle().WindowPadding.x * 2.f;
		if (iFlags & FDropdown_Right)
			SameLine(vSize.x + GetStyle().WindowPadding.x * 2.f);
		iSizeOffset = strstr(sLabel, "## Bind") ? 0 : H::Draw.Scale(iSizeOffset, Scale_Round);
		vSize.x += iSizeOffset;

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, GetStyle().WindowPadding.y });

		if (Disabled)
		{	// lol
			Button("##", vSize);
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, GetStyle().WindowPadding.y });
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, H::Draw.Scale(bTitle ? 13.5f : 5.5f) });
		PushItemWidth(vSize.x);

		bool bActive = BeginCombo(std::format("##{}", sLabel).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge);
		if (bActive)
		{
			DebugDummy({ 0, H::Draw.Scale(8) });

			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, H::Draw.Scale(19) });
			int i = 0; for (auto& sEntry : vEntries)
			{
				if (FNV1A::Hash32(sEntry) == FNV1A::Hash32Const("##Divider"))
				{
					ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
					ImColor tInactive = F::Render.Inactive; tInactive.Value.w *= GetStyle().Alpha;
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x + H::Draw.Scale(18), vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowWidth() - H::Draw.Scale(18), vDrawPos.y + flPosY + H::Draw.Scale(1) }, tInactive);
					DebugDummy({});
					continue;
				}

				std::string sStripped = StripDoubleHash(sEntry);
				if (iFlags & FDropdown_Multi)
				{
					bool bFlagActive = *pVar & vValues[i];

					ImVec2 vOriginalPos2 = GetCursorPos();
					if (FSelectable(std::format("##{}{}", sEntry, i).c_str(), nullptr, 0, bFlagActive, ImGuiSelectableFlags_DontClosePopups))
					{
						if (bFlagActive)
							*pVar &= ~vValues[i];
						else
							*pVar |= vValues[i];
						bReturn = true;
					}

					ImVec2 vOriginalPos3 = GetCursorPos();
					SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(40), vOriginalPos2.y + H::Draw.Scale(1) });
					PushStyleColor(ImGuiCol_Text, bFlagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(sStripped.c_str());
					PopStyleColor();

					SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(16), vOriginalPos2.y - H::Draw.Scale(1) });
					IconImage(bFlagActive ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, bFlagActive ? F::Render.Accent.Value : F::Render.Inactive.Value);
					SetCursorPos(vOriginalPos3);
				}
				else
				{
					ImVec2 vOriginalPos2 = GetCursorPos();
					if (iFlags & FDropdown_Modifiable)
					{
						SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(16), vOriginalPos2.y - H::Draw.Scale(1) });
						if (IconButton(vValues[i] == -1 ? ICON_MD_ADD_CIRCLE : ICON_MD_REMOVE_CIRCLE, {}) && pModified)
							*pModified = vValues[i];
						SetCursorPos(vOriginalPos2);
					}

					if (FSelectable(std::format("##{}{}", sEntry, i).c_str(), nullptr, 0, *pVar == vValues[i]))
						*pVar = vValues[i], bReturn = true;

					ImVec2 vOriginalPos3 = GetCursorPos();
					SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(iFlags & FDropdown_Modifiable ? 40 : 20), vOriginalPos2.y + H::Draw.Scale(1) });
					PushStyleColor(ImGuiCol_Text, *pVar == vValues[i] ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(sStripped.c_str());
					PopStyleColor();

					if (iFlags & FDropdown_Modifiable) // do second image here so as to not cover
					{
						SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(16), vOriginalPos2.y - H::Draw.Scale(1) });
						IconImage(vValues[i] == -1 ? ICON_MD_ADD_CIRCLE : ICON_MD_REMOVE_CIRCLE);
					}
					SetCursorPos(vOriginalPos3);
				}
				i++;
			}
			PopStyleVar();

			SetCursorPosY(GetCursorPosY() - H::Draw.Scale(10)); DebugDummy({});

			EndCombo();
		}
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			ImVec2 vOriginalPos2 = GetCursorPos();

			if (bTitle)
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y - H::Draw.Scale(6) });
				PushFont(F::Render.FontSmall);
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				TextUnformatted(TruncateText(StripDoubleHash(sLabel), vSize.x - H::Draw.Scale(45)).c_str());
				PopStyleColor();
				PopFont();

				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y + H::Draw.Scale(8) });
				TextUnformatted(TruncateText(sPreview, vSize.x - H::Draw.Scale(45)).c_str());

				SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
				IconImage(bActive ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
			}
			else
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y });
				TextUnformatted(TruncateText(sPreview, vSize.x - H::Draw.Scale(45)).c_str());

				SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
				IconImage(bActive ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
			}

			EndComboPreview();
		}

		PopItemWidth();
		PopStyleVar();

		SetCursorPos(vOriginalPos);
		AddRowSize(vOriginalPos.y, vSize.y + GetStyle().WindowPadding.y);
		DebugDummy({ vSize.x, GetRowSize(vSize.y + GetStyle().WindowPadding.y) });

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + GetStyle().WindowPadding.y, vSize.x, vSize.y);
		}

		return bReturn;
	}

	inline bool FSDropdown(const char* sLabel, std::string* pVar, std::vector<const char*> vEntries = {}, int iFlags = FDropdown_None, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		auto uHash = FNV1A::Hash32Const(sLabel);
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool bTitle = sLabel[0] != '#';

		ImVec2 vSize = { GetWindowWidth(), H::Draw.Scale(bTitle ? 40 : 24) };
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			vSize.x = vSize.x / 2 - GetStyle().WindowPadding.x * 1.5f;
		else
			vSize.x -= GetStyle().WindowPadding.x * 2.f;
		if (iFlags & FDropdown_Right)
			SameLine(vSize.x + GetStyle().WindowPadding.x * 2.f);
		iSizeOffset = strstr(sLabel, "## Bind") ? 0 : H::Draw.Scale(iSizeOffset, Scale_Round);
		vSize.x += iSizeOffset;

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, GetStyle().WindowPadding.y });

		if (Disabled)
		{	// lol
			Button("##", vSize);
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, GetStyle().WindowPadding.y });
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, H::Draw.Scale(bTitle ? 13.5f : 5.5f) });
		if (vEntries.empty())
		{
			PushStyleColor(ImGuiCol_PopupBg, {});
			PushStyleVar(ImGuiStyleVar_WindowPadding, { GetStyle().WindowPadding.x, 0 });
		}
		PushItemWidth(vSize.x);

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

				DebugDummy({ 0, H::Draw.Scale(8) });
				PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, H::Draw.Scale(19) });

				bool bDivider = false;
				for (int i = 0; i < vValid.size(); i++)
				{
					auto& sEntry = vValid[i];
					if (FNV1A::Hash32(sEntry.c_str()) == FNV1A::Hash32Const("##Divider"))
					{
						if (!bDivider)
						{
							ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
							ImColor vInactive = F::Render.Inactive; vInactive.Value.w *= GetStyle().Alpha;
							GetWindowDrawList()->AddRectFilled({ vDrawPos.x + H::Draw.Scale(18), vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowWidth() - H::Draw.Scale(18), vDrawPos.y + flPosY + H::Draw.Scale(1) }, vInactive);
							DebugDummy({});
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
					ImVec2 vOriginalPos3 = GetCursorPos();
					if (FSelectable(std::format("##{}{}", sEntry, i).c_str(), nullptr, 0, bActive))
						*pVar = sEntry, bReturn = true;

					ImVec2 vOriginalPos4 = GetCursorPos();
					SetCursorPos({ vOriginalPos3.x + H::Draw.Scale(20), vOriginalPos3.y + H::Draw.Scale(1) });
					PushStyleColor(ImGuiCol_Text, bActive ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(sEntry.c_str());
					PopStyleColor();
					SetCursorPos(vOriginalPos4);
				}

				PopStyleVar();
				SetCursorPosY(GetCursorPosY() - H::Draw.Scale(10)); DebugDummy({});
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

			if (bTitle)
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y - H::Draw.Scale(5) });
				PushFont(F::Render.FontSmall);
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				TextUnformatted(TruncateText(StripDoubleHash(sLabel), vSize.x - H::Draw.Scale(vEntries.empty() ? 24 : 45)).c_str());
				PopStyleColor();
				PopFont();

				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y + H::Draw.Scale(8) });
				TextUnformatted(TruncateText(mActiveMap[uHash] ? sPreview : *pVar, vSize.x - H::Draw.Scale(vEntries.empty() ? 24 : 45)).c_str());

				if (!vEntries.empty())
				{
					SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
					IconImage(mActiveMap[uHash] ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
				}

				if (mActiveMap[uHash] || iFlags & FSDropdown_Custom || vEntries.empty())
				{
					ImVec2 vDrawPos = GetDrawPos() + vOriginalPos2 + ImVec2(H::Draw.Scale(12), H::Draw.Scale(22));
					vDrawPos.x = floorf(vDrawPos.x), vDrawPos.y = floorf(vDrawPos.y);
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x, vDrawPos.y }, { vDrawPos.x + vSize.x - H::Draw.Scale(vEntries.empty() ? 25 : 45), vDrawPos.y + H::Draw.Scale(2) }, mActiveMap[uHash] ? F::Render.Active : F::Render.Inactive);
				}
			}
			else
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y });
				TextUnformatted(TruncateText(mActiveMap[uHash] ? sPreview : *pVar, vSize.x - H::Draw.Scale(vEntries.empty() ? 24 : 45)).c_str());

				if (!vEntries.empty())
				{
					SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
					IconImage(mActiveMap[uHash] ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
				}

				if (mActiveMap[uHash] || iFlags & FSDropdown_Custom || vEntries.empty())
				{
					ImVec2 vDrawPos = GetDrawPos() + vOriginalPos2 + ImVec2(H::Draw.Scale(12), H::Draw.Scale(14));
					vDrawPos.x = floorf(vDrawPos.x), vDrawPos.y = floorf(vDrawPos.y);
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x, vDrawPos.y }, { vDrawPos.x + vSize.x - H::Draw.Scale(vEntries.empty() ? 25 : 45), vDrawPos.y + H::Draw.Scale(2) }, mActiveMap[uHash] ? F::Render.Active : F::Render.Inactive);
				}
			}

			EndComboPreview();
		}

		PopItemWidth();
		if (vEntries.empty())
		{
			PopStyleColor();
			PopStyleVar();
		}
		PopStyleVar();

		SetCursorPos(vOriginalPos);
		AddRowSize(vOriginalPos.y, vSize.y + GetStyle().WindowPadding.y);
		DebugDummy({ vSize.x, GetRowSize(vSize.y + GetStyle().WindowPadding.y) });

		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + GetStyle().WindowPadding.y, vSize.x, vSize.y);
		}

		return bReturn;
	}

	/*
	// in it's current state it sees no use, so i'm commenting it out for now
	inline bool FVDropdown(const char* sLabel, std::vector<std::string>* pVar, std::vector<std::string> vEntries, int iFlags = FDropdown_None, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool bTitle = sLabel[0] != '#';

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

		ImVec2 vSize = { GetWindowWidth(), H::Draw.Scale(bTitle ? 40 : 24) };
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			vSize.x = vSize.x / 2 - GetStyle().WindowPadding.x * 1.5f;
		else
			vSize.x -= GetStyle().WindowPadding.x * 2.f;
		if (iFlags & FDropdown_Right)
			SameLine(vSize.x + GetStyle().WindowPadding.x * 2.f);
		iSizeOffset = strstr(sLabel, "## Bind") ? 0 : H::Draw.Scale(iSizeOffset, Scale_Round);
		vSize.x += iSizeOffset;

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, GetStyle().WindowPadding.y });

		if (Disabled)
		{	// lol
			Button("##", vSize);
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, GetStyle().WindowPadding.y });
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, H::Draw.Scale(bTitle ? 13.5f : 5.5f) });
		PushItemWidth(vSize.x);

		bool bActive = false;
		if (BeginCombo(std::format("##{}", sLabel).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			bActive = true;

			DebugDummy({ 0, H::Draw.Scale(8) });
			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, H::Draw.Scale(19) });
			for (int i = 0; i < vEntries.size(); i++)
			{
				auto& sEntry = vEntries[i];
				if (FNV1A::Hash32(sEntry.c_str()) == FNV1A::Hash32Const("##Divider"))
				{
					ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
					ImColor tInactive = F::Render.Inactive; tInactive.Value.w *= GetStyle().Alpha;
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x + H::Draw.Scale(18), vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowWidth() - H::Draw.Scale(18), vDrawPos.y + flPosY + H::Draw.Scale(1) }, tInactive);
					DebugDummy({});
					continue;
				}

				auto cFind = mIts.find(sEntry);
				bool bFlagActive = cFind != mIts.end();
				int iEntry = bFlagActive ? std::distance(pVar->begin(), mIts[sEntry]) + 1 : 0;

				ImVec2 vOriginalPos2 = GetCursorPos();
				if (FSelectable(std::format("##{}{}", sEntry, i).c_str(), nullptr, 0, bFlagActive, ImGuiSelectableFlags_DontClosePopups))
				{
					if (bFlagActive)
						pVar->erase(cFind->second);
					else
						pVar->push_back(sEntry);
					bReturn = true;
				}

				// shift based on number of digits in var size
				ImVec2 vOriginalPos3 = GetCursorPos();
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(40 + 6 * int(log10(std::max(pVar->size(), 1ui64)))), vOriginalPos2.y + H::Draw.Scale(1) });
				PushStyleColor(ImGuiCol_Text, bFlagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
				TextUnformatted(sEntry.c_str());
				PopStyleColor();

				if (bFlagActive)
				{
					SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(18), vOriginalPos2.y + H::Draw.Scale(1) });
					PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
					Text("%i", iEntry);
					PopStyleColor();
				}
				SetCursorPos(vOriginalPos3);
			}
			PopStyleVar();
			SetCursorPosY(GetCursorPosY() - H::Draw.Scale(10)); DebugDummy({});

			EndCombo();
		}
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			ImVec2 vOriginalPos2 = GetCursorPos();

			if (bTitle)
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y - H::Draw.Scale(5) });
				PushFont(F::Render.FontSmall);
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				TextUnformatted(TruncateText(StripDoubleHash(sLabel), vSize.x - H::Draw.Scale(45)).c_str());
				PopStyleColor();
				PopFont();

				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y + H::Draw.Scale(8) });
				TextUnformatted(TruncateText(sPreview, vSize.x - H::Draw.Scale(45)).c_str());

				SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
				IconImage(bActive ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
			}
			else
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y });
				TextUnformatted(TruncateText(sPreview, vSize.x - H::Draw.Scale(45)).c_str());

				SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
				IconImage(bActive ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
			}

			EndComboPreview();
		}

		PopItemWidth();
		PopStyleVar();

		SetCursorPos(vOriginalPos);
		AddRowSize(vOriginalPos.y, vSize.y + GetStyle().WindowPadding.y);
		DebugDummy({ vSize.x, GetRowSize(vSize.y + GetStyle().WindowPadding.y) });


		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + GetStyle().WindowPadding.y, vSize.x, vSize.y);
		}

		return bReturn;
	}
	*/

	inline bool ColorPicker(const char* sLabel, Color_t* tColor, bool bTooltip = true, int iFlags = FColorPicker_None, ImVec2 vSize = { H::Draw.Scale(12), H::Draw.Scale(12) })
	{
		ImVec2 vOriginalPos = GetCursorPos();
		if (Disabled)
		{	// lol
			Button("##", vSize);
			SetCursorPos(vOriginalPos);
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { H::Draw.Scale(2), H::Draw.Scale(2) });
		PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, H::Draw.Scale(4) });
		PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { H::Draw.Scale(4), 0 });
		PushStyleColor(ImGuiCol_PopupBg, F::Render.Background0p5.Value);
		ImVec4 tempColor = ColorToVec(*tColor);
		bool bReturn = ColorEdit4(std::format("##{}", sLabel).c_str(), &tempColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_LargeAlphaGrid, vSize);
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
	inline bool FColorPicker(const char* sLabel, Color_t* tColor, int iOffset = 0, int iFlags = FColorPicker_None, bool* pHovered = nullptr)
	{
		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool bReturn = false;

		ImVec2 vOriginalPos;
		ImVec2 vSize = { H::Draw.Scale(12), H::Draw.Scale(12) };
		if (!(iFlags & FColorPicker_Dropdown))
		{
			if (iFlags & (FColorPicker_Left | FColorPicker_Middle))
			{
				vSize.x = GetWindowWidth() / 2 - GetStyle().WindowPadding.x * 1.5f;
				if (iFlags & FColorPicker_Middle)
					SameLine(vSize.x + GetStyle().WindowPadding.x * 2.f);
				SetCursorPosX(GetCursorPosX() - H::Draw.Scale(iOffset * 12));

				vOriginalPos = GetCursorPos();

				auto vWrapped = WrapText(StripDoubleHash(sLabel), vSize.x - H::Draw.Scale(24));
				int iWraps = std::min(int(vWrapped.size()), 2); // prevent too many wraps
				vSize.y = H::Draw.Scale(6 + 18 * iWraps);

				SetCursorPos({ vOriginalPos.x + H::Draw.Scale(6), vOriginalPos.y + H::Draw.Scale(-3 + 9 * iWraps) });
				bReturn = ColorPicker(sLabel, tColor, iFlags & FColorPicker_Tooltip);

				for (size_t i = 0; i < iWraps; i++)
				{
					SetCursorPos({ vOriginalPos.x + H::Draw.Scale(24), vOriginalPos.y + H::Draw.Scale(5 + 18 * i) });
					TextUnformatted(vWrapped[i].c_str());
				}

				SetCursorPos(vOriginalPos);
				AddRowSize(vOriginalPos.y, vSize.y);
				DebugDummy({ vSize.x, GetRowSize(vSize.y) });
			}
			else
			{
				if (iFlags & FColorPicker_SameLine)
					SameLine();
				SetCursorPosX(GetContentRegionMax().x - H::Draw.Scale(18 + iOffset * 12));

				vOriginalPos = GetCursorPos();

				SetCursorPosY(vOriginalPos.y + H::Draw.Scale(6));
				bReturn = ColorPicker(sLabel, tColor, true);

				SetCursorPos(vOriginalPos);
				DebugDummy({});
				vOriginalPos.y += H::Draw.Scale(6);
			}
		}
		else
		{
			float flOriginalPosY = GetCursorPosY();
			SameLine();
			vSize = { H::Draw.Scale(10), flOriginalPosY - GetCursorPosY() - GetStyle().WindowPadding.y };

			SetCursorPosX(GetCursorPosX() - GetStyle().WindowPadding.y);
			vOriginalPos = GetCursorPos();
			SetCursorPosY(GetCursorPosY() + GetStyle().WindowPadding.y);

			bReturn = ColorPicker(sLabel, tColor, iFlags & FColorPicker_Tooltip, iFlags, vSize);
			SetCursorPos(vOriginalPos);
			AddRowSize(vOriginalPos.y, vSize.y + GetStyle().WindowPadding.y);
			DebugDummy({ vSize.x, GetRowSize(vSize.y + GetStyle().WindowPadding.y) });

			vOriginalPos.y += GetStyle().WindowPadding.y;
			vSize = { H::Draw.Scale(10), H::Draw.Scale(40) };
		}
		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y, vSize.x, vSize.y);
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
		case VK_VOLUME_MUTE: return "mute";
		case VK_VOLUME_DOWN: return "volume down";
		case VK_VOLUME_UP: return "volume up";
		case VK_MEDIA_STOP: return "stop";
		case VK_MEDIA_PLAY_PAUSE: return "play/pause";
		case VK_MEDIA_PREV_TRACK: return "previous track";
		case VK_MEDIA_NEXT_TRACK: return "next track";
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
	inline void FKeybind(const char* sLabel, int& iOutput, int iFlags = FKeybind_None, ImVec2 vSize = { 0, 30 }, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		ImGuiID uId = GetID(sLabel);
		PushID(sLabel);

		if (GetActiveID() == uId)
		{
			F::Menu.m_bInKeybind = true;

			bool bHovered = false;
			FButton(std::format("{}: ...", sLabel).c_str(), iFlags | FButton_NoUpper, vSize, iSizeOffset, &bHovered);
			if (pHovered)
				*pHovered = bHovered;

			if (bHovered && IsMouseReleased(ImGuiMouseButton_Left))
				ClearActiveID();
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

				if (iKeyPressed && (iKeyPressed != VK_LBUTTON || !bHovered))
				{
					switch (iKeyPressed)
					{
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

				GetCurrentContext()->ActiveIdAllowOverlap = true;
			}
		}
		else if (FButton(std::format("{}: {}", sLabel, VK2STR(iOutput)).c_str(), iFlags | FButton_NoUpper, vSize, iSizeOffset, pHovered))
			SetActiveID(uId, GetCurrentWindow());

		PopID();
	}

	// dropdown for materials
	inline bool FMDropdown(const char* sLabel, std::vector<std::pair<std::string, Color_t>>* pVar, int iFlags = FDropdown_None, int iSizeOffset = 0, bool* pHovered = nullptr)
	{
		// material stuff
		std::vector<Material_t> vMaterials;
		for (auto& [_, mat] : F::Materials.m_mMaterials)
		{
			if (FNV1A::Hash32(mat.m_sName.c_str()) != FNV1A::Hash32Const("None"))
				vMaterials.push_back(mat);
		}

		std::sort(vMaterials.begin(), vMaterials.end(), [&](const auto& a, const auto& b) -> bool
			{
				// keep locked materials higher
				if (a.m_bLocked && !b.m_bLocked)
					return true;
				if (!a.m_bLocked && b.m_bLocked)
					return false;

				return a.m_sName < b.m_sName;
			});

		std::vector<std::string> vEntries = { "Original" };
		for (auto& pair : vMaterials)
			vEntries.push_back(pair.m_sName.c_str());

		// actual dropdown
		bool bReturn = false;

		if (Transparent || Disabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool bTitle = sLabel[0] != '#';

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

		ImVec2 vSize = { GetWindowWidth(), H::Draw.Scale(bTitle ? 40 : 24) };
		if (iFlags & (FDropdown_Left | FDropdown_Right))
			vSize.x = vSize.x / 2 - GetStyle().WindowPadding.x * 1.5f;
		else
			vSize.x -= GetStyle().WindowPadding.x * 2.f;
		if (iFlags & FDropdown_Right)
			SameLine(vSize.x + GetStyle().WindowPadding.x * 2.f);
		iSizeOffset = strstr(sLabel, "## Bind") ? 0 : H::Draw.Scale(iSizeOffset, Scale_Round);
		vSize.x += iSizeOffset;

		ImVec2 vOriginalPos = GetCursorPos();
		DebugShift({ 0, GetStyle().WindowPadding.y });

		if (Disabled)
		{	// lol
			Button("##", vSize);
			SetCursorPos(vOriginalPos);
			DebugShift({ 0, GetStyle().WindowPadding.y });
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, H::Draw.Scale(bTitle ? 13.5f : 5.5f) });
		PushItemWidth(vSize.x);

		bool bActive = false;
		if (BeginCombo(std::format("##{}", sLabel).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			bActive = true;

			DebugDummy({ 0, 8 });
			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, H::Draw.Scale(19) });
			for (int i = 0; i < vEntries.size(); i++)
			{
				auto& sEntry = vEntries[i];
				if (FNV1A::Hash32(sEntry.c_str()) == FNV1A::Hash32Const("##Divider"))
				{
					ImVec2 vDrawPos = GetDrawPos(); float flPosY = GetCursorPosY();
					ImColor tInactive = F::Render.Inactive; tInactive.Value.w *= GetStyle().Alpha;
					GetWindowDrawList()->AddRectFilled({ vDrawPos.x + H::Draw.Scale(18), vDrawPos.y + flPosY }, { vDrawPos.x + GetWindowWidth() - H::Draw.Scale(18), vDrawPos.y + flPosY + H::Draw.Scale(1) }, tInactive);
					DebugDummy({});
					continue;
				}

				auto cFind = mIts.find(sEntry);
				bool bFlagActive = cFind != mIts.end();
				int iEntry = bFlagActive ? std::distance(pVar->begin(), mIts[sEntry]) + 1 : 0;

				ImVec2 vOriginalPos2 = GetCursorPos();
				if (bFlagActive) // do here so as to not sink input
				{
					SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(31), vOriginalPos2.y + H::Draw.Scale(1) });
					ColorPicker(std::format("MaterialColor{}", iEntry).c_str(), &cFind->second->second, false);
					SetCursorPos(vOriginalPos2);
				}
				bool bHovered = bFlagActive ? IsItemHovered() : false;

				if (FSelectable(std::format("##{}{}", sEntry, i).c_str(), nullptr, 0, bFlagActive, ImGuiSelectableFlags_DontClosePopups))
				{
					if (bFlagActive)
						pVar->erase(cFind->second);
					else
						pVar->emplace_back(sEntry, Color_t());
					bReturn = true;
				}
				bHovered = !bHovered && IsItemHovered();

				// shift based on number of digits in var size
				ImVec2 vOriginalPos3 = GetCursorPos();
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(40 + 6 * int(log10(std::max(pVar->size(), 1ui64)))), vOriginalPos2.y + H::Draw.Scale(1) });
				PushStyleColor(ImGuiCol_Text, bFlagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
				TextUnformatted(bHovered ? sEntry.c_str() : TruncateText(sEntry, vSize.x - (bFlagActive ? 82 : 60)).c_str());
				PopStyleColor();

				if (bFlagActive)
				{
					SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(18), vOriginalPos2.y + H::Draw.Scale(1) });
					PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
					Text("%i", iEntry);
					PopStyleColor();
				}
				SetCursorPos(vOriginalPos3);
			}
			PopStyleVar();
			SetCursorPosY(GetCursorPosY() - 10); DebugDummy({});

			EndCombo();
		}
		if (!Disabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			ImVec2 vOriginalPos2 = GetCursorPos();

			if (bTitle)
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y - H::Draw.Scale(5) });
				PushFont(F::Render.FontSmall);
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				TextUnformatted(TruncateText(StripDoubleHash(sLabel), vSize.x - H::Draw.Scale(45)).c_str());
				PopStyleColor();
				PopFont();

				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y + H::Draw.Scale(8) });
				TextUnformatted(TruncateText(sPreview, vSize.x - H::Draw.Scale(45)).c_str());

				SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
				IconImage(bActive ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
			}
			else
			{
				SetCursorPos({ vOriginalPos2.x + H::Draw.Scale(12), vOriginalPos2.y });
				TextUnformatted(TruncateText(sPreview, vSize.x - H::Draw.Scale(45)).c_str());

				SetCursorPos({ vOriginalPos2.x + vSize.x - H::Draw.Scale(24), vOriginalPos2.y - H::Draw.Scale(1) });
				IconImage(bActive ? ICON_MD_KEYBOARD_ARROW_UP : ICON_MD_KEYBOARD_ARROW_DOWN);
			}

			EndComboPreview();
		}

		PopItemWidth();
		PopStyleVar();

		SetCursorPos(vOriginalPos);
		AddRowSize(vOriginalPos.y, vSize.y + GetStyle().WindowPadding.y);
		DebugDummy({ vSize.x, GetRowSize(vSize.y + GetStyle().WindowPadding.y) });


		if (Transparent || Disabled)
			PopStyleVar();

		if (pHovered && IsWindowHovered())
		{
			vOriginalPos += GetDrawPos();
			*pHovered = IsMouseWithin(vOriginalPos.x, vOriginalPos.y + GetStyle().WindowPadding.y, vSize.x, vSize.y);
		}

		return bReturn;
	}

	// convar wrappers
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

	bool bPushedDisabled = false, bPushedTransparent = false;

	template <class T>
	inline T FGet(ConfigVar<T>& var, bool bDisable = false)
	{
		int iBind = GetBind(var);
		if (bDisable)
		{
			bPushedDisabled = false, bPushedTransparent = false;

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
							PushDisabled(true);
							bPushedDisabled = true;
							return tVal;
						}
					}
				}
			}
			else
			{
				PushTransparent(CurrentBind != iBind && !(var.m_iFlags & (NOSAVE | NOBIND)));
				bPushedTransparent = true;
			}
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

		if (bPushedDisabled)
			PopDisabled();
		if (bPushedTransparent)
			PopTransparent();
	}

	template <class T>
	inline void DrawBindInfo(ConfigVar<T>& var, T& val, std::string sBind, bool bNewPopup, bool& bLastHovered)
	{
		TextUnformatted(std::format("Bind '{}'", sBind).c_str());

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
			vStrings.push_back(std::format("{}## Bind{}", _iBind != DEFAULT_BIND && _iBind < F::Binds.m_vBinds.size() ? F::Binds.m_vBinds[_iBind].m_sName : sBind, _iBind));
		for (auto& sEntry : vStrings)
			vEntries.push_back(sEntry.c_str());

		vEntries.push_back("new bind");
		vValues.push_back(-1);

		int iModified = -2;
		if (FDropdown("Bind", &iBind, vEntries, vValues, FDropdown_Modifiable, -60, nullptr, &iModified))
		{
			if (iBind != DEFAULT_BIND && iBind < F::Binds.m_vBinds.size())
				tBind = F::Binds.m_vBinds[iBind];
			else
				tBind = { sBind };
			if (var.Map.contains(iBind))
				val = var.Map[iBind];
		}
		bool bClickedNew = iBind == DEFAULT_BIND && bLastHovered && IsMouseDown(ImGuiMouseButton_Left);
		if (iModified != -2 || bClickedNew)
		{
			if (iModified == -1 || bClickedNew)
			{
				iBind = int(F::Binds.m_vBinds.size());
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
		bool bHovered = false; bLastHovered = false;

		PushTransparent(iBind == DEFAULT_BIND);

		{
			ImVec2 vOriginalPos = GetCursorPos();

			SetCursorPos({ GetWindowWidth() - H::Draw.Scale(34), H::Draw.Scale(40) });
			if (IconButton(tBind.m_bVisible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF, { 1, 1, 1, -1 }, &bHovered))
				tBind.m_bVisible = !tBind.m_bVisible;
			bLastHovered = bLastHovered || bHovered;

			SetCursorPos({ GetWindowWidth() - H::Draw.Scale(59), H::Draw.Scale(40) });
			if (IconButton(!tBind.m_bNot ? ICON_MD_CODE : ICON_MD_CODE_OFF, { 1, 1, 1, -1 }, &bHovered))
				tBind.m_bNot = !tBind.m_bNot;
			bLastHovered = bLastHovered || bHovered;

			SetCursorPos(vOriginalPos);
		}

		FDropdown("Type", &tBind.m_iType, { "Key", "Class", "Weapon type", "Item slot" }, {}, FDropdown_Left, 0, &bHovered);
		bLastHovered = bLastHovered || bHovered;
		switch (tBind.m_iType)
		{
		case BindEnum::Key: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 2); FDropdown("Behavior", &tBind.m_iInfo, { "Hold", "Toggle", "Double click" }, {}, FDropdown_Right, 0, &bHovered); break;
		case BindEnum::Class: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 8); FDropdown("Class", &tBind.m_iInfo, { "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdown_Right, 0, &bHovered); break;
		case BindEnum::WeaponType: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 2); FDropdown("Weapon type", &tBind.m_iInfo, { "Hitscan", "Projectile", "Melee" }, {}, FDropdown_Right, 0, &bHovered); break;
		case BindEnum::ItemSlot: tBind.m_iInfo = std::max(tBind.m_iInfo, 0); FDropdown("Item slot", &tBind.m_iInfo, { "1", "2", "3", "4", "5", "6", "7", "8", "9" }, {}, FDropdown_Right, 0, &bHovered); break;
		}
		bLastHovered = bLastHovered || bHovered;

		if (tBind.m_iType == BindEnum::Key)
		{
			FKeybind("Key", tBind.m_iKey, FKeybind_None, { 0, 30 }, 0, &bHovered);
			bLastHovered = bLastHovered || bHovered;
		}

		if (!Disabled && iBind != DEFAULT_BIND && iBind < F::Binds.m_vBinds.size())
		{
			var.Map[iBind] = val;

			// don't completely override to retain misc info
			auto& _tBind = F::Binds.m_vBinds[iBind];
			_tBind.m_iType = tBind.m_iType;
			_tBind.m_iInfo = tBind.m_iInfo;
			_tBind.m_iKey = tBind.m_iKey;
			_tBind.m_bVisible = tBind.m_bVisible;
			_tBind.m_bNot = tBind.m_bNot;
		}

		DebugDummy({ 0, GetStyle().WindowPadding.y });
	}

	#define WRAPPER(function, type, parameters, arguments)\
	inline bool function(const char* sLabel, ConfigVar<type>& var, parameters, bool* pHovered = nullptr, const std::string& sBindOverride = "")\
	{\
		auto val = FGet(var, true);\
		bool bHovered = false;\
		const bool bReturn = function(sLabel, arguments, &bHovered);\
		FSet(var, val);\
		if (pHovered)\
			*pHovered = bHovered;\
		if (!(var.m_iFlags & (NOBIND | NOSAVE)) && !Disabled && CurrentBind == DEFAULT_BIND)\
		{	/*probably a better way to do this*/\
			static auto staticVal = val;\
			bool bNewPopup = bHovered && IsMouseReleased(ImGuiMouseButton_Right) && !IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId);\
			if (bNewPopup)\
			{\
				OpenPopup(std::format("{}::{}", var.m_sName.c_str(), sLabel).c_str());\
				staticVal = val;\
			}\
			PushStyleVar(ImGuiStyleVar_PopupBorderSize, H::Draw.Scale(1));\
			PushStyleColor(ImGuiCol_PopupBg, F::Render.Background0p5.Value);\
			SetNextWindowSize({ H::Draw.Scale(300), 0 });\
			bool bPopup = BeginPopup(std::format("{}::{}", var.m_sName.c_str(), sLabel).c_str());\
			PopStyleColor();\
			PopStyleVar();\
			if (bPopup)\
			{\
				std::string sLower = !sBindOverride.empty() ? sBindOverride : StripDoubleHash(sLabel);\
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
				static bool bLastHovered = false;\
				DrawBindInfo(var, staticVal, sLower, bNewPopup, bLastHovered);\
				val = staticVal;\
				function(std::format("{}## Bind", sLabel).c_str(), arguments, &bHovered);\
				bLastHovered = bLastHovered || bHovered;\
				staticVal = val;\
				PopTransparent(2);\
				EndPopup();\
			}\
		}\
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