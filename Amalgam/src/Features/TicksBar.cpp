#include "TicksBar.h"
#include <imgui.h>

TicksBar::TicksBar()
    : Module("TicksBar", "Shows a TICKS: [red bar] READY status", Category::HUD)
{
}

void TicksBar::SetProgress(float p) {
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    progress = p;
}

void TicksBar::SetReady(bool r) {
    ready = r;
}

void TicksBar::OnRender() {
    // ---- Colors ----
    const ImColor white   (1.0f, 1.0f, 1.0f, 1.0f);
    const ImColor red     (1.0f, 0.0f, 0.0f, 1.0f);
    const ImColor green   (0.0f, 1.0f, 0.0f, 1.0f);
    const ImColor barBg   (0.15f, 0.15f, 0.15f, 1.0f);

    // ---- Layout constants ----
    const char*  labelText = "TICKS:";
    const char*  stateText = ready ? "READY" : "WAIT";
    const ImVec2 labelSize = ImGui::CalcTextSize(labelText);
    const ImVec2 stateSize = ImGui::CalcTextSize(stateText);

    const float barWidth   = 120.0f;
    const float barHeight  = 14.0f;
    const float spacing    = 8.0f;

    const float totalW = labelSize.x + spacing + barWidth + spacing + stateSize.x;
    const float totalH = (barHeight > labelSize.y ? barHeight : labelSize.y);

    // ---- Position (top-left of the element) ----
    ImVec2 pos = ImVec2(20.0f, 20.0f);   // <-- hook this up to your settings/position system
    ImVec2 cursor = pos;

    // ---- "TICKS:" in white ----
    ImGui::GetBackgroundDrawList()->AddText(cursor, white, labelText);
    cursor.x += labelSize.x + spacing;

    // ---- Red progress bar ----
    ImVec2 barMin = cursor;
    ImVec2 barMax = ImVec2(cursor.x + barWidth, cursor.y + barHeight);

    // Background of the bar
    ImGui::GetBackgroundDrawList()->AddRectFilled(barMin, barMax, barBg, 2.0f);

    // Red fill (scaled by progress)
    ImVec2 fillMax = ImVec2(barMin.x + barWidth * progress, barMax.y);
    ImGui::GetBackgroundDrawList()->AddRectFilled(barMin, fillMax, red, 2.0f);

    // Outline
    ImGui::GetBackgroundDrawList()->AddRect(barMin, barMax, ImColor(0,0,0,180), 2.0f, 0, 1.0f);

    cursor.x += barWidth + spacing;

    // ---- "READY" / "WAIT" text, vertically centered against the bar ----
    float textY = barMin.y + (barHeight - stateSize.y) * 0.5f;
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(cursor.x, textY), ready ? green : red, stateText);
}
