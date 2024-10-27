#include "Render.h"

#include "../../Hooks/Direct3DDevice9.h"
#include <ImGui/imgui_impl_win32.h>
#include "MaterialDesign/MaterialIcons.h"
#include "MaterialDesign/IconDefinitions.h"
#include "Menu/Menu.h"

void CRender::Render(IDirect3DDevice9* pDevice)
{
	using namespace ImGui;

	static std::once_flag initFlag;
	std::call_once(initFlag, [&]
		{
			Initialize(pDevice);
		});

	DWORD dwOldRGB; pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &dwOldRGB);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	NewFrame();

	LoadColors();

	F::Menu.Render();

	EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(GetDrawData());
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, dwOldRGB);
}

void CRender::LoadColors()
{
	using namespace ImGui;

	auto ColorToVec = [](Color_t color) -> ImColor
		{
			return { float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f, float(color.a) / 255.f };
		};

	Accent = ColorToVec(Vars::Menu::Theme::Accent.Value);
	AccentLight = ImColor(Accent.Value.x * 1.1f, Accent.Value.y * 1.1f, Accent.Value.z * 1.1f, Accent.Value.w);
	Background = ColorToVec(Vars::Menu::Theme::Background.Value.Lerp({ 127, 127, 127, Vars::Menu::Theme::Background.Value.a }, 1.f / 9));
	Foreground = ColorToVec(Vars::Menu::Theme::Background.Value);
	Foremost = ColorToVec(Vars::Menu::Theme::Background.Value.Lerp({ 127, 127, 127, Vars::Menu::Theme::Background.Value.a }, 2.f / 9));
	ForemostLight = ImColor(Foremost.Value.x * 1.1f, Foremost.Value.y * 1.1f, Foremost.Value.z * 1.1f, Foremost.Value.w);
	Inactive = ColorToVec(Vars::Menu::Theme::Inactive.Value);
	Active = ColorToVec(Vars::Menu::Theme::Active.Value);

	ImVec4* colors = GetStyle().Colors;
	colors[ImGuiCol_Button] = {};
	colors[ImGuiCol_ButtonHovered] = {};
	colors[ImGuiCol_ButtonActive] = {};
	colors[ImGuiCol_FrameBg] = Foremost;
	colors[ImGuiCol_FrameBgHovered] = ForemostLight;
	colors[ImGuiCol_FrameBgActive] = Foremost;
	colors[ImGuiCol_Header] = {};
	colors[ImGuiCol_HeaderHovered] = { ForemostLight.Value.x * 1.1f, ForemostLight.Value.y * 1.1f, ForemostLight.Value.z * 1.1f, Foremost.Value.w }; // divd by 1.1
	colors[ImGuiCol_HeaderActive] = Foremost;
	colors[ImGuiCol_ModalWindowDimBg] = { Background.Value.x, Background.Value.y, Background.Value.z, 0.4f };
	colors[ImGuiCol_PopupBg] = ForemostLight;
	colors[ImGuiCol_ResizeGrip] = {};
	colors[ImGuiCol_ResizeGripActive] = {};
	colors[ImGuiCol_ResizeGripHovered] = {};
	colors[ImGuiCol_ScrollbarBg] = {};
	colors[ImGuiCol_SliderGrab] = Accent;
	colors[ImGuiCol_SliderGrabActive] = AccentLight;
	colors[ImGuiCol_Text] = Active;
	colors[ImGuiCol_WindowBg] = Foreground;
}

void CRender::LoadStyle()
{
	using namespace ImGui;

	auto& style = GetStyle();
	style.ButtonTextAlign = { 0.5f, 0.5f }; // Center button text
	style.CellPadding = { 4, 0 };
	style.ChildBorderSize = 0.f;
	style.ChildRounding = 0.f;
	style.FrameBorderSize = 0.f;
	style.FramePadding = { 0, 0 };
	style.FrameRounding = 3.f;
	style.ItemInnerSpacing = { 0, 0 };
	style.ItemSpacing = { 8, 8 };
	style.PopupBorderSize = 0.f;
	style.PopupRounding = 3.f;
	style.ScrollbarSize = 9.f;
	style.ScrollbarRounding = 0.f;
	style.WindowBorderSize = 0.f;
	style.WindowMinSize = { 100, 100 };
	style.WindowPadding = { 0, 0 };
	style.WindowRounding = 3.f;
}

void CRender::Initialize(IDirect3DDevice9* pDevice)
{
	// Initialize ImGui and device
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(WndProc::hwWindow);
	ImGui_ImplDX9_Init(pDevice);

	// Fonts
	{
		const auto& io = ImGui::GetIO();

		ImFontConfig fontConfig;
		fontConfig.OversampleH = 2;
		constexpr ImWchar fontRange[]{ 0x0020, 0x00FF, 0x0400, 0x044F, 0 }; // Basic Latin, Latin Supplement and Cyrillic

		FontSmall = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", 11.f, &fontConfig, fontRange);
		FontRegular = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", 13.f, &fontConfig, fontRange);
		FontBold = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdanab.ttf)", 13.f, &fontConfig, fontRange);
		FontLarge = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", 14.f, &fontConfig, fontRange);
		FontMono = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", 15.f, &fontConfig, fontRange);

		//FontSmall = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 11.f, &fontConfig, fontRange);
		//FontRegular = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 13.f, &fontConfig, fontRange);
		//FontBold = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBold_compressed_data, RobotoBold_compressed_size, 13.f, &fontConfig, fontRange);
		//FontLarge = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 15.f, &fontConfig, fontRange);
		//FontMono = io.Fonts->AddFontFromMemoryCompressedTTF(CascadiaMono_compressed_data, CascadiaMono_compressed_size, 15.f, &fontConfig, fontRange);

		ImFontConfig iconConfig;
		iconConfig.PixelSnapH = true;
		constexpr ImWchar iconRange[]{ short(ICON_MIN_MD), short(ICON_MAX_MD), 0 };

		IconFontRegular = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data, MaterialIcons_compressed_size, 15.f, &iconConfig, iconRange);
		IconFontLarge = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data, MaterialIcons_compressed_size, 16.f, &iconConfig, iconRange);

		io.Fonts->Build();
	}

	LoadStyle();
}