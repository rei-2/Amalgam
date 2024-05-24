#pragma once
#include "../Render.h"
#include "Menu.h"
#include "../MaterialDesign/IconDefinitions.h"
#include "../../Conditions/Conditions.h"
#include "../../Visuals/Materials/Materials.h"
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui_stdlib.h>

const char* CurrentCondition = "";

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
	FToggle_Middle = 1 << 0,
	FToggle_PlainColor = 1 << 1
};

enum FSlider_
{
	FSlider_None = 0,
	FSlider_Left = 1 << 0,
	FSlider_Right = 1 << 1,
	FSlider_Clamp = 1 << 2, // will keep within bounds when using text input
	FSlider_Precision = 1 << 3, // allow more precise values outside of step when using text input
};

enum FDropdown_
{
	FDropdown_None = 0,
	FDropdown_Left = 1 << 0,
	FDropdown_Right = 1 << 1,
	FDropdown_Multi = 1 << 2
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
	FColorPicker_Dropdown = 1 << 3
};

namespace ImGui
{
	std::unordered_map<std::string, int> mActives;
	std::string sCondition = "default";
	bool bDisabled = false, bTransparent = false;

	__inline float fnmodf(float _X, float _Y)
	{
		// silly fix for negative values
		return fmodf(_X, _Y) + (_X < 0 ? _Y : 0);
	}

	__inline bool IsColorBright(Color_t color)
	{
		return color.r + color.g + color.b > 510;
	}

	__inline bool IsColorBright(ImColor color)
	{
		return color.Value.x + color.Value.y + color.Value.z > 2.f;
	}

	/* Color_t to ImVec4 */
	__inline ImVec4 ColorToVec(Color_t color)
	{
		return { float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f, float(color.a) / 255.f };
	}

	/* ImVec4 to Color_t */
	__inline Color_t VecToColor(ImVec4 color)
	{
		return {
			static_cast<byte>(color.x * 256.0f > 255 ? 255 : color.x * 256.0f),
			static_cast<byte>(color.y * 256.0f > 255 ? 255 : color.y * 256.0f),
			static_cast<byte>(color.z * 256.0f > 255 ? 255 : color.z * 256.0f),
			static_cast<byte>(color.w * 256.0f > 255 ? 255 : color.w * 256.0f)
		};
	}



	__inline void DebugDummy(ImVec2 size)
	{
		const auto restorePos = GetCursorPos();

		//PushStyleColor(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.5f });
		//Button("##", { std::max(size.x, 2.f), std::max(size.y, 2.f) });
		//PopStyleColor();

		SetCursorPos(restorePos); Dummy(size);
	}
	__inline void DebugShift(ImVec2 size)
	{
		const auto restorePos = GetCursorPos();

		//PushStyleColor(ImGuiCol_Button, { 1.f, 1.f, 1.f, 0.5f });
		//Button("##", { std::max(size.x, 2.f), std::max(size.y, 2.f) });
		//PopStyleColor();

		SetCursorPos({ restorePos.x + size.x, restorePos.y + size.y });
	}

	__inline std::string StripDoubleHash(const char* text)
	{
		std::string strBegin = text, strEnd = FindRenderedTextEnd(text);
		return strBegin.replace(strBegin.end() - strEnd.size(), strBegin.end(), "");
	}

	__inline std::string TruncateText(const char* text, int pixels)
	{
		std::string original = text;
		if (!original.size())
			return "";

		std::string truncated = ""; int i = 0;
		while (CalcTextSize(truncated.c_str()).x < pixels)
		{
			i++; truncated = original.substr(0, i);
			if (i == original.size())
			{
				i = 0; break;
			}
		}
		if (i)
			truncated += "...";

		return truncated;
	}

	__inline const char* FormatText(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		const char* text;
		ImFormatStringToTempBufferV(&text, nullptr, fmt, args);
		va_end(args);

		return text;
	}

	__inline void AddSteppedRect(ImVec2 adjPos, ImVec2 clipmin, ImVec2 clipmax, ImVec2 posmin, ImVec2 posmax, float v_min, float v_max, float step, ImU32 primary, ImU32 secondary)
	{
		ImDrawList* drawList = GetWindowDrawList();
		drawList->PushClipRect({ adjPos.x + clipmin.x, adjPos.y + clipmin.y }, { adjPos.x + clipmax.x, adjPos.y + clipmax.y }, true);

		int steps = (v_max - v_min) / step;
		if (steps < 21)
		{
			std::vector<std::pair<int, int>> steps;

			float min = v_min - fnmodf(v_min + step / 2, step) + step / 2, max = v_max - fnmodf(v_max + step / 2, step) + step / 2;

			if (fabsf(v_min - min) < 0.001f)
				steps.push_back({ posmin.x, posmin.x + 2 });
			while (true)
			{
				min += step;
				if (min + step / 2 > v_max)
					break;

				float percent = std::clamp((min - v_min) / (v_max - v_min), 0.f, 1.f);
				auto position = posmin.x + (posmax.x - posmin.x) * percent;
				steps.push_back({ position - 1, position + 1 });
			}
			if (fabsf(v_max - max) < 0.001f)
				steps.push_back({ posmax.x - 2, posmax.x });

			if (steps.size())
			{
				for (size_t i = 0; i < steps.size(); i++)
				{
					if (!i)
						drawList->AddRectFilled({ adjPos.x + posmin.x, adjPos.y + posmin.y }, { adjPos.x + steps.front().first, adjPos.y + posmax.y }, primary);
					else
						drawList->AddRectFilled({ adjPos.x + steps[i - 1].second, adjPos.y + posmin.y }, { adjPos.x + steps[i].first, adjPos.y + posmax.y }, primary);
					drawList->AddRectFilled({ adjPos.x + steps[i].first, adjPos.y + posmin.y }, { adjPos.x + steps[i].second, adjPos.y + posmax.y }, secondary);
				}
				drawList->AddRectFilled({ adjPos.x + steps.back().second, adjPos.y + posmin.y }, { adjPos.x + posmax.x, adjPos.y + posmax.y }, primary);

				return drawList->PopClipRect();
			}
		}

		drawList->AddRectFilled({ adjPos.x + posmin.x, adjPos.y + posmin.y }, { adjPos.x + posmax.x, adjPos.y + posmax.y }, primary);

		drawList->PopClipRect();
	}

	__inline void HelpMarker(const char* desc)
	{
		if (IsItemHovered())
			SetTooltip(desc);
	}

	__inline void IconImage(const char* icon, bool large = false, ImVec4 color = { 1, 1, 1, -1 })
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		if (color.w > 0.f)
			PushStyleColor(ImGuiCol_Text, color);
		PushFont(large ? F::Render.IconFontLarge : F::Render.IconFontRegular);
		TextUnformatted(icon);
		PopFont();
		if (color.w > 0.f)
			PopStyleColor();

		if (bTransparent || bDisabled)
			PopStyleVar();
	}

	__inline bool IconButton(const char* icon, bool large = false, ImVec4 color = { 1, 1, 1, -1 })
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		const auto originalPos = GetCursorPos();

		if (color.w > 0.f)
			PushStyleColor(ImGuiCol_Text, color);
		PushFont(large ? F::Render.IconFontLarge : F::Render.IconFontRegular);
		TextUnformatted(icon);
		if (!bDisabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		const bool pressed = IsItemClicked();
		PopFont();
		if (color.w > 0.f)
			PopStyleColor();

		// prevent accidental dragging
		SetCursorPos(originalPos);
		Button("##", GetItemRectSize());

		if (bTransparent || bDisabled)
			PopStyleVar();

		return bDisabled ? false : pressed;
	}

	std::unordered_map<const char*, float> lastHeights;
	std::vector<const char*> storedTitles;
	__inline bool Section(const char* title, float minHeight = 1.f, bool forceHeight = false)
	{
		storedTitles.push_back(title);
		if (!forceHeight && lastHeights.contains(title) && lastHeights[title] > minHeight)
			minHeight = lastHeights[title];
		PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
		const bool active = BeginChild(title, { GetColumnWidth(), minHeight + 8 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding);

		PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8, 0 });
		if (title[0] != '#')
		{
			const auto restorePos = GetCursorPos();

			PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
			PushFont(F::Render.FontBold);
			SetCursorPosY(restorePos.y + 1);
			TextUnformatted(StripDoubleHash(title).c_str());
			PopFont();
			PopStyleColor();

			SetCursorPos(restorePos); DebugDummy({ 0, 16 });
		}

		return active;
	}
	__inline void EndSection()
	{
		const char* title = storedTitles.back();
		storedTitles.pop_back();
		if (GetItemRectMax().y - GetWindowPos().y > 0.f)
			lastHeights[title] = GetItemRectMax().y - GetWindowPos().y;

		PopStyleVar();
		EndChild();
		PopStyleVar();
	}

	// widgets
	__inline bool FTabs(std::vector<const char*> titles, int* current, const ImVec2 size, const ImVec2 pos, bool vertical = false, std::vector<const char*> icons = {})
	{
		if (icons.size() && icons.size() != titles.size())
			return false;

		const int originalTab = current ? *current : 0;
		for (size_t i = 0; i < titles.size(); i++)
		{
			ImVec2 newPos = pos;
			if (!vertical)
				newPos = { pos.x + size.x * i, pos.y };
			else
				newPos = { pos.x, pos.y + size.y * i };
			SetCursorPos(newPos);
			const ImVec2 adjPos = { newPos.x + GetWindowPos().x, newPos.y + GetWindowPos().y };

			if (i != originalTab)
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			else
			{
				ImDrawList* drawList = GetWindowDrawList();
				if (!vertical)
					drawList->AddRectFilled({ adjPos.x, adjPos.y + size.y - 2 }, { adjPos.x + size.x, adjPos.y + size.y }, F::Render.Accent);
				else
					drawList->AddRectFilled({ adjPos.x + size.x - 2, adjPos.y }, { adjPos.x + size.x, adjPos.y + size.y }, F::Render.Accent);
			}
			if (Button(std::format("##{}", titles[i]).c_str(), size) && i != originalTab && current)
			{
				if (storedTitles.size() == 0)
					lastHeights.clear();
				*current = int(i);
			}
			if (!bDisabled && IsItemHovered())
				SetMouseCursor(ImGuiMouseCursor_Hand);

			const auto originalPos = GetCursorPos();

			const auto stripped = StripDoubleHash(titles[i]);
			const auto textSize = CalcTextSize(stripped.c_str());
			SetCursorPos({ newPos.x + (size.x - textSize.x) / 2, newPos.y + (size.y - textSize.y) / 2 });
			if (icons.size())
				SetCursorPosY(GetCursorPosY() + 10);
			TextUnformatted(stripped.c_str());
			if (icons.size())
			{
				SetCursorPos({ newPos.x + size.x / 2 - 8, newPos.y + size.x / 2 - 14 });
				IconImage(icons[i]);
			}
			SetCursorPos(originalPos);

			if (i != originalTab)
				PopStyleColor();
		}
		return current ? (*current != originalTab ? true : false) : false;
	}

	__inline bool FBeginPopup(const char* title, int flags = 0)
	{
		const bool bReturn = BeginPopup(title, flags);
		if (bReturn)
			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8, 8 });
		return bReturn;
	}
	__inline void FEndPopup()
	{
		PopStyleVar();
		EndPopup();
	}
	__inline bool FSelectable(const char* label, ImVec4 color = { 0.2f, 0.6f, 0.85f, 1.f }, bool selected = false, int flags = 0, const ImVec2& size_arg = {})
	{
		PushStyleVar(ImGuiStyleVar_SelectableRounding, 3.f);
		PushStyleColor(ImGuiCol_HeaderHovered, color);
		color.x *= 1.1f; color.y *= 1.1f; color.z *= 1.1f;
		PushStyleColor(ImGuiCol_HeaderActive, color);

		const bool bReturn = Selectable(label, selected, flags, size_arg);

		PopStyleColor(2);
		PopStyleVar();

		return bReturn;
	}

	__inline void FText(const char* text, int flags = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		const auto windowWidth = GetWindowSize().x;
		const auto textWidth = CalcTextSize(text).x;
		if (flags & FText_Middle)
			SetCursorPosX((windowWidth - textWidth) * 0.5f);
		else if (flags & FText_Right)
			SetCursorPosX(windowWidth - textWidth - 8);
		TextUnformatted(text);

		if (bTransparent || bDisabled)
			PopStyleVar();
	}

	__inline bool FButton(const char* label, int flags = 0, int sizeOffset = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		std::string str = label;
		if (!(flags & FButton_NoUpper))
			std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		label = str.c_str(); // why is this fucjking needed ??? ?

		float sizex = GetWindowSize().x - 2 * GetStyle().WindowPadding.x;
		if (flags & FButton_Left || flags & FButton_Right)
			sizex = GetWindowSize().x / 2 - GetStyle().WindowPadding.x - 4;
		else if (flags & FButton_Fit)
			sizex = CalcTextSize(label).x + (flags & FButton_Large ? 28 : 18);
		if (flags & FButton_SameLine)
			SameLine();
		else if (flags & FButton_Right)
			SetCursorPosX(sizex + 20);

		const auto restorePos = GetCursorPos();
		DebugShift({ 0, 8 });

		PushStyleColor(ImGuiCol_Border, F::Render.Accent.Value);
		PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		const bool active = Button(label, { sizex + sizeOffset, flags & FButton_Large ? 40.f : 30.f });
		if (!bDisabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		PopStyleVar();
		PopStyleColor();

		SetCursorPos(restorePos); DebugDummy({ sizex + sizeOffset, flags & FButton_Large ? 48.f : 38.f });

		if (bTransparent || bDisabled)
			PopStyleVar();

		return bDisabled ? false : active;
	}

	__inline bool FToggle(const char* label, bool* var, int flags = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		if (flags & FToggle_Middle)
			SameLine(GetWindowSize().x / 2 + 4);

		const auto restorePos = GetCursorPos();

		bool changed = Button(std::format("##{}", label).c_str(), { GetWindowSize().x / 2 + 4 - 2 * GetStyle().WindowPadding.x, 24 });
		if (bDisabled)
			changed = false;
		if (changed)
			*var = !*var;
		if (!bDisabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);

		SetCursorPos({ restorePos.x + 4, restorePos.y + 3 });
		IconImage(*var ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, true, *var ? (flags & FToggle_PlainColor ? F::Render.Active.Value : F::Render.Accent.Value) : F::Render.Inactive.Value);

		SetCursorPos({ restorePos.x + 24, restorePos.y + 5 });
		if (*var)
			PushStyleColor(ImGuiCol_Text, F::Render.Active.Value);
		else
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
		TextUnformatted(StripDoubleHash(label).c_str());
		PopStyleColor();

		SetCursorPos(restorePos); DebugDummy({ 0, 24 });

		if (bTransparent || bDisabled)
			PopStyleVar();

		return changed;
	}

	__inline bool FSlider(const char* label, float* var1, float* var2, float v_min, float v_max, float step = 1.f, const char* fmt = "%.0f", int flags = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		float originalVar1 = *var1, originalVar2;
		if (var2)
			originalVar2 = *var2;

		if (flags & FSlider_Right)
			SameLine(GetWindowSize().x / 2 + 4);

		float sizex = GetWindowSize().x, sizexHalf = sizex / 2 + 4;
		if (flags & (FSlider_Left | FSlider_Right))
			sizex = sizexHalf;
		if (flags & FSlider_Right)
			SameLine(sizex);
		sizex = sizex - 2 * GetStyle().WindowPadding.x;
		sizexHalf = sizexHalf - 2 * GetStyle().WindowPadding.x;

		const auto restorePos = GetCursorPos(); auto adjPos = GetWindowPos();
		PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);

		if (flags & (FSlider_Left | FSlider_Right))
			SetCursorPos({ restorePos.x + 6, restorePos.y + 3 });
		else
			SetCursorPos({ restorePos.x + 6, restorePos.y + 5 });
		TextUnformatted(StripDoubleHash(label).c_str());

		{
			static std::string text, input; std::string index = std::format("{}## Text", label);
			if (!mActives[index])
			{
				if (var2)
					text = FormatText(fmt, *var1, *var2);
				else
					text = FormatText(fmt, *var1);
			}
			else
			{
				SetCursorPos({ -1000, flags & (FSlider_Left | FSlider_Right) ? restorePos.y + 3 : restorePos.y + 5 }); // lol
				SetKeyboardFocusHere();
				const bool enter = InputText("##SliderText", &input, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsDecimal); text = input;
				if (enter)
				{
					try // prevent the user from being a retard with invalid inputs
					{
						if (var2)
							switch (mActives[index])
							{
							case 1:
								*var1 = text.length() ? std::stof(text) : 0.f;
								*var1 = std::min(*var1, *var2 - step);
								if (!(flags & FSlider_Precision))
									*var1 = *var1 - fnmodf(*var1 - step / 2, step) + step / 2;
								if (flags & FSlider_Clamp)
									*var1 = std::clamp(*var1, v_min, v_max);
								break;
							case 2:
								*var2 = text.length() ? std::stof(text) : 0.f;
								*var2 = std::max(*var2, *var1 + step);
								if (!(flags & FSlider_Precision))
									*var2 = *var2 - fnmodf(*var2 - step / 2, step) + step / 2;
								if (flags & FSlider_Clamp)
									*var2 = std::clamp(*var2, v_min, v_max);
							}
						else
						{
							*var1 = text.length() ? std::stof(text) : 0.f;
							if (!(flags & FSlider_Precision))
								*var1 = *var1 - fnmodf(*var1 - step / 2, step) + step / 2;
							if (flags & FSlider_Clamp)
								*var1 = std::clamp(*var1, v_min, v_max);
						}
					}
					catch (...) {}
				}
				if (enter || IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_Escape))
					mActives[index] = false;
			}
			const float width = CalcTextSize(text.c_str()).x;
			if (flags & (FSlider_Left | FSlider_Right))
				SetCursorPos({ restorePos.x + sizex - width - 6, restorePos.y + 3 });
			else
				SetCursorPos({ restorePos.x + sizex - 40, restorePos.y + 5 });
			const auto original = GetCursorPos();

			TextUnformatted(text.c_str());
			if (!bDisabled)
			{
				if (!bDisabled && IsItemHovered() && ImGui::IsWindowHovered())
					SetMouseCursor(ImGuiMouseCursor_TextInput);
				if (mActives[index])
					GetWindowDrawList()->AddRectFilled({ adjPos.x + original.x, adjPos.y + original.y + 14 }, { adjPos.x + original.x + width, adjPos.y + original.y + 15 }, F::Render.Active);
				else if (IsItemClicked())
				{
					if (var2)
					{
						if (GetMousePos().x - adjPos.x - original.x < width / 2)
						{
							input = std::format("{}", *var1);
							mActives[index] = 1;
						}
						else
						{
							input = std::format("{}", *var2);
							mActives[index] = 2;
						}
					}
					else
					{
						input = std::format("{}", *var1);
						mActives[index] = true;
					}
				}
			}
		}

		auto accent = F::Render.Accent, muted = accent, washed = accent, transparent = accent;
		muted.Value.w *= 0.8f, washed.Value.w *= 0.4f, transparent.Value.w *= 0.2f;
		if (bTransparent || bDisabled)
			accent.Value.w /= 2, muted.Value.w /= 2, washed.Value.w /= 2, transparent.Value.w /= 2;
		adjPos.x += restorePos.x; adjPos.y += restorePos.y;
		ImVec2 mins = { sizex - sizexHalf - 16, 11 }, maxs = { sizex - 54, 13 };
		if (flags & (FSlider_Left | FSlider_Right))
			mins = { 6, 24 }, maxs = { sizex - 6, 26 };

		ImDrawList* drawList = GetWindowDrawList(); auto mouse = GetMousePos();
		const bool within = adjPos.x + mins.x - 6 < mouse.x && mouse.x < adjPos.x + maxs.x + 5 &&
			adjPos.y + mins.y - 6 < mouse.y && mouse.y < adjPos.y + maxs.y + 5;
		if (!bDisabled && within && ImGui::IsWindowHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		const float mousePerc = (mouse.x - (adjPos.x + mins.x)) / ((adjPos.x + maxs.x) - (adjPos.x + mins.x)) + (step / 2) / (v_max - v_min);
		if (var2)
		{
			float lowerPerc = std::clamp((*var1 - v_min) / (v_max - v_min), 0.f, 1.f), upperPerc = std::clamp((*var2 - v_min) / (v_max - v_min), 0.f, 1.f);
			auto lowerPos = mins.x + (maxs.x - mins.x) * lowerPerc, upperPos = mins.x + (maxs.x - mins.x) * upperPerc;

			AddSteppedRect(adjPos, mins, { lowerPos, maxs.y }, mins, maxs, v_min, v_max, step, washed, muted);
			AddSteppedRect(adjPos, { lowerPos, mins.y }, { upperPos, maxs.y }, mins, maxs, v_min, v_max, step, accent, washed);
			AddSteppedRect(adjPos, { upperPos, mins.y }, maxs, mins, maxs, v_min, v_max, step, washed, muted);
			drawList->AddCircleFilled({ adjPos.x + lowerPos, adjPos.y + mins.y + 1 }, 3.f, accent);
			drawList->AddCircleFilled({ adjPos.x + upperPos, adjPos.y + mins.y + 1 }, 3.f, accent);

			if (!bDisabled)
			{
				if (within && !mActives[label] && ImGui::IsWindowHovered())
				{
					if (fabsf(mouse.x - (adjPos.x + lowerPos)) < fabsf(mouse.x - (adjPos.x + upperPos)))
					{
						if (!IsMouseDown(ImGuiMouseButton_Left))
							drawList->AddCircleFilled({ adjPos.x + lowerPos, adjPos.y + mins.y + 1 }, 11.f, transparent);
						if (IsMouseClicked(ImGuiMouseButton_Left))
							mActives[label] = 1;
					}
					else
					{
						if (!IsMouseDown(ImGuiMouseButton_Left))
							drawList->AddCircleFilled({ adjPos.x + upperPos, adjPos.y + mins.y + 1 }, 11.f, transparent);
						if (IsMouseClicked(ImGuiMouseButton_Left))
							mActives[label] = 2;
					}
				}
				else if ((mActives[label] == 1 || mActives[label] == 2) && IsMouseDown(ImGuiMouseButton_Left))
				{
					//a + (b - a) * t [lerp]
					switch (mActives[label])
					{
					case 1:
						*var1 = std::min(v_min + (v_max - v_min) * mousePerc, *var2 - step);
						*var1 = std::clamp(*var1 - fnmodf(*var1, step), v_min, v_max);
						drawList->AddCircleFilled({ adjPos.x + lowerPos, adjPos.y + mins.y + 1 }, 11.f, washed);
						break;
					case 2:
						*var2 = std::max(v_min + (v_max - v_min) * mousePerc, *var1 + step);
						*var2 = std::clamp(*var2 - fnmodf(*var2, step), v_min, v_max);
						drawList->AddCircleFilled({ adjPos.x + upperPos, adjPos.y + mins.y + 1 }, 11.f, washed);
					}
				}
				else
					mActives[label] = false;
			}
		}
		else
		{
			float percent = std::clamp((*var1 - v_min) / (v_max - v_min), 0.f, 1.f);

			AddSteppedRect(adjPos, mins, { mins.x + (maxs.x - mins.x) * percent, maxs.y }, mins, maxs, v_min, v_max, step, accent, washed);
			AddSteppedRect(adjPos, { mins.x + (maxs.x - mins.x) * percent, mins.y }, maxs, mins, maxs, v_min, v_max, step, washed, muted);
			drawList->AddCircleFilled({ adjPos.x + mins.x + (maxs.x - mins.x) * percent, adjPos.y + mins.y + 1 }, 3.f, accent);

			if (!bDisabled)
			{
				if (within && !mActives[label] && ImGui::IsWindowHovered())
				{
					if (!IsMouseDown(ImGuiMouseButton_Left))
						drawList->AddCircleFilled({ adjPos.x + mins.x + (maxs.x - mins.x) * percent, adjPos.y + mins.y + 1 }, 11.f, transparent);
					if (IsMouseClicked(ImGuiMouseButton_Left))
						mActives[label] = 1;
				}
				else if (mActives[label] == 1 && IsMouseDown(ImGuiMouseButton_Left))
				{
					*var1 = v_min + (v_max - v_min) * mousePerc;
					*var1 = std::clamp(*var1 - fnmodf(*var1, step), v_min, v_max);
					drawList->AddCircleFilled({ adjPos.x + mins.x + (maxs.x - mins.x) * percent, adjPos.y + mins.y + 1 }, 11.f, washed);
				}
				else
					mActives[label] = false;
			}
		}

		PopStyleColor();
		SetCursorPos({ restorePos.x + mins.x - 5, restorePos.y + mins.y - 5 });
		Button("##", { maxs.x - mins.x + 10, 12 }); // don't drag it around
		SetCursorPos(restorePos); Dummy({ 0, flags & (FSlider_Left | FSlider_Right) ? 32.f : 24.f });

		bool changed = *var1 != originalVar1;
		if (!changed && var2)
			changed = *var2 != originalVar2;

		if (bTransparent || bDisabled)
			PopStyleVar();

		return changed;
	}

	__inline bool FSlider(const char* label, int* var1, int* var2, int v_min, int v_max, int step = 1, const char* fmt = "%d", int flags = 0)
	{
		// replace incorrect formats as it will be converted to float
		std::string replace = fmt;

		std::string from = "%d", to = "%.0f";
		auto found = replace.find(from);
		while (found != std::string::npos)
		{
			replace.replace(found, from.length(), to);
			found = replace.find(from);
		}
		from = "%i";
		found = replace.find(from);
		while (found != std::string::npos)
		{
			replace.replace(found, from.length(), to);
			found = replace.find(from);
		}

		fmt = replace.c_str();

		float redir1 = *var1; float redir2 = var2 ? *var2 : 0;
		const bool changed = FSlider(label, &redir1, var2 ? &redir2 : nullptr, v_min, v_max, step, fmt, flags);
		*var1 = redir1; if (var2) *var2 = redir2;
		return changed;
	}

	__inline bool FSlider(const char* label, float* var, float v_min, float v_max, float step = 1.f, const char* fmt = "%.0f", int flags = 0)
	{
		return FSlider(label, var, nullptr, v_min, v_max, step, fmt, flags);
	}

	__inline bool FSlider(const char* label, int* var, int v_min, int v_max, int step = 1, const char* fmt = "%d", int flags = 0)
	{
		return FSlider(label, var, nullptr, v_min, v_max, step, fmt, flags);
	}

	__inline bool FDropdown(const char* label, int* var, std::vector<const char*> titles, std::vector<int> values = {}, int flags = 0, int colors = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool changed = false;

		if (values.size() == 0)
		{
			for (size_t i = 0; i < titles.size(); i++)
			{
				if (flags & FDropdown_Multi)
					values.push_back(1 << int(i));
				else
					values.push_back(int(i));
			}
		}

		std::string preview = "";
		if (flags & FDropdown_Multi && *var == 0)
			preview = "None";
		else
		{
			for (size_t i = 0; i < values.size(); i++)
			{
				if (flags & FDropdown_Multi)
				{
					if (*var & values[i])
						preview += std::format("{}, ", StripDoubleHash(titles[i]).c_str());
				}
				else
				{
					if (*var == values[i])
						preview = std::format("{}##", StripDoubleHash(titles[i]).c_str());
				}
			}
			preview.pop_back(); preview.pop_back();
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 13.5f });
		float sizex = GetWindowSize().x;
		if (flags & (FDropdown_Left | FDropdown_Right))
			sizex = sizex / 2 + 4;
		if (flags & FDropdown_Right)
			SameLine(sizex);
		sizex = sizex - 2 * GetStyle().WindowPadding.x - 10 * colors;
		PushItemWidth(sizex);

		const auto restorePos = GetCursorPos();
		DebugShift({ 0, 8 });

		if (bDisabled)
		{	// lol
			Button("##", { sizex, 40 });
			SetCursorPos(restorePos);
			DebugShift({ 0, 8 });
		}

		bool active = false;
		if (BeginCombo(std::format("##{}", label).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			active = true;

			DebugDummy({ 0, 8 });
			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 19 });
			for (size_t i = 0; i < titles.size(); i++)
			{
				const auto stripped = StripDoubleHash(titles[i]);
				if (flags & FDropdown_Multi)
				{
					bool flagActive = *var & values[i];

					if (Selectable(std::format("##{}", titles[i]).c_str(), flagActive, ImGuiSelectableFlags_DontClosePopups))
					{
						if (flagActive)
							*var &= ~values[i];
						else
							*var |= values[i];
						changed = true;
					}

					const auto originalPos = GetCursorPos();
					SetCursorPos({ originalPos.x + 40, originalPos.y - 31 });
					PushStyleColor(ImGuiCol_Text, flagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(stripped.c_str());
					PopStyleColor();

					SetCursorPos({ originalPos.x + 16, originalPos.y - 33 });
					IconImage(flagActive ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK, true, flagActive ? F::Render.Accent.Value : F::Render.Inactive.Value);
					SetCursorPos(originalPos);
				}
				else
				{
					if (Selectable(std::format("##{}", titles[i]).c_str(), *var == values[i]))
					{
						*var = values[i]; changed = true;
					}

					const auto originalPos = GetCursorPos();
					SetCursorPos({ originalPos.x + 20, originalPos.y - 31 });
					PushStyleColor(ImGuiCol_Text, *var == values[i] ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(stripped.c_str());
					PopStyleColor();
					SetCursorPos(originalPos);
				}
			}
			PopStyleVar();
			SetCursorPosY(GetCursorPosY() - 10); Dummy({});

			EndCombo();
		}
		if (!bDisabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			const auto originalPos = GetCursorPos();

			SetCursorPos({ originalPos.x + 12, originalPos.y - 5 });
			PushFont(F::Render.FontSmall);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			TextUnformatted(StripDoubleHash(label).c_str());
			PopStyleColor();
			PopFont();

			SetCursorPos({ originalPos.x + 12, originalPos.y + 8 });
			TextUnformatted(TruncateText(preview.c_str(), sizex - 55).c_str());

			SetCursorPos({ originalPos.x + sizex - 25, originalPos.y - 2 });
			IconImage(active ? ICON_MD_ARROW_DROP_UP : ICON_MD_ARROW_DROP_DOWN, true);

			EndComboPreview();
		}
		SetCursorPos(restorePos); DebugDummy({ sizex, 48 });

		PopItemWidth();
		PopStyleVar();

		if (bTransparent || bDisabled)
			PopStyleVar();

		return changed;
	}

	__inline bool FSDropdown(const char* label, std::string* var, std::vector<const char*> entries = {}, int flags = 0, int colors = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool changed = false;

		if (!entries.size())
		{
			PushStyleColor(ImGuiCol_PopupBg, {});
			PushStyleVar(ImGuiStyleVar_WindowPadding, { GetStyle().WindowPadding.x, 0 });
		}
		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 13.5f });
		float sizex = GetWindowSize().x;
		if (flags & (FDropdown_Left | FDropdown_Right))
			sizex = sizex / 2 + 4;
		if (flags & FDropdown_Right)
			SameLine(sizex);
		sizex = sizex - 2 * GetStyle().WindowPadding.x - 10 * colors;
		PushItemWidth(sizex);

		const auto restorePos = GetCursorPos();
		DebugShift({ 0, 8 });

		if (bDisabled)
		{	// lol
			Button("##", { sizex, 40 });
			SetCursorPos(restorePos);
			DebugShift({ 0, 8 });
		}

		static std::string preview = "", input = "", staticif = "\n";
		if (BeginCombo(std::format("##{}", label).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			if (!mActives[label])
				preview = input = "";

			mActives[label] = true;

			int tab = U::KeyHandler.Pressed(VK_TAB) ? 1 : 0;

			// this textinput is being used as a temporary measure to prevent the main window drawing over the popup
			const auto restorePos = GetCursorPos();
			SetCursorPos({ -1000, entries.size() ? GetScrollY() : -100 }); // lol
			if (!IsMouseDown(ImGuiMouseButton_Left))
				SetKeyboardFocusHere();
			const bool enter = InputText("##FSDropdown", &input, ImGuiInputTextFlags_EnterReturnsTrue);
			if (input != staticif)
			{
				preview = input;
				staticif = "\n";
			}
			SetCursorPos(restorePos);

			std::vector<std::pair<std::string, std::string>> valid = {};
			std::string current = *var, search = input, display = preview;
			std::transform(current.begin(), current.end(), current.begin(), ::tolower);
			std::transform(search.begin(), search.end(), search.begin(), ::tolower);
			std::transform(display.begin(), display.end(), display.begin(), ::tolower);
			for (size_t i = 0; i < entries.size(); i++)
			{
				std::string entry = entries[i];
				std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);

				auto found = entry.find(search);
				if (found != std::string::npos)
					valid.push_back({ entries[i], entry });
			}

			if (valid.size())
			{
				DebugDummy({ 0, 8 });
				PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 19 });

				for (size_t i = 0; i < valid.size(); i++)
				{
					if (enter && !(flags & FSDropdown_Custom))
					{
						*var = valid[i].first; changed = true;
						CloseCurrentPopup(); break;
					}
					if (tab == 1)
					{
						preview = valid[i].first;
						staticif = input;
						tab = 2;
					}
					if (tab && display == valid[i].second)
						tab = 1;

					if (Selectable(std::format("##{}", valid[i].first).c_str(), current == valid[i].second))
					{
						*var = valid[i].first; changed = true;
					}

					const auto originalPos = GetCursorPos();
					SetCursorPos({ originalPos.x + 20, originalPos.y - 31 });
					PushStyleColor(ImGuiCol_Text, current == valid[i].second ? F::Render.Active.Value : F::Render.Inactive.Value);
					TextUnformatted(valid[i].first.c_str());
					PopStyleColor();
					SetCursorPos(originalPos);
				}

				PopStyleVar();
				SetCursorPosY(GetCursorPosY() - 10); Dummy({});
			}

			if ((enter || flags & FSDropdown_AutoUpdate) && (flags & FSDropdown_Custom || !entries.size()))
				*var = preview; changed = true;
			if (enter)
				CloseCurrentPopup();

			EndCombo();
		}
		else
			mActives[label] = false;
		if (!bDisabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_TextInput);
		if (BeginComboPreview())
		{
			const auto originalPos = GetCursorPos();

			SetCursorPos({ originalPos.x + 12, originalPos.y - 5 });
			PushFont(F::Render.FontSmall);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			TextUnformatted(StripDoubleHash(label).c_str());
			PopStyleColor();
			PopFont();

			SetCursorPos({ originalPos.x + 12, originalPos.y + 8 });
			// would like this to work properly, text looks nicer but overrides window
			/*
			if (active)
			{
				PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
				PushStyleColor(ImGuiCol_FrameBg, {});
				PushItemWidth(sizex - 12);
				if (!IsAnyItemActive()) // silly, but afaik no way to have a one time focus
					SetKeyboardFocusHere();
				enter = FInputText("##FSDropdown", &preview, ImGuiInputTextFlags_EnterReturnsTrue);
				PopItemWidth();
				PopStyleColor();
				PopStyleVar();
			}
			else
				TextUnformatted(TruncateText(var->c_str(), sizex - (entries.size() ? 55 : 15)).c_str());
			*/
			TextUnformatted(TruncateText(mActives[label] ? preview.c_str() : var->c_str(), sizex - (entries.size() ? 55 : 35)).c_str());

			if (entries.size())
			{
				SetCursorPos({ originalPos.x + sizex - 25, originalPos.y - 2 });
				IconImage(mActives[label] ? ICON_MD_ARROW_DROP_UP : ICON_MD_ARROW_DROP_DOWN, true);
			}

			if (mActives[label] || flags & FSDropdown_Custom || !entries.size())
			{
				ImVec2 adjPos = GetWindowPos(); adjPos.x += originalPos.x; adjPos.y += originalPos.y;
				GetWindowDrawList()->AddRectFilled({ adjPos.x + 12, adjPos.y + 22 }, { adjPos.x + sizex - (entries.size() ? 33 : 13), adjPos.y + 23 }, mActives[label] ? F::Render.Active : F::Render.Inactive);
			}

			EndComboPreview();
		}
		PopItemWidth();
		PopStyleVar();
		if (!entries.size())
		{
			PopStyleColor();
			PopStyleVar();
		}

		SetCursorPos(restorePos); DebugDummy({ sizex, 48 });

		if (bTransparent || bDisabled)
			PopStyleVar();

		return changed;
	}

	__inline bool FVDropdown(const char* label, std::vector<std::string>* var, std::vector<std::string> titles, int flags = 0, int colors = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool changed = false;

		std::unordered_map<std::string, std::vector<std::string>::iterator> iterators = {};
		for (auto it = var->begin(); it != var->end(); it++)
			iterators[*it] = it;
		std::unordered_map<std::string, int> integers = {};
		for (size_t i = 0; i < var->size(); i++)
			integers[(*var)[i]] = int(i) + 1;

		std::string preview = "";
		if (!var->size())
			preview = "None";
		else
		{
			for (size_t i = 0; i < var->size(); i++)
				preview += std::format("{}, ", (*var)[i].c_str());
			preview.pop_back(); preview.pop_back();
		}

		PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 13.5f });
		float sizex = GetWindowSize().x;
		if (flags & (FDropdown_Left | FDropdown_Right))
			sizex = sizex / 2 + 4;
		if (flags & FDropdown_Right)
			SameLine(sizex);
		sizex = sizex - 2 * GetStyle().WindowPadding.x - 10 * colors;
		PushItemWidth(sizex);

		const auto restorePos = GetCursorPos();
		DebugShift({ 0, 8 });

		if (bDisabled)
		{	// lol
			Button("##", { sizex, 40 });
			SetCursorPos(restorePos);
			DebugShift({ 0, 8 });
		}

		bool active = false;
		if (BeginCombo(std::format("##{}", label).c_str(), "", ImGuiComboFlags_CustomPreview | ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightLarge))
		{
			active = true;

			DebugDummy({ 0, 8 });
			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 19 });
			for (size_t i = 0; i < titles.size(); i++)
			{
				auto find = iterators.find(titles[i]);
				bool flagActive = find != iterators.end();

				if (Selectable(std::format("##{}", titles[i]).c_str(), flagActive, ImGuiSelectableFlags_DontClosePopups))
				{
					if (flagActive)
						var->erase(find->second);
					else
						var->push_back(titles[i]);
					changed = true;
				}

				// shift based on number of digits in var size
				const auto originalPos = GetCursorPos();
				SetCursorPos({ originalPos.x + 40 + 6 * std::min(int(log10(var->size())), 0), originalPos.y - 31 });
				PushStyleColor(ImGuiCol_Text, flagActive ? F::Render.Active.Value : F::Render.Inactive.Value);
				TextUnformatted(titles[i].c_str());
				PopStyleColor();

				if (flagActive)
				{
					SetCursorPos({ originalPos.x + 18, originalPos.y - 31 });
					PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
					TextUnformatted(std::format("{}", integers[titles[i]]).c_str());
					PopStyleColor();
				}
				SetCursorPos(originalPos);
			}
			PopStyleVar();
			SetCursorPosY(GetCursorPosY() - 10); Dummy({});

			EndCombo();
		}
		if (!bDisabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (BeginComboPreview())
		{
			const auto originalPos = GetCursorPos();

			SetCursorPos({ originalPos.x + 12, originalPos.y - 5 });
			PushFont(F::Render.FontSmall);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			TextUnformatted(StripDoubleHash(label).c_str());
			PopStyleColor();
			PopFont();

			SetCursorPos({ originalPos.x + 12, originalPos.y + 8 });
			TextUnformatted(TruncateText(preview.c_str(), sizex - 55).c_str());

			SetCursorPos({ originalPos.x + sizex - 25, originalPos.y - 2 });
			IconImage(active ? ICON_MD_ARROW_DROP_UP : ICON_MD_ARROW_DROP_DOWN, true);

			EndComboPreview();
		}
		SetCursorPos(restorePos); DebugDummy({ sizex, 48 });

		PopItemWidth();
		PopStyleVar();

		if (bTransparent || bDisabled)
			PopStyleVar();

		return changed;
	}

	__inline bool ColorPicker(const char* label, Color_t* color, bool marker = true, int flags = 0)
	{
		if (bDisabled)
		{	// lol
			const auto restorePos = GetCursorPos();
			Button("##", flags & FColorPicker_Dropdown ? ImVec2(10, 40) : ImVec2(12, 12));
			SetCursorPos(restorePos);
		}

		ImVec4 tempColor = ColorToVec(*color);

		bool changed = false;
		PushStyleVar(ImGuiStyleVar_FramePadding, { 2, 2 });
		PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 4 });
		PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 4, 0 });
		PushStyleColor(ImGuiCol_PopupBg, F::Render.Foreground.Value);
		if (ColorEdit4(std::format("##{}", label).c_str(), &tempColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Round, flags & FColorPicker_Dropdown ? ImVec2(10, 40) : ImVec2(12, 12)))
		{
			*color = VecToColor(tempColor);
			changed = true;
		}
		PopStyleColor();
		PopStyleVar(3);
		if (!bDisabled && IsItemHovered())
			SetMouseCursor(ImGuiMouseCursor_Hand);
		if (marker)
			HelpMarker(label);

		return changed;
	}

	// if items overlap, use before to have working input, e.g. a middle toggle and a color picker
	__inline bool FColorPicker(const char* label, Color_t* color, int offset = 0, int flags = 0)
	{
		if (bTransparent || bDisabled)
			PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

		bool changed = false;
		if (!(flags & FColorPicker_Dropdown))
		{
			int pos;
			if (flags & FColorPicker_Left)
				pos = 14 + (offset * 12);
			else if (flags & FColorPicker_Middle)
				pos = GetContentRegionMax().x / 2 + 5 - (offset * 12);
			else
				pos = GetContentRegionMax().x - 20 - (offset * 12);
			if (flags & FColorPicker_SameLine)
				SameLine(pos);
			else
				SetCursorPosX(pos);

			const auto restorePos = GetCursorPos();
			DebugShift({ 0, 5 });

			changed = ColorPicker(label, color, !(flags & (FColorPicker_Left | FColorPicker_Middle)));
			if (flags & (FColorPicker_Left | FColorPicker_Middle))
			{
				SameLine(); TextUnformatted(label);
				SetCursorPos(restorePos); DebugDummy({ 0, 24 });
			}
			else
			{
				SetCursorPos(restorePos); Dummy({ 0, 0 });
			}
		}
		else
		{
			SameLine(); DebugShift({ -8, 0 });
			const auto restorePos = GetCursorPos(); DebugShift({ 0, 8 });
			changed = ColorPicker(label, color, false, flags);
			SetCursorPos(restorePos); DebugDummy({ 10, 48 });
		}

		if (bTransparent || bDisabled)
			PopStyleVar();

		return changed;
	}

	short iKeyPressed = 0;
	__inline void KeyHandler()
	{
		static std::map<short, bool> mOldKeys = {};
		std::map<short, bool> mNewKeys = {};

		for (short iKey = 0; iKey < 255; iKey++)
			mNewKeys[iKey] = GetAsyncKeyState(iKey) & 0x8000;

		iKeyPressed = 0;
		for (auto& [iKey, bPressed] : mNewKeys)
		{
			if (bPressed && (!mOldKeys.contains(iKey) || !mOldKeys[iKey]))
			{
				iKeyPressed = iKey;
				break;
			}
		}

		mOldKeys = mNewKeys;
	}

	__inline std::string VK2STR(const short key)
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

		if (Vars::Debug::Info.Value && FNV1A::Hash(str.c_str()) == FNV1A::HashConst("unknown"))
			str = std::format("{:#x}", key);

		return str;
	}
	__inline bool FKeybind(const char* label, int& output, int flags = 0, int sizeOffset = 0)
	{
		static bool bCanceled = false;

		const auto id = GetID(label);
		PushID(label);

		if (GetActiveID() == id)
		{
			F::Menu.InKeybind = true;

			//FButton("...", flags | FButton_NoUpper, sizeOffset);
			FButton(std::format("{}: ...", label).c_str(), flags | FButton_NoUpper, sizeOffset);
			const bool bHovered = IsItemHovered();

			if (bHovered && IsMouseClicked(ImGuiMouseButton_Left))
			{
				bCanceled = true;
				ClearActiveID();
			}
			else
			{
				SetActiveID(id, GetCurrentWindow());

				if (iKeyPressed)
				{
					switch (iKeyPressed)
					{
					case VK_LBUTTON:
						output = bHovered ? output : iKeyPressed;
						break;
					case VK_ESCAPE:
						if (flags & FKeybind_AllowNone)
						{
							output = 0x0;
							break;
						}
						[[fallthrough]];
					default:
						if (flags & FKeybind_AllowMenu || iKeyPressed != Vars::Menu::MenuPrimaryKey.Value && iKeyPressed != Vars::Menu::MenuSecondaryKey.Value)
							output = iKeyPressed;
					}
					ClearActiveID();
				}
			}

			GetCurrentContext()->ActiveIdAllowOverlap = true;
		}
		//else if (FButton(VK2STR(output).c_str(), flags | FButton_NoUpper) && !bCanceled)
		else if (FButton(std::format("{}: {}", label, VK2STR(output)).c_str(), flags | FButton_NoUpper, sizeOffset) && !bCanceled)
			SetActiveID(id, GetCurrentWindow());

		if (bCanceled && !IsMouseDown(ImGuiMouseButton_Left) && !IsMouseReleased(ImGuiMouseButton_Left))
			bCanceled = false;

		PopID();

		return true;
	}

	// dropdown for materials
	__inline bool FMDropdown(const char* label, std::vector<std::string>* var, int flags = 0, int colors = 0)
	{
		std::vector<std::pair<std::string, Material_t>> vMaterials;
		for (auto const& [sName, mat] : F::Materials.mChamMaterials)
		{
			if (FNV1A::Hash(sName.c_str()) != FNV1A::HashConst("None"))
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

		std::vector<std::string> entries = { "Original" };
		for (const auto& pair : vMaterials)
			entries.push_back(pair.first.c_str());

		return FVDropdown(label, var, entries, flags, colors);
	}

	// convar wrappers
	bool bOldDisabled, bOldTransparent;

	template <class T>
	__inline std::string GetCondition(ConfigVar<T>& var, bool bForce = false)
	{
		if (var.m_iFlags & (NOSAVE | NOCOND))
			return "default";

		if (bForce)
			return sCondition;

		std::string parent = sCondition;
		while (true)
		{
			if (FNV1A::Hash(parent.c_str()) == FNV1A::HashConst("default") || var.Map.contains(parent))
				break;
			parent = F::Conditions.GetParent(parent);
		}
		return parent;
	}

	template <class T>
	__inline T GetParentValue(ConfigVar<T>& var, std::string sCond) // oh my god
	{
		std::string parent = sCond;
		while (true)
		{
			parent = F::Conditions.GetParent(parent);
			if (FNV1A::Hash(parent.c_str()) == FNV1A::HashConst("default") || var.Map.contains(parent))
				break;
		}
		return var.Map[parent];
	}

	template <class T>
	__inline T FGet(ConfigVar<T>& var, const bool bDisable = false)
	{
		bOldDisabled = bDisabled, bOldTransparent = bTransparent;

		const auto condition = GetCondition(var);
		if (bDisable)
		{
			if (FNV1A::Hash(sCondition.c_str()) == FNV1A::HashConst("default"))
			{
				if (Vars::Menu::MenuShowsBinds.Value && var.Map["default"] != var.Value)
				{
					for (auto& [sCond, tVal] : var.Map)
					{
						if (FNV1A::Hash(sCond.c_str()) == FNV1A::HashConst("default"))
							continue;

						if (tVal == var.Value)
						{
							bDisabled = true;
							return tVal;
						}
					}
				}
			}
			else
				bTransparent = sCondition != condition && !(var.m_iFlags & (NOSAVE | NOCOND));
		}
		return var.Map[condition];
	}

	template <class T>
	__inline void FSet(ConfigVar<T>& var, T val)
	{
		if (!bDisabled)
		{
			const auto condition = GetCondition(var, true);
			const auto value = GetParentValue(var, condition);

			if (value != val)
				var.Map[condition] = val;
			else if (FNV1A::Hash(condition.c_str()) != FNV1A::HashConst("default"))
			{
				for (auto it = var.Map.begin(); it != var.Map.end();)
				{
					if (it->first == condition)
						it = var.Map.erase(it);
					else
						++it;
				}
			}
		}

		bDisabled = bOldDisabled, bTransparent = bOldTransparent;
	}

	__inline bool FToggle(const char* label, ConfigVar<bool>& var, int flags = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FToggle(label, &val, flags);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FSlider(const char* label, ConfigVar<FloatRange_t>& var, float v_min, float v_max, float step = 1.f, const char* fmt = "%.0f", int flags = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FSlider(label, &val.Min, &val.Max, v_min, v_max, step, fmt, flags);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FSlider(const char* label, ConfigVar<IntRange_t>& var, int v_min, int v_max, int step = 1, const char* fmt = "%d", int flags = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FSlider(label, &val.Min, &val.Max, v_min, v_max, step, fmt, flags);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FSlider(const char* label, ConfigVar<float>& var, float v_min, float v_max, float step = 1.f, const char* fmt = "%.0f", int flags = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FSlider(label, &val, v_min, v_max, step, fmt, flags);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FSlider(const char* label, ConfigVar<int>& var, int v_min, int v_max, int step = 1, const char* fmt = "%d", int flags = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FSlider(label, &val, v_min, v_max, step, fmt, flags);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FDropdown(const char* label, ConfigVar<int>& var, std::vector<const char*> titles, std::vector<int> values = {}, int flags = 0, int colors = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FDropdown(label, &val, titles, values, flags, colors);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FSDropdown(const char* label, ConfigVar<std::string>& var, std::vector<const char*> entries = {}, int flags = 0, int colors = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FSDropdown(label, &val, entries, flags, colors);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FVDropdown(const char* label, ConfigVar<std::vector<std::string>>& var, std::vector<std::string> titles, int flags = 0, int colors = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FVDropdown(label, &val, titles, flags, colors);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FMDropdown(const char* label, ConfigVar<std::vector<std::string>>& var, int flags = 0, int colors = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FMDropdown(label, &val, flags, colors);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FColorPicker(const char* label, ConfigVar<Color_t>& var, int offset = 0, int flags = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FColorPicker(label, &val, offset, flags);
		FSet(var, val);
		return bReturn;
	}
	__inline bool FColorPicker(const char* label, ConfigVar<Gradient_t>& var, bool start = true, int offset = 0, int flags = 0)
	{
		auto val = FGet(var, true);
		const bool bReturn = FColorPicker(label, start ? &val.StartColor : &val.EndColor, offset, flags);
		FSet(var, val);
		return bReturn;
	}
}