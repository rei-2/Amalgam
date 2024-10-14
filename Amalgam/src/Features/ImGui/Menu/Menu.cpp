#include "Menu.h"

#include "Components.h"
#include "../../Configs/Configs.h"
#include "../../Binds/Binds.h"
#include "../../Players/PlayerUtils.h"
#include "../../CameraWindow/CameraWindow.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"
#include "../../Resolver/Resolver.h"
#include "../../Misc/Misc.h"
#include "../../Records/Records.h"
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
		case 4: FTabs({ "CONFIG", "BINDS", "PLAYERLIST", "MATERIALS" }, &CurrentConfigTab, SubTabSize, { TabSize.x, 0 }); break;
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

	// Bind Text
	Bind_t tBind;
	if (F::Binds.GetBind(CurrentBind, &tBind))
	{
		const auto textSize = CalcTextSize(std::format("Editing for condition {}", tBind.Name).c_str());
		SetNextWindowSize({ std::min(textSize.x + 56.f, mainWindowSize.x), 40.f });
		SetNextWindowPos({ mainWindowPos.x, mainWindowPos.y + mainWindowSize.y + 8.f });
		PushStyleVar(ImGuiStyleVar_WindowMinSize, { 40.f, 40.f });
		if (Begin("ConditionWindow", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
		{
			const auto windowPos = GetWindowPos();
			const auto preSize = CalcTextSize("Editing for condition ");
			GetWindowDrawList()->AddText(F::Render.FontRegular, F::Render.FontRegular->FontSize, { windowPos.x + 16.f, windowPos.y + 13.f }, F::Render.Active, "Editing for condition ");
			GetWindowDrawList()->AddText(F::Render.FontRegular, F::Render.FontRegular->FontSize, { windowPos.x + 16.f + preSize.x, windowPos.y + 13.f }, F::Render.Accent, tBind.Name.c_str());

			SetCursorPos({ textSize.x + 28, 11 });
			if (IconButton(ICON_MD_CANCEL))
				CurrentBind = DEFAULT_BIND;

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
				FSlider("Aim FOV", Vars::Aimbot::General::AimFOV, 0.f, 180.f, 1.f, "%g", FSlider_Clamp | FSlider_Precision);
				PushTransparent(FGet(Vars::Aimbot::General::AimType) != 2);
					FSlider("Smoothing## Hitscan", Vars::Aimbot::General::Smoothing, 0.f, 100.f, 1.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				PopTransparent();
				FSlider("Max targets", Vars::Aimbot::General::MaxTargets, 1, 6, 1, "%i", FSlider_Min);
				PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & (1 << 1)));
					FSlider("Ignore cloak", Vars::Aimbot::General::IgnoreCloakPercentage, 0, 100, 10, "%d%%", FSlider_Clamp);
				PopTransparent();
				PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & (1 << 4)));
					FSlider("Tick tolerance", Vars::Aimbot::General::TickTolerance, 0, 21, 1, "%i", FSlider_Clamp);
				PopTransparent();
				FColorPicker("Aimbot FOV circle", Vars::Colors::FOVCircle);
				FToggle("Autoshoot", Vars::Aimbot::General::AutoShoot);
				FToggle("FOV Circle", Vars::Aimbot::General::FOVCircle, FToggle_Middle);
				FToggle("Force crits", Vars::CritHack::ForceCrits);
				FToggle("Avoid random crits", Vars::CritHack::AvoidRandom, FToggle_Middle);
				FToggle("Always melee crit", Vars::CritHack::AlwaysMeleeCrit);
				FToggle("No spread", Vars::Aimbot::General::NoSpread, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## aimbot"))
				{
					FSlider("hitscan peek", Vars::Aimbot::General::HitscanPeek, 0, 5);
					FToggle("peek dt only", Vars::Aimbot::General::PeekDTOnly); // this should probably stay on if you want to be able to target hitboxes other than the highest priority one
					FSlider("offset## nospread", Vars::Aimbot::General::NoSpreadOffset, -1.f, 1.f, 0.5f, "%g", FSlider_Precision);
					FSlider("average", Vars::Aimbot::General::NoSpreadAverage, 1, 25);
					FDropdown("aim holds fire", Vars::Aimbot::General::AimHoldsFire, { "false", "minigun only", "true" });
				} EndSection();
			}
			if (Section("Backtrack"))
			{
				FToggle("Enabled", Vars::Backtrack::Enabled);
				FToggle("Prefer on shot", Vars::Backtrack::PreferOnShot, FToggle_Middle);
				FSlider("Fake latency", Vars::Backtrack::Latency, 0, F::Backtrack.flMaxUnlag * 1000, 5, "%i", FSlider_Clamp); // unreliable above 1000 - ping probably
				FSlider("Fake interp", Vars::Backtrack::Interp, 0, F::Backtrack.flMaxUnlag * 1000, 5, "%i", FSlider_Clamp | FSlider_Precision);
				FSlider("Window", Vars::Backtrack::Window, 1, 200, 5, "%i", FSlider_Clamp);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## backtrack"))
				{
					FSlider("offset", Vars::Backtrack::Offset, -1, 1);
				} EndSection();
			}
			if (Section("Auto Heal"))
			{
				FToggle("Auto heal", Vars::Aimbot::Healing::AutoHeal);
				FToggle("Friends only", Vars::Aimbot::Healing::FriendsOnly, FToggle_Middle);
				FToggle("Activate on voice", Vars::Aimbot::Healing::ActivateOnVoice);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Hitscan"))
			{
				FDropdown("Hitboxes", Vars::Aimbot::Hitscan::Hitboxes, { "Head", "Body", "Pelvis", "Arms", "Legs" }, { 1 << 0, 1 << 2, 1 << 1, 1 << 3, 1 << 4 }, FDropdown_Multi);
				FDropdown("Modifiers## Hitscan", Vars::Aimbot::Hitscan::Modifiers, { "Tapfire", "Wait for heatshot", "Wait for charge", "Scoped only", "Auto scope", "Bodyaim if lethal", "Extinguish team" }, {}, FDropdown_Multi);
				FSlider("Point scale", Vars::Aimbot::Hitscan::PointScale, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				PushTransparent(!(FGet(Vars::Aimbot::Hitscan::Modifiers) & (1 << 0)));
					FSlider("Tapfire distance", Vars::Aimbot::Hitscan::TapFireDist, 250.f, 1000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
					PopTransparent();
			} EndSection();
			if (Section("Projectile"))
			{
				FDropdown("Predict", Vars::Aimbot::Projectile::StrafePrediction, { "Air strafing", "Ground strafing" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Splash", Vars::Aimbot::Projectile::SplashPrediction, { "Off", "Include", "Prefer", "Only" }, {}, FDropdown_Right);
				FDropdown("Auto detonate", Vars::Aimbot::Projectile::AutoDetonate, { "Stickies", "Flares" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Auto airblast", Vars::Aimbot::Projectile::AutoAirblast, { "Off", "Legit", "Rage" }, {}, FDropdown_Right);
				FDropdown("Modifiers## Projectile", Vars::Aimbot::Projectile::Modifiers, { "Charge shot", "Cancel charge", "Bodyaim if lethal", "Use prime time" }, {}, FDropdown_Multi);
				FSlider("Max simulation time", Vars::Aimbot::Projectile::PredictionTime, 0.1f, 10.f, 0.25f, "%gs", FSlider_Min | FSlider_Precision);
				PushTransparent(!FGet(Vars::Aimbot::Projectile::StrafePrediction));
					FSlider("Hit chance", Vars::Aimbot::Projectile::Hitchance, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				PopTransparent();
				FSlider("Autodet radius", Vars::Aimbot::Projectile::AutodetRadius, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				FSlider("Splash radius", Vars::Aimbot::Projectile::SplashRadius, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				PushTransparent(!FGet(Vars::Aimbot::Projectile::AutoRelease));
					FSlider("Auto release", Vars::Aimbot::Projectile::AutoRelease, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				PopTransparent();
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## projectile"))
				{
					FText("ground");
					FSlider("samples##ground", Vars::Aimbot::Projectile::GroundSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("straight fuzzy value##ground", Vars::Aimbot::Projectile::GroundStraightFuzzyValue, 0.f, 500.f, 25.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					FSlider("low min samples##ground", Vars::Aimbot::Projectile::GroundLowMinimumSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("high min samples##ground", Vars::Aimbot::Projectile::GroundHighMinimumSamples, 3, 66, 1, "%i", FSlider_Right);
					FSlider("low min distance##ground", Vars::Aimbot::Projectile::GroundLowMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("high min distance##ground", Vars::Aimbot::Projectile::GroundHighMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);

					FText("air");
					FSlider("samples##air", Vars::Aimbot::Projectile::AirSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("straight fuzzy value##air", Vars::Aimbot::Projectile::AirStraightFuzzyValue, 0.f, 500.f, 25.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					FSlider("low min samples##air", Vars::Aimbot::Projectile::AirLowMinimumSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("high min samples##air", Vars::Aimbot::Projectile::AirHighMinimumSamples, 3, 66, 1, "%i", FSlider_Right);
					FSlider("low min distance##air", Vars::Aimbot::Projectile::AirLowMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("high min distance##air", Vars::Aimbot::Projectile::AirHighMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);

					FText("");
					FSlider("velocity average count", Vars::Aimbot::Projectile::VelocityAverageCount, 1, 10, 1, "%i", FSlider_Left);
					FSlider("vertical shift", Vars::Aimbot::Projectile::VerticalShift, 0.f, 10.f, 0.5f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					FSlider("huntsman lerp", Vars::Aimbot::Projectile::HuntsmanLerp, 0.f, 100.f, 1.f, "%g%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FSlider("latency offset", Vars::Aimbot::Projectile::LatencyOffset, -1.f, 1.f, 0.1f, "%g", FSlider_Right | FSlider_Precision);
					FSlider("hull increase", Vars::Aimbot::Projectile::HullIncrease, 0.f, 3.f, 0.5f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("drag override", Vars::Aimbot::Projectile::DragOverride, 0.f, 1.f, 0.01f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					FSlider("time override", Vars::Aimbot::Projectile::TimeOverride, 0.f, 1.f, 0.01f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("splash points", Vars::Aimbot::Projectile::SplashPoints, 0, 100, 1, "%i", FSlider_Right);
					FSlider("splash count", Vars::Aimbot::Projectile::SplashCount, 1, 5, 1, "%i", FSlider_Left);
					FSlider("delta count", Vars::Aimbot::Projectile::DeltaCount, 1, 5, 1, "%i", FSlider_Right);
					FDropdown("delta mode", Vars::Aimbot::Projectile::DeltaMode, { "average", "max" });
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
				{
					FSlider("swing ticks", Vars::Aimbot::Melee::SwingTicks, 10, 14);
					FToggle("backstab account ping", Vars::Aimbot::Melee::BackstabAccountPing);
					FToggle("backstab double test", Vars::Aimbot::Melee::BackstabDoubleTest, FToggle_Middle);
				}
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
				FSlider("Tick limit", Vars::CL_Move::Doubletap::TickLimit, 2, 22, 1, "%i", FSlider_Clamp);
				FSlider("Warp rate", Vars::CL_Move::Doubletap::WarpRate, 2, 22, 1, "%i", FSlider_Clamp);
				FSlider("Passive recharge", Vars::CL_Move::Doubletap::PassiveRecharge, 0, 67, 1, "%i", FSlider_Clamp);
			} EndSection();
			if (Section("Fakelag"))
			{
				FDropdown("Fakelag", Vars::CL_Move::Fakelag::Fakelag, { "Off", "Plain", "Random", "Adaptive" }, {}, FSlider_Left);
				FDropdown("Options", Vars::CL_Move::Fakelag::Options, { "Only moving", "On unduck", "Not airborne" }, {}, FDropdown_Multi | FSlider_Right);
				PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != 1);
					FSlider("Plain ticks", Vars::CL_Move::Fakelag::PlainTicks, 1, 22, 1, "%i", FSlider_Clamp | FSlider_Left);
				PopTransparent();
				PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != 2);
					FSlider("Random ticks", Vars::CL_Move::Fakelag::RandomTicks, 1, 22, 1, "%i - %i", FSlider_Clamp | FSlider_Right);
				PopTransparent();
				FToggle("Unchoke on attack", Vars::CL_Move::Fakelag::UnchokeOnAttack);
				FToggle("Retain blastjump", Vars::CL_Move::Fakelag::RetainBlastJump, FToggle_Middle);
			} EndSection();
			if (Section("Anti Aim"))
			{
				FToggle("Enabled", Vars::AntiHack::AntiAim::Enabled);
				FDropdown("Real pitch", Vars::AntiHack::AntiAim::PitchReal, { "None", "Up", "Down", "Zero", "Jitter", "Reverse jitter" }, {}, FDropdown_Left);
				FDropdown("Fake pitch", Vars::AntiHack::AntiAim::PitchFake, { "None", "Up", "Down", "Jitter", "Reverse jitter" }, {}, FDropdown_Right);
				FDropdown("Real yaw", Vars::AntiHack::AntiAim::YawReal, { "Forward", "Left", "Right", "Backwards", "Edge", "Jitter", "Spin" }, {}, FDropdown_Left);
				FDropdown("Fake yaw", Vars::AntiHack::AntiAim::YawFake, { "Forward", "Left", "Right", "Backwards", "Edge", "Jitter", "Spin" }, {}, FDropdown_Right);
				FDropdown("Real offset", Vars::AntiHack::AntiAim::RealYawMode, { "View", "Target" }, {}, FDropdown_Left);
				FDropdown("Fake offset", Vars::AntiHack::AntiAim::FakeYawMode, { "View", "Target" }, {}, FDropdown_Right);
				FSlider("Real offset## Offset", Vars::AntiHack::AntiAim::RealYawOffset, -180, 180, 5, "%i", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				FSlider("Fake offset## Offset", Vars::AntiHack::AntiAim::FakeYawOffset, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawReal) != 5);
					FSlider("Real jitter", Vars::AntiHack::AntiAim::RealJitter, -90.f, 90.f, 1.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				PopTransparent();
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != 5);
					FSlider("Fake jitter", Vars::AntiHack::AntiAim::FakeJitter, -90.f, 90.f, 1.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				PopTransparent();
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != 6 && FGet(Vars::AntiHack::AntiAim::YawReal) != 6);
					FSlider("Spin speed", Vars::AntiHack::AntiAim::SpinSpeed, -30.f, 30.f, 1.f, "%g", FSlider_Left | FSlider_Precision);
				PopTransparent();
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
				PushTransparent(!FGet(Vars::AntiHack::Resolver::Resolver));
					FToggle("Ignore in-air", Vars::AntiHack::Resolver::IgnoreAirborne, FToggle_Middle);
				PopTransparent();
			} EndSection();
			if (Section("Auto Peek"))
			{
				FToggle("Auto peek", Vars::CL_Move::AutoPeek);
			} EndSection();
			if (Section("Speedhack"))
			{
				FToggle("Speedhack", Vars::CL_Move::SpeedEnabled);
				PushTransparent(!FGet(Vars::CL_Move::SpeedEnabled));
					FSlider("SpeedHack factor", Vars::CL_Move::SpeedFactor, 1, 50, 1);
				PopTransparent();
			} EndSection();
			if (Section("Cheater Detection"))
			{
				PushTransparent(!FGet(Vars::CheaterDetection::Methods));
					FDropdown("Detection methods", Vars::CheaterDetection::Methods, { "Invalid pitch", "Packet choking", "Aim flicking", "Duck Speed" }, {}, FDropdown_Multi);
					FSlider("Detections required", Vars::CheaterDetection::DetectionsRequired, 10, 50, 1);

					PushTransparent(!(FGet(Vars::CheaterDetection::Methods) & (1 << 1)));
						FSlider("Minimum choking", Vars::CheaterDetection::MinimumChoking, 4, 22, 1);
					PopTransparent();

					PushTransparent(!(FGet(Vars::CheaterDetection::Methods) & (1 << 2)));
						FSlider("Minimum flick angle", Vars::CheaterDetection::MinimumFlick, 10.f, 30.f, 1.f, "%.0f", FSlider_Left);
						FSlider("Maximum noise", Vars::CheaterDetection::MaximumNoise, 1.f, 10.f, 1.f, "%.0f", FSlider_Right);
					PopTransparent();
				PopTransparent();
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
				FDropdown("Draw", Vars::ESP::Draw, { "Players", "Buildings", "Projectiles", "Objective", "NPCs", "Health", "Ammo", "Money", "Powerups", "Bombs", "Spellbook", "Gargoyle" }, {}, FDropdown_Multi);
				PushTransparent(!(FGet(Vars::ESP::Draw) & (1 << 0)));
					FDropdown("Player", Vars::ESP::Player, { "Enemy", "Team", "Local", "Friends", "Prioritized", "##Divider", "Name", "Box", "Distance", "Bones", "Health bar", "Health text", "Uber bar", "Uber text", "Class icon", "Class text", "Weapon icon", "Weapon text", "Priority", "Labels", "Buffs", "Debuffs", "Misc", "Lag compensation", "Ping", "KDR" }, {}, FDropdown_Multi);
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & (1 << 1)));
					FDropdown("Building", Vars::ESP::Building, { "Enemy", "Team", "Local", "Friends", "Prioritized", "##Divider", "Name", "Box", "Distance", "Health bar", "Health text", "Owner", "Level", "Flags" }, {}, FDropdown_Multi);
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & (1 << 2)));
					FDropdown("Projectile", Vars::ESP::Projectile, { "Enemy", "Team", "Local", "Friends", "Prioritized", "##Divider", "Name", "Box", "Distance", "Flags" }, {}, FDropdown_Multi);
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & (1 << 3)));
					FDropdown("Objective", Vars::ESP::Objective, { "Enemy", "Team", "##Divider", "Name", "Box", "Distance", "Flags", "Intel return time" }, {}, FDropdown_Multi);
				PopTransparent();
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
				FColorPicker("Money color", Vars::Colors::Money, 0, FColorPicker_Left);
				FColorPicker("Powerup color", Vars::Colors::Powerup, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("NPC color", Vars::Colors::NPC, 0, FColorPicker_Left);
				FColorPicker("Halloween color", Vars::Colors::Halloween, 0, FColorPicker_Middle | FColorPicker_SameLine);

				FSlider("Active alpha", Vars::ESP::ActiveAlpha, 0, 255, 5, "%i", FSlider_Clamp);
				FSlider("Dormant alpha", Vars::ESP::DormantAlpha, 0, 255, 5, "%i", FSlider_Clamp);
				FSlider("Dormant decay", Vars::ESP::DormantTime, 0.015f, 5.0f, 0.1f, "%gs", FSlider_Left | FSlider_Min | FSlider_Precision);
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

				FMDropdown("Visible material", Vars::Chams::Friendly::Visible, FDropdown_Left);
				FMDropdown("Occluded material", Vars::Chams::Friendly::Occluded, FDropdown_Right);
			} EndSection();
			if (Section("Enemy"))
			{
				FToggle("Players", Vars::Chams::Enemy::Players);
				FToggle("Ragdolls", Vars::Chams::Enemy::Ragdolls, FToggle_Middle);
				FToggle("Buildings", Vars::Chams::Enemy::Buildings);
				FToggle("Projectiles", Vars::Chams::Enemy::Projectiles, FToggle_Middle);

				FMDropdown("Visible material", Vars::Chams::Enemy::Visible, FDropdown_Left);
				FMDropdown("Occluded material", Vars::Chams::Enemy::Occluded, FDropdown_Right);
			} EndSection();
			if (Section("World"))
			{
				FToggle("NPCs", Vars::Chams::World::NPCs);
				FToggle("Pickups", Vars::Chams::World::Pickups, FToggle_Middle);
				FToggle("Objective", Vars::Chams::World::Objective);
				FToggle("Powerups", Vars::Chams::World::Powerups, FToggle_Middle);
				FToggle("Bombs", Vars::Chams::World::Bombs);
				FToggle("Halloween", Vars::Chams::World::Halloween, FToggle_Middle);

				FMDropdown("Visible material", Vars::Chams::World::Visible, FDropdown_Left);
				FMDropdown("Occluded material", Vars::Chams::World::Occluded, FDropdown_Right);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Player"))
			{
				FToggle("Local", Vars::Chams::Player::Local);
				FToggle("Friend", Vars::Chams::Player::Friend, FToggle_Middle);
				FToggle("Priority", Vars::Chams::Player::Priority);

				FMDropdown("Visible material", Vars::Chams::Player::Visible, FDropdown_Left);
				FMDropdown("Occluded material", Vars::Chams::Player::Occluded, FDropdown_Right);
			} EndSection();
			if (Section("Backtrack"))
			{
				FToggle("Enabled", Vars::Chams::Backtrack::Enabled);
				SameLine(GetWindowSize().x / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
				FDropdown("Draw", Vars::Chams::Backtrack::Draw, { "Last", "Last + first", "All" }, {}, FDropdown_Left);

				FMDropdown("Material", Vars::Chams::Backtrack::Visible, FDropdown_None);
			} EndSection();
			if (Section("Fake Angle"))
			{
				FToggle("Enabled", Vars::Chams::FakeAngle::Enabled);

				FMDropdown("Material", Vars::Chams::FakeAngle::Visible, FDropdown_None);
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Weapon", Vars::Chams::Viewmodel::Weapon);
				FToggle("Hands", Vars::Chams::Viewmodel::Hands, FToggle_Middle);

				FMDropdown("Weapon material", Vars::Chams::Viewmodel::WeaponVisible, FDropdown_Left);
				FMDropdown("Hands material", Vars::Chams::Viewmodel::HandsVisible, FDropdown_Right);
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

				PushTransparent(!FGet(Vars::Glow::Friendly::Stencil));
					FSlider("Stencil scale## Friendly", Vars::Glow::Friendly::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Friendly::Blur));
					FSlider("Blur scale## Friendly", Vars::Glow::Friendly::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
				PopTransparent();
			} EndSection();
			if (Section("Enemy"))
			{
				FToggle("Players", Vars::Glow::Enemy::Players);
				FToggle("Ragdolls", Vars::Glow::Enemy::Ragdolls, FToggle_Middle);
				FToggle("Buildings", Vars::Glow::Enemy::Buildings);
				FToggle("Projectiles", Vars::Glow::Enemy::Projectiles, FToggle_Middle);

				PushTransparent(!FGet(Vars::Glow::Enemy::Stencil));
					FSlider("Stencil scale## Enemy", Vars::Glow::Enemy::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Enemy::Blur));
					FSlider("Blur scale## Enemy", Vars::Glow::Enemy::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
				PopTransparent();
			} EndSection();
			if (Section("World"))
			{
				FToggle("NPCs", Vars::Glow::World::NPCs);
				FToggle("Pickups", Vars::Glow::World::Pickups, FToggle_Middle);
				FToggle("Objective", Vars::Glow::World::Objective);
				FToggle("Powerups", Vars::Glow::World::Powerups, FToggle_Middle);
				FToggle("Bombs", Vars::Glow::World::Bombs);
				FToggle("Halloween", Vars::Glow::World::Halloween, FToggle_Middle);

				PushTransparent(!FGet(Vars::Glow::World::Stencil));
					FSlider("Stencil scale## World", Vars::Glow::World::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::World::Blur));
					FSlider("Blur scale## World", Vars::Glow::World::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Player"))
			{
				FToggle("Local", Vars::Glow::Player::Local);
				FToggle("Friend", Vars::Glow::Player::Friend, FToggle_Middle);
				FToggle("Priority", Vars::Glow::Player::Priority);

				PushTransparent(!FGet(Vars::Glow::Player::Stencil));
					FSlider("Stencil scale## Player", Vars::Glow::Player::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Player::Blur));
					FSlider("Blur scale## Player", Vars::Glow::Player::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
				PopTransparent();
			} EndSection();
			if (Section("Backtrack"))
			{
				FToggle("Enabled", Vars::Glow::Backtrack::Enabled);
				SameLine(GetWindowSize().x / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
				FDropdown("Draw", Vars::Glow::Backtrack::Draw, { "Last", "Last + first", "All" }, {}, FDropdown_Left);

				PushTransparent(!FGet(Vars::Glow::Backtrack::Stencil));
					FSlider("Stencil scale## Backtrack", Vars::Glow::Backtrack::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Backtrack::Blur));
					FSlider("Blur scale## Backtrack", Vars::Glow::Backtrack::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
				PopTransparent();
			} EndSection();
			if (Section("Fake Angle"))
			{
				FToggle("Enabled", Vars::Glow::FakeAngle::Enabled);

				PushTransparent(!FGet(Vars::Glow::FakeAngle::Stencil));
					FSlider("Stencil scale## FakeAngle", Vars::Glow::FakeAngle::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::FakeAngle::Blur));
					FSlider("Blur scale## FakeAngle", Vars::Glow::FakeAngle::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
				PopTransparent();
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Weapon", Vars::Glow::Viewmodel::Weapon);
				FToggle("Hands", Vars::Glow::Viewmodel::Hands, FToggle_Middle);

				PushTransparent(!FGet(Vars::Glow::Viewmodel::Stencil));
					FSlider("Stencil scale## Viewmodel", Vars::Glow::Viewmodel::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Viewmodel::Blur));
					FSlider("Blur scale## Viewmodel", Vars::Glow::Viewmodel::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
				PopTransparent();
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
				FDropdown("Streamer mode", Vars::Visuals::UI::StreamerMode, { "Off", "Local", "Friends", "All" }, {}, FDropdown_Left);
				FDropdown("Chat tags", Vars::Visuals::UI::ChatTags, { "Local", "Friends", "Assigned" }, {}, FDropdown_Right | FDropdown_Multi);
				FSlider("Field of view", Vars::Visuals::UI::FieldOfView, 0, 160, 1, "%i");
				FSlider("Zoomed field of view", Vars::Visuals::UI::ZoomFieldOfView, 0, 160, 1, "%i");
				FToggle("Reveal scoreboard", Vars::Visuals::UI::RevealScoreboard);
				FToggle("Scoreboard utility", Vars::Visuals::UI::ScoreboardUtility, FToggle_Middle);
				FToggle("Scoreboard colors", Vars::Visuals::UI::ScoreboardColors);
				FToggle("Clean screenshots", Vars::Visuals::UI::CleanScreenshots, FToggle_Middle);
				FToggle("Sniper sightlines", Vars::Visuals::UI::SniperSightlines);
				FToggle("Pickup timers", Vars::Visuals::UI::PickupTimers, FToggle_Middle);
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Crosshair aim position", Vars::Visuals::Viewmodel::CrosshairAim);
				FToggle("Viewmodel aim position", Vars::Visuals::Viewmodel::ViewmodelAim, FToggle_Middle);
				FSlider("Offset X", Vars::Visuals::Viewmodel::OffsetX, -45, 45, 5, "%i", FSlider_Precision);
				FSlider("Offset Y", Vars::Visuals::Viewmodel::OffsetY, -45, 45, 5, "%i", FSlider_Precision);
				FSlider("Offset Z", Vars::Visuals::Viewmodel::OffsetZ, -45, 45, 5, "%i", FSlider_Precision);
				FSlider("Roll", Vars::Visuals::Viewmodel::Roll, -180, 180, 5, "%i", FSlider_Clamp | FSlider_Precision);
				PushTransparent(!FGet(Vars::Visuals::Viewmodel::SwayScale) || !FGet(Vars::Visuals::Viewmodel::SwayInterp));
					FSlider("Sway scale", Vars::Visuals::Viewmodel::SwayScale, 0.f, 5.f, 0.5f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("Sway interp", Vars::Visuals::Viewmodel::SwayInterp, 0.f, 1.f, 0.1f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
				PopTransparent();
			} EndSection();
			if (Section("Particles"))
			{
				// https://developer.valvesoftware.com/wiki/Team_Fortress_2/Particles
				FSDropdown("Bullet trail", Vars::Visuals::Particles::BulletTrail, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink", "Line", "Beam" }, FDropdown_Left | FSDropdown_Custom);
				FSDropdown("Crit trail", Vars::Visuals::Particles::CritTrail, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink", "Line", "Beam" }, FDropdown_Right | FSDropdown_Custom);
				FSDropdown("Medigun beam", Vars::Visuals::Particles::MedigunBeam, { "Off", "None", "Uber", "Dispenser", "Passtime", "Bombonomicon", "White" }, FDropdown_Left | FSDropdown_Custom);
				FSDropdown("Medigun charge", Vars::Visuals::Particles::MedigunCharge, { "Off", "None", "Electrocuted", "Halloween", "Fireball", "Burning", "Scorching", "Purple energy", "Green energy", "Nebula", "Purple stars", "Green stars", "Sunbeams", "Spellbound", "Purple sparks", "Yellow sparks", "Green zap", "Yellow zap", "Plasma", "Frostbite", "Time warp", "Purple souls", "Green souls", "Bubbles", "Hearts" }, FDropdown_Right | FSDropdown_Custom);
				FSDropdown("Projectile trail", Vars::Visuals::Particles::ProjectileTrail, { "Off", "None", "Rocket", "Critical", "Energy", "Charged", "Ray", "Fireball", "Fire", "Flame", "Sparks", "Flare", "Trail", "Health", "Smoke", "Bubbles", "Halloween", "Monoculus", "Sparkles", "Rainbow" }, FDropdown_Left | FSDropdown_Custom);
				FDropdown("Spell footsteps", Vars::Visuals::Particles::SpellFootsteps, { "Off", "Color", "Team", "Halloween" }, {}, FDropdown_Right, 1);
				FColorPicker("Spell footstep", Vars::Colors::SpellFootstep, 0, FColorPicker_Dropdown);
				FToggle("Draw icons through walls", Vars::Visuals::Particles::DrawIconsThroughWalls);
				FToggle("Draw damage numbers through walls", Vars::Visuals::Particles::DrawDamageNumbersThroughWalls);
			} EndSection();
			if (Section("Ragdolls"))
			{
				FToggle("No ragdolls", Vars::Visuals::Ragdolls::NoRagdolls);
				FToggle("No gibs", Vars::Visuals::Ragdolls::NoGib, FToggle_Middle);
				FToggle("Mods", Vars::Visuals::Ragdolls::Enabled);
				PushTransparent(!FGet(Vars::Visuals::Ragdolls::Enabled));
					FToggle("Enemy only", Vars::Visuals::Ragdolls::EnemyOnly, FToggle_Middle);
					FDropdown("Ragdoll effects", Vars::Visuals::Ragdolls::Effects, { "Burning", "Electrocuted", "Ash", "Dissolve" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Ragdoll model", Vars::Visuals::Ragdolls::Type, { "None", "Gold", "Ice" }, {}, FDropdown_Right);
					FSlider("Ragdoll force", Vars::Visuals::Ragdolls::Force, -10.f, 10.f, 0.5f, "%g", FSlider_Precision);
					FSlider("Horizontal force", Vars::Visuals::Ragdolls::ForceHorizontal, -10.f, 10.f, 0.5f, "%g", FSlider_Precision);
					FSlider("Vertical force", Vars::Visuals::Ragdolls::ForceVertical, -10.f, 10.f, 0.5f, "%g", FSlider_Precision);
				PopTransparent();
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
				FDropdown("Style", Vars::Visuals::Simulation::Style, { "Line", "Separators", "Spaced" }, {}, FDropdown_Left);
				FDropdown("Splash radius", Vars::Visuals::Simulation::SplashRadius, { "Simulation", "##Divider", "Priority", "Enemy", "Team", "Local", "Friends", "##Divider", "Rockets", "Stickies", "Pipes", "Scorch shot", "##Divider", "Trace" }, {}, FDropdown_Right | FDropdown_Multi);
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
					FSlider("off x", Vars::Visuals::Trajectory::OffX, -25.f, 25.f, 0.5f, "%g", FSlider_Precision);
					FSlider("off y", Vars::Visuals::Trajectory::OffY, -25.f, 25.f, 0.5f, "%g", FSlider_Precision);
					FSlider("off z", Vars::Visuals::Trajectory::OffZ, -25.f, 25.f, 0.5f, "%g", FSlider_Precision);
					FToggle("pipes", Vars::Visuals::Trajectory::Pipes);
					FSlider("hull", Vars::Visuals::Trajectory::Hull, 0.f, 10.f, 0.5f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("speed", Vars::Visuals::Trajectory::Speed, 0.f, 5000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("gravity", Vars::Visuals::Trajectory::Gravity, 0.f, 1.f, 0.1f, "%g", FSlider_Precision);
					FToggle("no spin", Vars::Visuals::Trajectory::NoSpin);
					FSlider("lifetime", Vars::Visuals::Trajectory::LifeTime, 0.f, 10.f, 0.1f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("up vel", Vars::Visuals::Trajectory::UpVelocity, 0.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("ang vel x", Vars::Visuals::Trajectory::AngVelocityX, -1000.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("ang vel y", Vars::Visuals::Trajectory::AngVelocityY, -1000.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("ang vel z", Vars::Visuals::Trajectory::AngVelocityZ, -1000.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("drag", Vars::Visuals::Trajectory::Drag, 0.f, 2.f, 0.1f, "%g", FSlider_Precision);
					FSlider("drag x", Vars::Visuals::Trajectory::DragBasisX, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("drag y", Vars::Visuals::Trajectory::DragBasisY, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("drag z", Vars::Visuals::Trajectory::DragBasisZ, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("ang drag x", Vars::Visuals::Trajectory::AngDragBasisX, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("ang drag y", Vars::Visuals::Trajectory::AngDragBasisY, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("ang drag z", Vars::Visuals::Trajectory::AngDragBasisZ, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("max vel", Vars::Visuals::Trajectory::MaxVelocity, 0.f, 4000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("max ang vel", Vars::Visuals::Trajectory::MaxAngularVelocity, 0.f, 7200.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
				} EndSection();
			}
			if (Section("Hitbox"))
			{
				FColorPicker("Bound edge color", Vars::Colors::BoundHitboxEdge, 1); FColorPicker("Bound face color", Vars::Colors::BoundHitboxFace);
				FColorPicker("Bone edge color", Vars::Colors::BoneHitboxEdge, 4); FColorPicker("Bone face color", Vars::Colors::BoneHitboxFace, 3);
				FToggle("Draw Hitboxes", Vars::Visuals::Hitbox::ShowHitboxes);
			} EndSection();
			if (Section("Thirdperson"))
			{
				FToggle("Thirdperson", Vars::Visuals::ThirdPerson::Enabled);
				FToggle("Thirdperson crosshair", Vars::Visuals::ThirdPerson::Crosshair, FToggle_Middle);
				FSlider("Thirdperson distance", Vars::Visuals::ThirdPerson::Distance, 0.f, 500.f, 5.f, "%g", FSlider_Precision);
				FSlider("Thirdperson right", Vars::Visuals::ThirdPerson::Right, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
				FSlider("Thirdperson up", Vars::Visuals::ThirdPerson::Up, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
			} EndSection();
			if (Section("Out of FOV arrows"))
			{
				FToggle("Enabled", Vars::Visuals::FOVArrows::Enabled);
				FSlider("Offset", Vars::Visuals::FOVArrows::Offset, 0, 500, 25, "%i", FSlider_Min | FSlider_Precision);
				FSlider("Max distance", Vars::Visuals::FOVArrows::MaxDist, 0.f, 5000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
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
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations)& (1 << 0)));
					FColorPicker("World modulation", Vars::Colors::WorldModulation, 0, FColorPicker_Left);
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations)& (1 << 1)));
					FColorPicker("Sky modulation", Vars::Colors::SkyModulation, 0, FColorPicker_Middle | FColorPicker_SameLine);
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations)& (1 << 2)));
					FColorPicker("Prop modulation", Vars::Colors::PropModulation, 0, FColorPicker_Left);
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations)& (1 << 3)));
					FColorPicker("Particle modulation", Vars::Colors::ParticleModulation, 0, FColorPicker_Middle | FColorPicker_SameLine);
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations)& (1 << 4)));
					FColorPicker("Fog modulation", Vars::Colors::FogModulation, 0, FColorPicker_Left);
				PopTransparent();
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
				FSlider("Range", Vars::Radar::Main::Range, 50, 3000, 50, "%i", FSlider_Precision);
				FSlider("Background alpha", Vars::Radar::Main::BackAlpha, 0, 255, 1, "%i", FSlider_Clamp);
				FSlider("Line alpha", Vars::Radar::Main::LineAlpha, 0, 255, 1, "%i", FSlider_Clamp);
			} EndSection();
			if (Section("Player"))
			{
				FToggle("Enabled", Vars::Radar::Players::Enabled);
				FToggle("Background", Vars::Radar::Players::Background, FToggle_Middle);
				FDropdown("Draw", Vars::Radar::Players::Draw, { "Local", "Enemy", "Team", "Friends", "Prioritized", "Cloaked" }, {}, FDropdown_Multi | FDropdown_Left);
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
				FDropdown("Draw", Vars::Radar::Buildings::Draw, { "Local", "Enemy", "Team", "Friends", "Prioritized" }, {}, FDropdown_Multi);
				FSlider("Icon size## Building", Vars::Radar::Buildings::IconSize, 12, 30, 2);
				FToggle("Health bar", Vars::Radar::Buildings::Health);
			} EndSection();
			if (Section("World"))
			{
				FToggle("Enabled", Vars::Radar::World::Enabled);
				FToggle("Background", Vars::Radar::World::Background, FToggle_Middle);
				FDropdown("Draw", Vars::Radar::World::Draw, { "Health", "Ammo", "Money", "Bombs", "Powerup", "Spellbook", "Gargoyle" }, {}, FDropdown_Multi);
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
				FKeybind("Menu primary key", Vars::Menu::MenuPrimaryKey.Map[DEFAULT_BIND], FButton_Left | FKeybind_AllowMenu);
				FKeybind("Menu secondary key", Vars::Menu::MenuSecondaryKey.Map[DEFAULT_BIND], FButton_Right | FButton_SameLine | FKeybind_AllowMenu);
				if (Vars::Menu::MenuPrimaryKey.Map[DEFAULT_BIND] == VK_LBUTTON)
					Vars::Menu::MenuPrimaryKey.Map[DEFAULT_BIND] = VK_INSERT;
				if (Vars::Menu::MenuSecondaryKey.Map[DEFAULT_BIND] == VK_LBUTTON)
					Vars::Menu::MenuSecondaryKey.Map[DEFAULT_BIND] = VK_F3;
			} EndSection();
			if (Section("Indicators"))
			{
				FDropdown("Indicators", Vars::Menu::Indicators, { "Ticks", "Crit hack", "Spectators", "Ping", "Conditions", "Seed prediction" }, {}, FDropdown_Multi);
				if (FSlider("DPI", Vars::Menu::DPI, 0.75f, 2.f, 0.25f, "%g", FSlider_Precision))
					H::Fonts.Reload(Vars::Menu::DPI.Map[DEFAULT_BIND]);
			} EndSection();

			/* Column 2 */
			TableNextColumn();

			if (Section("Fonts"))
			{
				static std::vector fontFlagNames{ "Italic", "Underline", "Strikeout", "Symbol", "Antialias", "Gaussian", "Rotary", "Dropshadow", "Additive", "Outline", "Custom" };
				static std::vector fontFlagValues{ 0x001, 0x002, 0x004, 0x008, 0x010, 0x020, 0x040, 0x080, 0x100, 0x200, 0x400 };
				FSDropdown("Name font", Vars::Fonts::FONT_NAME::szName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
				FDropdown("Name font flags", Vars::Fonts::FONT_NAME::nFlags, fontFlagNames, fontFlagValues, FDropdown_Multi | FDropdown_Right);
				FSlider("Name font height", Vars::Fonts::FONT_NAME::nTall, 7, 15);
				FSlider("Name font weight", Vars::Fonts::FONT_NAME::nWeight, 0, 900, 100);
				FSDropdown("Conds font", Vars::Fonts::FONT_CONDS::szName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
				FDropdown("Conds font flags", Vars::Fonts::FONT_CONDS::nFlags, fontFlagNames, fontFlagValues, FDropdown_Multi | FDropdown_Right);
				FSlider("Conds font height", Vars::Fonts::FONT_CONDS::nTall, 7, 15);
				FSlider("Conds font weight", Vars::Fonts::FONT_CONDS::nWeight, 0, 900, 100);
				FSDropdown("ESP font", Vars::Fonts::FONT_ESP::szName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
				FDropdown("ESP font flags", Vars::Fonts::FONT_ESP::nFlags, fontFlagNames, fontFlagValues, FDropdown_Multi | FDropdown_Right);

				FSlider("ESP font height", Vars::Fonts::FONT_ESP::nTall, 7, 15);
				FSlider("ESP font weight", Vars::Fonts::FONT_ESP::nWeight, 0, 900, 100);
				FSDropdown("Indicator font", Vars::Fonts::FONT_INDICATORS::szName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
				FDropdown("Indicator font flags", Vars::Fonts::FONT_INDICATORS::nFlags, fontFlagNames, fontFlagValues, FDropdown_Multi | FDropdown_Right);
				FSlider("Indicator font height", Vars::Fonts::FONT_INDICATORS::nTall, 7, 15);
				FSlider("Indicator font weight", Vars::Fonts::FONT_INDICATORS::nWeight, 0, 900, 100);
				if (FButton("Apply fonts"))
					H::Fonts.Reload();
			} EndSection();


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
			PushTransparent(FGet(Vars::Misc::Movement::AutoStrafe) != 2);
				FSlider("Autostrafe turn scale", Vars::Misc::Movement::AutoStrafeTurnScale, 0.f, 1.f, 0.1f, "%g", FSlider_Clamp | FSlider_Precision);
			PopTransparent();
			FToggle("Bunnyhop", Vars::Misc::Movement::Bunnyhop);
			FToggle("Auto jumpbug", Vars::Misc::Movement::AutoJumpbug, FToggle_Middle); // this is unreliable without setups, do not depend on it!
			FToggle("Auto rocketjump", Vars::Misc::Movement::AutoRocketJump);
			FToggle("Auto ctap", Vars::Misc::Movement::AutoCTap, FToggle_Middle);
			FToggle("Fast stop", Vars::Misc::Movement::FastStop);
			FToggle("Fast accelerate", Vars::Misc::Movement::FastAccel, FToggle_Middle);
			FToggle("No push", Vars::Misc::Movement::NoPush);
			FToggle("Crouch speed", Vars::Misc::Movement::CrouchSpeed, FToggle_Middle);
			FToggle("Movement lock", Vars::Misc::Movement::MovementLock);
		} EndSection();
		if (Vars::Debug::Info.Value)
		{
			if (Section("debug"))
			{
				FSlider("timing offset", Vars::Misc::Movement::TimingOffset, 0, 3);
				FSlider("choke count", Vars::Misc::Movement::ChokeCount, 0, 3);
				FSlider("apply timing offset above", Vars::Misc::Movement::ApplyAbove, 0, 8);
			} EndSection();
		}
		if (Section("Exploits"))
		{
			FToggle("Cheats bypass", Vars::Misc::Exploits::CheatsBypass);
			FToggle("Pure bypass", Vars::Misc::Exploits::BypassPure, FToggle_Middle);
			FToggle("Ping reducer", Vars::Misc::Exploits::PingReducer);
			PushTransparent(!FGet(Vars::Misc::Exploits::PingReducer));
				FSlider("cl_cmdrate", Vars::Misc::Exploits::PingTarget, 1, 66, 1, "%i", FSlider_Right | FSlider_Clamp);
			PopTransparent();
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
						SDK::Output("Convar", std::format("Sent {} as {}", sName, sValue).c_str(), Vars::Menu::Theme::Accent.Value);
						NET_SetConVar cmd(sName.c_str(), sValue.c_str());
						pNetChan->SendNetMsg(cmd);

						sName = sValue = "";
					}
				}
			} EndSection();
		}
		if (Section("Automation"))
		{
			FDropdown("Anti-backstab", Vars::Misc::Automation::AntiBackstab, { "Off", "Yaw", "Pitch", "Fake" }); // pitch/fake _might_ slip up some auto backstabs
			FToggle("Anti-AFK", Vars::Misc::Automation::AntiAFK);
			FToggle("Anti autobalance", Vars::Misc::Automation::AntiAutobalance, FToggle_Middle);
			FToggle("Auto accept item drops", Vars::Misc::Automation::AcceptItemDrops);
			FToggle("Taunt control", Vars::Misc::Automation::TauntControl, FToggle_Middle);
			FToggle("Kart control", Vars::Misc::Automation::KartControl);
			FToggle("Backpack expander", Vars::Misc::Automation::BackpackExpander, FToggle_Middle);
		} EndSection();

		/* Column 2 */
		TableNextColumn();
		if (Section("Sound"))
		{
			FDropdown("Block", Vars::Misc::Sound::Block, { "Footsteps", "Noisemaker" }, {}, FDropdown_Multi);
			FToggle("Giant weapon sounds", Vars::Misc::Sound::GiantWeaponSounds);
			FToggle("Hitsound always", Vars::Misc::Sound::HitsoundAlways, FToggle_Middle);
		} EndSection();
		if (Section("Game"))
		{
			FToggle("Network fix", Vars::Misc::Game::NetworkFix);
			FToggle("Prediction error jitter fix", Vars::Misc::Game::PredictionErrorJitterFix, FToggle_Middle);
			FToggle("Bones optimization", Vars::Misc::Game::SetupBonesOptimization);
			FToggle("F2P chat bypass", Vars::Misc::Game::F2PChatBypass, FToggle_Middle);
		} EndSection();
		if (Section("Queueing"))
		{
			FDropdown("Force regions", Vars::Misc::Queueing::ForceRegions,
				{ "Atlanta", "Chicago", "Texas", "Los Angeles", "Moses Lake", "New York", "Seattle", "Virginia", "##Divider", "Amsterdam", "Frankfurt", "Helsinki", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "##Divider", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "##Divider", "Bombay", "Chennai", "Dubai", "Hong Kong", "Madras", "Mumbai", "Seoul", "Singapore", "Tokyo", "Sydney", "##Divider", "Johannesburg" },
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
				FDropdown("Logs", Vars::Logging::Logs, { "Vote start", "Vote cast", "Class changes", "Damage", "Cheat detection", "Tags", "Aliases" }, {}, FDropdown_Multi);
				FSlider("Notification time", Vars::Logging::Lifetime, 0.5f, 5.f, 0.5f, "%g");
			} EndSection();
			if (Section("Vote Start"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & (1 << 0)));
					FDropdown("Log to", Vars::Logging::VoteStart::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				PopTransparent();
			} EndSection();
			if (Section("Vote Cast"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & (1 << 1)));
					FDropdown("Log to", Vars::Logging::VoteCast::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				PopTransparent();
			} EndSection();
			if (Section("Class Change"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & (1 << 2)));
					FDropdown("Log to", Vars::Logging::ClassChange::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Damage"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & (1 << 3)));
					FDropdown("Log to", Vars::Logging::Damage::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				PopTransparent();
			} EndSection();
			if (Section("Cheat Detection"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & (1 << 4)));
					FDropdown("Log to", Vars::Logging::CheatDetection::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				PopTransparent();
			} EndSection();
			if (Section("Tags"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & (1 << 5)));
					FDropdown("Log to", Vars::Logging::Tags::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				PopTransparent();
			} EndSection();
			if (Section("Aliases"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & (1 << 6)));
					FDropdown("Log to", Vars::Logging::Aliases::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				PopTransparent();
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

					for (auto& entry : std::filesystem::directory_iterator(F::Configs.sConfigPath))
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

					for (auto& entry : std::filesystem::directory_iterator(F::Configs.sVisualsPath))
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
				FToggle("Show server hitboxes", Vars::Debug::ServerHitbox); HelpMarker("Only localhost servers");
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
						OpenPopup("UnlockAchievements");
					if (FButton("Lock achievements", FButton_Right | FButton_SameLine))
						OpenPopup("LockAchievements");

					if (BeginPopupModal("UnlockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						Text("Do you really want to unlock all achievements?");

						if (FButton("Yes, unlock", FButton_Left))
						{
							F::Misc.UnlockAchievements();
							CloseCurrentPopup();
						}
						if (FButton("No", FButton_Right | FButton_SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
					if (BeginPopupModal("LockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						Text("Do you really want to lock all achievements?");

						if (FButton("Yes, lock", FButton_Left))
						{
							F::Misc.LockAchievements();
							CloseCurrentPopup();
						}
						if (FButton("No", FButton_Right | FButton_SameLine))
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
		// Binds
	case 1:
		if (Section("Settings"))
		{
			FToggle("Show bind window", Vars::Menu::ShowBinds);
			FToggle("Menu shows binds", Vars::Menu::MenuShowsBinds, FToggle_Middle);
		} EndSection();
		if (Section("Binds"))
		{
			static int iBind = DEFAULT_BIND;
			static Bind_t tBind = {};

			if (BeginTable("BindsTable", 2))
			{
				/* Column 1 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 8, GetCursorPos().y - 8 });
				if (BeginChild("BindsTableTable1", { GetColumnWidth() + 4, 104 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					FSDropdown("Name", &tBind.Name, {}, FSDropdown_AutoUpdate | FDropdown_Left);
					//FSDropdown("Parent", &tBind.Parent, {}, FSDropdown_AutoUpdate | FDropdown_Right);
					FDropdown("Type", &tBind.Type, { "Key", "Class", "Weapon type" }, {}, FDropdown_Left);
					switch (tBind.Type)
					{
					case 0: tBind.Info = std::min(tBind.Info, 2); FDropdown("Behavior", &tBind.Info, { "Hold", "Toggle", "Double click" }, {}, FDropdown_Right); break;
					case 1: tBind.Info = std::min(tBind.Info, 8); FDropdown("Class", &tBind.Info, { "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdown_Right); break;
					case 2: tBind.Info = std::min(tBind.Info, 2); FDropdown("Weapon type", &tBind.Info, { "Hitscan", "Projectile", "Melee" }, {}, FDropdown_Right); break;
					}
				} EndChild();

				/* Column 2 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 4, GetCursorPos().y - 8 });
				if (BeginChild("BindsTableTable2", { GetColumnWidth() + 8, 104 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					SetCursorPos({ 8, 24 });
					FToggle("Not", &tBind.Not); // change to dropdown
					FToggle("Visible", &tBind.Visible, FToggle_Middle);
					if (tBind.Type == 0)
					{
						SetCursorPos({ 8, 56 });
						FKeybind("Key", tBind.Key, FButton_Large, -96);
					}

					// create/modify button
					bool bCreate = false, bClear = false, bMatch = false, bParent = true;
					if (tBind.Parent != DEFAULT_BIND)
						bParent = F::Binds.vBinds.size() > tBind.Parent;

					SetCursorPos({ GetWindowSize().x - 96, 64 });
					PushStyleColor(ImGuiCol_Button, F::Render.Foremost.Value);
					PushStyleColor(ImGuiCol_ButtonActive, F::Render.Foremost.Value);
					if (bParent && (!tBind.Type ? tBind.Key : true))
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
					if (bParent && (!tBind.Type ? tBind.Key : true))
					{
						bMatch = iBind != DEFAULT_BIND && F::Binds.vBinds.size() > iBind;
						IconImage(bMatch ? ICON_MD_SETTINGS : ICON_MD_ADD);
					}
					else
					{
						PushTransparent(true);
							IconImage(ICON_MD_ADD);
						PopTransparent();
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
						F::Binds.AddBind(iBind, tBind);
					if (bCreate || bClear)
					{
						iBind = DEFAULT_BIND;
						tBind = {};
					}
				} EndChild();

				EndTable();
			}

			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			SetCursorPos({ 14, 128 }); FText("Binds");
			PopStyleColor();

			auto vBinds = F::Binds.vBinds; // intentional copy
			std::function<int(int, int, int)> getBinds = [&](int iParent, int x, int y)
				{
					for (auto it = vBinds.begin(); it != vBinds.end(); it++)
					{
						int _iBind = std::distance(vBinds.begin(), it);
						auto& _tBind = *it;
						if (iParent != _tBind.Parent)
							continue;

						y++;

						std::string info; std::string state;
						switch (_tBind.Type)
						{
							// key
						case 0:
							switch (_tBind.Info)
							{
							case 0: { info = "hold"; break; }
							case 1: { info = "toggle"; break; }
							case 2: { info = "double"; break; }
							}
							state = VK2STR(_tBind.Key);
							break;
							// class
						case 1:
							info = "class";
							switch (_tBind.Info)
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
							switch (_tBind.Info)
							{
							case 0: { state = "hitscan"; break; }
							case 1: { state = "projectile"; break; }
							case 2: { state = "melee"; break; }
							}
						}
						if (_tBind.Not)
							info = std::format("not {}", info);
						std::string str = std::format("{}, {}", info, state);

						bool bClicked = false, bDelete = false, bEdit = false;

						const ImVec2 restorePos = { 8.f + 28 * x, 108.f + 36.f * y };

						// background
						const float width = GetWindowSize().x - 16 - 28 * x; const auto winPos = GetWindowPos();
						GetWindowDrawList()->AddRectFilled({ winPos.x + restorePos.x, winPos.y + restorePos.y }, { winPos.x + restorePos.x + width, winPos.y + restorePos.y + 28 }, F::Render.Foremost, 3);

						// text
						SetCursorPos({ restorePos.x + 10, restorePos.y + 7 });
						TextUnformatted(_tBind.Name.c_str());

						SetCursorPos({ restorePos.x + width / 2 - CalcTextSize(str.c_str()).x / 2, restorePos.y + 7 });
						TextUnformatted(str.c_str());

						// buttons
						SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
						bDelete = IconButton(ICON_MD_DELETE);

						SetCursorPos({ restorePos.x + width - 47, restorePos.y + 5 });
						bEdit = IconButton(ICON_MD_EDIT);

						SetCursorPos(restorePos);
						bClicked = Button(std::format("##{}", _iBind).c_str(), { width, 28 });

						if (bClicked)
						{
							iBind = _iBind;
							tBind = _tBind;
						}
						if (bDelete)
							OpenPopup(std::format("Confirmation## DeleteCond{}", _iBind).c_str());
						if (BeginPopupModal(std::format("Confirmation## DeleteCond{}", _iBind).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							Text(std::format("Do you really want to delete '{}'{}?", _tBind.Name, F::Binds.HasChildren(_iBind) ? " and all of its children" : "").c_str());

							if (FButton("Yes", FButton_Left))
							{
								F::Binds.RemoveBind(_iBind);
								CloseCurrentPopup();

								iBind = DEFAULT_BIND;
								tBind = {};
							}
							if (FButton("No", FButton_Right | FButton_SameLine))
								CloseCurrentPopup();

							EndPopup();
						}
						if (bEdit)
							CurrentBind = CurrentBind != _iBind ? _iBind : DEFAULT_BIND;

						y = getBinds(_iBind, x + 1, y);
					}

					return y;
				};
			getBinds(DEFAULT_BIND, 0, 0);
		} EndSection();
		break;
		// PlayerList
	case 2:
		if (Section("Players"))
		{
			if (I::EngineClient->IsInGame())
			{
				std::lock_guard lock(F::PlayerUtils.m_mutex);
				const auto& playerCache = F::PlayerUtils.m_vPlayerCache;

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
						bool bClicked = false, bAdd = false, bAlias = false, bPitch = false, bYaw = false;

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
							bool bResolver = Vars::AntiHack::Resolver::Resolver.Value && !player.Local;

							// right
							SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
							bAlias = IconButton(ICON_MD_EDIT);

							SetCursorPos({ restorePos.x + width - 42, restorePos.y + 5 });
							bAdd = IconButton(ICON_MD_ADD);

							if (bResolver)
							{
								SetCursorPos({ restorePos.x + width - 62, restorePos.y + 5 });
								bYaw = IconButton(ICON_MD_ARROW_FORWARD);

								SetCursorPos({ restorePos.x + width - 82, restorePos.y + 5 });
								bPitch = IconButton(ICON_MD_ARROW_UPWARD);
							}

							// tag bar
							SetCursorPos({ restorePos.x + lOffset, restorePos.y });
							if (BeginChild(std::format("TagBar{}", player.FriendsID).c_str(), { width - lOffset - (bResolver ? 88 : 48), 28 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
							{
								PushFont(F::Render.FontSmall);

								const auto childPos = GetWindowPos();
								float tOffset = 0;
								for (auto& iID : F::PlayerUtils.m_mPlayerTags[player.FriendsID])
								{
									auto pTag = F::PlayerUtils.GetTag(iID);
									if (!pTag)
										continue;

									const ImColor tagColor = ColorToVec(pTag->Color);
									const float tagWidth = CalcTextSize(pTag->Name.c_str()).x + 25;
									const ImVec2 tagPos = { tOffset, 4 };

									PushStyleColor(ImGuiCol_Text, IsColorBright(tagColor) ? ImVec4{ 0, 0, 0, 1 } : ImVec4{ 1, 1, 1, 1 });

									GetWindowDrawList()->AddRectFilled({ childPos.x + tagPos.x, childPos.y + tagPos.y }, { childPos.x + tagPos.x + tagWidth, childPos.y + tagPos.y + 20 }, tagColor, 3);
									SetCursorPos({ tagPos.x + 5, tagPos.y + 4 });
									TextUnformatted(pTag->Name.c_str());
									SetCursorPos({ tagPos.x + tagWidth - 18, tagPos.y + 2 });
									if (IconButton(ICON_MD_CANCEL))
										F::PlayerUtils.RemoveTag(player.FriendsID, iID, true, player.Name);

									PopStyleColor();

									tOffset += tagWidth + 4;
								}
								PopFont();
							} EndChild();

							//bClicked = IsItemClicked();
							bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;

							SetCursorPos(restorePos);
							/*bClicked = */Button(std::format("##{}", player.Name).c_str(), { width, 28 }) || bClicked;
							bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;
						}

						SetCursorPos(restorePos);
						DebugDummy({ 0, 28 });

						if (bClicked)
							OpenPopup(std::format("Clicked{}", player.FriendsID).c_str());
						else if (bAdd)
							OpenPopup(std::format("Add{}", player.FriendsID).c_str());
						else if (bAlias)
							OpenPopup(std::format("Alias{}", player.FriendsID).c_str());
						else if (bPitch)
							OpenPopup(std::format("Pitch{}", player.FriendsID).c_str());
						else if (bYaw)
							OpenPopup(std::format("Yaw{}", player.FriendsID).c_str());

						// popups
						if (FBeginPopup(std::format("Clicked{}", player.FriendsID).c_str()))
						{
							if (FSelectable("Profile"))
								I::SteamFriends->ActivateGameOverlayToUser("steamid", CSteamID(player.FriendsID, k_EUniversePublic, k_EAccountTypeIndividual));

							if (FSelectable("History"))
								I::SteamFriends->ActivateGameOverlayToWebPage(std::format("https://steamhistory.net/id/{}", CSteamID(player.FriendsID, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64()).c_str());

							if (!player.Local && FSelectable("Votekick"))
								I::ClientState->SendStringCmd(std::format("callvote kick {}", player.UserID).c_str());

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Add{}", player.FriendsID).c_str()))
						{
							int iID = -1;
							for (auto& tTag : F::PlayerUtils.m_vTags)
							{
								iID++;

								if (!tTag.Assignable || F::PlayerUtils.HasTag(player.FriendsID, iID))
									continue;

								auto imColor = ColorToVec(tTag.Color);
								PushStyleColor(ImGuiCol_Text, imColor);
								imColor.x /= 3; imColor.y /= 3; imColor.z /= 3;
								if (FSelectable(tTag.Name.c_str(), imColor))
									F::PlayerUtils.AddTag(player.FriendsID, iID, true, player.Name);
								PopStyleColor();
							}

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Alias{}", player.FriendsID).c_str()))
						{
							FText("Alias");

							bool bHasAlias = F::PlayerUtils.m_mPlayerAliases.contains(player.FriendsID);
							static std::string sInput = "";

							PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
							PushItemWidth(150);
							bool bEnter = InputText("##Alias", &sInput, ImGuiInputTextFlags_EnterReturnsTrue);
							if (!IsItemFocused())
								sInput = bHasAlias ? F::PlayerUtils.m_mPlayerAliases[player.FriendsID] : "";
							PopItemWidth();
							PopStyleVar();

							if (bEnter)
							{
								if (sInput.empty() && F::PlayerUtils.m_mPlayerAliases.contains(player.FriendsID))
								{
									F::Records.AliasChanged(player.Name, "Removed", F::PlayerUtils.m_mPlayerAliases[player.FriendsID]);

									auto find = F::PlayerUtils.m_mPlayerAliases.find(player.FriendsID);
									if (find != F::PlayerUtils.m_mPlayerAliases.end())
										F::PlayerUtils.m_mPlayerAliases.erase(find);
									F::PlayerUtils.m_bSavePlayers = true;
								}
								else
								{
									F::PlayerUtils.m_mPlayerAliases[player.FriendsID] = sInput;
									F::PlayerUtils.m_bSavePlayers = true;

									F::Records.AliasChanged(player.Name, bHasAlias ? "Changed" : "Added", sInput);
								}
							}

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Pitch{}", player.FriendsID).c_str()))
						{
							for (size_t i = 0; i < F::PlayerUtils.m_vListPitch.size(); i++)
							{
								if (FSelectable(F::PlayerUtils.m_vListPitch[i]))
									F::Resolver.mResolverMode[player.FriendsID].second = int(i);
							}
							FEndPopup();
						}
						else if (FBeginPopup(std::format("Yaw{}", player.FriendsID).c_str()))
						{
							for (size_t i = 0; i < F::PlayerUtils.m_vListYaw.size(); i++)
							{
								if (FSelectable(F::PlayerUtils.m_vListYaw[i]))
									F::Resolver.mResolverMode[player.FriendsID].second = int(i);
							}
							FEndPopup();
						}
					};

				// display players
				std::vector<ListPlayer> vBlu, vRed, vOther;
				for (auto& player : playerCache)
				{
					switch (player.Team)
					{
					case 3: vBlu.push_back(player); break;
					case 2: vRed.push_back(player); break;
					default: vOther.push_back(player); break;
					}
				}

				if (vBlu.size() < vRed.size()) // display whichever one has more last
				{
					for (size_t i = 0; i < vBlu.size(); i++)
						drawPlayer(vBlu[i], 0, int(i));
					for (size_t i = 0; i < vRed.size(); i++)
						drawPlayer(vRed[i], 1, int(i));
				}
				else
				{
					for (size_t i = 0; i < vRed.size(); i++)
						drawPlayer(vRed[i], 1, int(i));
					for (size_t i = 0; i < vBlu.size(); i++)
						drawPlayer(vBlu[i], 0, int(i));
				}
				size_t iMax = std::max(vBlu.size(), vRed.size());
				for (size_t i = 0; i < vOther.size(); i++)
					drawPlayer(vOther[i], i % 2, int(iMax + i / 2));
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
			static int iID = -1;
			static PriorityLabel_t tTag = {};

			if (BeginTable("TagTable", 2))
			{
				/* Column 1 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 8, GetCursorPos().y - 8 });
				if (BeginChild("TagTable1", { GetColumnWidth() + 4, 56 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					FSDropdown("Name", &tTag.Name, {}, FSDropdown_AutoUpdate | FDropdown_Left, 1);
					FColorPicker("Color", &tTag.Color, 0, FColorPicker_Dropdown);

					PushDisabled(iID == DEFAULT_TAG || iID == IGNORED_TAG);
						int iLabel = Disabled ? 0 : tTag.Label;
						FDropdown("Type", &iLabel, { "Priority", "Label" }, {}, FDropdown_Right);
						tTag.Label = iLabel;
						if (Disabled)
							tTag.Label = false;
					PopDisabled();
				} EndChild();

				/* Column 2 */
				TableNextColumn(); SetCursorPos({ GetCursorPos().x - 4, GetCursorPos().y - 8 });
				if (BeginChild("TagTable2", { GetColumnWidth() + 8, 56 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					PushTransparent(tTag.Label); // transparent if we want a label, user can still use to sort
						SetCursorPosY(GetCursorPos().y + 12);
						FSlider("Priority", &tTag.Priority, -10, 10, 1, "%i", FSlider_Left);
					PopTransparent();

					// create/modify button
					bool bCreate = false, bClear = false;

					SetCursorPos({ GetWindowSize().x - 96, 16 });
					PushStyleColor(ImGuiCol_Button, F::Render.Foremost.Value);
					PushStyleColor(ImGuiCol_ButtonActive, F::Render.Foremost.Value);
					if (tTag.Name.length())
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
					PushTransparent(tTag.Name.empty());
						IconImage(iID != -1 ? ICON_MD_SETTINGS : ICON_MD_ADD);
					PopTransparent();

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
						F::PlayerUtils.m_bSaveTags = true;
						if (iID > -1 || iID < F::PlayerUtils.m_vTags.size())
						{
							F::PlayerUtils.m_vTags[iID].Name = tTag.Name;
							F::PlayerUtils.m_vTags[iID].Color = tTag.Color;
							F::PlayerUtils.m_vTags[iID].Priority = tTag.Priority;
							F::PlayerUtils.m_vTags[iID].Label = tTag.Label;
						}
						else
							F::PlayerUtils.m_vTags.push_back(tTag);
					}
					if (bCreate || bClear)
					{
						iID = -1;
						tTag = {};
					}
				} EndChild();

				EndTable();
			}

			auto drawTag = [](std::vector<PriorityLabel_t>::iterator it, PriorityLabel_t& _tTag, int y)
				{
					int _iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);

					bool bClicked = false, bDelete = false;

					ImColor imColor = ColorToVec(_tTag.Color);
					imColor.Value.x /= 3; imColor.Value.y /= 3; imColor.Value.z /= 3;

					const ImVec2 restorePos = { _tTag.Label ? GetWindowSize().x * 2 / 3 + 4.f : 8.f, 96.f + 36.f * y };

					// background
					const float width = GetWindowSize().x * (_tTag.Label ? 1.f / 3 : 2.f / 3) - 12; const auto winPos = GetWindowPos();
					GetWindowDrawList()->AddRectFilled({ winPos.x + restorePos.x, winPos.y + restorePos.y }, { winPos.x + restorePos.x + width, winPos.y + restorePos.y + 28 }, imColor, 3);

					// text
					SetCursorPos({ restorePos.x + 10, restorePos.y + 7 });
					TextUnformatted(_tTag.Name.c_str());

					if (!_tTag.Label)
					{
						SetCursorPos({ restorePos.x + width / 2, restorePos.y + 7 });
						TextUnformatted(std::format("{}", _tTag.Priority).c_str());
					}

					// buttons
					SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
					if (!_tTag.Locked)
						bDelete = IconButton(ICON_MD_DELETE);
					else
					{
						switch (_iID)
						{
							//case DEFAULT_TAG: // no image
						case IGNORED_TAG: IconImage(ICON_MD_DO_NOT_DISTURB); break;
						case CHEATER_TAG: IconImage(ICON_MD_FLAG); break;
						case FRIEND_TAG: IconImage(ICON_MD_GROUP);
						}
					}

					SetCursorPos(restorePos);
					bClicked = Button(std::format("##{}", _tTag.Name).c_str(), { width, 28 });

					if (bClicked)
					{
						iID = _iID;
						tTag.Name = _tTag.Name;
						tTag.Color = _tTag.Color;
						tTag.Priority = _tTag.Priority;
						tTag.Label = _tTag.Label;
					}
					if (bDelete)
						OpenPopup(std::format("Confirmation## DeleteTag{}", _iID).c_str());
					if (BeginPopupModal(std::format("Confirmation## DeleteTag{}", _iID).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						Text(std::format("Do you really want to delete '{}'?", _tTag.Name).c_str());

						if (FButton("Yes", FButton_Left))
						{
							F::PlayerUtils.m_vTags.erase(it);
							F::PlayerUtils.m_bSaveTags = F::PlayerUtils.m_bSavePlayers = true;

							for (auto& [friendsID, vTags] : F::PlayerUtils.m_mPlayerTags)
							{
								for (auto it = vTags.begin(); it != vTags.end();)
								{
									if (_iID == *it)
										vTags.erase(it);
									else
									{
										if (_iID < *it)
											(*it)--;
										it++;
									}
								}
							}

							if (iID == _iID)
							{
								iID = -1;
								tTag = {};
							}
							else if (iID > _iID)
								iID--;

							CloseCurrentPopup();
						}
						if (FButton("No", FButton_Right | FButton_SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
				};

			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			SetCursorPos({ 14, 80 }); FText("Priorities");
			SetCursorPos({ GetWindowSize().x * 2 / 3 + 10, 80 }); FText("Labels");
			PopStyleColor();

			std::vector<std::pair<std::vector<PriorityLabel_t>::iterator, PriorityLabel_t>> vPriorities = {}, vLabels = {};
			for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
			{
				auto& _tTag = *it;

				if (!_tTag.Label)
					vPriorities.push_back({ it, _tTag });
				else
					vLabels.push_back({ it, _tTag });
			}

			std::sort(vPriorities.begin(), vPriorities.end(), [&](const auto& a, const auto& b) -> bool
				{
					// override for default tag
					if (std::distance(F::PlayerUtils.m_vTags.begin(), a.first) == DEFAULT_TAG)
						return true;
					if (std::distance(F::PlayerUtils.m_vTags.begin(), b.first) == DEFAULT_TAG)
						return false;

					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.second.Name < b.second.Name;
				});
			std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
				{
					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.second.Name < b.second.Name;
				});

			// display tags
			int iPriorities = 0, iLabels = 0;
			for (auto& pair : vPriorities)
			{
				drawTag(pair.first, pair.second, iPriorities);
				iPriorities++;
			}
			for (auto& pair : vLabels)
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
				for (auto& [sName, mat] : F::Materials.mChamMaterials)
					vMaterials.push_back({ sName, mat });

				std::sort(vMaterials.begin(), vMaterials.end(), [&](const auto& a, const auto& b) -> bool
					{
						// override for none material
						if (FNV1A::Hash32(a.first.c_str()) == FNV1A::Hash32Const("None"))
							return true;
						if (FNV1A::Hash32(b.first.c_str()) == FNV1A::Hash32Const("None"))
							return false;

						// keep locked materials higher
						if (a.second.bLocked && !b.second.bLocked)
							return true;
						if (!a.second.bLocked && b.second.bLocked)
							return false;

						return a.first < b.first;
					});

				for (auto& pair : vMaterials)
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
	const float sizeX = 100.f * Vars::Menu::DPI.Map[DEFAULT_BIND], sizeY = 40.f * Vars::Menu::DPI.Map[DEFAULT_BIND];
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
	std::function<void(int)> getBinds = [&](int iParent)
		{
			for (auto it = F::Binds.vBinds.begin(); it != F::Binds.vBinds.end(); it++)
			{
				int iBind = std::distance(F::Binds.vBinds.begin(), it);
				auto& tBind = *it;
				if (iParent != tBind.Parent)
					continue;

				if (tBind.Visible)
				{
					std::string info; std::string state;
					switch (tBind.Type)
					{
						// key
					case 0:
						switch (tBind.Info)
						{
						case 0: { info = "hold"; break; }
						case 1: { info = "toggle"; break; }
						case 2: { info = "double"; break; }
						}
						state = VK2STR(tBind.Key);
						break;
						// class
					case 1:
						info = "class";
						switch (tBind.Info)
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
						switch (tBind.Info)
						{
						case 0: { state = "hitscan"; break; }
						case 1: { state = "projectile"; break; }
						case 2: { state = "melee"; break; }
						}
					}
					if (tBind.Not)
						info = std::format("not {}", info);

					actives.push_back(tBind.Active);
					titles.push_back(tBind.Name);
					infos.push_back(info);
					states.push_back(state);
					titleWidth = std::max(titleWidth, CalcTextSize(tBind.Name.c_str()).x);
					infoWidth = std::max(infoWidth, CalcTextSize(info.c_str()).x);
					stateWidth = std::max(stateWidth, CalcTextSize(state.c_str()).x);
				}

				if (tBind.Active)
					getBinds(iBind);
			}
		};
	getBinds(DEFAULT_BIND);

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

	for (int iKey = 0; iKey < 256; iKey++)
		U::KeyHandler.StoreKey(iKey);

	if (!ConfigLoaded || !(ImGui::GetIO().DisplaySize.x > 160.f && ImGui::GetIO().DisplaySize.y > 28.f))
		return;

	InKeybind = false;
	if (U::KeyHandler.Pressed(Vars::Menu::MenuPrimaryKey.Value) || U::KeyHandler.Pressed(Vars::Menu::MenuSecondaryKey.Value))
		I::MatSystemSurface->SetCursorAlwaysVisible(IsOpen = !IsOpen);

	PushFont(F::Render.FontRegular);

	DrawBinds();

	if (IsOpen)
	{
		vDisabled.clear();
		vTransparent.clear();

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
		mActiveMap.clear();

	PopFont();
}