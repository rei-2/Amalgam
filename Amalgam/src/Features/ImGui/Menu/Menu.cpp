#include "Menu.h"

#include "Components.h"
#include "../../Configs/Configs.h"
#include "../../Conditions/Conditions.h"
#include "../../Players/PlayerUtils.h"
#include "../../CameraWindow/CameraWindow.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"
#include "../../Resolver/Resolver.h"
#include <mutex>

/* The main menu */
void CMenu::DrawMenu()
{
	using namespace ImGui;

	ImVec2 mainWindowPos = {};
	ImVec2 mainWindowSize = {};

	SetNextWindowSize(ImVec2(750, 500), ImGuiCond_FirstUseEver);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { 750, 500 });
	if (Begin("MainWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
	{
		const auto windowPos = mainWindowPos = GetWindowPos();
		const auto windowSize = mainWindowSize = GetWindowSize();

		// Main tabs
		FTabs({ "AIMBOT", "VISUALS", "MISC", "LOGS", "SETTINGS" }, &CurrentTab, TabSize, { 0, SubTabSize.y }, true, { ICON_MD_GROUP, ICON_MD_IMAGE, ICON_MD_PUBLIC, ICON_MD_MENU_BOOK, ICON_MD_SETTINGS });

		// Sub tabs
		switch (CurrentTab)
		{
		case 0: FTabs({ "GENERAL", "HVH" }, &CurrentAimbotTab, SubTabSize, { TabSize.x, 0 }); break;
		case 1: FTabs({ "ESP", "CHAMS", "GLOW", "MISC##", "RADAR", "MENU" }, &CurrentVisualsTab, SubTabSize, { TabSize.x, 0 }); break;
		case 2: FTabs({ "MISC##" }, nullptr, SubTabSize, { TabSize.x, 0 }); break;
		case 3: FTabs({ "LOGS##", "SETTINGS##" }, &CurrentLogsTab, SubTabSize, { TabSize.x, 0 }); break;
		case 4: FTabs({ "CONFIG", "CONDITIONS", "PLAYERLIST", "MATERIALS" }, &CurrentConfigTab, SubTabSize, { TabSize.x, 0 }); break;
		}

		// Main content
		SetCursorPos({ TabSize.x, SubTabSize.y });
		PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.f, 8.f });
		PushStyleColor(ImGuiCol_ChildBg, {});
		if (BeginChild("Content", { windowSize.x - TabSize.x, windowSize.y - SubTabSize.y }, false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			PushStyleColor(ImGuiCol_ChildBg, F::Render.Foreground.Value);
			PushStyleVar(ImGuiStyleVar_ChildRounding, 3.f);

			switch (CurrentTab)
			{
			case 0: MenuAimbot(); break;
			case 1: MenuVisuals(); break;
			case 2: MenuMisc(); break;
			case 3: MenuLogs(); break;
			case 4: MenuSettings(); break;
			}

			PopStyleVar();
			PopStyleColor();
		} EndChild();
		PopStyleColor();
		PopStyleVar();

		// End
		End();
	}
	PopStyleVar();

	// Title Text
	if (Vars::Menu::CheatName.Value.length())
	{
		PushFont(F::Render.FontTitle);
		const auto textSize = CalcTextSize(Vars::Menu::CheatName.Value.c_str());
		SetNextWindowSize({ std::min(textSize.x + 26.f, mainWindowSize.x), 40.f });
		SetNextWindowPos({ mainWindowPos.x, mainWindowPos.y - 48.f });
		PushStyleVar(ImGuiStyleVar_WindowMinSize, { 40.f, 40.f });
		if (Begin("TitleWindow", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing))
		{
			const auto windowPos = GetWindowPos();
			GetWindowDrawList()->AddText(F::Render.FontTitle, F::Render.FontTitle->FontSize, { windowPos.x + 13.f, windowPos.y + 10.f }, F::Render.Accent, Vars::Menu::CheatName.Value.c_str());

			End();
		}
		PopStyleVar();
		PopFont();
	}

	// Condition Text
	if (FNV1A::Hash(sCondition.c_str()) != FNV1A::HashConst("default"))
	{
		const auto textSize = CalcTextSize(std::format("Editing for condition {}", sCondition).c_str());
		SetNextWindowSize({ std::min(textSize.x + 56.f, mainWindowSize.x), 40.f });
		SetNextWindowPos({ mainWindowPos.x, mainWindowPos.y + mainWindowSize.y + 8.f });
		PushStyleVar(ImGuiStyleVar_WindowMinSize, { 40.f, 40.f });
		if (Begin("ConditionWindow", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
		{
			const auto windowPos = GetWindowPos();
			const auto preSize = CalcTextSize("Editing for condition ");
			GetWindowDrawList()->AddText(F::Render.FontRegular, F::Render.FontRegular->FontSize, { windowPos.x + 16.f, windowPos.y + 13.f }, F::Render.Active, "Editing for condition ");
			GetWindowDrawList()->AddText(F::Render.FontRegular, F::Render.FontRegular->FontSize, { windowPos.x + 16.f + preSize.x, windowPos.y + 13.f }, F::Render.Accent, sCondition.c_str());

			SetCursorPos({ textSize.x + 28, 11 });
			if (IconButton(ICON_MD_CANCEL))
				sCondition = "default";

			End();
		}
		PopStyleVar();
	}
}

#pragma region Tabs
/* Tab: Aimbot */
void CMenu::MenuAimbot()
{
	using namespace ImGui;

	switch (CurrentAimbotTab)
	{
		// General
	case 0:
		if (BeginTable("AimbotTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("General"))
			{
				FDropdown("Aim type", Vars::Aimbot::General::AimType, { "Off", "Plain", "Smooth", "Silent" }, {}, FDropdown_Left);
				FDropdown("Target selection", Vars::Aimbot::General::TargetSelection, { "FOV", "Distance" }, {}, FDropdown_Right);
				FDropdown("Target", Vars::Aimbot::General::Target, { "Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Ignore", Vars::Aimbot::General::Ignore, { "Invulnerable", "Cloaked", "Dead Ringer", "Vaccinator", "Unsimulated Players", "Disguised", "Taunting" }, {}, FDropdown_Multi | FDropdown_Right);
				FSlider("Aim FOV", Vars::Aimbot::General::AimFOV, 1.f, 180.f, 1.f, "%.0f", FSlider_Clamp);
				bTransparent = FGet(Vars::Aimbot::General::AimType) != 2;
					FSlider("Smoothing## Hitscan", Vars::Aimbot::General::Smoothing, 0.f, 100.f, 1.f, "%.0f%%", FSlider_Clamp);
				bTransparent = false;
				FSlider("Max targets", Vars::Aimbot::General::MaxTargets, 1, 6, 1, "%d", FSlider_Clamp);
				bTransparent = !(FGet(Vars::Aimbot::General::Ignore) & 1 << 1);
					FSlider("Ignore cloak", Vars::Aimbot::General::IgnoreCloakPercentage, 0, 100, 10, "%d%%", FSlider_Clamp);
				bTransparent = !(FGet(Vars::Aimbot::General::Ignore) & 1 << 4);
					FSlider("Tick tolerance", Vars::Aimbot::General::TickTolerance, 0, 21, 1, "%d", FSlider_Clamp);
				bTransparent = false;
				FColorPicker("Aimbot FOV circle", Vars::Colors::FOVCircle);
				FToggle("Autoshoot", Vars::Aimbot::General::AutoShoot);
				FToggle("FOV Circle", Vars::Aimbot::General::FOVCircle, FToggle_Middle);
				FToggle("Force crits", Vars::CritHack::ForceCrits);
				FToggle("Avoid random crits", Vars::CritHack::AvoidRandom, FToggle_Middle);
				FToggle("Always melee", Vars::CritHack::AlwaysMelee);
				FToggle("No spread", Vars::Aimbot::General::NoSpread, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## aimbot"))
				{
					FSlider("hitscan peek", Vars::Aimbot::General::HitscanPeek, 0, 10);
					FSlider("offset## nospread", Vars::Aimbot::General::NoSpreadOffset, -5.f, 5.f, 1.f, "%.1f", FSlider_Precision);
					FSlider("average", Vars::Aimbot::General::NoSpreadAverage, 1, 25);
				} EndSection();
			}
			if (Section("Backtrack"))
			{
				FToggle("Enabled", Vars::Backtrack::Enabled);
				FToggle("Prefer on shot", Vars::Backtrack::PreferOnShot, FToggle_Middle);
				FSlider("Fake latency", Vars::Backtrack::Latency, 0, F::Backtrack.flMaxUnlag * 1000, 5, "%d", FSlider_Clamp); // unreliable above 1000 - ping probably
				FSlider("Fake interp", Vars::Backtrack::Interp, 0, F::Backtrack.flMaxUnlag * 1000, 5, "%d", FSlider_Clamp);
				FSlider("Window", Vars::Backtrack::Window, 1, 200, 5, "%d", FSlider_Clamp);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## backtrack"))
				{
					FSlider("offset", Vars::Backtrack::Offset, -5, 5);
				} EndSection();
			}

			/* Column 2 */
			TableNextColumn();
			if (Section("Hitscan"))
			{
				FDropdown("Hitboxes", Vars::Aimbot::Hitscan::Hitboxes, { "Head", "Body", "Pelvis", "Arms", "Legs" }, { 1 << 0, 1 << 2, 1 << 1, 1 << 3, 1 << 4 }, FDropdown_Multi);
				FDropdown("Modifiers## Hitscan", Vars::Aimbot::Hitscan::Modifiers, { "Tapfire", "Wait for heatshot", "Wait for charge", "Scoped only", "Auto scope", "Bodyaim if lethal", "Extinguish team" }, {}, FDropdown_Multi);
				FSlider("Point scale", Vars::Aimbot::Hitscan::PointScale, 0.f, 100.f, 5.f, "%.0f%%", FSlider_Clamp | FSlider_Precision);
				bTransparent = !(FGet(Vars::Aimbot::Hitscan::Modifiers) & 1 << 0);
					FSlider("Tapfire distance", Vars::Aimbot::Hitscan::TapFireDist, 250.f, 1000.f, 50.f, "%.0f", FSlider_Clamp | FSlider_Precision);
				bTransparent = false;
			} EndSection();
			if (Section("Projectile"))
			{
				FDropdown("Predict", Vars::Aimbot::Projectile::StrafePrediction, { "Air strafing", "Ground strafing" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Splash", Vars::Aimbot::Projectile::SplashPrediction, { "Off", "Include", "Prefer", "Only" }, {}, FDropdown_Right);
				FDropdown("Auto detonate", Vars::Aimbot::Projectile::AutoDetonate, { "Stickies", "Flares" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Auto airblast", Vars::Aimbot::Projectile::AutoAirblast, { "Off", "Legit", "Rage" }, {}, FDropdown_Right);
				FDropdown("Modifiers## Projectile", Vars::Aimbot::Projectile::Modifiers, { "Charge shot", "Cancel charge", "Bodyaim if lethal" }, {}, FDropdown_Multi);
				FSlider("Max simulation time", Vars::Aimbot::Projectile::PredictionTime, 0.1f, 10.f, 0.1f, "%.1fs");
				bTransparent = !FGet(Vars::Aimbot::Projectile::StrafePrediction);
					FSlider("Hit chance", Vars::Aimbot::Projectile::Hitchance, 0.f, 100.f, 5.f, "%.0f%%", FSlider_Clamp | FSlider_Precision);
				bTransparent = false;
				FSlider("Autodet radius", Vars::Aimbot::Projectile::AutodetRadius, 0.f, 100.f, 5.f, "%.0f%%", FSlider_Clamp | FSlider_Precision);
				FSlider("Splash radius", Vars::Aimbot::Projectile::SplashRadius, 0.f, 100.f, 5.f, "%.0f%%", FSlider_Clamp | FSlider_Precision);
				bTransparent = !FGet(Vars::Aimbot::Projectile::AutoRelease);
					FSlider("Auto release", Vars::Aimbot::Projectile::AutoRelease, 0.f, 100.f, 5.f, "%.0f%%", FSlider_Clamp | FSlider_Precision);
				bTransparent = false;
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## projectile"))
				{
					FSlider("ground samples", Vars::Aimbot::Projectile::GroundSamples, 3, 66, 1, "%d", FSlider_Left);
					FSlider("air samples", Vars::Aimbot::Projectile::AirSamples, 3, 66, 1, "%d", FSlider_Right);
					FSlider("vertical shift", Vars::Aimbot::Projectile::VerticalShift, 0.f, 20.f, 0.5f, "%.1f", FSlider_Left);
					FSlider("huntsman lerp", Vars::Aimbot::Projectile::HuntsmanLerp, 0.f, 100.f, 1.f, "%.0f%%", FSlider_Right);
					FSlider("latency offset", Vars::Aimbot::Projectile::LatencyOffset, -1.f, 1.f, 0.1f, "%.1f", FSlider_Left);
					FSlider("hull increase", Vars::Aimbot::Projectile::HullIncrease, 0.f, 3.f, 0.5f, "%.1f", FSlider_Right);
					FSlider("drag override", Vars::Aimbot::Projectile::DragOverride, 0.f, 1.f, 0.001f, "%.3f", FSlider_Left);
					FSlider("time override", Vars::Aimbot::Projectile::TimeOverride, 0.f, 1.f, 0.001f, "%.3f", FSlider_Right);
					FSlider("splash points", Vars::Aimbot::Projectile::SplashPoints, 0, 100, 1, "%d", FSlider_Left);
					FSlider("splash count", Vars::Aimbot::Projectile::SplashCount, 1, 5, 1, "%d", FSlider_Right);
				} EndSection();
			}
			if (Section("Melee"))
			{
				FToggle("Auto backstab", Vars::Aimbot::Melee::AutoBackstab);
				FToggle("Ignore razorback", Vars::Aimbot::Melee::IgnoreRazorback, FToggle_Middle);
				FToggle("Swing prediction", Vars::Aimbot::Melee::SwingPrediction);
				FToggle("Whip teammates", Vars::Aimbot::Melee::WhipTeam, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## melee"))
					FSlider("swing ticks", Vars::Aimbot::Melee::SwingTicks, 10, 14);
				EndSection();
			}

			EndTable();
		}
		break;
		// HvH
	case 1:
		if (BeginTable("HvHTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Doubletap"))
			{
				FToggle("Doubletap", Vars::CL_Move::Doubletap::Doubletap);
				FToggle("Warp", Vars::CL_Move::Doubletap::Warp, FToggle_Middle);
				FToggle("Recharge ticks", Vars::CL_Move::Doubletap::RechargeTicks);
				FToggle("Anti-warp", Vars::CL_Move::Doubletap::AntiWarp, FToggle_Middle);
				FSlider("Tick limit", Vars::CL_Move::Doubletap::TickLimit, 2, 22, 1, "%d", FSlider_Clamp);
				FSlider("Warp rate", Vars::CL_Move::Doubletap::WarpRate, 2, 22, 1, "%d", FSlider_Clamp);
				FSlider("Passive recharge", Vars::CL_Move::Doubletap::PassiveRecharge, 0, 66, 1, "%d", FSlider_Clamp);
			} EndSection();
			if (Section("Fakelag"))
			{
				FDropdown("Fakelag", Vars::CL_Move::Fakelag::Fakelag, { "Off", "Plain", "Random", "Adaptive" }, {}, FSlider_Left);
				FDropdown("Options", Vars::CL_Move::Fakelag::Options, { "Only moving", "On unduck", "Not airborne" }, {}, FDropdown_Multi | FSlider_Right);
				bTransparent = FGet(Vars::CL_Move::Fakelag::Fakelag) != 1;
					FSlider("Plain ticks", Vars::CL_Move::Fakelag::PlainTicks, 1, 22, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = FGet(Vars::CL_Move::Fakelag::Fakelag) != 2;
					FSlider("Random ticks", Vars::CL_Move::Fakelag::RandomTicks, 1, 22, 1, "%d - %d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
				FToggle("Unchoke on attack", Vars::CL_Move::Fakelag::UnchokeOnAttack);
				FToggle("Retain blastjump", Vars::CL_Move::Fakelag::RetainBlastJump, FToggle_Middle);
			} EndSection();
			if (Section("Anti Aim"))
			{
				FToggle("Enabled", Vars::AntiHack::AntiAim::Enabled);
				FDropdown("Real pitch", Vars::AntiHack::AntiAim::PitchReal, { "None", "Up", "Down", "Zero" }, {}, FDropdown_Left);
				FDropdown("Fake pitch", Vars::AntiHack::AntiAim::PitchFake, { "None", "Up", "Down" }, {}, FDropdown_Right);
				FDropdown("Real yaw", Vars::AntiHack::AntiAim::YawReal, { "Forward", "Left", "Right", "Backwards", "Spin", "Edge" }, {}, FDropdown_Left);
				FDropdown("Fake yaw", Vars::AntiHack::AntiAim::YawFake, { "Forward", "Left", "Right", "Backwards", "Spin", "Edge" }, {}, FDropdown_Right);
				FDropdown("Real offset", Vars::AntiHack::AntiAim::RealYawMode, { "View", "Target" }, {}, FDropdown_Left);
				FDropdown("Fake offset", Vars::AntiHack::AntiAim::FakeYawMode, { "View", "Target" }, {}, FDropdown_Right);
				FSlider("Real offset## Offset", Vars::AntiHack::AntiAim::RealYawOffset, -180, 180, 5, "%d", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				FSlider("Fake offset## Offset", Vars::AntiHack::AntiAim::FakeYawOffset, -180, 180, 5, "%d", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				bTransparent = FGet(Vars::AntiHack::AntiAim::YawFake) != 4 && FGet(Vars::AntiHack::AntiAim::YawReal) != 4;
					FSlider("Spin Speed", Vars::AntiHack::AntiAim::SpinSpeed, -30.f, 30.f, 1.f, "%.0f", FSlider_Left);
				bTransparent = false;
				SetCursorPos({ GetWindowSize().x / 2 + 4, GetCursorPosY() - 24 });
				FToggle("Minwalk", Vars::AntiHack::AntiAim::MinWalk);
				FToggle("Anti-overlap", Vars::AntiHack::AntiAim::AntiOverlap);
				FToggle("Hide pitch on shot", Vars::AntiHack::AntiAim::InvalidShootPitch, FToggle_Middle);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Resolver"))
			{
				FToggle("Enabled", Vars::AntiHack::Resolver::Resolver);
				bTransparent = !FGet(Vars::AntiHack::Resolver::Resolver);
					FToggle("Ignore in-air", Vars::AntiHack::Resolver::IgnoreAirborne, FToggle_Middle);
				bTransparent = false;
			} EndSection();
			if (Section("Auto Peek"))
			{
				FToggle("Auto peek", Vars::CL_Move::AutoPeek);
			} EndSection();
			if (Section("Speedhack"))
			{
				FToggle("Speedhack", Vars::CL_Move::SpeedEnabled);
				bTransparent = !FGet(Vars::CL_Move::SpeedEnabled);
					FSlider("SpeedHack factor", Vars::CL_Move::SpeedFactor, 1, 50, 1);
				bTransparent = false;
			} EndSection();
			if (Section("Cheater Detection"))
			{
				const bool transparent = bTransparent = !FGet(Vars::CheaterDetection::Methods);
				FDropdown("Detection methods", Vars::CheaterDetection::Methods, { "Invalid pitch", "Packet choking", "Aim flicking", "Duck Speed" }, {}, FDropdown_Multi);
				FSlider("Detections required", Vars::CheaterDetection::DetectionsRequired, 10, 50, 1);

				bTransparent = !(FGet(Vars::CheaterDetection::Methods) & 1 << 1) || bTransparent;
					FSlider("Minimum choking", Vars::CheaterDetection::MinimumChoking, 4, 22, 1);
				bTransparent = transparent;

				bTransparent = !(FGet(Vars::CheaterDetection::Methods) & 1 << 2) || bTransparent;
					FSlider("Minimum flick angle", Vars::CheaterDetection::MinimumFlick, 10.f, 30.f, 1.f, "%.0f", FSlider_Left);
					FSlider("Maximum noise", Vars::CheaterDetection::MaximumNoise, 1.f, 10.f, 1.f, "%.0f", FSlider_Right);
				bTransparent = false;
			} EndSection();

			EndTable();
		}
		break;
	}
}

/* Tab: Visuals */
void CMenu::MenuVisuals()
{
	using namespace ImGui;

	switch (CurrentVisualsTab)
	{
		// ESP
	case 0:
		if (BeginTable("VisualsESPTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("ESP"))
			{
				FDropdown("Draw", Vars::ESP::Draw, { "Enemy", "Team", "Local", "Friends", "Priority", "NPCs", "Health", "Ammo", "Money", "Bombs", "Spellbook", "Gargoyle" }, {}, FDropdown_Multi);
				FDropdown("Player", Vars::ESP::Player, { "Name", "Health bar", "Health text", "Uber bar", "Uber text", "Class icon", "Class text", "Weapon icon", "Weapon text", "Distance", "Box", "Bones", "Priority", "Labels", "Buffs", "Debuffs", "Misc", "Lag compensation", "Ping", "KDR" }, {}, FDropdown_Multi);
				FDropdown("Building", Vars::ESP::Building, { "Name", "Health bar", "Health text", "Distance", "Box", "Owner", "Level", "Conditions" }, {}, FDropdown_Multi);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Colors"))
			{
				FToggle("Relative colors", Vars::Colors::Relative);
				if (FGet(Vars::Colors::Relative))
				{
					FColorPicker("Enemy color", Vars::Colors::Enemy, 0, FColorPicker_Left);
					FColorPicker("Team color", Vars::Colors::Team, 0, FColorPicker_Middle | FColorPicker_SameLine);
				}
				else
				{
					FColorPicker("RED color", Vars::Colors::TeamRed, 0, FColorPicker_Left);
					FColorPicker("BLU color", Vars::Colors::TeamBlu, 0, FColorPicker_Middle | FColorPicker_SameLine);
				}

				FColorPicker("Health bar top", Vars::Colors::HealthBar, false, 0, FColorPicker_Left);
				FColorPicker("Health bar bottom", Vars::Colors::HealthBar, true, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Uber bar", Vars::Colors::UberBar, 0, FColorPicker_Left);
				FColorPicker("Invulnerable color", Vars::Colors::Invulnerable, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Overheal color", Vars::Colors::Overheal, 0, FColorPicker_Left);
				FColorPicker("Cloaked color", Vars::Colors::Cloak, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Local color", Vars::Colors::Local, 0, FColorPicker_Left);
				FColorPicker("Target color", Vars::Colors::Target, 0, FColorPicker_Middle | FColorPicker_SameLine);

				FColorPicker("Healthpack color", Vars::Colors::Health, 0, FColorPicker_Left);
				FColorPicker("Ammopack color", Vars::Colors::Ammo, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("NPC color", Vars::Colors::NPC, 0, FColorPicker_Left);
				FColorPicker("Bomb color", Vars::Colors::Bomb, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Money color", Vars::Colors::Money, 0, FColorPicker_Left);
				FColorPicker("Halloween color", Vars::Colors::Halloween, 0, FColorPicker_Middle | FColorPicker_SameLine);

				FSlider("Active alpha", Vars::ESP::ActiveAlpha, 0, 255, 5, "%d", FSlider_Clamp);
				FSlider("Dormant alpha", Vars::ESP::DormantAlpha, 0, 255, 5, "%d", FSlider_Clamp);
				FSlider("Dormant Decay Time", Vars::ESP::DormantTime, 0.015f, 5.0f, 0.1f, "%.1f", FSlider_Left | FSlider_Clamp);
				FToggle("Dormant priority only", Vars::ESP::DormantPriority, FToggle_Middle); Dummy({ 0, 8 });
			} EndSection();

			EndTable();
		}
		break;
		// Chams
	case 1:
		if (BeginTable("VisualsChamsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Friendly"))
			{
				FToggle("Players", Vars::Chams::Friendly::Players);
				FToggle("Ragdolls", Vars::Chams::Friendly::Ragdolls, FToggle_Middle);
				FToggle("Buildings", Vars::Chams::Friendly::Buildings);
				FToggle("Projectiles", Vars::Chams::Friendly::Projectiles, FToggle_Middle);

				FMDropdown("Visible material", Vars::Chams::Friendly::VisibleMaterial, FDropdown_Left, 1);
				FColorPicker("Visible color", Vars::Chams::Friendly::VisibleColor, 0, FColorPicker_Dropdown);
				FMDropdown("Occluded material", Vars::Chams::Friendly::OccludedMaterial, FDropdown_Right, 1);
				FColorPicker("Occluded color", Vars::Chams::Friendly::OccludedColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("Enemy"))
			{
				FToggle("Players", Vars::Chams::Enemy::Players);
				FToggle("Ragdolls", Vars::Chams::Enemy::Ragdolls, FToggle_Middle);
				FToggle("Buildings", Vars::Chams::Enemy::Buildings);
				FToggle("Projectiles", Vars::Chams::Enemy::Projectiles, FToggle_Middle);

				FMDropdown("Visible material", Vars::Chams::Enemy::VisibleMaterial, FDropdown_Left, 1);
				FColorPicker("Visible color", Vars::Chams::Enemy::VisibleColor, 0, FColorPicker_Dropdown);
				FMDropdown("Occluded material", Vars::Chams::Enemy::OccludedMaterial, FDropdown_Right, 1);
				FColorPicker("Occluded color", Vars::Chams::Enemy::OccludedColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("World"))
			{
				FToggle("NPCs", Vars::Chams::World::NPCs);
				FToggle("Pickups", Vars::Chams::World::Pickups, FToggle_Middle);
				FToggle("Bombs", Vars::Chams::World::Bombs);
				FToggle("Halloween", Vars::Chams::World::Halloween, FToggle_Middle);

				FMDropdown("Visible material", Vars::Chams::World::VisibleMaterial, FDropdown_Left, 1);
				FColorPicker("Visible color", Vars::Chams::World::VisibleColor, 0, FColorPicker_Dropdown);
				FMDropdown("Occluded material", Vars::Chams::World::OccludedMaterial, FDropdown_Right, 1);
				FColorPicker("Occluded color", Vars::Chams::World::OccludedColor, 0, FColorPicker_Dropdown);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Player"))
			{
				FToggle("Local", Vars::Chams::Player::Local);
				FToggle("Friend", Vars::Chams::Player::Friend, FToggle_Middle);
				FToggle("Priority", Vars::Chams::Player::Priority);

				FMDropdown("Visible material", Vars::Chams::Player::VisibleMaterial, FDropdown_Left, 1);
				FColorPicker("Visible color", Vars::Chams::Player::VisibleColor, 0, FColorPicker_Dropdown);
				FMDropdown("Occluded material", Vars::Chams::Player::OccludedMaterial, FDropdown_Right, 1);
				FColorPicker("Occluded color", Vars::Chams::Player::OccludedColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("Backtrack"))
			{
				FToggle("Enabled", Vars::Chams::Backtrack::Enabled);
				SameLine(GetWindowSize().x / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
				FDropdown("Draw", Vars::Chams::Backtrack::Draw, { "Last", "Last + first", "All" }, {}, FDropdown_Left);

				FMDropdown("Material", Vars::Chams::Backtrack::VisibleMaterial, FDropdown_None, 1);
				FColorPicker("Color", Vars::Chams::Backtrack::VisibleColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("Fake Angle"))
			{
				FToggle("Enabled", Vars::Chams::FakeAngle::Enabled);

				FMDropdown("Material", Vars::Chams::FakeAngle::VisibleMaterial, FDropdown_None, 1);
				FColorPicker("Color", Vars::Chams::FakeAngle::VisibleColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Weapon", Vars::Chams::Viewmodel::Weapon);
				FToggle("Hands", Vars::Chams::Viewmodel::Hands, FToggle_Middle);

				FMDropdown("Material", Vars::Chams::Viewmodel::VisibleMaterial, FDropdown_None, 1);
				FColorPicker("Color", Vars::Chams::Viewmodel::VisibleColor, 0, FColorPicker_Dropdown);
			} EndSection();

			EndTable();
		}
		break;
		// Glow
	case 2:
		if (BeginTable("VisualsGlowTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Friendly"))
			{
				FToggle("Players", Vars::Glow::Friendly::Players);
				FToggle("Ragdolls", Vars::Glow::Friendly::Ragdolls, FToggle_Middle);
				FToggle("Buildings", Vars::Glow::Friendly::Buildings);
				FToggle("Projectiles", Vars::Glow::Friendly::Projectiles, FToggle_Middle);
				Dummy({ 0, 8 });

				FToggle("Stencil", Vars::Glow::Friendly::Stencil);
				FToggle("Blur", Vars::Glow::Friendly::Blur, FToggle_Middle);
				bTransparent = !FGet(Vars::Glow::Friendly::Stencil);
					FSlider("Stencil scale## Friendly", Vars::Glow::Friendly::StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !FGet(Vars::Glow::Friendly::Blur);
					FSlider("Blur scale## Friendly", Vars::Glow::Friendly::BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Enemy"))
			{
				FToggle("Players", Vars::Glow::Enemy::Players);
				FToggle("Ragdolls", Vars::Glow::Enemy::Ragdolls, FToggle_Middle);
				FToggle("Buildings", Vars::Glow::Enemy::Buildings);
				FToggle("Projectiles", Vars::Glow::Enemy::Projectiles, FToggle_Middle);
				Dummy({ 0, 8 });

				FToggle("Stencil", Vars::Glow::Enemy::Stencil);
				FToggle("Blur", Vars::Glow::Enemy::Blur, FToggle_Middle);
				bTransparent = !FGet(Vars::Glow::Enemy::Stencil);
					FSlider("Stencil scale## Enemy", Vars::Glow::Enemy::StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !FGet(Vars::Glow::Enemy::Blur);
					FSlider("Blur scale## Enemy", Vars::Glow::Enemy::BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("World"))
			{
				FToggle("NPCs", Vars::Glow::World::NPCs);
				FToggle("Pickups", Vars::Glow::World::Pickups, FToggle_Middle);
				FToggle("Bombs", Vars::Glow::World::Bombs);
				FToggle("Halloween", Vars::Glow::World::Halloween, FToggle_Middle);
				Dummy({ 0, 8 });

				FToggle("Stencil", Vars::Glow::World::Stencil);
				FToggle("Blur", Vars::Glow::World::Blur, FToggle_Middle);
				bTransparent = !FGet(Vars::Glow::World::Stencil);
					FSlider("Stencil scale## World", Vars::Glow::World::StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !FGet(Vars::Glow::World::Blur);
					FSlider("Blur scale## World", Vars::Glow::World::BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Player"))
			{
				FToggle("Local", Vars::Glow::Player::Local);
				FToggle("Friend", Vars::Glow::Player::Friend, FToggle_Middle);
				FToggle("Priority", Vars::Glow::Player::Priority);
				Dummy({ 0, 8 });

				FToggle("Stencil", Vars::Glow::Player::Stencil);
				FToggle("Blur", Vars::Glow::Player::Blur, FToggle_Middle);
				bTransparent = !FGet(Vars::Glow::Player::Stencil);
					FSlider("Stencil scale## Player", Vars::Glow::Player::StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !FGet(Vars::Glow::Player::Blur);
					FSlider("Blur scale## Player", Vars::Glow::Player::BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Backtrack"))
			{
				FToggle("Enabled", Vars::Glow::Backtrack::Enabled);
				SameLine(GetWindowSize().x / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
				FDropdown("Draw", Vars::Glow::Backtrack::Draw, { "Last", "Last + first", "All" }, {}, FDropdown_Left);
				Dummy({ 0, 8 });

				FToggle("Stencil", Vars::Glow::Backtrack::Stencil);
				FToggle("Blur", Vars::Glow::Backtrack::Blur, FToggle_Middle);
				bTransparent = !FGet(Vars::Glow::Backtrack::Stencil);
					FSlider("Stencil scale## Backtrack", Vars::Glow::Backtrack::StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !FGet(Vars::Glow::Backtrack::Blur);
					FSlider("Blur scale## Backtrack", Vars::Glow::Backtrack::BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Fake Angle"))
			{
				FToggle("Enabled", Vars::Glow::FakeAngle::Enabled);
				Dummy({ 0, 8 });

				FToggle("Stencil", Vars::Glow::FakeAngle::Stencil);
				FToggle("Blur", Vars::Glow::FakeAngle::Blur, FToggle_Middle);
				bTransparent = !FGet(Vars::Glow::FakeAngle::Stencil);
					FSlider("Stencil scale## FakeAngle", Vars::Glow::FakeAngle::StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !FGet(Vars::Glow::FakeAngle::Blur);
					FSlider("Blur scale## FakeAngle", Vars::Glow::FakeAngle::BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Weapon", Vars::Glow::Viewmodel::Weapon);
				FToggle("Hands", Vars::Glow::Viewmodel::Hands, FToggle_Middle);
				Dummy({ 0, 8 });

				FToggle("Stencil", Vars::Glow::Viewmodel::Stencil);
				FToggle("Blur", Vars::Glow::Viewmodel::Blur, FToggle_Middle);
				bTransparent = !FGet(Vars::Glow::Viewmodel::Stencil);
					FSlider("Stencil scale## Viewmodel", Vars::Glow::Viewmodel::StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !FGet(Vars::Glow::Viewmodel::Blur);
					FSlider("Blur scale## Viewmodel", Vars::Glow::Viewmodel::BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();

			EndTable();
		}
		break;
		// Misc
	case 3:
		if (BeginTable("VisualsMiscTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Removals"))
			{
				FToggle("Scope", Vars::Visuals::Removals::Scope);
				FToggle("Interpolation", Vars::Visuals::Removals::Interpolation, FToggle_Middle);
				FToggle("Disguises", Vars::Visuals::Removals::Disguises);
				FToggle("Screen overlays", Vars::Visuals::Removals::ScreenOverlays, FToggle_Middle);
				FToggle("Taunts", Vars::Visuals::Removals::Taunts);
				FToggle("Screen effects", Vars::Visuals::Removals::ScreenEffects, FToggle_Middle);
				FToggle("View punch", Vars::Visuals::Removals::ViewPunch);
				FToggle("Angle forcing", Vars::Visuals::Removals::AngleForcing, FToggle_Middle);
				FToggle("MOTD", Vars::Visuals::Removals::MOTD);
				FToggle("Convar queries", Vars::Visuals::Removals::ConvarQueries, FToggle_Middle);
				FToggle("Post processing", Vars::Visuals::Removals::PostProcessing);
				FToggle("DSP", Vars::Visuals::Removals::DSP, FToggle_Middle);
			} EndSection();
			if (Section("UI"))
			{
				FSlider("Field of view", Vars::Visuals::UI::FieldOfView, 0, 160, 1, "%d");
				FSlider("Zoomed field of view", Vars::Visuals::UI::ZoomFieldOfView, 0, 160, 1, "%d");
				FToggle("Reveal scoreboard", Vars::Visuals::UI::RevealScoreboard);
				FToggle("Scoreboard playerlist", Vars::Visuals::UI::ScoreboardPlayerlist, FToggle_Middle);
				FToggle("Scoreboard colors", Vars::Visuals::UI::ScoreboardColors);
				FToggle("Clean screenshots", Vars::Visuals::UI::CleanScreenshots, FToggle_Middle);
				FToggle("Sniper sightlines", Vars::Visuals::UI::SniperSightlines);
				FToggle("Pickup timers", Vars::Visuals::UI::PickupTimers, FToggle_Middle);
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Crosshair aim position", Vars::Visuals::Viewmodel::CrosshairAim);
				FToggle("Viewmodel aim position", Vars::Visuals::Viewmodel::ViewmodelAim, FToggle_Middle);
				FSlider("Offset X", Vars::Visuals::Viewmodel::OffsetX, -45, 45, 5, "%d", FSlider_Precision);
				FSlider("Offset Y", Vars::Visuals::Viewmodel::OffsetY, -45, 45, 5, "%d", FSlider_Precision);
				FSlider("Offset Z", Vars::Visuals::Viewmodel::OffsetZ, -45, 45, 5, "%d", FSlider_Precision);
				FSlider("Roll", Vars::Visuals::Viewmodel::Roll, -180, 180, 5, "%d", FSlider_Clamp | FSlider_Precision);
				FToggle("Sway", Vars::Visuals::Viewmodel::Sway);
				bTransparent = !FGet(Vars::Visuals::Viewmodel::Sway);
					FSlider("Sway scale", Vars::Visuals::Viewmodel::SwayScale, 0.01f, 5.f, 0.1f, "%.1f", FSlider_Left);
					FSlider("Sway interp", Vars::Visuals::Viewmodel::SwayInterp, 0.01f, 1.f, 0.1f, "%.1f", FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Tracers"))
			{
				FSDropdown("Bullet trail", Vars::Visuals::Tracers::ParticleTracer, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink", "Line", "Beam" }, FSDropdown_Custom | FDropdown_Left);
				FSDropdown("Crit trail", Vars::Visuals::Tracers::ParticleTracerCrits, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink", "Line", "Beam" }, FSDropdown_Custom | FDropdown_Right);
			} EndSection();
			if (Section("Ragdolls"))
			{
				FToggle("No ragdolls", Vars::Visuals::Ragdolls::NoRagdolls);
				FToggle("No gibs", Vars::Visuals::Ragdolls::NoGib, FToggle_Middle);
				FToggle("Mods", Vars::Visuals::Ragdolls::Enabled);
				bTransparent = !FGet(Vars::Visuals::Ragdolls::Enabled);
					FToggle("Enemy only", Vars::Visuals::Ragdolls::EnemyOnly, FToggle_Middle);
					FDropdown("Ragdoll effects", Vars::Visuals::Ragdolls::Effects, { "Burning", "Electrocuted", "Ash", "Dissolve" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Ragdoll model", Vars::Visuals::Ragdolls::Type, { "None", "Gold", "Ice" }, {}, FDropdown_Right);
					FSlider("Ragdoll force", Vars::Visuals::Ragdolls::Force, -10.f, 10.f, 0.5f, "%.1f", FSlider_Precision);
					FSlider("Horizontal force", Vars::Visuals::Ragdolls::ForceHorizontal, -10.f, 10.f, 0.5f, "%.1f", FSlider_Precision);
					FSlider("Vertical force", Vars::Visuals::Ragdolls::ForceVertical, -10.f, 10.f, 0.5f, "%.1f", FSlider_Precision);
				bTransparent = false;
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Bullet"))
			{
				FColorPicker("Bullet tracer color", Vars::Colors::BulletTracer);
				FToggle("Bullet tracers", Vars::Visuals::Bullet::BulletTracer);
			} EndSection();
			if (Section("Simulation"))
			{
				FColorPicker("Prediction line color", Vars::Colors::PredictionColor, 1); FColorPicker("Projectile line color", Vars::Colors::ProjectileColor);
				FToggle("Enabled", Vars::Visuals::Simulation::Enabled);
				FToggle("Timed", Vars::Visuals::Simulation::Timed, FToggle_Middle);
				FToggle("Seperators", Vars::Visuals::Simulation::Separators);
				bTransparent = !FGet(Vars::Visuals::Simulation::Separators);
					FSlider("Seperator length", Vars::Visuals::Simulation::SeparatorLength, 2, 16, 1, "%d", FSlider_Left);
					FSlider("Seperator spacing", Vars::Visuals::Simulation::SeparatorSpacing, 1, 16, 1, "%d", FSlider_Right);
				bTransparent = false;
				FColorPicker("Clipped line color", Vars::Colors::ClippedColor);
				FToggle("Projectile trajectory", Vars::Visuals::Simulation::ProjectileTrajectory);
				FToggle("Projectile camera", Vars::Visuals::Simulation::ProjectileCamera, FToggle_Middle);
				FToggle("Trajectory on shot", Vars::Visuals::Simulation::TrajectoryOnShot);
				FToggle("Swing prediction lines", Vars::Visuals::Simulation::SwingLines, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug"))
				{
					FToggle("overwrite", Vars::Visuals::Trajectory::Overwrite);
					FSlider("off x", Vars::Visuals::Trajectory::OffX, -25.f, 25.f, 0.5f, "%.1f", FSlider_Precision);
					FSlider("off y", Vars::Visuals::Trajectory::OffY, -25.f, 25.f, 0.5f, "%.1f", FSlider_Precision);
					FSlider("off z", Vars::Visuals::Trajectory::OffZ, -25.f, 25.f, 0.5f, "%.1f", FSlider_Precision);
					FToggle("pipes", Vars::Visuals::Trajectory::Pipes);
					FSlider("hull", Vars::Visuals::Trajectory::Hull, 0.f, 10.f, 0.5f, "%.1f", FSlider_Precision);
					FSlider("speed", Vars::Visuals::Trajectory::Speed, 0.f, 5000.f, 50.f, "%.0f", FSlider_Precision);
					FSlider("gravity", Vars::Visuals::Trajectory::Gravity, 0.f, 2.f, 0.1f, "%.1f", FSlider_Precision);
					FToggle("no spin", Vars::Visuals::Trajectory::NoSpin);
					FSlider("lifetime", Vars::Visuals::Trajectory::LifeTime, 0.f, 10.f, 0.1f, "%.1f", FSlider_Precision);
					FSlider("up vel", Vars::Visuals::Trajectory::UpVelocity, 0.f, 1000.f, 50.f, "%.0f", FSlider_Precision);
					FSlider("ang vel x", Vars::Visuals::Trajectory::AngVelocityX, -1000.f, 1000.f, 50.f, "%.0f", FSlider_Precision);
					FSlider("ang vel y", Vars::Visuals::Trajectory::AngVelocityY, -1000.f, 1000.f, 50.f, "%.0f", FSlider_Precision);
					FSlider("ang vel z", Vars::Visuals::Trajectory::AngVelocityZ, -1000.f, 1000.f, 50.f, "%.0f", FSlider_Precision);
					FSlider("drag", Vars::Visuals::Trajectory::Drag, 0.f, 2.f, 0.1f, "%.1f", FSlider_Precision);
					FSlider("drag x", Vars::Visuals::Trajectory::DragBasisX, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("drag y", Vars::Visuals::Trajectory::DragBasisY, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("drag z", Vars::Visuals::Trajectory::DragBasisZ, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("ang drag x", Vars::Visuals::Trajectory::AngDragBasisX, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("ang drag y", Vars::Visuals::Trajectory::AngDragBasisY, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("ang drag z", Vars::Visuals::Trajectory::AngDragBasisZ, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("max vel", Vars::Visuals::Trajectory::MaxVelocity, 0.f, 4000.f, 50.f, "%.0f", FSlider_Precision);
					FSlider("max ang vel", Vars::Visuals::Trajectory::MaxAngularVelocity, 0.f, 7200.f, 50.f, "%.0f", FSlider_Precision);
				} EndSection();
			}
			if (Section("Hitbox"))
			{
				FColorPicker("Edge color", Vars::Colors::HitboxEdge, 1); FColorPicker("Face color", Vars::Colors::HitboxFace);
				FToggle("Draw Hitboxes", Vars::Visuals::Hitbox::ShowHitboxes);
			} EndSection();
			if (Section("Thirdperson"))
			{
				FToggle("Thirdperson", Vars::Visuals::ThirdPerson::Enabled);
				FToggle("Thirdperson crosshair", Vars::Visuals::ThirdPerson::Crosshair, FToggle_Middle);
				FSlider("Thirdperson distance", Vars::Visuals::ThirdPerson::Distance, 0.f, 500.f, 5.f, "%.0f", FSlider_Precision);
				FSlider("Thirdperson right", Vars::Visuals::ThirdPerson::Right, -500.f, 500.f, 5.f, "%.0f", FSlider_Precision);
				FSlider("Thirdperson up", Vars::Visuals::ThirdPerson::Up, -500.f, 500.f, 5.f, "%.0f", FSlider_Precision);
			} EndSection();
			if (Section("Out of FOV arrows"))
			{
				FToggle("Enabled", Vars::Visuals::FOVArrows::Enabled);
				FSlider("Offset", Vars::Visuals::FOVArrows::Offset, 0, 500, 25, "%d", FSlider_Precision);
				FSlider("Max distance", Vars::Visuals::FOVArrows::MaxDist, 0.f, 5000.f, 50.f, "%.0f", FSlider_Precision);
			} EndSection();
			if (Section("World"))
			{
				FSDropdown("World texture", Vars::Visuals::World::WorldTexture, { "Default", "Dev", "Camo", "Black", "White", "Flat" }, FSDropdown_Custom);
				FDropdown("Modulations", Vars::Visuals::World::Modulations, { "World", "Sky", "Prop", "Particle", "Fog" }, { }, FDropdown_Left | FDropdown_Multi);
				static std::vector skyNames = {
					"Off", "sky_tf2_04", "sky_upward", "sky_dustbowl_01", "sky_goldrush_01", "sky_granary_01", "sky_well_01", "sky_gravel_01", "sky_badlands_01",
					"sky_hydro_01", "sky_night_01", "sky_nightfall_01", "sky_trainyard_01", "sky_stormfront_01", "sky_morningsnow_01","sky_alpinestorm_01",
					"sky_harvest_01", "sky_harvest_night_01", "sky_halloween", "sky_halloween_night_01", "sky_halloween_night2014_01", "sky_island_01", "sky_rainbow_01"
				};
				FSDropdown("Skybox changer", Vars::Visuals::World::SkyboxChanger, skyNames, FSDropdown_Custom | FDropdown_Right);
				bTransparent = !(FGet(Vars::Visuals::World::Modulations) & 1 << 0);
					FColorPicker("World modulation", Vars::Colors::WorldModulation, 0, FColorPicker_Left);
				bTransparent = !(FGet(Vars::Visuals::World::Modulations) & 1 << 1);
					FColorPicker("Sky modulation", Vars::Colors::SkyModulation, 0, FColorPicker_Middle | FColorPicker_SameLine);
				bTransparent = !(FGet(Vars::Visuals::World::Modulations) & 1 << 2);
					FColorPicker("Prop modulation", Vars::Colors::PropModulation, 0, FColorPicker_Left);
				bTransparent = !(FGet(Vars::Visuals::World::Modulations) & 1 << 3);
					FColorPicker("Particle modulation", Vars::Colors::ParticleModulation, 0, FColorPicker_Middle | FColorPicker_SameLine);
				bTransparent = !(FGet(Vars::Visuals::World::Modulations) & 1 << 4);
					FColorPicker("Fog modulation", Vars::Colors::FogModulation, 0, FColorPicker_Left);
				bTransparent = false;
				FToggle("Near prop fade", Vars::Visuals::World::NearPropFade);
				FToggle("No prop fade", Vars::Visuals::World::NoPropFade, FToggle_Middle);
			} EndSection();

			EndTable();
		}
		break;
		// Radar
	case 4:
		if (BeginTable("VisualsRadarTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Main"))
			{
				FToggle("Enabled", Vars::Radar::Main::Enabled);
				FToggle("Draw out of range", Vars::Radar::Main::AlwaysDraw, FToggle_Middle);
				FDropdown("Style", Vars::Radar::Main::Style, { "Circle", "Rectangle" });
				FSlider("Range", Vars::Radar::Main::Range, 50, 3000, 50, "%d", FSlider_Precision);
				FSlider("Background alpha", Vars::Radar::Main::BackAlpha, 0, 255, 1, "%d", FSlider_Clamp);
				FSlider("Line alpha", Vars::Radar::Main::LineAlpha, 0, 255, 1, "%d", FSlider_Clamp);
			} EndSection();
			if (Section("Player"))
			{
				FToggle("Enabled", Vars::Radar::Players::Enabled);
				FToggle("Background", Vars::Radar::Players::Background, FToggle_Middle);
				FDropdown("Draw", Vars::Radar::Players::Draw, { "Local", "Enemy", "Team", "Friends", "Priority", "Cloaked" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Icon", Vars::Radar::Players::IconType, { "Icons", "Portraits", "Avatar" }, {}, FDropdown_Right);
				FSlider("Icon size## Player", Vars::Radar::Players::IconSize, 12, 30, 2);
				FToggle("Health bar", Vars::Radar::Players::Health);
				FToggle("Height indicator", Vars::Radar::Players::Height, FToggle_Middle);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Building"))
			{
				FToggle("Enabled", Vars::Radar::Buildings::Enabled);
				FToggle("Background", Vars::Radar::Buildings::Background, FToggle_Middle);
				FDropdown("Draw", Vars::Radar::Buildings::Draw, { "Local", "Enemy", "Team", "Friends", "Priority" }, {}, FDropdown_Multi);
				FSlider("Icon size## Building", Vars::Radar::Buildings::IconSize, 12, 30, 2);
				FToggle("Health bar", Vars::Radar::Buildings::Health);
			} EndSection();
			if (Section("World"))
			{
				FToggle("Enabled", Vars::Radar::World::Enabled);
				FToggle("Background", Vars::Radar::World::Background, FToggle_Middle);
				FDropdown("Draw", Vars::Radar::World::Draw, { "Health", "Ammo", "Money", "Bombs", "Halloween" }, {}, FDropdown_Multi);
				FSlider("Icon size## World", Vars::Radar::World::IconSize, 12, 30, 2);
			} EndSection();

			EndTable();
		}
		break;
		// Menu
	case 5:
	{
		if (BeginTable("MenuTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("General"))
			{
				FColorPicker("Accent color", Vars::Menu::Theme::Accent, 0, FColorPicker_Left);
				FColorPicker("Foremost color", Vars::Menu::Theme::Foremost, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Background color", Vars::Menu::Theme::Background, 0, FColorPicker_Left);
				FColorPicker("Foreground color", Vars::Menu::Theme::Foreground, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Active color", Vars::Menu::Theme::Active, 0, FColorPicker_Left);
				FColorPicker("Inactive color", Vars::Menu::Theme::Inactive, 0, FColorPicker_Middle | FColorPicker_SameLine);

				FSDropdown("Cheat title", Vars::Menu::CheatName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
				FSDropdown("Chat info prefix", Vars::Menu::CheatPrefix, {}, FDropdown_Right);
				FKeybind("Menu primary key", Vars::Menu::MenuPrimaryKey.Map["default"], FButton_Left | FKeybind_AllowMenu);
				FKeybind("Menu secondary key", Vars::Menu::MenuSecondaryKey.Map["default"], FButton_Right | FButton_SameLine | FKeybind_AllowMenu);
				if (Vars::Menu::MenuPrimaryKey.Map["default"] == VK_LBUTTON)
					Vars::Menu::MenuPrimaryKey.Map["default"] = VK_INSERT;
				if (Vars::Menu::MenuSecondaryKey.Map["default"] == VK_LBUTTON)
					Vars::Menu::MenuSecondaryKey.Map["default"] = VK_F3;
			} EndSection();
			if (Section("Indicators"))
			{
				FDropdown("Indicators", Vars::Menu::Indicators, { "Ticks", "Crit hack", "Spectators", "Ping", "Conditions", "Seed prediction" }, {}, FDropdown_Multi);
				if (FSlider("DPI", Vars::Menu::DPI, 0.8f, 1.8f, 0.2f, "%.1f", FSlider_Precision))
					H::Fonts.Reload(Vars::Menu::DPI.Map["default"]);
			} EndSection();

			/* Column 2 */
			TableNextColumn();

			EndTable();
		}
	}
	}
}

/* Tab: Misc */
void CMenu::MenuMisc()
{
	using namespace ImGui;

	if (BeginTable("MiscTable", 2))
	{
		/* Column 1 */
		TableNextColumn();
		if (Section("Movement"))
		{
			FDropdown("Autostrafe", Vars::Misc::Movement::AutoStrafe, { "Off", "Legit", "Directional" });
			bTransparent = FGet(Vars::Misc::Movement::AutoStrafe) != 2;
				FSlider("Autostrafe turn scale", Vars::Misc::Movement::AutoStrafeTurnScale, 0.f, 1.f, 0.1f, "%.1f", FSlider_Clamp | FSlider_Precision);
			bTransparent = false;
			FToggle("Bunnyhop", Vars::Misc::Movement::Bunnyhop);
			FToggle("Auto jumpbug", Vars::Misc::Movement::AutoJumpbug, FToggle_Middle); // this is unreliable without setups, do not depend on it!
			FToggle("Auto rocketjump", Vars::Misc::Movement::AutoRocketJump);
			FToggle("Auto ctap", Vars::Misc::Movement::AutoCTap, FToggle_Middle);
			FToggle("Fast stop", Vars::Misc::Movement::FastStop);
			FToggle("Fast accelerate", Vars::Misc::Movement::FastAccel, FToggle_Middle);
			FToggle("Fast strafe", Vars::Misc::Movement::FastStrafe);
			FToggle("No push", Vars::Misc::Movement::NoPush, FToggle_Middle);
			FToggle("Crouch speed", Vars::Misc::Movement::CrouchSpeed);
		} EndSection();
		if (Vars::Debug::Info.Value)
		{
			if (Section("debug"))
			{
				FSlider("timing offset", Vars::Misc::Movement::TimingOffset, -1, 1);
				FSlider("apply timing offset above", Vars::Misc::Movement::ApplyAbove, 0, 8);
			} EndSection();
		}
		if (Section("Exploits"))
		{
			FToggle("Cheats bypass", Vars::Misc::Exploits::CheatsBypass);
			FToggle("Pure bypass", Vars::Misc::Exploits::BypassPure, FToggle_Middle);
			FToggle("Ping reducer", Vars::Misc::Exploits::PingReducer);
			bTransparent = !FGet(Vars::Misc::Exploits::PingReducer);
				FSlider("cl_cmdrate", Vars::Misc::Exploits::PingTarget, 1, 66, 1, "%d", FSlider_Right | FSlider_Clamp);
			bTransparent = false;
			SetCursorPosY(GetCursorPosY() - 8);
			FToggle("Equip region unlock", Vars::Misc::Exploits::EquipRegionUnlock);
		} EndSection();
		if (Vars::Debug::Info.Value)
		{
			if (Section("Convar spoofer"))
			{
				static std::string sName = "", sValue = "";

				FSDropdown("Convar", &sName, {}, FDropdown_Left);
				FSDropdown("Value", &sValue, {}, FDropdown_Right);
				if (FButton("Send"))
				{
					if (auto pNetChan = static_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo()))
					{
						SDK::Output("Convar", std::format("Sent {} as {}", sName, sValue).c_str(), VecToColor(F::Render.Accent));
						NET_SetConVar cmd(sName.c_str(), sValue.c_str()); // this doesn't crash, but doesn't do anything either
						pNetChan->SendNetMsg(cmd);

						//sName = "";
						//sValue = "";
					}
				}
			} EndSection();
		}
		if (Section("Automation"))
		{
			FToggle("Anti-backstab", Vars::Misc::Automation::AntiBackstab);
			FToggle("Anti-AFK", Vars::Misc::Automation::AntiAFK, FToggle_Middle);
			FToggle("Anti autobalance", Vars::Misc::Automation::AntiAutobalance);
			FToggle("Auto accept item drops", Vars::Misc::Automation::AcceptItemDrops, FToggle_Middle);
			FToggle("Taunt control", Vars::Misc::Automation::TauntControl);
			FToggle("Kart control", Vars::Misc::Automation::KartControl, FToggle_Middle);
			FToggle("Backpack expander", Vars::Misc::Automation::BackpackExpander);
		} EndSection();
		if (Section("Sound"))
		{
			FDropdown("Block", Vars::Misc::Sound::Block, { "Footsteps", "Noisemaker" }, {}, FDropdown_Multi);
			FToggle("Giant weapon sounds", Vars::Misc::Sound::GiantWeaponSounds);
			FToggle("Hitsound always", Vars::Misc::Sound::HitsoundAlways, FToggle_Middle);
		} EndSection();

		/* Column 2 */
		TableNextColumn();
		if (Section("Game"))
		{
			FToggle("Network fix", Vars::Misc::Game::NetworkFix);
			FToggle("Prediction error jitter fix", Vars::Misc::Game::PredictionErrorJitterFix);
			FToggle("Bones optimization", Vars::Misc::Game::SetupBonesOptimization, FToggle_Middle);
		} EndSection();
		if (Section("Queueing"))
		{
			FDropdown("Force regions", Vars::Misc::Queueing::ForceRegions,
				{ "Atlanta", "Chicago", "Texas", "Los Angeles", "Moses Lake", "New York", "Seattle", "Virginia", "Amsterdam", "Frankfurt", "Helsinki", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "Buenos" "Aires", "Lima", "Santiago", "Sao" "Paulo", "Bombay", "Chennai", "Dubai", "Hong Kong", "Madras", "Mumbai", "Seoul", "Singapore", "Tokyo", "Sydney", "Johannesburg" },
				{}, FDropdown_Multi
			);
			FToggle("Freeze queue", Vars::Misc::Queueing::FreezeQueue);
			FToggle("Auto queue", Vars::Misc::Queueing::AutoCasualQueue, FToggle_Middle);
		} EndSection();
		if (Section("Mann vs. Machine"))
		{
			FToggle("Instant respawn", Vars::Misc::MannVsMachine::InstantRespawn);
			FToggle("Instant revive", Vars::Misc::MannVsMachine::InstantRevive, FToggle_Middle);
		} EndSection();
		if (Section("Chat"))
		{
			FToggle("Chat tags", Vars::Misc::Chat::Tags);
		} EndSection();
		if (Section("Steam RPC"))
		{
			FToggle("Steam RPC", Vars::Misc::Steam::EnableRPC);
			FToggle("Override in menu", Vars::Misc::Steam::OverrideMenu, FToggle_Middle);
			FDropdown("Match group", Vars::Misc::Steam::MatchGroup, { "Special Event", "MvM Mann Up", "Competitive", "Casual", "MvM Boot Camp" }, {}, FDropdown_Left);
			FSDropdown("Map text", Vars::Misc::Steam::MapText, {}, FSDropdown_Custom | FDropdown_Right);
			FSlider("Group size", Vars::Misc::Steam::GroupSize, 0, 6);
		} EndSection();

		EndTable();
	}
}

/* Tab: Settings */
void CMenu::MenuLogs()
{
	using namespace ImGui;

	switch (CurrentLogsTab)
	{
		// Logs
	case 0:
		// Eventually put all logs here, regardless of any settings
		break;
		// Settings
	case 1:
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Logging"))
			{
				FDropdown("Logs", Vars::Logging::Logs, { "Vote start", "Vote cast", "Class changes", "Damage", "Cheat detection", "Tags" }, {}, FDropdown_Multi);
				FSlider("Notification time", Vars::Logging::Lifetime, 0.5f, 5.f, 0.5f, "%.1f");
			} EndSection();
			if (Section("Vote Start"))
			{
				bTransparent = !(FGet(Vars::Logging::Logs) & 1 << 0);
					FDropdown("Log to", Vars::Logging::VoteStart::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Vote Cast"))
			{
				bTransparent = !(FGet(Vars::Logging::Logs) & 1 << 1);
					FDropdown("Log to", Vars::Logging::VoteCast::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Class Change"))
			{
				bTransparent = !(FGet(Vars::Logging::Logs) & 1 << 2);
					FDropdown("Log to", Vars::Logging::ClassChange::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Damage"))
			{
				bTransparent = !(FGet(Vars::Logging::Logs) & 1 << 3);
					FDropdown("Log to", Vars::Logging::Damage::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Cheat Detection"))
			{
				bTransparent = !(FGet(Vars::Logging::Logs) & 1 << 4);
					FDropdown("Log to", Vars::Logging::CheatDetection::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Tags"))
			{
				bTransparent = !(FGet(Vars::Logging::Logs) & 1 << 5);
					FDropdown("Log to", Vars::Logging::Tags::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();

			EndTable();
		}
	}
}

/* Tab: Config */
void CMenu::MenuSettings()
{
	using namespace ImGui;

	switch (CurrentConfigTab)
	{
		// Settings
	case 0:
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Config"))
			{
				if (FButton("Configs folder", FButton_Left))
					ShellExecuteA(NULL, NULL, F::Configs.sConfigPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				if (FButton("Visuals folder", FButton_Right | FButton_SameLine))
					ShellExecuteA(NULL, NULL, F::Configs.sVisualsPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

				FTabs({ "GENERAL", "VISUALS", }, &CurrentConfigType, { GetColumnWidth() / 2 + 2, SubTabSize.y }, { 6, GetCursorPos().y }, false);

				switch (CurrentConfigType)
				{
					// General
				case 0:
				{
					static std::string newName;
					FSDropdown("Config name", &newName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
					if (FButton("Create", FButton_Fit | FButton_SameLine | FButton_Large) && newName.length() > 0)
					{
						if (!std::filesystem::exists(F::Configs.sConfigPath + "\\" + newName))
							F::Configs.SaveConfig(newName);
						newName.clear();
					}

					for (const auto& entry : std::filesystem::directory_iterator(F::Configs.sConfigPath))
					{
						if (!entry.is_regular_file() || entry.path().extension() != F::Configs.sConfigExtension)
							continue;

						std::string configName = entry.path().filename().string();
						configName.erase(configName.end() - F::Configs.sConfigExtension.size(), configName.end());

						const auto current = GetCursorPos().y;

						SetCursorPos({ 14, current + 11 });
						TextColored(configName == F::Configs.sCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, configName.c_str());

						int o = 26;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteConfig{}", configName).c_str());
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_SAVE))
						{
							if (configName != F::Configs.sCurrentConfig || F::Configs.sCurrentVisuals.length())
								OpenPopup(std::format("Confirmation## SaveConfig{}", configName).c_str());
							else
								F::Configs.SaveConfig(configName);
						}
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DOWNLOAD))
							F::Configs.LoadConfig(configName);

						// Dialogs
						{
							// Save config dialog
							if (BeginPopupModal(std::format("Confirmation## SaveConfig{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to override '{}'?", configName).c_str());

								if (FButton("Yes, override", FButton_Left))
								{
									F::Configs.SaveConfig(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}

							// Delete config dialog
							if (BeginPopupModal(std::format("Confirmation## DeleteConfig{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to delete '{}'?", configName).c_str());

								if (FButton("Yes, delete", FButton_Left))
								{
									F::Configs.RemoveConfig(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}
						}

						SetCursorPos({ 6, current }); DebugDummy({ 0, 28 });
					}
					break;
				}
				// Visuals
				case 1:
				{
					static std::string newName;
					FSDropdown("Config name", &newName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
					if (FButton("Create", FButton_Fit | FButton_SameLine | FButton_Large) && newName.length() > 0)
					{
						if (!std::filesystem::exists(F::Configs.sVisualsPath + "\\" + newName))
							F::Configs.SaveVisual(newName);
						newName.clear();
					}

					for (const auto& entry : std::filesystem::directory_iterator(F::Configs.sVisualsPath))
					{
						if (!entry.is_regular_file() || entry.path().extension() != F::Configs.sConfigExtension)
							continue;

						std::string configName = entry.path().filename().string();
						configName.erase(configName.end() - F::Configs.sConfigExtension.size(), configName.end());

						const auto current = GetCursorPos().y;

						SetCursorPos({ 14, current + 11 });
						TextColored(configName == F::Configs.sCurrentVisuals ? F::Render.Active.Value : F::Render.Inactive.Value, configName.c_str());

						int o = 26;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteVisual{}", configName).c_str());
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_SAVE))
						{
							if (configName != F::Configs.sCurrentVisuals)
								OpenPopup(std::format("Confirmation## SaveVisual{}", configName).c_str());
							else
								F::Configs.SaveVisual(configName);
						}
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DOWNLOAD))
							F::Configs.LoadVisual(configName);

						// Dialogs
						{
							// Save config dialog
							if (BeginPopupModal(std::format("Confirmation## SaveVisual{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to override '{}'?", configName).c_str());

								if (FButton("Yes, override", FButton_Left))
								{
									F::Configs.SaveVisual(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}

							// Delete config dialog
							if (BeginPopupModal(std::format("Confirmation## DeleteVisual{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to delete '{}'?", configName).c_str());

								if (FButton("Yes, delete", FButton_Left))
								{
									F::Configs.RemoveVisual(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}
						}

						SetCursorPos({ 6, current }); DebugDummy({ 0, 28 });
					}
				}
				}
			} EndSection();
			SetCursorPosX(GetCursorPosX() + 8);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			Text("Built @ %s, %s", __DATE__, __TIME__);
			PopStyleColor();

			/* Column 2 */
			TableNextColumn();
			if (Section("Debug"))
			{
				FToggle("Debug info", Vars::Debug::Info);
				FToggle("Debug logging", Vars::Debug::Logging, FToggle_Middle);
				FToggle("Show server hitboxes", Vars::Debug::ServerHitbox); HelpMarker("localhost servers");
				FToggle("Anti aim lines", Vars::Debug::AntiAimLines, FToggle_Middle);
			} EndSection();
			if (Section("Extra"))
			{
				if (FButton("cl_fullupdate", FButton_Left))
					I::EngineClient->ClientCmd_Unrestricted("cl_fullupdate");
				if (FButton("retry", FButton_Right | FButton_SameLine))
					I::EngineClient->ClientCmd_Unrestricted("retry");
				if (FButton("Console", FButton_Left))
					I::EngineClient->ClientCmd_Unrestricted("toggleconsole");
				if (FButton("Fix Chams", FButton_Right | FButton_SameLine))
					F::Materials.ReloadMaterials();

				if (!I::EngineClient->IsConnected())
				{
					if (FButton("Unlock achievements", FButton_Left))
						OpenPopup("Unlock achievements?");
					if (FButton("Lock achievements", FButton_Right | FButton_SameLine))
						OpenPopup("Lock achievements?");

					if (BeginPopupModal("Unlock achievements?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						Text("Do you really want to unlock all achievements?");

						Separator();
						if (Button("Yes, unlock", ImVec2(150, 0)))
						{
							//F::Misc.UnlockAchievements();
							CloseCurrentPopup();
						}
						SameLine();
						if (Button("No", ImVec2(150, 0)))
							CloseCurrentPopup();
						EndPopup();
					}
					if (BeginPopupModal("Lock achievements?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						Text("Do you really want to lock all achievements?");

						Separator();
						if (Button("Yes, lock", ImVec2(150, 0)))
						{
							//F::Misc.LockAchievements();
							CloseCurrentPopup();
						}
						SameLine();
						if (Button("No", ImVec2(150, 0)))
							CloseCurrentPopup();
						EndPopup();
					}
				}
				if (Vars::Debug::Info.Value)
				{
					if (FButton("Reveal bullet lines", FButton_Left))
						F::Visuals.RevealBulletLines();
					if (FButton("Reveal prediction lines", FButton_Right | FButton_SameLine))
						F::Visuals.RevealSimLines();
					if (FButton("Reveal boxes", FButton_Left))
						F::Visuals.RevealBoxes();
				}
			} EndSection();

			EndTable();
		}
		break;
		// Conditions
	case 1:
		if (Section("Settings"))
		{
			FToggle("Show bind window", Vars::Menu::ShowBinds);
			FToggle("Menu shows binds", Vars::Menu::MenuShowsBinds, FToggle_Middle);
		} EndSection();
		if (Section("Conditions"))
		{
			static std::string sName = "";
			static Condition_t tCond = {};

			if (BeginTable("ConditionsTable", 2))
			{
				/* Column 1 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 8, GetCursorPos().y - 8 });
				if (BeginChild("ConditionsTableTable1", { GetColumnWidth() + 4, 104 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					FSDropdown("Name", &sName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
					FSDropdown("Parent", &tCond.Parent, {}, FSDropdown_AutoUpdate | FDropdown_Right);
					FDropdown("Type", &tCond.Type, { "Key", "Class", "Weapon type" }, {}, FDropdown_Left);
					switch (tCond.Type)
					{
					case 0: tCond.Info = std::min(tCond.Info, 2); FDropdown("Behavior", &tCond.Info, { "Hold", "Toggle", "Double click" }, {}, FDropdown_Right); break;
					case 1: tCond.Info = std::min(tCond.Info, 8); FDropdown("Class", &tCond.Info, { "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdown_Right); break;
					case 2: tCond.Info = std::min(tCond.Info, 2); FDropdown("Weapon type", &tCond.Info, { "Hitscan", "Projectile", "Melee" }, {}, FDropdown_Right); break;
					}
				} EndChild();

				/* Column 2 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 4, GetCursorPos().y - 8 });
				if (BeginChild("ConditionsTableTable2", { GetColumnWidth() + 8, 104 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					SetCursorPos({ 8, 24 });
					FToggle("Visible", &tCond.Visible);
					FToggle("Not", &tCond.Not, FToggle_Middle);
					if (tCond.Type == 0)
					{
						SetCursorPos({ 8, 56 });
						FKeybind("Key", tCond.Key, FButton_Large, -96);
					}

					// create/modify button
					bool bCreate = false, bClear = false, bMatch = false, bParent = true;
					if (tCond.Parent.length())
						bParent = F::Conditions.mConditions.contains(tCond.Parent);

					SetCursorPos({ GetWindowSize().x - 96, 64 });
					PushStyleColor(ImGuiCol_Button, F::Render.Foremost.Value);
					PushStyleColor(ImGuiCol_ButtonActive, F::Render.Foremost.Value);
					if (sName.length() && FNV1A::Hash(sName.c_str()) != FNV1A::HashConst("default") && bParent && (!tCond.Type ? tCond.Key : true))
					{
						PushStyleColor(ImGuiCol_ButtonHovered, F::Render.ForemostLight.Value);
						bCreate = Button("##CreateButton", { 40, 40 });
					}
					else
					{
						PushStyleColor(ImGuiCol_ButtonHovered, F::Render.Foremost.Value);
						Button("##CreateButton", { 40, 40 });
					}
					PopStyleColor(3);
					SetCursorPos({ GetWindowSize().x - 83, 76 });
					if (sName.length() && FNV1A::Hash(sName.c_str()) != FNV1A::HashConst("default") && bParent && (!tCond.Type ? tCond.Key : true))
					{
						bMatch = F::Conditions.mConditions.contains(sName);
						IconImage(bMatch ? ICON_MD_SETTINGS : ICON_MD_ADD);
					}
					else
					{
						bTransparent = true;
							IconImage(ICON_MD_ADD);
						bTransparent = false;
					}

					// clear button
					SetCursorPos({ GetWindowSize().x - 48, 64 });
					PushStyleColor(ImGuiCol_Button, F::Render.Foremost.Value);
					PushStyleColor(ImGuiCol_ButtonHovered, F::Render.ForemostLight.Value);
					PushStyleColor(ImGuiCol_ButtonActive, F::Render.Foremost.Value);
					bClear = Button("##ClearButton", { 40, 40 });
					PopStyleColor(3);
					SetCursorPos({ GetWindowSize().x - 35, 76 });
					IconImage(ICON_MD_CLEAR);

					if (bCreate)
						F::Conditions.AddCondition(sName, tCond);
					if (bCreate || bClear)
					{
						sName = "";
						tCond = {};
					}
				} EndChild();

				EndTable();
			}

			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			SetCursorPos({ 14, 128 }); FText("Conditions");
			PopStyleColor();

			std::function<int(std::string, int, int)> getConds = [&](std::string sParent, int x, int y)
				{
					auto uHash = FNV1A::Hash(sParent.c_str());
					for (auto& sCond : F::Conditions.vConditions)
					{
						auto& cCond = F::Conditions.mConditions[sCond];
						if (uHash != FNV1A::Hash(cCond.Parent.c_str()))
							continue;

						y++;

						std::string info; std::string state;
						switch (cCond.Type)
						{
							// key
						case 0:
							switch (cCond.Info)
							{
							case 0: { info = "hold"; break; }
							case 1: { info = "toggle"; break; }
							case 2: { info = "double"; break; }
							}
							state = VK2STR(cCond.Key);
							break;
							// class
						case 1:
							info = "class";
							switch (cCond.Info)
							{
							case 0: { state = "scout"; break; }
							case 1: { state = "soldier"; break; }
							case 2: { state = "pyro"; break; }
							case 3: { state = "demoman"; break; }
							case 4: { state = "heavy"; break; }
							case 5: { state = "engineer"; break; }
							case 6: { state = "medic"; break; }
							case 7: { state = "sniper"; break; }
							case 8: { state = "spy"; break; }
							}
							break;
							// weapon type
						case 2:
							info = "weapon";
							switch (cCond.Info)
							{
							case 0: { state = "hitscan"; break; }
							case 1: { state = "projectile"; break; }
							case 2: { state = "melee"; break; }
							}
						}
						if (cCond.Not)
							info = std::format("not {}", info);
						std::string str = std::format("{}, {}", info, state);

						bool bClicked = false, bDelete = false, bEdit = false;

						const ImVec2 restorePos = { 8.f + 28 * x, 108.f + 36.f * y };

						// background
						const float width = GetWindowSize().x - 16 - 28 * x; const auto winPos = GetWindowPos();
						GetWindowDrawList()->AddRectFilled({ winPos.x + restorePos.x, winPos.y + restorePos.y }, { winPos.x + restorePos.x + width, winPos.y + restorePos.y + 28 }, F::Render.Foremost, 3);

						// text
						SetCursorPos({ restorePos.x + 10, restorePos.y + 7 });
						TextUnformatted(sCond.c_str());

						SetCursorPos({ restorePos.x + width / 2 - CalcTextSize(str.c_str()).x / 2, restorePos.y + 7 });
						TextUnformatted(std::format("{}", str).c_str());

						// buttons
						SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
						bDelete = IconButton(ICON_MD_DELETE);

						SetCursorPos({ restorePos.x + width - 47, restorePos.y + 5 });
						bEdit = IconButton(ICON_MD_EDIT);

						SetCursorPos(restorePos);
						bClicked = Button(std::format("##{}", sCond).c_str(), { width, 28 });

						if (bClicked)
						{
							sName = sCond;
							tCond = cCond;
						}
						if (bDelete)
							OpenPopup(std::format("Confirmation## DeleteCond{}", sCond).c_str());
						if (BeginPopupModal(std::format("Confirmation## DeleteCond{}", sCond).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							const bool bChildren = F::Conditions.HasChildren(sCond);
							Text(std::format("Do you really want to delete '{}'{}?", sCond, bChildren ? " and all of its children" : "").c_str());

							if (FButton("Yes", FButton_Left))
							{
								F::Conditions.RemoveCondition(sCond);
								CloseCurrentPopup();
							}
							if (FButton("No", FButton_Right | FButton_SameLine))
								CloseCurrentPopup();

							EndPopup();
						}
						if (bEdit)
						{
							if (sCondition != sCond)
								sCondition = sCond;
							else
								sCondition = "default";
						}

						y = getConds(sCond, x + 1, y);
					}

					return y;
				};
			getConds("", 0, 0);
		} EndSection();
		break;
		// PlayerList
	case 2:
		if (Section("Players"))
		{
			if (I::EngineClient->IsInGame())
			{
				std::lock_guard lock(F::PlayerUtils.mutex);
				const auto& playerCache = F::PlayerUtils.vPlayerCache;

				auto getTeamColor = [](int team, bool alive)
					{
						switch (team)
						{
						case 3: return Color_t(50, 75, 100, alive ? 255 : 127);
						case 2: return Color_t(125, 50, 50, alive ? 255 : 127);
						}
						return Color_t(100, 100, 100, 255);
					};
				auto drawPlayer = [getTeamColor](const ListPlayer& player, int x, int y)
					{
						bool bClicked = false, bAdd = false, bPitch = false, bYaw = false;

						const Color_t teamColor = getTeamColor(player.Team, player.Alive);
						const ImColor imColor = ColorToVec(teamColor);

						const ImVec2 restorePos = { x ? GetWindowSize().x / 2 + 4.f : 8.f, 32.f + 36.f * y };

						// background
						const float width = GetWindowSize().x / 2 - 12; const auto winPos = GetWindowPos();
						GetWindowDrawList()->AddRectFilled({ winPos.x + restorePos.x, winPos.y + restorePos.y }, { winPos.x + restorePos.x + width, winPos.y + restorePos.y + 28 }, imColor, 3);

						// text + icons
						if (player.Local)
						{
							SetCursorPos({ restorePos.x + 7, restorePos.y + 5 });
							IconImage(ICON_MD_PERSON);
						}
						else if (player.Friend)
						{
							SetCursorPos({ restorePos.x + 7, restorePos.y + 5 });
							IconImage(ICON_MD_GROUP);
						}
						int lOffset = player.Local || player.Friend ? 29 : 10;
						SetCursorPos({ restorePos.x + lOffset, restorePos.y + 7 });
						TextUnformatted(player.Name);
						lOffset += CalcTextSize(player.Name).x + 8;

						// buttons
						if (!player.Fake)
						{
							// right
							SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
							bAdd = IconButton(ICON_MD_ADD);
							if (Vars::AntiHack::Resolver::Resolver.Value && !player.Local)
							{
								SetCursorPos({ restorePos.x + width - 42, restorePos.y + 5 });
								bYaw = IconButton(ICON_MD_ARROW_FORWARD);

								SetCursorPos({ restorePos.x + width - 62, restorePos.y + 5 });
								bPitch = IconButton(ICON_MD_ARROW_UPWARD);
							}

							// tag bar
							SetCursorPos({ restorePos.x + lOffset, restorePos.y });
							if (BeginChild(std::format("TagBar{}", player.FriendsID).c_str(), { width - lOffset - (Vars::AntiHack::Resolver::Resolver.Value ? 68 : 28), 28 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
							{
								PushFont(F::Render.FontSmall);

								const auto childPos = GetWindowPos();
								float tOffset = 0;
								for (const auto& sTag : F::PlayerUtils.mPlayerTags[player.FriendsID])
								{
									PriorityLabel_t plTag;
									if (!F::PlayerUtils.GetTag(sTag, &plTag))
										continue;

									const ImColor tagColor = ColorToVec(plTag.Color);
									const float tagWidth = CalcTextSize(sTag.c_str()).x + 25;
									const ImVec2 tagPos = { tOffset, 4 };

									PushStyleColor(ImGuiCol_Text, IsColorBright(tagColor) ? ImVec4{ 0, 0, 0, 1 } : ImVec4{ 1, 1, 1, 1 });

									GetWindowDrawList()->AddRectFilled({ childPos.x + tagPos.x, childPos.y + tagPos.y }, { childPos.x + tagPos.x + tagWidth, childPos.y + tagPos.y + 20 }, tagColor, 3);
									SetCursorPos({ tagPos.x + 5, tagPos.y + 4 });
									TextUnformatted(sTag.c_str());
									SetCursorPos({ tagPos.x + tagWidth - 18, tagPos.y + 2 });
									if (IconButton(ICON_MD_CANCEL))
										F::PlayerUtils.RemoveTag(player.FriendsID, sTag, true, player.Name);

									PopStyleColor();

									tOffset += tagWidth + 4;
								}
								PopFont();
							} EndChild();

							if (!player.Local)
							{
								//bClicked = IsItemClicked();
								bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;

								SetCursorPos(restorePos);
								/*bClicked = */Button(std::format("##{}", player.Name).c_str(), { width, 28 }) || bClicked;
								bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;
							}
						}

						SetCursorPos(restorePos);
						DebugDummy({ 0, 28 });

						if (bClicked)
							OpenPopup(std::format("Clicked{}", player.FriendsID).c_str());
						else if (bAdd)
							OpenPopup(std::format("Add{}", player.FriendsID).c_str());
						else if (bPitch)
							OpenPopup(std::format("Pitch{}", player.FriendsID).c_str());
						else if (bYaw)
							OpenPopup(std::format("Yaw{}", player.FriendsID).c_str());

						// popups
						if (FBeginPopup(std::format("Clicked{}", player.FriendsID).c_str()))
						{
							if (FSelectable("Profile"))
							{
								//g_SteamInterfaces.Friends->ActivateGameOverlayToUser("steamid", CSteamID(0x0110000100000000ULL + player.FriendsID));
							}

							if (FSelectable("Votekick"))
								I::EngineClient->ClientCmd_Unrestricted(std::format("callvote kick {}", player.UserID).c_str());

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Add{}", player.FriendsID).c_str()))
						{
							for (const auto& [sTag, plTag] : F::PlayerUtils.mTags)
							{
								if (!plTag.Assignable || F::PlayerUtils.HasTag(player.FriendsID, sTag))
									continue;

								auto imColor = ColorToVec(plTag.Color);
								PushStyleColor(ImGuiCol_Text, imColor);
								imColor.x /= 3; imColor.y /= 3; imColor.z /= 3;
								if (FSelectable(sTag.c_str(), imColor))
									F::PlayerUtils.AddTag(player.FriendsID, sTag, true, player.Name);
								PopStyleColor();
							}

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Pitch{}", player.FriendsID).c_str()))
						{
							for (size_t i = 0; i < F::PlayerUtils.vListPitch.size(); i++)
							{
								if (FSelectable(F::PlayerUtils.vListPitch[i]))
									F::Resolver.mResolverMode[player.FriendsID].second = int(i);
							}
							FEndPopup();
						}
						else if (FBeginPopup(std::format("Yaw{}", player.FriendsID).c_str()))
						{
							for (size_t i = 0; i < F::PlayerUtils.vListYaw.size(); i++)
							{
								if (FSelectable(F::PlayerUtils.vListYaw[i]))
									F::Resolver.mResolverMode[player.FriendsID].second = int(i);
							}
							FEndPopup();
						}
					};

				// display players
				int iBlu = 0, iRed = 0;
				for (const auto& player : playerCache)
				{
					switch (player.Team)
					{
					case 3:
						drawPlayer(player, 0, iBlu);
						iBlu++; break;
					case 2:
						drawPlayer(player, 1, iRed);
						iRed++; break;
					}
				}
				int iOther = 0; const int iMax = std::max(iBlu, iRed);
				for (const auto& player : playerCache)
				{
					if (player.Team != 3 && player.Team != 2)
					{
						drawPlayer(player, iOther % 2, iMax + iOther / 2);
						iOther++;
					}
				}
			}
			else
			{
				SetCursorPos({ 18, 39 });
				Text("Not ingame");
				DebugDummy({ 0, 8 });
			}
		} EndSection();
		if (Section("Tags"))
		{
			static std::string sName = "";
			static PriorityLabel_t tTag = {};

			if (BeginTable("TagTable", 2))
			{
				/* Column 1 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 8, GetCursorPos().y - 8 });
				if (BeginChild("TagTable1", { GetColumnWidth() + 4, 56 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					FSDropdown("Name", &sName, {}, FSDropdown_AutoUpdate | FDropdown_Left, 1);
					FColorPicker("Color", &tTag.Color, 0, FColorPicker_Dropdown);

					auto uHash = FNV1A::Hash(sName.c_str());
					bDisabled = uHash == FNV1A::HashConst("Original") || uHash == FNV1A::HashConst("Ignored");
					int iLabel = bDisabled ? 0 : tTag.Label;
					FDropdown("Type", &iLabel, { "Priority", "Label" }, {}, FDropdown_Right);
					tTag.Label = iLabel;
					if (bDisabled)
						tTag.Label = false;
					bDisabled = false;
				} EndChild();

				/* Column 2 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 4, GetCursorPos().y - 8 });
				if (BeginChild("TagTable2", { GetColumnWidth() + 8, 56 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					bTransparent = tTag.Label; // transparent if we want a label, user can still use to sort
						SetCursorPosY(GetCursorPos().y + 12);
						FSlider("Priority", &tTag.Priority, -10, 10, 1, "%d", FSlider_Left);
					bTransparent = false;

					// create/modify button
					bool bCreate = false, bClear = false;

					SetCursorPos({ GetWindowSize().x - 96, 16 });
					PushStyleColor(ImGuiCol_Button, F::Render.Foremost.Value);
					PushStyleColor(ImGuiCol_ButtonActive, F::Render.Foremost.Value);
					if (sName.length())
					{
						PushStyleColor(ImGuiCol_ButtonHovered, F::Render.ForemostLight.Value);
						bCreate = Button("##CreateButton", { 40, 40 });
					}
					else
					{
						PushStyleColor(ImGuiCol_ButtonHovered, F::Render.Foremost.Value);
						Button("##CreateButton", { 40, 40 });
					}
					PopStyleColor(3);
					SetCursorPos({ GetWindowSize().x - 83, 28 });
					if (sName.length())
					{
						bool bMatch = F::PlayerUtils.mTags.contains(sName);
						IconImage(bMatch ? ICON_MD_SETTINGS : ICON_MD_ADD);
					}
					else
					{
						bTransparent = true;
							IconImage(ICON_MD_ADD);
						bTransparent = false;
					}

					// clear button
					SetCursorPos({ GetWindowSize().x - 48, 16 });
					PushStyleColor(ImGuiCol_Button, F::Render.Foremost.Value);
					PushStyleColor(ImGuiCol_ButtonHovered, F::Render.ForemostLight.Value);
					PushStyleColor(ImGuiCol_ButtonActive, F::Render.Foremost.Value);
					bClear = Button("##ClearButton", { 40, 40 });
					PopStyleColor(3);
					SetCursorPos({ GetWindowSize().x - 35, 28 });
					IconImage(ICON_MD_CLEAR);

					if (bCreate)
					{
						F::PlayerUtils.bSaveTags = true;
						F::PlayerUtils.mTags[sName].Color = tTag.Color;
						F::PlayerUtils.mTags[sName].Priority = tTag.Priority;
						F::PlayerUtils.mTags[sName].Label = tTag.Label;
					}
					if (bCreate || bClear)
					{
						sName = "";
						tTag = {};
					}
				} EndChild();

				EndTable();
			}

			auto drawTag = [](const std::string sTag, const PriorityLabel_t& plTag, int y)
				{
					bool bClicked = false, bDelete = false;

					ImColor imColor = ColorToVec(plTag.Color);
					imColor.Value.x /= 3; imColor.Value.y /= 3; imColor.Value.z /= 3;

					const ImVec2 restorePos = { plTag.Label ? GetWindowSize().x * 2 / 3 + 4.f : 8.f, 96.f + 36.f * y };

					// background
					const float width = GetWindowSize().x * (plTag.Label ? 1.f / 3 : 2.f / 3) - 12; const auto winPos = GetWindowPos();
					GetWindowDrawList()->AddRectFilled({ winPos.x + restorePos.x, winPos.y + restorePos.y }, { winPos.x + restorePos.x + width, winPos.y + restorePos.y + 28 }, imColor, 3);

					// text
					SetCursorPos({ restorePos.x + 10, restorePos.y + 7 });
					TextUnformatted(sTag.c_str());

					if (!plTag.Label)
					{
						SetCursorPos({ restorePos.x + width / 2, restorePos.y + 7 });
						TextUnformatted(std::format("{}", plTag.Priority).c_str());
					}

					// buttons
					if (!plTag.Locked)
					{
						SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
						bDelete = IconButton(ICON_MD_DELETE);
					}

					SetCursorPos(restorePos);
					bClicked = Button(std::format("##{}", sTag).c_str(), { width, 28 });

					if (bClicked)
					{
						sName = sTag;
						tTag.Color = plTag.Color;
						tTag.Priority = plTag.Priority;
						tTag.Label = plTag.Label;
					}
					if (bDelete)
					{
						F::PlayerUtils.mTags.erase(sTag);
						F::PlayerUtils.bSaveTags = true;
						if (sName == sTag)
						{
							sName = "";
							tTag = {};
						}
					}
				};

			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			SetCursorPos({ 14, 80 }); FText("Priorities");
			SetCursorPos({ GetWindowSize().x * 2 / 3 + 10, 80 }); FText("Labels");
			PopStyleColor();

			std::vector<std::pair<std::string, PriorityLabel_t>> vPriorities = {}, vLabels = {};
			for (const auto& [sTag, plTag] : F::PlayerUtils.mTags)
			{
				if (!plTag.Label)
					vPriorities.push_back({ sTag, plTag });
				else
					vLabels.push_back({ sTag, plTag });
			}

			std::sort(vPriorities.begin(), vPriorities.end(), [&](const auto& a, const auto& b) -> bool
				{
					// override for default tag
					if (FNV1A::Hash(a.first.c_str()) == FNV1A::HashConst("Default"))
						return true;
					if (FNV1A::Hash(b.first.c_str()) == FNV1A::HashConst("Default"))
						return false;

					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.first < b.first;
				});
			std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
				{
					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.first < b.first;
				});

			// display tags
			int iPriorities = 0, iLabels = 0;
			for (const auto& pair : vPriorities)
			{
				drawTag(pair.first, pair.second, iPriorities);
				iPriorities++;
			}
			for (const auto& pair : vLabels)
			{
				drawTag(pair.first, pair.second, iLabels);
				iLabels++;
			}
			SetCursorPos({ 0, 60.f + 36.f * std::max(iPriorities, iLabels) }); DebugDummy({ 0, 28 });
		} EndSection();
		break;
		// MaterialManager
	case 3:
		if (BeginTable("MaterialsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Manager"))
			{
				static std::string newName;
				FSDropdown("Material name", &newName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
				if (FButton("Create", FButton_Fit | FButton_SameLine | FButton_Large) && newName.length() > 0)
				{
					F::Materials.AddMaterial(newName);
					newName.clear();
				}

				if (FButton("Folder", FButton_Fit | FButton_SameLine | FButton_Large))
					ShellExecuteA(nullptr, "open", MaterialFolder.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);

				std::vector<std::pair<std::string, Material_t>> vMaterials;
				for (auto const& [sName, mat] : F::Materials.mChamMaterials)
					vMaterials.push_back({ sName, mat });

				std::sort(vMaterials.begin(), vMaterials.end(), [&](const auto& a, const auto& b) -> bool
					{
						// override for none material
						if (FNV1A::Hash(a.first.c_str()) == FNV1A::HashConst("None"))
							return true;
						if (FNV1A::Hash(b.first.c_str()) == FNV1A::HashConst("None"))
							return false;

						// keep locked materials higher
						if (a.second.bLocked && !b.second.bLocked)
							return true;
						if (!a.second.bLocked && b.second.bLocked)
							return false;

						return a.first < b.first;
					});

				for (auto const& pair : vMaterials)
				{
					const auto current = GetCursorPos().y;

					SetCursorPos({ 14, current + 11 });
					TextColored(pair.second.bLocked ? F::Render.Inactive.Value : F::Render.Active.Value, pair.first.c_str());

					int o = 26;

					if (!pair.second.bLocked)
					{
						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteMat{}", pair.first).c_str());
						if (BeginPopupModal(std::format("Confirmation## DeleteMat{}", pair.first).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							Text(std::format("Do you really want to delete '{}'?", pair.first).c_str());

							if (FButton("Yes", FButton_Left))
							{
								F::Materials.RemoveMaterial(pair.first);
								CloseCurrentPopup();
							}
							if (FButton("No", FButton_Right | FButton_SameLine))
								CloseCurrentPopup();

							EndPopup();
						}
						o += 25;
					}

					SetCursorPos({ GetWindowSize().x - o, current + 9 });
					if (IconButton(ICON_MD_EDIT))
					{
						CurrentMaterial = pair.first;
						LockedMaterial = pair.second.bLocked;

						TextEditor.SetText(F::Materials.GetVMT(CurrentMaterial));
						TextEditor.SetReadOnly(LockedMaterial);
					}

					SetCursorPos({ 6, current }); DebugDummy({ 0, 28 });
				}
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (CurrentMaterial.length())
			{
				auto count = std::ranges::count(TextEditor.GetText(), '\n'); // doesn't account for text editor size otherwise
				if (Section("Editor", 81 + 15 * count, true))
				{
					// Toolbar
					if (!LockedMaterial)
					{
						if (FButton("Save", FButton_Fit))
						{
							auto text = TextEditor.GetText();
							text.erase(text.end() - 1, text.end()); // get rid of random newline
							F::Materials.EditMaterial(CurrentMaterial, text);
						}
						SameLine();
					}
					if (FButton("Close", FButton_Fit))
						CurrentMaterial = "";
					SameLine(); SetCursorPosY(GetCursorPosY() + 27);
					PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
					FText(LockedMaterial ? std::format("Viewing: {}", CurrentMaterial).c_str() : std::format("Editing: {}", CurrentMaterial).c_str(), FText_Right);
					PopStyleColor();

					// Text editor
					Dummy({ 0, 8 });

					PushFont(F::Render.FontMono);
					TextEditor.Render("TextEditor");
					PopFont();
				} EndSection();
			}

			EndTable();
		}
		break;
	}
}
#pragma endregion

void CMenu::AddDraggable(const char* szTitle, ConfigVar<DragBox_t>& var, bool bShouldDraw)
{
	using namespace ImGui;

	if (!bShouldDraw)
		return;

	static std::unordered_map<const char*, std::pair<DragBox_t, float>> old = {};
	DragBox_t info = FGet(var);
	const float sizeX = 100.f * Vars::Menu::DPI.Map["default"], sizeY = 40.f * Vars::Menu::DPI.Map["default"];
	SetNextWindowSize({ sizeX, sizeY }, ImGuiCond_Always);
	if (!old.contains(szTitle) || info != old[szTitle].first || sizeX != old[szTitle].second)
		SetNextWindowPos({ float(info.x - sizeX / 2), float(info.y) }, ImGuiCond_Always);

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { sizeX, sizeY });
	if (Begin(szTitle, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		const auto winPos = GetWindowPos();

		info.x = winPos.x + sizeX / 2; info.y = winPos.y; old[szTitle] = { info, sizeX };
		FSet(var, info);

		PushFont(F::Render.FontBlack);
		auto size = CalcTextSize(szTitle);
		SetCursorPos({ (sizeX - size.x) * 0.5f, (sizeY - size.y) * 0.5f });
		Text(szTitle);
		PopFont();

		End();
	}
	PopStyleVar(3);
	PopStyleColor(2);
}

void CMenu::DrawBinds()
{
	using namespace ImGui;

	if ((IsOpen ? !FGet(Vars::Menu::ShowBinds) : !Vars::Menu::ShowBinds.Value) || !IsOpen && I::EngineVGui->IsGameUIVisible())
		return;

	static DragBox_t old = {};
	DragBox_t info = IsOpen ? FGet(Vars::Menu::BindsDisplay) : Vars::Menu::BindsDisplay.Value;
	if (info != old)
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);

	std::vector<bool> actives;
	std::vector<std::string> titles;
	std::vector<std::string> infos;
	std::vector<std::string> states;
	float titleWidth = 0;
	float infoWidth = 0;
	float stateWidth = 0;

	PushFont(F::Render.FontSmall);
	std::function<void(std::string)> getConds = [&](std::string sParent)
		{
			auto uHash = FNV1A::Hash(sParent.c_str());
			for (auto& sCond : F::Conditions.vConditions)
			{
				auto& tCond = F::Conditions.mConditions[sCond];
				if (uHash != FNV1A::Hash(tCond.Parent.c_str()))
					continue;

				if (tCond.Visible)
				{
					std::string info; std::string state;
					switch (tCond.Type)
					{
						// key
					case 0:
						switch (tCond.Info)
						{
						case 0: { info = "hold"; break; }
						case 1: { info = "toggle"; break; }
						case 2: { info = "double"; break; }
						}
						state = VK2STR(tCond.Key);
						break;
						// class
					case 1:
						info = "class";
						switch (tCond.Info)
						{
						case 0: { state = "scout"; break; }
						case 1: { state = "soldier"; break; }
						case 2: { state = "pyro"; break; }
						case 3: { state = "demoman"; break; }
						case 4: { state = "heavy"; break; }
						case 5: { state = "engineer"; break; }
						case 6: { state = "medic"; break; }
						case 7: { state = "sniper"; break; }
						case 8: { state = "spy"; break; }
						}
						break;
						// weapon type
					case 2:
						info = "weapon";
						switch (tCond.Info)
						{
						case 0: { state = "hitscan"; break; }
						case 1: { state = "projectile"; break; }
						case 2: { state = "melee"; break; }
						}
					}
					if (tCond.Not)
						info = std::format("not {}", info);

					actives.push_back(tCond.Active);
					titles.push_back(sCond);
					infos.push_back(info);
					states.push_back(state);
					titleWidth = std::max(titleWidth, CalcTextSize(sCond.c_str()).x);
					infoWidth = std::max(infoWidth, CalcTextSize(info.c_str()).x);
					stateWidth = std::max(stateWidth, CalcTextSize(state.c_str()).x);
				}

				if (tCond.Active)
					getConds(sCond);
			}
		};
	getConds("");

	SetNextWindowSize({ std::max(titleWidth + infoWidth + stateWidth + 42, 56.f), 18.f * actives.size() + 38 });
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { 40.f, 40.f });
	if (Begin("Binds", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
	{
		const auto winPos = GetWindowPos();

		info.x = winPos.x; info.y = winPos.y; old = info;
		if (IsOpen)
			FSet(Vars::Menu::BindsDisplay, info);

		PushFont(F::Render.FontLarge);
		SetCursorPos({ 11, 9 });
		Text("Binds");
		PopFont();

		const float width = std::max(titleWidth + infoWidth + stateWidth + 42, 56.f);
		GetWindowDrawList()->AddRectFilled({ winPos.x + 8, winPos.y + 26 }, { winPos.x + width - 8, winPos.y + 27 }, F::Render.Accent, 3);

		for (size_t i = 0; i < actives.size(); i++)
		{
			SetCursorPos({ 12, 18.f * i + 35 });
			PushStyleColor(ImGuiCol_Text, actives[i] ? F::Render.Accent.Value : F::Render.Inactive.Value);
			Text(titles[i].c_str());
			PopStyleColor();

			SetCursorPos({ titleWidth + 22, 18.f * i + 35 });
			PushStyleColor(ImGuiCol_Text, actives[i] ? F::Render.Active.Value : F::Render.Inactive.Value);
			Text(infos[i].c_str());

			SetCursorPos({ titleWidth + infoWidth + 32, 18.f * i + 35 });
			Text(states[i].c_str());
			PopStyleColor();
		}

		End();
	}
	PopStyleVar();
	PopFont();
}

/* Window for the camera feature */
void CMenu::DrawCameraWindow()
{
	using namespace ImGui;

	if (!FGet(Vars::Visuals::Simulation::ProjectileCamera))
		return;

	static WindowBox_t old = {};
	WindowBox_t info = FGet(Vars::Visuals::Simulation::ProjectileWindow);
	if (info != old)
	{
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);
		SetNextWindowSize({ float(info.w), float(info.h) }, ImGuiCond_Always);
	}

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { 100.f, 100.f });
	if (Begin("Camera", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		const auto winPos = GetWindowPos();
		const auto winSize = GetWindowSize();

		info.x = winPos.x; info.y = winPos.y; info.w = winSize.x; info.h = winSize.y; old = info;
		FSet(Vars::Visuals::Simulation::ProjectileWindow, info);

		PushFont(F::Render.FontBlack);
		auto size = CalcTextSize("Camera");
		SetCursorPos({ (winSize.x - size.x) * 0.5f, (winSize.y - size.y) * 0.5f });
		Text("Camera");
		PopFont();

		End();
	}
	PopStyleVar(3);
	PopStyleColor(2);
}

static void SquareConstraints(ImGuiSizeCallbackData* data) { data->DesiredSize.x = data->DesiredSize.y = std::max(data->DesiredSize.x, data->DesiredSize.y); }
void CMenu::DrawRadar()
{
	using namespace ImGui;

	if (!FGet(Vars::Radar::Main::Enabled))
		return;

	static WindowBox_t old = {};
	WindowBox_t info = FGet(Vars::Radar::Main::Window);
	if (info != old)
	{
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);
		SetNextWindowSize({ float(info.w), float(info.w) }, ImGuiCond_Always);
	}

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
	SetNextWindowSizeConstraints({ 100.f, 100.f }, { 1000.f, 1000.f }, SquareConstraints);
	if (Begin("Radar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		const ImVec2 winPos = GetWindowPos();
		const ImVec2 winSize = GetWindowSize();

		info.x = winPos.x; info.y = winPos.y; info.w = winSize.x; old = info;
		FSet(Vars::Radar::Main::Window, info);

		PushFont(F::Render.FontBlack);
		auto size = CalcTextSize("Radar");
		SetCursorPos({ (winSize.x - size.x) * 0.5f, (winSize.y - size.y) * 0.5f });
		Text("Radar");
		PopFont();

		End();
	}
	PopStyleVar(2);
	PopStyleColor(2);
}

void CMenu::Render()
{
	using namespace ImGui;

	if (!ConfigLoaded)
		return;

	InKeybind = false;
	if (U::KeyHandler.Pressed(Vars::Menu::MenuPrimaryKey.Value) || U::KeyHandler.Pressed(Vars::Menu::MenuSecondaryKey.Value))
		I::MatSystemSurface->SetCursorAlwaysVisible(IsOpen = !IsOpen);

	DrawBinds();

	if (IsOpen)
	{
		KeyHandler();

		DrawMenu();

		DrawCameraWindow();
		DrawRadar();

		AddDraggable("Ticks", Vars::Menu::TicksDisplay, FGet(Vars::Menu::Indicators) & (1 << 0));
		AddDraggable("Crit hack", Vars::Menu::CritsDisplay, FGet(Vars::Menu::Indicators) & (1 << 1));
		AddDraggable("Spectators", Vars::Menu::SpectatorsDisplay, FGet(Vars::Menu::Indicators) & (1 << 2));
		AddDraggable("Ping", Vars::Menu::PingDisplay, FGet(Vars::Menu::Indicators) & (1 << 3));
		AddDraggable("Conditions", Vars::Menu::ConditionsDisplay, FGet(Vars::Menu::Indicators) & (1 << 4));
		AddDraggable("Seed prediction", Vars::Menu::SeedPredictionDisplay, FGet(Vars::Menu::Indicators) & (1 << 5));

		F::Render.Cursor = GetMouseCursor();
	}
	else
		mActives.clear();
}