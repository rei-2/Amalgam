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
#include "../../Spectate/Spectate.h"

/* The main menu */
void CMenu::DrawMenu()
{
	using namespace ImGui;

	ImVec2 vMainWindowPos = {};
	ImVec2 vMainWindowSize = {};

	SetNextWindowSize({ 750, 500 }, ImGuiCond_FirstUseEver);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(750), H::Draw.Scale(500) });
	if (Begin("Background", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
	{
		auto vWindowPos = vMainWindowPos = GetWindowPos();
		auto vWindowSize = vMainWindowSize = GetWindowSize();

		SetCursorPos({ H::Draw.Scale(8), H::Draw.Scale(8) });
		PushStyleVar(ImGuiStyleVar_ChildRounding, H::Draw.Scale(3));
		PushStyleColor(ImGuiCol_ChildBg, F::Render.Background.Value);
		if (BeginChild("Main", { vWindowSize.x - H::Draw.Scale(16), vWindowSize.y - H::Draw.Scale(16) }, ImGuiWindowFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			vWindowPos = GetWindowPos();
			vWindowSize = GetWindowSize();

			if (!Vars::Menu::CheatName.Value.empty())
			{
				auto vDrawPos = GetDrawPos();
				auto pDrawList = GetWindowDrawList();

				const auto textSize = FCalcTextSize(Vars::Menu::CheatName.Value.c_str(), F::Render.FontBold);
				vDrawPos.x += vWindowSize.x;
				pDrawList->AddRectFilled({ vDrawPos.x - textSize.x - H::Draw.Scale(24), vDrawPos.y + H::Draw.Scale(8) }, { vDrawPos.x - H::Draw.Scale(8), vDrawPos.y + H::Draw.Scale(32) }, F::Render.Foreground, 3);
				SetCursorPos({ vWindowSize.x - textSize.x - H::Draw.Scale(16), H::Draw.Scale(13) });
				PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
				FText(Vars::Menu::CheatName.Value.c_str(), 0, F::Render.FontBold);
				PopStyleColor();
			}

			static int iCurrentTab = 0;
			PushFont(F::Render.FontBold);
			FTabs({ "AIMBOT", "VISUALS", "MISC", "LOGS", "SETTINGS" }, &iCurrentTab, { H::Draw.Scale(100), H::Draw.Scale(40) }, { 0, 0 }, FTabs_HorizontalIcons, { ICON_MD_GROUP, ICON_MD_IMAGE, ICON_MD_PUBLIC, ICON_MD_MENU_BOOK, ICON_MD_SETTINGS });
			PopFont();

			SetCursorPos({ 0, H::Draw.Scale(40) });
			PushStyleColor(ImGuiCol_ChildBg, {});
			if (BeginChild("Page", { vWindowSize.x, vWindowSize.y - H::Draw.Scale(40) }, ImGuiWindowFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
			{
				switch (iCurrentTab)
				{
				case 0: MenuAimbot(); break;
				case 1: MenuVisuals(); break;
				case 2: MenuMisc(); break;
				case 3: MenuLogs(); break;
				case 4: MenuSettings(); break;
				}
			} EndChild();
			PopStyleColor();
		} EndChild();
		PopStyleColor();
		PopStyleVar();

		End();
	}
	PopStyleVar();

	// Bind Text
	Bind_t tBind;
	if (F::Binds.GetBind(CurrentBind, &tBind))
	{
		const auto textSize = FCalcTextSize(std::format("Editing bind {}", tBind.Name).c_str());
		SetNextWindowSize({ std::clamp(textSize.x + H::Draw.Scale(56), H::Draw.Scale(40), vMainWindowSize.x), H::Draw.Scale(40) });
		SetNextWindowPos({ vMainWindowPos.x, vMainWindowPos.y + vMainWindowSize.y + H::Draw.Scale(8) });
		if (Begin("Bind", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
		{
			const auto preSize = FCalcTextSize("Editing bind ");
			SetCursorPos({ H::Draw.Scale(16), H::Draw.Scale(13) });
			FText("Editing bind ");
			SetCursorPos({ H::Draw.Scale(16) + preSize.x, H::Draw.Scale(13) });
			PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
			FText(tBind.Name.c_str());
			PopStyleColor();

			SetCursorPos({ textSize.x + H::Draw.Scale(28), H::Draw.Scale(11) });
			if (IconButton(ICON_MD_CANCEL))
				CurrentBind = DEFAULT_BIND;

			End();
		}
	}
}

#pragma region Tabs
void CMenu::MenuAimbot()
{
	using namespace ImGui;

	const auto vWindowSize = GetWindowSize();
	static int iCurrentTab = 0;
	PushFont(F::Render.FontBold);
	FTabs({ "GENERAL", "HVH" }, &iCurrentTab, { H::Draw.Scale(100), H::Draw.Scale(40) }, { 0, 0 });
	PopFont();

	SetCursorPos({ 0, H::Draw.Scale(40) });
	PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
	if (BeginChild("Content", { vWindowSize.x, vWindowSize.y - H::Draw.Scale(40) }, ImGuiWindowFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		PushStyleColor(ImGuiCol_ChildBg, F::Render.Foreground.Value);
		
		switch (iCurrentTab)
		{
		// General
		case 0:
			if (BeginTable("AimbotTable", 2))
			{
				/* Column 1 */
				TableNextColumn();
				if (Section("General"))
				{
					FDropdown("Aim type", Vars::Aimbot::General::AimType, { "Off", "Plain", "Smooth", "Silent", "Locking" }, {}, FDropdown_Left);
					FDropdown("Target selection", Vars::Aimbot::General::TargetSelection, { "FOV", "Distance" }, {}, FDropdown_Right);
					FDropdown("Target", Vars::Aimbot::General::Target, { "Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Ignore", Vars::Aimbot::General::Ignore, { "Friends", "Party", "Invulnerable", "Cloaked", "Unsimulated players", "Dead Ringer", "Vaccinator", "Disguised", "Taunting" }, {}, FDropdown_Multi | FDropdown_Right);
					FSlider("Aim FOV", Vars::Aimbot::General::AimFOV, 0.f, 180.f, 1.f, "%g", FSlider_Clamp | FSlider_Precision);
					PushTransparent(FGet(Vars::Aimbot::General::AimType) != Vars::Aimbot::General::AimTypeEnum::Smooth);
						FSlider("Smoothing## Hitscan", Vars::Aimbot::General::Smoothing, 0.f, 100.f, 1.f, "%g%%", FSlider_Clamp | FSlider_Precision);
					PopTransparent();
					FSlider("Max targets", Vars::Aimbot::General::MaxTargets, 1, 6, 1, "%i", FSlider_Min);
					PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Cloaked));
						FSlider("Ignore cloak", Vars::Aimbot::General::IgnoreCloakPercentage, 0, 100, 10, "%d%%", FSlider_Clamp | FSlider_Precision);
					PopTransparent();
					PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Unsimulated));
						FSlider("Tick tolerance", Vars::Aimbot::General::TickTolerance, 0, 21, 1, "%i", FSlider_Clamp);
					PopTransparent();
					FColorPicker("Aimbot FOV circle", Vars::Colors::FOVCircle);
					FToggle("Autoshoot", Vars::Aimbot::General::AutoShoot, FToggle_Left);
					FToggle("FOV Circle", Vars::Aimbot::General::FOVCircle, FToggle_Right);
					FToggle("Force crits", Vars::CritHack::ForceCrits, FToggle_Left);
					FToggle("Avoid random crits", Vars::CritHack::AvoidRandom, FToggle_Right);
					FToggle("Always melee crit", Vars::CritHack::AlwaysMeleeCrit, FToggle_Left);
					FToggle("No spread", Vars::Aimbot::General::NoSpread, FToggle_Right);
				} EndSection();
				if (Vars::Debug::Info.Value)
				{
					if (Section("debug## aimbot"))
					{
						FSlider("hitscan peek", Vars::Aimbot::General::HitscanPeek, 0, 5);
						FToggle("peek dt only", Vars::Aimbot::General::PeekDTOnly);
						FTooltip("this should probably stay on if you want to be able to\ntarget hitboxes other than the highest priority one");
						FSlider("nospread offset## nospread", Vars::Aimbot::General::NoSpreadOffset, -1.f, 1.f, 0.1f, "%g", FSlider_Precision);
						FSlider("nospread average", Vars::Aimbot::General::NoSpreadAverage, 1, 25, 1, "%d", FSlider_Min);
						FSlider("nospread interval", Vars::Aimbot::General::NoSpreadInterval, 0.05f, 5.f, 0.1f, "%gs", FSlider_Min);
						FSlider("nospread backup", Vars::Aimbot::General::NoSpreadBackupInterval, 2.f, 10.f, 0.1f, "%gs", FSlider_Min);
						FDropdown("aim holds fire", Vars::Aimbot::General::AimHoldsFire, { "false", "minigun only", "always" });
					} EndSection();
				}
				if (Section("Backtrack"))
				{
					FToggle("Enabled", Vars::Backtrack::Enabled, FToggle_Left);
					FToggle("Prefer on shot", Vars::Backtrack::PreferOnShot, FToggle_Right);
					FSlider("Fake latency", Vars::Backtrack::Latency, 0, F::Backtrack.m_flMaxUnlag * 1000, 5, "%i", FSlider_Clamp);
					FSlider("Fake interp", Vars::Backtrack::Interp, 0, F::Backtrack.m_flMaxUnlag * 1000, 5, "%i", FSlider_Clamp | FSlider_Precision);
					FSlider("Window", Vars::Backtrack::Window, 1, 200, 5, "%i", FSlider_Clamp);
				} EndSection();
				if (Vars::Debug::Info.Value)
				{
					if (Section("debug## backtrack"))
					{
						FSlider("offset", Vars::Backtrack::Offset, -1, 1);
					} EndSection();
				}
				if (Section("Healing"))
				{
					FToggle("Auto heal", Vars::Aimbot::Healing::AutoHeal, FToggle_Left);
					FToggle("Friends only", Vars::Aimbot::Healing::FriendsOnly, FToggle_Right);
					FToggle("Activate on voice", Vars::Aimbot::Healing::ActivateOnVoice, FToggle_Left);
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("Hitscan"))
				{
					FDropdown("Hitboxes", Vars::Aimbot::Hitscan::Hitboxes, { "Head", "Body", "Pelvis", "Arms", "Legs" }, { 1 << 0, 1 << 2, 1 << 1, 1 << 3, 1 << 4 }, FDropdown_Multi);
					FDropdown("Modifiers## Hitscan", Vars::Aimbot::Hitscan::Modifiers, { "Tapfire", "Wait for headshot", "Wait for charge", "Scoped only", "Auto scope", "Bodyaim if lethal", "Auto rev minigun", "Extinguish team" }, {}, FDropdown_Multi);
					FSlider("Point scale", Vars::Aimbot::Hitscan::PointScale, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
					PushTransparent(!(FGet(Vars::Aimbot::Hitscan::Modifiers) & Vars::Aimbot::Hitscan::ModifiersEnum::Tapfire));
						FSlider("Tapfire distance", Vars::Aimbot::Hitscan::TapFireDist, 250.f, 1000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
					PopTransparent();
				} EndSection();
				if (Section("Projectile"))
				{
					FDropdown("Predict", Vars::Aimbot::Projectile::StrafePrediction, { "Air strafing", "Ground strafing" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Splash", Vars::Aimbot::Projectile::SplashPrediction, { "Off", "Include", "Prefer", "Only" }, {}, FDropdown_Right);
					FDropdown("Auto detonate", Vars::Aimbot::Projectile::AutoDetonate, { "Stickies", "Flares", "##Divider", "Ignore self damage" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Auto airblast", Vars::Aimbot::Projectile::AutoAirblast, { "Enabled", "##Divider", "Redirect simple", "Redirect advanced", "##Divider", "Respect FOV"}, {}, FDropdown_Multi | FDropdown_Right); // todo: finish redirect advanced!!
					FDropdown("Modifiers## Projectile", Vars::Aimbot::Projectile::Modifiers, { "Charge shot", "Cancel charge", "Bodyaim if lethal", "Use prime time", "Aim blast at feet" }, {}, FDropdown_Multi);
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
						FText("\nground");
						FSlider("samples##ground", Vars::Aimbot::Projectile::GroundSamples, 3, 66, 1, "%i", FSlider_Left);
						FSlider("straight fuzzy value##ground", Vars::Aimbot::Projectile::GroundStraightFuzzyValue, 0.f, 500.f, 25.f, "%g", FSlider_Right | FSlider_Precision);
						FSlider("low min samples##ground", Vars::Aimbot::Projectile::GroundLowMinimumSamples, 3, 66, 1, "%i", FSlider_Left);
						FSlider("high min samples##ground", Vars::Aimbot::Projectile::GroundHighMinimumSamples, 3, 66, 1, "%i", FSlider_Right);
						FSlider("low min distance##ground", Vars::Aimbot::Projectile::GroundLowMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("high min distance##ground", Vars::Aimbot::Projectile::GroundHighMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
						FSlider("max changes##ground", Vars::Aimbot::Projectile::GroundMaxChanges, 0, 5, 1, "%i", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("max change time##ground", Vars::Aimbot::Projectile::GroundMaxChangeTime, 0, 66, 1, "%i", FSlider_Right | FSlider_Min | FSlider_Precision);
						FSlider("new weight##ground", Vars::Aimbot::Projectile::GroundNewWeight, 0.f, 200.f, 5.f, "%g%%", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("old weight##ground", Vars::Aimbot::Projectile::GroundOldWeight, 0.f, 200.f, 5.f, "%g%%", FSlider_Right | FSlider_Min | FSlider_Precision);

						FText("air");
						FSlider("samples##air", Vars::Aimbot::Projectile::AirSamples, 3, 66, 1, "%i", FSlider_Left);
						FSlider("straight fuzzy value##air", Vars::Aimbot::Projectile::AirStraightFuzzyValue, 0.f, 500.f, 25.f, "%g", FSlider_Right | FSlider_Precision);
						FSlider("low min samples##air", Vars::Aimbot::Projectile::AirLowMinimumSamples, 3, 66, 1, "%i", FSlider_Left);
						FSlider("high min samples##air", Vars::Aimbot::Projectile::AirHighMinimumSamples, 3, 66, 1, "%i", FSlider_Right);
						FSlider("low min distance##air", Vars::Aimbot::Projectile::AirLowMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("high min distance##air", Vars::Aimbot::Projectile::AirHighMinimumDistance, 0.f, 10000.f, 100.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
						FSlider("max changes##air", Vars::Aimbot::Projectile::AirMaxChanges, 0, 5, 1, "%i", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("max change time##air", Vars::Aimbot::Projectile::AirMaxChangeTime, 0, 66, 1, "%i", FSlider_Right | FSlider_Min | FSlider_Precision);
						FSlider("new weight##air", Vars::Aimbot::Projectile::AirNewWeight, 0.f, 200.f, 5.f, "%g%%", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("old weight##air", Vars::Aimbot::Projectile::AirOldWeight, 0.f, 200.f, 5.f, "%g%%", FSlider_Right | FSlider_Min | FSlider_Precision);

						FText("");
						FSlider("velocity average count", Vars::Aimbot::Projectile::VelocityAverageCount, 1, 10, 1, "%i", FSlider_Left);
						FSlider("vertical shift", Vars::Aimbot::Projectile::VerticalShift, 0.f, 10.f, 0.5f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
						FSlider("latency offset", Vars::Aimbot::Projectile::LatencyOffset, -1.f, 1.f, 0.1f, "%g", FSlider_Left | FSlider_Precision);
						FSlider("hull increase", Vars::Aimbot::Projectile::HullIncrease, 0.f, 3.f, 0.5f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);

						FSlider("drag override", Vars::Aimbot::Projectile::DragOverride, 0.f, 1.f, 0.01f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("time override", Vars::Aimbot::Projectile::TimeOverride, 0.f, 2.f, 0.01f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
						FSlider("huntsman lerp", Vars::Aimbot::Projectile::HuntsmanLerp, 0.f, 100.f, 1.f, "%g%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);
						FSlider("huntsman lerp low", Vars::Aimbot::Projectile::HuntsmanLerpLow, 0.f, 100.f, 1.f, "%g%%", FSlider_Right | FSlider_Clamp | FSlider_Precision);
						FSlider("huntsman add", Vars::Aimbot::Projectile::HuntsmanAdd, 0.f, 20.f, 1.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
						FSlider("huntsman add low", Vars::Aimbot::Projectile::HuntsmanAddLow, 0.f, 20.f, 1.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
						FSlider("huntsman clamp", Vars::Aimbot::Projectile::HuntsmanClamp, 0.f, 10.f, 0.5f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);

						FToggle("splash grates", Vars::Aimbot::Projectile::SplashGrates, FToggle_Left);
						bool bHovered; FDropdown("rocket splash mode", Vars::Aimbot::Projectile::RocketSplashMode, { "regular", "special light", "special heavy" }, {}, FDropdown_Right, 0, &bHovered);
						FTooltip("special splash type for rockets, more expensive", bHovered);
						SetCursorPosY(GetCursorPosY() - 24);
						FSlider("splash points", Vars::Aimbot::Projectile::SplashPoints, 1, 400, 5, "%i", FSlider_Left | FSlider_Min);
						FSlider("direct splash count", Vars::Aimbot::Projectile::SplashCountDirect, 1, 100, 1, "%i", FSlider_Left | FSlider_Min);
						FSlider("arc splash count", Vars::Aimbot::Projectile::SplashCountArc, 1, 100, 1, "%i", FSlider_Right | FSlider_Min);
						FSlider("splash trace interval", Vars::Aimbot::Projectile::SplashTraceInterval, 1, 10, 1, "%i", FSlider_Left);
						FSlider("delta count", Vars::Aimbot::Projectile::DeltaCount, 1, 5, 1, "%i", FSlider_Right);
						FToggle("strafe delta", Vars::Aimbot::Projectile::StrafeDelta);
						FTooltip("this was a test and should probably stay off");
						FDropdown("delta mode", Vars::Aimbot::Projectile::DeltaMode, { "average", "max" }, {}, FDropdown_Right);
					} EndSection();
				}
				if (Section("Melee"))
				{
					FToggle("Auto backstab", Vars::Aimbot::Melee::AutoBackstab, FToggle_Left);
					FToggle("Ignore razorback", Vars::Aimbot::Melee::IgnoreRazorback, FToggle_Right);
					FToggle("Swing prediction", Vars::Aimbot::Melee::SwingPrediction, FToggle_Left);
					FToggle("Whip teammates", Vars::Aimbot::Melee::WhipTeam, FToggle_Right);
				} EndSection();
				if (Vars::Debug::Info.Value)
				{
					if (Section("debug## melee"))
					{
						FSlider("swing ticks", Vars::Aimbot::Melee::SwingTicks, 10, 14, 1, "%i", FSlider_Left);
						FToggle("swing predict lag", Vars::Aimbot::Melee::SwingPredictLag, FToggle_Right);
						SetCursorPosY(GetCursorPosY() + 8);
						FToggle("backstab account ping", Vars::Aimbot::Melee::BackstabAccountPing, FToggle_Left);
						FToggle("backstab double test", Vars::Aimbot::Melee::BackstabDoubleTest, FToggle_Right);
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
					FToggle("Doubletap", Vars::CL_Move::Doubletap::Doubletap, FToggle_Left);
					FToggle("Warp", Vars::CL_Move::Doubletap::Warp, FToggle_Right);
					FToggle("Recharge ticks", Vars::CL_Move::Doubletap::RechargeTicks, FToggle_Left);
					FToggle("Anti-warp", Vars::CL_Move::Doubletap::AntiWarp, FToggle_Right);
					FSlider("Tick limit", Vars::CL_Move::Doubletap::TickLimit, 2, 22, 1, "%i", FSlider_Clamp);
					FSlider("Warp rate", Vars::CL_Move::Doubletap::WarpRate, 2, 22, 1, "%i", FSlider_Clamp);
					FSlider("Passive recharge", Vars::CL_Move::Doubletap::PassiveRecharge, 0, 67, 1, "%i", FSlider_Clamp);
					FSlider("Recharge limit", Vars::CL_Move::Doubletap::RechargeLimit, 1, 24, 1, "%i", FSlider_Clamp);
				} EndSection();
				if (Section("Fakelag"))
				{
					FDropdown("Fakelag", Vars::CL_Move::Fakelag::Fakelag, { "Off", "Plain", "Random", "Adaptive" }, {}, FSlider_Left);
					FDropdown("Options", Vars::CL_Move::Fakelag::Options, { "Only moving", "On unduck", "Not airborne" }, {}, FDropdown_Multi | FSlider_Right);
					PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != Vars::CL_Move::Fakelag::FakelagEnum::Plain);
						FSlider("Plain ticks", Vars::CL_Move::Fakelag::PlainTicks, 1, 22, 1, "%i", FSlider_Clamp | FSlider_Left);
					PopTransparent();
					PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != Vars::CL_Move::Fakelag::FakelagEnum::Random);
						FSlider("Random ticks", Vars::CL_Move::Fakelag::RandomTicks, 1, 22, 1, "%i - %i", FSlider_Clamp | FSlider_Right);
					PopTransparent();
					FToggle("Unchoke on attack", Vars::CL_Move::Fakelag::UnchokeOnAttack, FToggle_Left);
					FToggle("Retain blastjump", Vars::CL_Move::Fakelag::RetainBlastJump, FToggle_Right);
				} EndSection();
				if (Vars::Debug::Info.Value)
				{
					if (Section("debug"))
					{
						FToggle("retain blastjump soldier only", Vars::CL_Move::Fakelag::RetainSoldierOnly);
					} EndSection();
				}
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
					PushTransparent(FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Edge && FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Jitter);
						FSlider("Real value", Vars::AntiHack::AntiAim::RealYawValue, -180, 180.f, 5.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					PopTransparent();
					PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Edge && FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Jitter);
						FSlider("Fake value", Vars::AntiHack::AntiAim::FakeYawValue, -180.f, 180.f, 5.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
					PopTransparent();
					PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Spin && FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Spin);
						FSlider("Spin speed", Vars::AntiHack::AntiAim::SpinSpeed, -30.f, 30.f, 1.f, "%g", FSlider_Left | FSlider_Precision);
					PopTransparent();
					SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetCursorPosY() - H::Draw.Scale(24) });
					FToggle("Minwalk", Vars::AntiHack::AntiAim::MinWalk, FToggle_Left);
					FToggle("Anti-overlap", Vars::AntiHack::AntiAim::AntiOverlap, FToggle_Left);
					FToggle("Hide pitch on shot", Vars::AntiHack::AntiAim::InvalidShootPitch, FToggle_Right);
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("Resolver"))
				{
					FToggle("Enabled", Vars::AntiHack::Resolver::Enabled, FToggle_Left);
					PushTransparent(!FGet(Vars::AntiHack::Resolver::Enabled));
						FToggle("Auto resolve", Vars::AntiHack::Resolver::AutoResolve, FToggle_Right);
						PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolve));
							FToggle("Auto resolve cheaters only", Vars::AntiHack::Resolver::AutoResolveCheatersOnly, FToggle_Left);
							FToggle("Auto resolve headshot only", Vars::AntiHack::Resolver::AutoResolveHeadshotOnly, FToggle_Right);
							PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolveYawAmount));
								FSlider("Auto resolve yaw", Vars::AntiHack::Resolver::AutoResolveYawAmount, -180.f, 180.f, 45.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
							PopTransparent();
							PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolvePitchAmount));
								FSlider("Auto resolve pitch", Vars::AntiHack::Resolver::AutoResolvePitchAmount, -180.f, 180.f, 90.f, "%g", FSlider_Right | FSlider_Clamp);
							PopTransparent();
						PopTransparent();
						FSlider("Cycle yaw", Vars::AntiHack::Resolver::CycleYaw, -180.f, 180.f, 45.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
						FSlider("Cycle pitch", Vars::AntiHack::Resolver::CyclePitch, -180.f, 180.f, 90.f, "%g", FSlider_Right | FSlider_Clamp);
						FToggle("Cycle view", Vars::AntiHack::Resolver::CycleView, FToggle_Left);
						FToggle("Cycle minwalk", Vars::AntiHack::Resolver::CycleMinwalk, FToggle_Right);
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
						PushTransparent(Transparent || !FGet(Vars::CheaterDetection::DetectionsRequired));
							FSlider("Detections required", Vars::CheaterDetection::DetectionsRequired, 0, 50, 1);
						PopTransparent();

						PushTransparent(Transparent || !(FGet(Vars::CheaterDetection::Methods) & Vars::CheaterDetection::MethodsEnum::PacketChoking));
							FSlider("Minimum choking", Vars::CheaterDetection::MinimumChoking, 4, 22, 1);
						PopTransparent();

						PushTransparent(Transparent || !(FGet(Vars::CheaterDetection::Methods) & Vars::CheaterDetection::MethodsEnum::AimFlicking));
							FSlider("Minimum flick angle", Vars::CheaterDetection::MinimumFlick, 10.f, 30.f, 1.f, "%.0f", FSlider_Left);
							FSlider("Maximum noise", Vars::CheaterDetection::MaximumNoise, 1.f, 10.f, 1.f, "%.0f", FSlider_Right);
						PopTransparent();
					PopTransparent();
				} EndSection();

				EndTable();
			}
			break;
		}

		PopStyleColor();
	} EndChild();
	PopStyleVar();
}

void CMenu::MenuVisuals()
{
	using namespace ImGui;

	const auto vWindowSize = GetWindowSize();
	static int iCurrentTab = 0;
	PushFont(F::Render.FontBold);
	FTabs({ "ESP", "CHAMS", "GLOW", "MISC##", "RADAR", "MENU" }, &iCurrentTab, { H::Draw.Scale(100), H::Draw.Scale(40) }, { 0, 0 });
	PopFont();

	SetCursorPos({ 0, H::Draw.Scale(40) });
	PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
	if (BeginChild("Content", { vWindowSize.x, vWindowSize.y - H::Draw.Scale(40) }, ImGuiWindowFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		PushStyleColor(ImGuiCol_ChildBg, F::Render.Foreground.Value);
		
		switch (iCurrentTab)
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
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Players));
						FDropdown("Player", Vars::ESP::Player, { "Enemy", "Team", "Local", "Prioritized", "Friends", "Party", "##Divider", "Name", "Box", "Distance", "Bones", "Health bar", "Health text", "Uber bar", "Uber text", "Class icon", "Class text", "Weapon icon", "Weapon text", "Priority", "Labels", "Buffs", "Debuffs", "Misc", "Lag compensation", "Ping", "KDR" }, {}, FDropdown_Multi);
					PopTransparent();
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Buildings));
						FDropdown("Building", Vars::ESP::Building, { "Enemy", "Team", "Local", "Prioritized", "Friends", "Party", "##Divider", "Name", "Box", "Distance", "Health bar", "Health text", "Owner", "Level", "Flags" }, {}, FDropdown_Multi);
					PopTransparent();
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Projectiles));
						FDropdown("Projectile", Vars::ESP::Projectile, { "Enemy", "Team", "Local", "Prioritized", "Friends", "Party", "##Divider", "Name", "Box", "Distance", "Flags" }, {}, FDropdown_Multi);
					PopTransparent();
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Objective));
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

					FColorPicker("Local color", Vars::Colors::Local, 0, FColorPicker_Left);
					FColorPicker("Target color", Vars::Colors::Target, 0, FColorPicker_Middle | FColorPicker_SameLine);

					FColorPicker("Healthpack color", Vars::Colors::Health, 0, FColorPicker_Left);
					FColorPicker("Ammopack color", Vars::Colors::Ammo, 0, FColorPicker_Middle | FColorPicker_SameLine);
					FColorPicker("Money color", Vars::Colors::Money, 0, FColorPicker_Left);
					FColorPicker("Powerup color", Vars::Colors::Powerup, 0, FColorPicker_Middle | FColorPicker_SameLine);
					FColorPicker("NPC color", Vars::Colors::NPC, 0, FColorPicker_Left);
					FColorPicker("Halloween color", Vars::Colors::Halloween, 0, FColorPicker_Middle | FColorPicker_SameLine);
				} EndSection();
				if (Section("Dormancy"))
				{
					FSlider("Active alpha", Vars::ESP::ActiveAlpha, 0, 255, 5, "%i", FSlider_Clamp);
					FSlider("Dormant alpha", Vars::ESP::DormantAlpha, 0, 255, 5, "%i", FSlider_Clamp);
					FSlider("Dormant decay", Vars::ESP::DormantTime, 0.015f, 5.0f, 0.1f, "%gs", FSlider_Left | FSlider_Min | FSlider_Precision);
					FToggle("Dormant priority only", Vars::ESP::DormantPriority, FToggle_Right); DebugDummy({ 0, H::Draw.Scale(8) });
				} EndSection();
				if (Vars::Debug::Info.Value)
				{
					if (Section("debug"))
					{
						FColorPicker("indicator good", Vars::Colors::IndicatorGood, 0, FColorPicker_Left);
						FColorPicker("indicator text good", Vars::Colors::IndicatorTextGood, 0, FColorPicker_Middle | FColorPicker_SameLine);
						FColorPicker("indicator bad", Vars::Colors::IndicatorBad, 0, FColorPicker_Left);
						FColorPicker("indicator text bad", Vars::Colors::IndicatorTextBad, 0, FColorPicker_Middle | FColorPicker_SameLine);
						FColorPicker("indicator mid", Vars::Colors::IndicatorMid, 0, FColorPicker_Left);
						FColorPicker("indicator text mid", Vars::Colors::IndicatorTextMid, 0, FColorPicker_Middle | FColorPicker_SameLine);
						FColorPicker("indicator misc", Vars::Colors::IndicatorMisc, 0, FColorPicker_Left);
						FColorPicker("indicator text misc", Vars::Colors::IndicatorTextMisc, 0, FColorPicker_Middle | FColorPicker_SameLine);
					}
					EndSection();
				}

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
					FToggle("Players", Vars::Chams::Friendly::Players, FToggle_Left);
					FToggle("Ragdolls", Vars::Chams::Friendly::Ragdolls, FToggle_Right);
					FToggle("Buildings", Vars::Chams::Friendly::Buildings, FToggle_Left);
					FToggle("Projectiles", Vars::Chams::Friendly::Projectiles, FToggle_Right);

					FMDropdown("Visible material", Vars::Chams::Friendly::Visible, FDropdown_Left);
					FMDropdown("Occluded material", Vars::Chams::Friendly::Occluded, FDropdown_Right);
				} EndSection();
				if (Section("Enemy"))
				{
					FToggle("Players", Vars::Chams::Enemy::Players, FToggle_Left);
					FToggle("Ragdolls", Vars::Chams::Enemy::Ragdolls, FToggle_Right);
					FToggle("Buildings", Vars::Chams::Enemy::Buildings, FToggle_Left);
					FToggle("Projectiles", Vars::Chams::Enemy::Projectiles, FToggle_Right);

					FMDropdown("Visible material", Vars::Chams::Enemy::Visible, FDropdown_Left);
					FMDropdown("Occluded material", Vars::Chams::Enemy::Occluded, FDropdown_Right);
				} EndSection();
				if (Section("World"))
				{
					FToggle("NPCs", Vars::Chams::World::NPCs, FToggle_Left);
					FToggle("Pickups", Vars::Chams::World::Pickups, FToggle_Right);
					FToggle("Objective", Vars::Chams::World::Objective, FToggle_Left);
					FToggle("Powerups", Vars::Chams::World::Powerups, FToggle_Right);
					FToggle("Bombs", Vars::Chams::World::Bombs, FToggle_Left);
					FToggle("Halloween", Vars::Chams::World::Halloween, FToggle_Right);

					FMDropdown("Visible material", Vars::Chams::World::Visible, FDropdown_Left);
					FMDropdown("Occluded material", Vars::Chams::World::Occluded, FDropdown_Right);
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("Player"))
				{
					FToggle("Local", Vars::Chams::Player::Local, FToggle_Left);
					FToggle("Priority", Vars::Chams::Player::Priority, FToggle_Right);
					FToggle("Friend", Vars::Chams::Player::Friend, FToggle_Left);
					FToggle("Party", Vars::Chams::Player::Party, FToggle_Right);
					FToggle("Target", Vars::Chams::Player::Target, FToggle_Left);

					FMDropdown("Visible material", Vars::Chams::Player::Visible, FDropdown_Left);
					FMDropdown("Occluded material", Vars::Chams::Player::Occluded, FDropdown_Right);
				} EndSection();
				if (Section("Backtrack"))
				{
					FToggle("Enabled", Vars::Chams::Backtrack::Enabled, FToggle_Left);
					SameLine(GetWindowWidth() / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
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
					FToggle("Weapon", Vars::Chams::Viewmodel::Weapon, FToggle_Left);
					FToggle("Hands", Vars::Chams::Viewmodel::Hands, FToggle_Right);

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
					FToggle("Players", Vars::Glow::Friendly::Players, FToggle_Left);
					FToggle("Ragdolls", Vars::Glow::Friendly::Ragdolls, FToggle_Right);
					FToggle("Buildings", Vars::Glow::Friendly::Buildings, FToggle_Left);
					FToggle("Projectiles", Vars::Glow::Friendly::Projectiles, FToggle_Right);

					PushTransparent(!FGet(Vars::Glow::Friendly::Stencil));
						FSlider("Stencil scale## Friendly", Vars::Glow::Friendly::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Friendly::Blur));
						FSlider("Blur scale## Friendly", Vars::Glow::Friendly::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
					PopTransparent();
				} EndSection();
				if (Section("Enemy"))
				{
					FToggle("Players", Vars::Glow::Enemy::Players, FToggle_Left);
					FToggle("Ragdolls", Vars::Glow::Enemy::Ragdolls, FToggle_Right);
					FToggle("Buildings", Vars::Glow::Enemy::Buildings, FToggle_Left);
					FToggle("Projectiles", Vars::Glow::Enemy::Projectiles, FToggle_Right);

					PushTransparent(!FGet(Vars::Glow::Enemy::Stencil));
						FSlider("Stencil scale## Enemy", Vars::Glow::Enemy::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Enemy::Blur));
						FSlider("Blur scale## Enemy", Vars::Glow::Enemy::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
					PopTransparent();
				} EndSection();
				if (Section("World"))
				{
					FToggle("NPCs", Vars::Glow::World::NPCs, FToggle_Left);
					FToggle("Pickups", Vars::Glow::World::Pickups, FToggle_Right);
					FToggle("Objective", Vars::Glow::World::Objective, FToggle_Left);
					FToggle("Powerups", Vars::Glow::World::Powerups, FToggle_Right);
					FToggle("Bombs", Vars::Glow::World::Bombs, FToggle_Left);
					FToggle("Halloween", Vars::Glow::World::Halloween, FToggle_Right);

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
					FToggle("Local", Vars::Glow::Player::Local, FToggle_Left);
					FToggle("Priority", Vars::Glow::Player::Priority, FToggle_Right);
					FToggle("Friend", Vars::Glow::Player::Friend, FToggle_Left);
					FToggle("Party", Vars::Glow::Player::Party, FToggle_Right);
					FToggle("Target", Vars::Glow::Player::Target, FToggle_Left);

					PushTransparent(!FGet(Vars::Glow::Player::Stencil));
						FSlider("Stencil scale## Player", Vars::Glow::Player::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min);
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Player::Blur));
						FSlider("Blur scale## Player", Vars::Glow::Player::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min);
					PopTransparent();
				} EndSection();
				if (Section("Backtrack"))
				{
					FToggle("Enabled", Vars::Glow::Backtrack::Enabled, FToggle_Left);
					SameLine(GetWindowWidth() / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
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
					FToggle("Weapon", Vars::Glow::Viewmodel::Weapon, FToggle_Left);
					FToggle("Hands", Vars::Glow::Viewmodel::Hands, FToggle_Right);

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
					FToggle("Scope", Vars::Visuals::Removals::Scope, FToggle_Left);
					FToggle("Interpolation", Vars::Visuals::Removals::Interpolation, FToggle_Right);
					FToggle("Disguises", Vars::Visuals::Removals::Disguises, FToggle_Left);
					FToggle("Screen overlays", Vars::Visuals::Removals::ScreenOverlays, FToggle_Right);
					FToggle("Taunts", Vars::Visuals::Removals::Taunts, FToggle_Left);
					FToggle("Screen effects", Vars::Visuals::Removals::ScreenEffects, FToggle_Right);
					FToggle("View punch", Vars::Visuals::Removals::ViewPunch, FToggle_Left);
					FToggle("Angle forcing", Vars::Visuals::Removals::AngleForcing, FToggle_Right);
					FToggle("MOTD", Vars::Visuals::Removals::MOTD, FToggle_Left);
					FToggle("Convar queries", Vars::Visuals::Removals::ConvarQueries, FToggle_Right);
					FToggle("Post processing", Vars::Visuals::Removals::PostProcessing, FToggle_Left);
					FToggle("DSP", Vars::Visuals::Removals::DSP, FToggle_Right);
				} EndSection();
				if (Section("UI"))
				{
					FDropdown("Streamer mode", Vars::Visuals::UI::StreamerMode, { "Off", "Local", "Friends", "Party", "All" }, {}, FDropdown_Left);
					FDropdown("Chat tags", Vars::Visuals::UI::ChatTags, { "Local", "Friends", "Party", "Assigned" }, {}, FDropdown_Right | FDropdown_Multi);
					PushTransparent(!FGet(Vars::Visuals::UI::FieldOfView));
						FSlider("Field of view", Vars::Visuals::UI::FieldOfView, 0, 160, 1, "%i", FSlider_Min);
					PopTransparent();
					PushTransparent(!FGet(Vars::Visuals::UI::ZoomFieldOfView));
						FSlider("Zoomed field of view", Vars::Visuals::UI::ZoomFieldOfView, 0, 160, 1, "%i", FSlider_Min);
					PopTransparent();
					PushTransparent(!FGet(Vars::Visuals::UI::AspectRatio));
						FSlider("Aspect ratio", Vars::Visuals::UI::AspectRatio, 0.f, 5.f, 0.01f, "%g", FSlider_Min | FSlider_Precision);
					PopTransparent();
					FToggle("Reveal scoreboard", Vars::Visuals::UI::RevealScoreboard, FToggle_Left);
					FToggle("Scoreboard utility", Vars::Visuals::UI::ScoreboardUtility, FToggle_Right);
					FToggle("Scoreboard colors", Vars::Visuals::UI::ScoreboardColors, FToggle_Left);
					FToggle("Clean screenshots", Vars::Visuals::UI::CleanScreenshots, FToggle_Right);
					FToggle("Sniper sightlines", Vars::Visuals::UI::SniperSightlines, FToggle_Left);
					FToggle("Pickup timers", Vars::Visuals::UI::PickupTimers, FToggle_Right);
				} EndSection();
				if (Section("Viewmodel"))
				{
					FToggle("Crosshair aim position", Vars::Visuals::Viewmodel::CrosshairAim, FToggle_Left);
					FToggle("Viewmodel aim position", Vars::Visuals::Viewmodel::ViewmodelAim, FToggle_Right);
					FSlider("Offset X", Vars::Visuals::Viewmodel::OffsetX, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision);
					FSlider("Pitch", Vars::Visuals::Viewmodel::Pitch, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision);
					FSlider("Offset Y", Vars::Visuals::Viewmodel::OffsetY, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision);
					FSlider("Yaw", Vars::Visuals::Viewmodel::Yaw, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision);
					FSlider("Offset Z", Vars::Visuals::Viewmodel::OffsetZ, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision);
					FSlider("Roll", Vars::Visuals::Viewmodel::Roll, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision);
					PushTransparent(!FGet(Vars::Visuals::Viewmodel::FieldOfView));
						FSlider("Field of view## Viewmodel", Vars::Visuals::Viewmodel::FieldOfView, 0.f, 180.f, 1.f, "%.0f", FSlider_Clamp | FSlider_Precision);
					PopTransparent();
					PushTransparent(!FGet(Vars::Visuals::Viewmodel::SwayScale) || !FGet(Vars::Visuals::Viewmodel::SwayInterp));
						FSlider("Sway scale", Vars::Visuals::Viewmodel::SwayScale, 0.f, 5.f, 0.5f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
						FSlider("Sway interp", Vars::Visuals::Viewmodel::SwayInterp, 0.f, 1.f, 0.1f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					PopTransparent();
				} EndSection();
				if (Section("Particles"))
				{
					// https://developer.valvesoftware.com/wiki/Team_Fortress_2/Particles
					// https://forums.alliedmods.net/showthread.php?t=127111
					FSDropdown("Bullet trail", Vars::Visuals::Particles::BulletTrail, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink", "Line", "Beam" }, FDropdown_Left | FSDropdown_Custom);
					FSDropdown("Crit trail", Vars::Visuals::Particles::CritTrail, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink", "Line", "Beam" }, FDropdown_Right | FSDropdown_Custom);
					FSDropdown("Medigun beam", Vars::Visuals::Particles::MedigunBeam, { "Off", "None", "Uber", "Dispenser", "Passtime", "Bombonomicon", "White", "Orange" }, FDropdown_Left | FSDropdown_Custom);
					FSDropdown("Medigun charge", Vars::Visuals::Particles::MedigunCharge, { "Off", "None", "Electrocuted", "Halloween", "Fireball", "Teleport", "Burning", "Scorching", "Purple energy", "Green energy", "Nebula", "Purple stars", "Green stars", "Sunbeams", "Spellbound", "Purple sparks", "Yellow sparks", "Green zap", "Yellow zap", "Plasma", "Frostbite", "Time warp", "Purple souls", "Green souls", "Bubbles", "Hearts" }, FDropdown_Right | FSDropdown_Custom);
					FSDropdown("Projectile trail", Vars::Visuals::Particles::ProjectileTrail, { "Off", "None", "Rocket", "Critical", "Energy", "Charged", "Ray", "Fireball", "Teleport", "Fire", "Flame", "Sparks", "Flare", "Trail", "Health", "Smoke", "Bubbles", "Halloween", "Monoculus", "Sparkles", "Rainbow" }, FDropdown_Left | FSDropdown_Custom);
					FDropdown("Spell footsteps", Vars::Visuals::Particles::SpellFootsteps, { "Off", "Color", "Team", "Halloween" }, {}, FDropdown_Right, -10);
					FColorPicker("Spell footstep", Vars::Colors::SpellFootstep, 0, FColorPicker_Dropdown);
					FToggle("Draw icons through walls", Vars::Visuals::Particles::DrawIconsThroughWalls);
					FToggle("Draw damage numbers through walls", Vars::Visuals::Particles::DrawDamageNumbersThroughWalls);
				} EndSection();
				if (Section("Ragdolls"))
				{
					FToggle("No ragdolls", Vars::Visuals::Ragdolls::NoRagdolls, FToggle_Left);
					FToggle("No gibs", Vars::Visuals::Ragdolls::NoGib, FToggle_Right);
					FToggle("Mods", Vars::Visuals::Ragdolls::Enabled, FToggle_Left);
					PushTransparent(!FGet(Vars::Visuals::Ragdolls::Enabled));
						FToggle("Enemy only", Vars::Visuals::Ragdolls::EnemyOnly, FToggle_Right);
						FDropdown("Ragdoll effects", Vars::Visuals::Ragdolls::Effects, { "Burning", "Electrocuted", "Ash", "Dissolve" }, {}, FDropdown_Multi | FDropdown_Left);
						FDropdown("Ragdoll model", Vars::Visuals::Ragdolls::Type, { "None", "Gold", "Ice" }, {}, FDropdown_Right);
						FSlider("Ragdoll force", Vars::Visuals::Ragdolls::Force, -10.f, 10.f, 0.5f, "%g", FSlider_Precision);
						FSlider("Horizontal force", Vars::Visuals::Ragdolls::ForceHorizontal, -10.f, 10.f, 0.5f, "%g", FSlider_Precision);
						FSlider("Vertical force", Vars::Visuals::Ragdolls::ForceVertical, -10.f, 10.f, 0.5f, "%g", FSlider_Precision);
					PopTransparent();
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("Line"))
				{
					FColorPicker("Line tracer clipped", Vars::Colors::LineClipped);
					FColorPicker("Line tracer", Vars::Colors::Line, 1);
					FToggle("Line tracers", Vars::Visuals::Line::Enabled);
					FSlider("Draw duration## Line", Vars::Visuals::Line::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision);
				} EndSection();
				if (Section("Simulation"))
				{
					FDropdown("Player path", Vars::Visuals::Simulation::PlayerPath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Left, -20);
					FColorPicker("Player path", Vars::Colors::PlayerPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip); FColorPicker("Player path clipped", Vars::Colors::PlayerPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FDropdown("Projectile path", Vars::Visuals::Simulation::ProjectilePath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Right, -20);
					FColorPicker("Projectile path", Vars::Colors::ProjectilePath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip); FColorPicker("Projectile path clipped", Vars::Colors::ProjectilePathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FDropdown("Trajectory path", Vars::Visuals::Simulation::TrajectoryPath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Left, -20);
					FColorPicker("Trajectory path", Vars::Colors::TrajectoryPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip); FColorPicker("Trajectory path clipped", Vars::Colors::TrajectoryPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FDropdown("Shot path", Vars::Visuals::Simulation::ShotPath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Right, -20);
					FColorPicker("Shot path", Vars::Colors::ShotPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip); FColorPicker("Shot path clipped", Vars::Colors::ShotPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FDropdown("Splash radius", Vars::Visuals::Simulation::SplashRadius, { "Simulation", "##Divider", "Priority", "Enemy", "Team", "Local", "Friends", "Party", "##Divider", "Rockets", "Stickies", "Pipes", "Scorch shot", "##Divider", "Trace" }, {}, FDropdown_Multi, -20);
					FColorPicker("Splash radius", Vars::Colors::SplashRadius, 0, FColorPicker_Dropdown | FColorPicker_Tooltip); FColorPicker("Splash radius clipped", Vars::Colors::SplashRadiusClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FToggle("Timed", Vars::Visuals::Simulation::Timed, FToggle_Left);
					FToggle("Box", Vars::Visuals::Simulation::Box, FToggle_Right);
					FToggle("Swing prediction lines", Vars::Visuals::Simulation::SwingLines, FToggle_Left);
					FToggle("Projectile camera", Vars::Visuals::Simulation::ProjectileCamera, FToggle_Right);
					PushTransparent(FGet(Vars::Visuals::Simulation::Timed));
						FSlider("Draw duration## Simulation", Vars::Visuals::Simulation::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision);
					PopTransparent();
				} EndSection();
				if (Vars::Debug::Info.Value)
				{
					if (Section("debug## part1"))
					{
						FSlider("seperator spacing", Vars::Visuals::Simulation::SeparatorSpacing, 1, 16, 1, "%d", FSlider_Left);
						FSlider("seperator length", Vars::Visuals::Simulation::SeparatorLength, 2, 16, 1, "%d", FSlider_Right);
					} EndSection();
					if (Section("debug## part2"))
					{
						FToggle("simulation overwrite", Vars::Visuals::Trajectory::Overwrite);
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
					FDropdown("Bones enabled", Vars::Visuals::Hitbox::BonesEnabled, { "On shot", "On hit" }, {}, FDropdown_Multi, -110);
					FColorPicker("Target edge color", Vars::Colors::TargetHitboxEdge, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FColorPicker("Target edge color clipped", Vars::Colors::TargetHitboxEdgeClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker("Target face color", Vars::Colors::TargetHitboxFace, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FColorPicker("Target face color clipped", Vars::Colors::TargetHitboxFaceClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker("Bone edge color", Vars::Colors::BoneHitboxEdge, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FColorPicker("Bone edge color clipped", Vars::Colors::BoneHitboxEdgeClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker("Bone face color", Vars::Colors::BoneHitboxFace, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FColorPicker("Bone face color clipped", Vars::Colors::BoneHitboxFaceClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);

					FDropdown("Bounds enabled", Vars::Visuals::Hitbox::BoundsEnabled, { "On shot", "On hit", "Aim point" }, {}, FDropdown_Multi, -50);
					FColorPicker("Bound edge color", Vars::Colors::BoundHitboxEdge, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FColorPicker("Bound edge color clipped", Vars::Colors::BoundHitboxEdgeClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker("Bound face color", Vars::Colors::BoundHitboxFace, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);
					FColorPicker("Bound face color clipped", Vars::Colors::BoundHitboxFaceClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip);

					FSlider("Draw duration## Hitbox", Vars::Visuals::Hitbox::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision);
				} EndSection();
				if (Section("Thirdperson"))
				{
					FToggle("Thirdperson", Vars::Visuals::ThirdPerson::Enabled, FToggle_Left);
					FToggle("Thirdperson crosshair", Vars::Visuals::ThirdPerson::Crosshair, FToggle_Right);
					FSlider("Thirdperson distance", Vars::Visuals::ThirdPerson::Distance, 0.f, 500.f, 5.f, "%g", FSlider_Precision);
					FSlider("Thirdperson right", Vars::Visuals::ThirdPerson::Right, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
					FSlider("Thirdperson up", Vars::Visuals::ThirdPerson::Up, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
				} EndSection();
				if (Vars::Debug::Info.Value)
				{
					if (Section("debug"))
					{
						FToggle("thirdperson scales", Vars::Visuals::ThirdPerson::Scale);
					} EndSection();
				}
				if (Section("Out of FOV arrows"))
				{
					FToggle("Enabled", Vars::Visuals::FOVArrows::Enabled);
					FSlider("Offset", Vars::Visuals::FOVArrows::Offset, 0, 500, 25, "%i", FSlider_Min | FSlider_Precision);
					FSlider("Max distance", Vars::Visuals::FOVArrows::MaxDist, 0.f, 5000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
				} EndSection();
				if (Section("World"))
				{
					FSDropdown("World texture", Vars::Visuals::World::WorldTexture, { "Default", "Dev", "Camo", "Black", "White", "Flat" }, FSDropdown_Custom);
					FDropdown("Modulations", Vars::Visuals::World::Modulations, { "World", "Sky", "Prop", "Particle", "Fog" }, {}, FDropdown_Left | FDropdown_Multi);
					static std::vector skyNames = {
						"Off", "sky_tf2_04", "sky_upward", "sky_dustbowl_01", "sky_goldrush_01", "sky_granary_01", "sky_well_01", "sky_gravel_01", "sky_badlands_01",
						"sky_hydro_01", "sky_night_01", "sky_nightfall_01", "sky_trainyard_01", "sky_stormfront_01", "sky_morningsnow_01","sky_alpinestorm_01",
						"sky_harvest_01", "sky_harvest_night_01", "sky_halloween", "sky_halloween_night_01", "sky_halloween_night2014_01", "sky_island_01", "sky_rainbow_01"
					};
					FSDropdown("Skybox changer", Vars::Visuals::World::SkyboxChanger, skyNames, FSDropdown_Custom | FDropdown_Right);
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::World));
						FColorPicker("World modulation", Vars::Colors::WorldModulation, 0, FColorPicker_Left);
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Sky));
						FColorPicker("Sky modulation", Vars::Colors::SkyModulation, 0, FColorPicker_Middle | FColorPicker_SameLine);
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Prop));
						FColorPicker("Prop modulation", Vars::Colors::PropModulation, 0, FColorPicker_Left);
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Particle));
						FColorPicker("Particle modulation", Vars::Colors::ParticleModulation, 0, FColorPicker_Middle | FColorPicker_SameLine);
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Fog));
						FColorPicker("Fog modulation", Vars::Colors::FogModulation, 0, FColorPicker_Left);
					PopTransparent();
					FToggle("Near prop fade", Vars::Visuals::World::NearPropFade, FToggle_Left);
					FToggle("No prop fade", Vars::Visuals::World::NoPropFade, FToggle_Right);
				} EndSection();
				if (Section("Misc"))
				{
					FSDropdown("Local domination override", Vars::Visuals::Misc::LocalDominationOverride, {}, FDropdown_Left);
					FSDropdown("Local revenge override", Vars::Visuals::Misc::LocalRevengeOverride, {}, FDropdown_Right);
					FSDropdown("Domination override", Vars::Visuals::Misc::DominationOverride, {}, FDropdown_Left);
					FSDropdown("Revenge override", Vars::Visuals::Misc::RevengeOverride, {}, FDropdown_Right);
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
					FToggle("Enabled", Vars::Radar::Main::Enabled, FToggle_Left);
					FToggle("Draw out of range", Vars::Radar::Main::AlwaysDraw, FToggle_Right);
					FDropdown("Style", Vars::Radar::Main::Style, { "Circle", "Rectangle" });
					FSlider("Range", Vars::Radar::Main::Range, 50, 3000, 50, "%i", FSlider_Precision);
					FSlider("Background alpha", Vars::Radar::Main::BackAlpha, 0, 255, 1, "%i", FSlider_Clamp);
					FSlider("Line alpha", Vars::Radar::Main::LineAlpha, 0, 255, 1, "%i", FSlider_Clamp);
				} EndSection();
				if (Section("Player"))
				{
					FToggle("Enabled", Vars::Radar::Players::Enabled, FToggle_Left);
					FToggle("Background", Vars::Radar::Players::Background, FToggle_Right);
					FDropdown("Draw", Vars::Radar::Players::Draw, { "Local", "Enemy", "Team", "Friends", "Party", "Prioritized", "Cloaked" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Icon", Vars::Radar::Players::IconType, { "Icons", "Portraits", "Avatar" }, {}, FDropdown_Right);
					FSlider("Icon size## Player", Vars::Radar::Players::IconSize, 12, 30, 2);
					FToggle("Health bar", Vars::Radar::Players::Health, FToggle_Left);
					FToggle("Height indicator", Vars::Radar::Players::Height, FToggle_Right);
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("Building"))
				{
					FToggle("Enabled", Vars::Radar::Buildings::Enabled, FToggle_Left);
					FToggle("Background", Vars::Radar::Buildings::Background, FToggle_Right);
					FDropdown("Draw", Vars::Radar::Buildings::Draw, { "Local", "Enemy", "Team", "Friends", "Party", "Prioritized" }, {}, FDropdown_Multi);
					FSlider("Icon size## Building", Vars::Radar::Buildings::IconSize, 12, 30, 2);
					FToggle("Health bar", Vars::Radar::Buildings::Health);
				} EndSection();
				if (Section("World"))
				{
					FToggle("Enabled", Vars::Radar::World::Enabled, FToggle_Left);
					FToggle("Background", Vars::Radar::World::Background, FToggle_Right);
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
					FColorPicker("Background color", Vars::Menu::Theme::Background, 0, FColorPicker_Middle | FColorPicker_SameLine);
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

				/* Column 2 */
				TableNextColumn();
				if (Section("Indicators"))
				{
					FDropdown("Indicators", Vars::Menu::Indicators, { "Ticks", "Crit hack", "Spectators", "Ping", "Conditions", "Seed prediction" }, {}, FDropdown_Multi);
					if (FSlider("Scale", Vars::Menu::Scale, 0.75f, 2.f, 0.25f, "%g", FSlider_Min | FSlider_Precision | FSlider_NoAutoUpdate))
						H::Fonts.Reload(Vars::Menu::Scale.Map[DEFAULT_BIND]);
					FToggle("Cheap text", Vars::Menu::CheapText);
				} EndSection();

				EndTable();
			}
		}
		}

		PopStyleColor();
	} EndChild();
	PopStyleVar();
}

void CMenu::MenuMisc()
{
	using namespace ImGui;

	const auto vWindowSize = GetWindowSize();
	static int iCurrentTab = 0;
	PushFont(F::Render.FontBold);
	FTabs({ "MISC##" }, &iCurrentTab, { H::Draw.Scale(100), H::Draw.Scale(40) }, { 0, 0 });
	PopFont();

	SetCursorPos({ 0, H::Draw.Scale(40) });
	PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
	if (BeginChild("Content", { vWindowSize.x, vWindowSize.y - H::Draw.Scale(40) }, ImGuiWindowFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		PushStyleColor(ImGuiCol_ChildBg, F::Render.Foreground.Value);
		
		switch (iCurrentTab)
		{
		case 0:
			if (BeginTable("MiscTable", 2))
			{
				/* Column 1 */
				TableNextColumn();
				if (Section("Movement"))
				{
					FDropdown("Auto strafe", Vars::Misc::Movement::AutoStrafe, { "Off", "Legit", "Directional" });
					PushTransparent(FGet(Vars::Misc::Movement::AutoStrafe) != Vars::Misc::Movement::AutoStrafeEnum::Directional);
						FSlider("Auto strafe turn scale", Vars::Misc::Movement::AutoStrafeTurnScale, 0.f, 1.f, 0.1f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
						FSlider("Auto strafe max delta", Vars::Misc::Movement::AutoStrafeMaxDelta, 0.f, 180.f, 1.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
					PopTransparent();
					FToggle("Bunnyhop", Vars::Misc::Movement::Bunnyhop, FToggle_Left);
					FToggle("Auto jumpbug", Vars::Misc::Movement::AutoJumpbug, FToggle_Right); // this is unreliable without setups, do not depend on it!
					FToggle("Auto rocketjump", Vars::Misc::Movement::AutoRocketJump, FToggle_Left);
					FToggle("Auto ctap", Vars::Misc::Movement::AutoCTap, FToggle_Right);
					FToggle("Fast stop", Vars::Misc::Movement::FastStop, FToggle_Left);
					FToggle("Fast accelerate", Vars::Misc::Movement::FastAccel, FToggle_Right);
					FToggle("No push", Vars::Misc::Movement::NoPush, FToggle_Left);
					FToggle("Crouch speed", Vars::Misc::Movement::CrouchSpeed, FToggle_Right);
					FToggle("Movement lock", Vars::Misc::Movement::MovementLock, FToggle_Left);
					FToggle("Break jump", Vars::Misc::Movement::BreakJump, FToggle_Right);
					FToggle("Shield turn rate", Vars::Misc::Movement::ShieldTurnRate);
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
					FToggle("Cheats bypass", Vars::Misc::Exploits::CheatsBypass, FToggle_Left);
					FToggle("Pure bypass", Vars::Misc::Exploits::BypassPure, FToggle_Right);
					FToggle("Ping reducer", Vars::Misc::Exploits::PingReducer, FToggle_Left);
					PushTransparent(!FGet(Vars::Misc::Exploits::PingReducer));
						FSlider("cl_cmdrate", Vars::Misc::Exploits::PingTarget, 1, 66, 1, "%i", FSlider_Right | FSlider_Clamp);
					PopTransparent();
					SetCursorPosY(GetCursorPosY() - 8);
					FToggle("Equip region unlock", Vars::Misc::Exploits::EquipRegionUnlock, FToggle_Left);
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
					FToggle("Anti-AFK", Vars::Misc::Automation::AntiAFK, FToggle_Left);
					FToggle("Anti autobalance", Vars::Misc::Automation::AntiAutobalance, FToggle_Right);
					FToggle("Auto accept item drops", Vars::Misc::Automation::AcceptItemDrops, FToggle_Left);
					FToggle("Taunt control", Vars::Misc::Automation::TauntControl, FToggle_Right);
					FToggle("Kart control", Vars::Misc::Automation::KartControl, FToggle_Left);
					FToggle("Backpack expander", Vars::Misc::Automation::BackpackExpander, FToggle_Right);
					FToggle("Auto F2 ignored", Vars::Misc::Automation::AutoF2Ignored, FToggle_Left);
					FToggle("Auto F1 priority", Vars::Misc::Automation::AutoF1Priority, FToggle_Right);
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("Sound"))
				{
					FDropdown("Block", Vars::Misc::Sound::Block, { "Footsteps", "Noisemaker", "Frying pan" }, {}, FDropdown_Multi);
					FToggle("Giant weapon sounds", Vars::Misc::Sound::GiantWeaponSounds, FToggle_Left);
					FToggle("Hitsound always", Vars::Misc::Sound::HitsoundAlways, FToggle_Right);
				} EndSection();
				if (Section("Game"))
				{
					FToggle("Network fix", Vars::Misc::Game::NetworkFix, FToggle_Left);
					FToggle("Prediction error jitter fix", Vars::Misc::Game::PredictionErrorJitterFix, FToggle_Right);
					FToggle("Bones optimization", Vars::Misc::Game::SetupBonesOptimization, FToggle_Left);
					FToggle("F2P chat bypass", Vars::Misc::Game::F2PChatBypass, FToggle_Right);
				} EndSection();
				if (Section("Queueing"))
				{
					FDropdown("Force regions", Vars::Misc::Queueing::ForceRegions,
						{ "Atlanta", "Chicago", "Texas", "Los Angeles", "Moses Lake", "New York", "Seattle", "Virginia", "##Divider", "Amsterdam", "Frankfurt", "Helsinki", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "##Divider", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "##Divider", "Bombay", "Chennai", "Dubai", "Hong Kong", "Madras", "Mumbai", "Seoul", "Singapore", "Tokyo", "Sydney", "##Divider", "Johannesburg" },
						{}, FDropdown_Multi
					);
					FToggle("Freeze queue", Vars::Misc::Queueing::FreezeQueue, FToggle_Left);
					FToggle("Auto queue", Vars::Misc::Queueing::AutoCasualQueue, FToggle_Right);
				} EndSection();
				if (Section("Mann vs. Machine"))
				{
					FToggle("Instant respawn", Vars::Misc::MannVsMachine::InstantRespawn, FToggle_Left);
					FToggle("Instant revive", Vars::Misc::MannVsMachine::InstantRevive, FToggle_Right);
					FToggle("Allow MVM inspect", Vars::Misc::MannVsMachine::AllowInspect);
				} EndSection();
				if (Section("Steam RPC"))
				{
					FToggle("Steam RPC", Vars::Misc::Steam::EnableRPC, FToggle_Left);
					FToggle("Override in menu", Vars::Misc::Steam::OverrideMenu, FToggle_Right);
					FDropdown("Match group", Vars::Misc::Steam::MatchGroup, { "Special Event", "MvM Mann Up", "Competitive", "Casual", "MvM Boot Camp" }, {}, FDropdown_Left);
					FSDropdown("Map text", Vars::Misc::Steam::MapText, {}, FSDropdown_Custom | FDropdown_Right);
					FSlider("Group size", Vars::Misc::Steam::GroupSize, 0, 6);
				} EndSection();

				EndTable();
			}
		}

		PopStyleColor();
	} EndChild();
	PopStyleVar();
}

void CMenu::MenuLogs()
{
	using namespace ImGui;

	const auto vWindowSize = GetWindowSize();
	static int iCurrentTab = 0;
	PushFont(F::Render.FontBold);
	FTabs({ "LOGS##", "SETTINGS##" }, &iCurrentTab, { H::Draw.Scale(100), H::Draw.Scale(40) }, { 0, 0 });
	PopFont();

	SetCursorPos({ 0, H::Draw.Scale(40) });
	PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
	if (BeginChild("Content", { vWindowSize.x, vWindowSize.y - H::Draw.Scale(40) }, ImGuiWindowFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		PushStyleColor(ImGuiCol_ChildBg, F::Render.Foreground.Value);
		
		switch (iCurrentTab)
		{
		// Logs
		case 0:
			// Eventually put logs here
			break;
		// Settings
		case 1:
			if (BeginTable("ConfigSettingsTable", 2))
			{
				/* Column 1 */
				TableNextColumn();
				if (Section("Logging"))
				{
					FDropdown("Logs", Vars::Logging::Logs, { "Vote start", "Vote cast", "Class changes", "Damage", "Cheat detection", "Tags", "Aliases", "Resolver" }, {}, FDropdown_Multi);
					FSlider("Notification time", Vars::Logging::Lifetime, 0.5f, 5.f, 0.5f, "%g");
				} EndSection();
				if (Section("Vote Start"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteStart));
						FDropdown("Log to", Vars::Logging::VoteStart::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();
				if (Section("Vote Cast"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteCast));
						FDropdown("Log to", Vars::Logging::VoteCast::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();
				if (Section("Class Change"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::ClassChanges));
						FDropdown("Log to", Vars::Logging::ClassChange::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (Section("Damage"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Damage));
						FDropdown("Log to", Vars::Logging::Damage::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();
				if (Section("Cheat Detection"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::CheatDetection));
						FDropdown("Log to", Vars::Logging::CheatDetection::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();
				if (Section("Tags"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Tags));
						FDropdown("Log to", Vars::Logging::Tags::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();
				if (Section("Aliases"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Aliases));
						FDropdown("Log to", Vars::Logging::Aliases::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();
				if (Section("Resolver"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Resolver));
						FDropdown("Log to", Vars::Logging::Resolver::LogTo, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
					PopTransparent();
				} EndSection();

				EndTable();
			}
		}

		PopStyleColor();
	} EndChild();
	PopStyleVar();
}

void CMenu::MenuSettings()
{
	using namespace ImGui;

	const auto vWindowSize = GetWindowSize();
	static int iCurrentTab = 0;
	PushFont(F::Render.FontBold);
	FTabs({ "CONFIG", "BINDS", "PLAYERLIST", "MATERIALS" }, &iCurrentTab, { H::Draw.Scale(100), H::Draw.Scale(40) }, { 0, 0 });
	PopFont();

	SetCursorPos({ 0, H::Draw.Scale(40) });
	PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
	if (BeginChild("Content", { vWindowSize.x, vWindowSize.y - H::Draw.Scale(40) }, ImGuiWindowFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		PushStyleColor(ImGuiCol_ChildBg, F::Render.Foreground.Value);
		
		switch (iCurrentTab)
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

					static int iCurrentType = 0;
					FTabs({ "GENERAL", "VISUALS", }, &iCurrentType, { GetColumnWidth() / 2, H::Draw.Scale(40) }, { H::Draw.Scale(8), GetCursorPos().y }, false);

					switch (iCurrentType)
					{
					// General
					case 0:
					{
						static std::string newName;
						FSDropdown("Config name", &newName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
						if (FButton("Create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
						{
							if (!std::filesystem::exists(F::Configs.sConfigPath + "\\" + newName))
								F::Configs.SaveConfig(newName);
							newName.clear();
						}

						std::vector<std::pair<std::filesystem::directory_entry, std::string>> vConfigs = {};
						bool bDefaultFound = false;
						for (auto& entry : std::filesystem::directory_iterator(F::Configs.sConfigPath))
						{
							if (!entry.is_regular_file() || entry.path().extension() != F::Configs.sConfigExtension)
								continue;

							std::string sConfigName = entry.path().filename().string();
							sConfigName.erase(sConfigName.end() - F::Configs.sConfigExtension.size(), sConfigName.end());
							if (FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"))
								bDefaultFound = true;

							vConfigs.push_back({ entry, sConfigName });
						}
						if (!bDefaultFound)
							F::Configs.SaveConfig("default");
						std::sort(vConfigs.begin(), vConfigs.end(), [&](const auto& a, const auto& b) -> bool
							{
								// override for default config
								if (FNV1A::Hash32(a.second.c_str()) == FNV1A::Hash32Const("default"))
									return true;
								if (FNV1A::Hash32(b.second.c_str()) == FNV1A::Hash32Const("default"))
									return false;

								return a.second < b.second;
							});

						for (auto& [entry, sConfigName] : vConfigs)
						{
							bool bCurrentConfig = FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32(F::Configs.sCurrentConfig.c_str());
							ImVec2 vOriginalPos = GetCursorPos();

							SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
							TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, sConfigName.c_str());

							int iOffset = 0;
							SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
							if (IconButton(ICON_MD_DELETE))
								OpenPopup(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str());

							SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
							if (IconButton(ICON_MD_SAVE))
							{
								if (!bCurrentConfig || F::Configs.sCurrentVisuals.length())
									OpenPopup(std::format("Confirmation## SaveConfig{}", sConfigName).c_str());
								else
									F::Configs.SaveConfig(sConfigName);
							}

							SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
							if (IconButton(ICON_MD_DOWNLOAD))
								F::Configs.LoadConfig(sConfigName);

							if (BeginPopupModal(std::format("Confirmation## SaveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								FText(std::format("Do you really want to override '{}'?", sConfigName).c_str());

								if (FButton("Yes, override", FButton_Left))
								{
									F::Configs.SaveConfig(sConfigName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}

							if (BeginPopupModal(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								FText(std::format("Do you really want to remove '{}'?", sConfigName).c_str());

								PushDisabled(FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"));
								if (FButton("Yes, delete", FButton_Fit))
								{
									F::Configs.RemoveConfig(sConfigName);
									CloseCurrentPopup();
								}
								PopDisabled();
								if (FButton("Yes, reset", FButton_Fit | FButton_SameLine))
								{
									F::Configs.ResetConfig(sConfigName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Fit | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}

							SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
						}
						break;
					}
					// Visuals
					case 1:
					{
						static std::string newName;
						FSDropdown("Config name", &newName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
						if (FButton("Create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
						{
							if (!std::filesystem::exists(F::Configs.sVisualsPath + "\\" + newName))
								F::Configs.SaveVisual(newName);
							newName.clear();
						}

						for (auto& entry : std::filesystem::directory_iterator(F::Configs.sVisualsPath))
						{
							if (!entry.is_regular_file() || entry.path().extension() != F::Configs.sConfigExtension)
								continue;

							std::string sConfigName = entry.path().filename().string();
							sConfigName.erase(sConfigName.end() - F::Configs.sConfigExtension.size(), sConfigName.end());

							bool bCurrentConfig = FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32(F::Configs.sCurrentVisuals.c_str());
							ImVec2 vOriginalPos = GetCursorPos();

							SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
							TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, sConfigName.c_str());

							int iOffset = 0;
							SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
							if (IconButton(ICON_MD_DELETE))
								OpenPopup(std::format("Confirmation## DeleteVisual{}", sConfigName).c_str());

							SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
							if (IconButton(ICON_MD_SAVE))
							{
								if (!bCurrentConfig)
									OpenPopup(std::format("Confirmation## SaveVisual{}", sConfigName).c_str());
								else
									F::Configs.SaveVisual(sConfigName);
							}

							SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
							if (IconButton(ICON_MD_DOWNLOAD))
								F::Configs.LoadVisual(sConfigName);

							// Dialogs
							{
								// Save config dialog
								if (BeginPopupModal(std::format("Confirmation## SaveVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
								{
									FText(std::format("Do you really want to override '{}'?", sConfigName).c_str());

									if (FButton("Yes, override", FButton_Left))
									{
										F::Configs.SaveVisual(sConfigName);
										CloseCurrentPopup();
									}
									if (FButton("No", FButton_Right | FButton_SameLine))
										CloseCurrentPopup();

									EndPopup();
								}

								// Delete config dialog
								if (BeginPopupModal(std::format("Confirmation## DeleteVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
								{
									FText(std::format("Do you really want to delete '{}'?", sConfigName).c_str());

									if (FButton("Yes, delete", FButton_Left))
									{
										F::Configs.RemoveVisual(sConfigName);
										CloseCurrentPopup();
									}
									if (FButton("No", FButton_Right | FButton_SameLine))
										CloseCurrentPopup();

									EndPopup();
								}
							}

							SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
						}
					}
					}
				} EndSection();
				SetCursorPosX(GetCursorPosX() + 8);
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				FText("Built @ " __DATE__ ", " __TIME__);
				PopStyleColor();

				/* Column 2 */
				TableNextColumn();
				if (Section("Debug"))
				{
					FToggle("Debug info", Vars::Debug::Info, FToggle_Left);
					FToggle("Debug logging", Vars::Debug::Logging, FToggle_Right);
					FToggle("Show server hitboxes", Vars::Debug::ServerHitbox, FToggle_Left); FTooltip("Only localhost servers");
					FToggle("Anti aim lines", Vars::Debug::AntiAimLines, FToggle_Right);
	#ifdef DEBUG_TRACES
					FToggle("Visualize traces", Vars::Debug::VisualizeTraces, FToggle_Left);
					FToggle("Visualize trace hits", Vars::Debug::VisualizeTraceHits, FToggle_Right);
	#endif
					FToggle("Crash logging", Vars::Debug::CrashLogging);
				} EndSection();
				if (Section("Extra"))
				{
					if (FButton("cl_fullupdate", FButton_Left))
						I::EngineClient->ClientCmd_Unrestricted("cl_fullupdate");
					if (FButton("retry", FButton_Right | FButton_SameLine))
						I::EngineClient->ClientCmd_Unrestricted("retry");
					if (FButton("Console", FButton_Left))
						I::EngineClient->ClientCmd_Unrestricted("toggleconsole");
					if (FButton("Fix materials", FButton_Right | FButton_SameLine) && F::Materials.m_bLoaded)
						F::Materials.ReloadMaterials();

					if (!I::EngineClient->IsConnected())
					{
						if (FButton("Unlock achievements", FButton_Left))
							OpenPopup("UnlockAchievements");
						if (FButton("Lock achievements", FButton_Right | FButton_SameLine))
							OpenPopup("LockAchievements");

						if (BeginPopupModal("UnlockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText("Do you really want to unlock all achievements?");

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
							FText("Do you really want to lock all achievements?");

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
						if (FButton("Restore lines", FButton_Left))
							F::Visuals.RestoreLines();
						if (FButton("Restore paths", FButton_Right | FButton_SameLine))
							F::Visuals.RestorePaths();
						if (FButton("Restore boxes", FButton_Left))
							F::Visuals.RestoreBoxes();
						if (FButton("Clear visuals", FButton_Right | FButton_SameLine))
						{
							G::LineStorage.clear();
							G::PathStorage.clear();
							G::BoxStorage.clear();
						}
					}
				} EndSection();

				EndTable();
			}
			break;
		// Binds
		case 1:
			if (Section("Settings"))
			{
				FToggle("Show bind window", Vars::Menu::ShowBinds, FToggle_Left);
				FToggle("Menu shows binds", Vars::Menu::MenuShowsBinds, FToggle_Right);
			} EndSection();
			if (Section("Binds"))
			{
				static int iBind = DEFAULT_BIND;
				static Bind_t tBind = {};

				static int bParent = false;
				if (bParent)
					SetMouseCursor(ImGuiMouseCursor_Hand);

				{
					ImVec2 vOriginalPos = GetCursorPos();

					SetCursorPos({ 0, vOriginalPos.y - H::Draw.Scale(8) });
					if (BeginChild("Split1", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(112) }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
					{
						FSDropdown("Name", &tBind.Name, {}, FSDropdown_AutoUpdate | FDropdown_Left);
						{
							auto sParent = bParent ? "..." : tBind.Parent != DEFAULT_BIND && tBind.Parent < F::Binds.vBinds.size() ? F::Binds.vBinds[tBind.Parent].Name : "None";
							if (FButton(std::format("Parent: {}", sParent).c_str(), FButton_Right | FButton_SameLine | FButton_NoUpper, { 0, 40 }))
								bParent = 2;
						}
						FDropdown("Type", &tBind.Type, { "Key", "Class", "Weapon type", "Item slot" }, {}, FDropdown_Left);
						switch (tBind.Type)
						{
						case BindEnum::Key: tBind.Info = std::clamp(tBind.Info, 0, 2); FDropdown("Behavior", &tBind.Info, { "Hold", "Toggle", "Double click" }, {}, FDropdown_Right); break;
						case BindEnum::Class: tBind.Info = std::clamp(tBind.Info, 0, 8); FDropdown("Class", &tBind.Info, { "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdown_Right); break;
						case BindEnum::WeaponType: tBind.Info = std::clamp(tBind.Info, 0, 2); FDropdown("Weapon type", &tBind.Info, { "Hitscan", "Projectile", "Melee" }, {}, FDropdown_Right); break;
						case BindEnum::ItemSlot: tBind.Info = std::max(tBind.Info, 0); FDropdown("Item slot", &tBind.Info, { "1", "2", "3", "4", "5", "6", "7", "8", "9" }, {}, FDropdown_Right); break;
						}
					} EndChild();

					SetCursorPos({ GetWindowWidth() / 2 - GetStyle().WindowPadding.x / 2, vOriginalPos.y - H::Draw.Scale(8) });
					if (BeginChild("Split2", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(112) }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
					{
						int iNot = tBind.Not;
						FDropdown("While", &iNot, { "Active", "Not active" }, {}, FDropdown_Left);
						tBind.Not = iNot;
						int iVisible = tBind.Visible;
						FDropdown("Visibility", &iVisible, { "Visible", "Hidden" }, { 1, 0 }, FDropdown_Right);
						tBind.Visible = iVisible;
						if (tBind.Type == 0)
							FKeybind("Key", tBind.Key, FButton_None, { 0, 40 }, -96);

						// create/modify button
						bool bCreate = false, bClear = false, bMatch = false, bParent = true;
						if (tBind.Parent != DEFAULT_BIND)
							bParent = F::Binds.vBinds.size() > tBind.Parent;

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(96), H::Draw.Scale(56) });
						PushDisabled(!(bParent && (tBind.Type == 0 ? tBind.Key : true)));
							bCreate = FButton("##CreateButton", FButton_None, { 40, 40 });
						PopDisabled();
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(84), H::Draw.Scale(76) });
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
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(48), H::Draw.Scale(56) });
						bClear = FButton("##ClearButton", FButton_None, { 40, 40 });
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(36), H::Draw.Scale(76) });
						IconImage(ICON_MD_CLEAR);

						if (bCreate)
							F::Binds.AddBind(iBind, tBind);
						if (bCreate || bClear)
						{
							iBind = DEFAULT_BIND;
							tBind = {};
						}
					} EndChild();
				}

				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				SetCursorPos({ H::Draw.Scale(14), H::Draw.Scale(128) }); FText("Binds");
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

							std::string sType; std::string sInfo;
							switch (_tBind.Type)
							{
							case BindEnum::Key:
								switch (_tBind.Info)
								{
								case BindEnum::KeyEnum::Hold: { sType = "hold"; break; }
								case BindEnum::KeyEnum::Toggle: { sType = "toggle"; break; }
								case BindEnum::KeyEnum::DoubleClick: { sType = "double"; break; }
								}
								sInfo = VK2STR(_tBind.Key);
								break;
							case BindEnum::Class:
								sType = "class";
								switch (_tBind.Info)
								{
								case BindEnum::ClassEnum::Scout: { sInfo = "scout"; break; }
								case BindEnum::ClassEnum::Soldier: { sInfo = "soldier"; break; }
								case BindEnum::ClassEnum::Pyro: { sInfo = "pyro"; break; }
								case BindEnum::ClassEnum::Demoman: { sInfo = "demoman"; break; }
								case BindEnum::ClassEnum::Heavy: { sInfo = "heavy"; break; }
								case BindEnum::ClassEnum::Engineer: { sInfo = "engineer"; break; }
								case BindEnum::ClassEnum::Medic: { sInfo = "medic"; break; }
								case BindEnum::ClassEnum::Sniper: { sInfo = "sniper"; break; }
								case BindEnum::ClassEnum::Spy: { sInfo = "spy"; break; }
								}
								break;
							case BindEnum::WeaponType:
								sType = "weapon";
								switch (_tBind.Info)
								{
								case BindEnum::WeaponTypeEnum::Hitscan: { sInfo = "hitscan"; break; }
								case BindEnum::WeaponTypeEnum::Projectile: { sInfo = "projectile"; break; }
								case BindEnum::WeaponTypeEnum::Melee: { sInfo = "melee"; break; }
								}
								break;
							case BindEnum::ItemSlot:
								sType = "slot";
								sInfo = std::format("{}", _tBind.Info + 1);
								break;
							}
							if (_tBind.Not)
								sType = std::format("not {}", sType);

							bool bClicked = false, bDelete = false, bEdit = false, bVisibility = false, bNot = false;

							ImVec2 vOriginalPos = { H::Draw.Scale(8) + H::Draw.Scale(28) * x, H::Draw.Scale(108) + H::Draw.Scale(36) * y };

							// background
							float flWidth = GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(28) * x;
							float flHeight = H::Draw.Scale(28);
							ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
							GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, F::Render.Foremost, H::Draw.Scale(3));

							// text
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(10), vOriginalPos.y + H::Draw.Scale(7) });
							FText(_tBind.Name.c_str());

							SetCursorPos({ vOriginalPos.x + (flWidth - H::Draw.Scale(105)) * (1.f / 3), vOriginalPos.y + H::Draw.Scale(7) });
							FText(sType.c_str());

							SetCursorPos({ vOriginalPos.x + (flWidth - H::Draw.Scale(105)) * (2.f / 3), vOriginalPos.y + H::Draw.Scale(7) });
							FText(sInfo.c_str());

							// buttons
							int iOffset = -3;

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(5) });
							bDelete = IconButton(ICON_MD_DELETE);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(5) });
							bEdit = IconButton(ICON_MD_EDIT);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(5) });
							bVisibility = IconButton(_tBind.Visible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(5) });
							bNot = IconButton(!_tBind.Not ? ICON_MD_CODE : ICON_MD_CODE_OFF);

							SetCursorPos(vOriginalPos);
							bClicked = Button(std::format("##{}", _iBind).c_str(), { flWidth, flHeight });

							if (bClicked)
							{
								if (bParent)
								{
									bParent = false;
									tBind.Parent = _iBind;
								}
								else
								{
									iBind = _iBind;
									tBind = _tBind;
								}
							}
							if (bDelete)
								OpenPopup(std::format("Confirmation## DeleteCond{}", _iBind).c_str());
							if (BeginPopupModal(std::format("Confirmation## DeleteCond{}", _iBind).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								FText(std::format("Do you really want to delete '{}'{}?", _tBind.Name, F::Binds.HasChildren(_iBind) ? " and all of its children" : "").c_str());

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
							if (bVisibility)
								_tBind.Visible = !_tBind.Visible;
							if (bNot)
								_tBind.Not = !_tBind.Not;

							y = getBinds(_iBind, x + 1, y);
						}

						return y;
					};
				getBinds(DEFAULT_BIND, 0, 0);

				if (bParent == 2) // dumb
					bParent = 1;
				else if (bParent && IsMouseReleased(ImGuiMouseButton_Left))
				{
					bParent = false;
					tBind.Parent = DEFAULT_BIND;
				}
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
							bool bClicked = false, bAdd = false, bAlias = false, bPitch = false, bYaw = false, bMinwalk = false, bView = false;

							const Color_t teamColor = getTeamColor(player.m_iTeam, player.m_bAlive);
							const ImColor imColor = ColorToVec(teamColor);

							ImVec2 vOriginalPos = { !x ? GetStyle().WindowPadding.x : GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(32 + 36 * y) };

							// background
							float flWidth = GetWindowWidth() / 2 - GetStyle().WindowPadding.x * 1.5f;
							float flHeight = H::Draw.Scale(28);
							ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
							GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, imColor, H::Draw.Scale(3));

							// text + icons
							int lOffset = H::Draw.Scale(10);
							if (player.m_bLocal)
							{
								lOffset = H::Draw.Scale(29);
								SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(5) });
								IconImage(ICON_MD_PERSON);
							}
							else if (player.m_bFriend)
							{
								lOffset = H::Draw.Scale(29);
								SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(5) });
								IconImage(ICON_MD_GROUP);
							}
							else if (player.m_bParty)
							{
								lOffset = H::Draw.Scale(29);
								SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(5) });
								IconImage(ICON_MD_GROUPS);
							}
							else if (F::Spectate.m_iIntendedTarget == player.m_iUserID)
							{
								lOffset = H::Draw.Scale(29);
								SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(5) });
								IconImage(ICON_MD_VISIBILITY);
							}
							SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y + H::Draw.Scale(7) });
							FText(player.m_sName);
							lOffset += FCalcTextSize(player.m_sName).x + H::Draw.Scale(8);

							// buttons
							int iOffset = 2;

							if (!player.m_bFake)
							{
								// right
								SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(5) });
								bAlias = IconButton(ICON_MD_EDIT);

								SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(5) });
								bAdd = IconButton(ICON_MD_ADD);
							}

							bool bResolver = Vars::AntiHack::Resolver::Enabled.Value && !player.m_bLocal;
							if (bResolver)
							{
								SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + 5 });
								bView = IconButton(ICON_MD_NEAR_ME);

								SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + 5 });
								bMinwalk = IconButton(ICON_MD_DIRECTIONS_WALK);

								SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + 5 });
								bYaw = IconButton(ICON_MD_ARROW_FORWARD);

								SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + 5 });
								bPitch = IconButton(ICON_MD_ARROW_UPWARD);
							}

							if (!player.m_bFake)
							{
								// tag bar
								SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y });
								if (BeginChild(std::format("TagBar{}", player.m_uFriendsID).c_str(), { flWidth - lOffset - H::Draw.Scale(bResolver ? 128 : 48), flHeight }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
								{
									PushFont(F::Render.FontSmall);

									const auto vDrawPos = GetDrawPos();
									float flTagOffset = 0;
									for (auto& iID : F::PlayerUtils.m_mPlayerTags[player.m_uFriendsID])
									{
										auto pTag = F::PlayerUtils.GetTag(iID);
										if (!pTag)
											continue;

										ImColor tagColor = ColorToVec(pTag->Color);
										float flTagWidth = FCalcTextSize(pTag->Name.c_str()).x + H::Draw.Scale(25);
										float flTagHeight = H::Draw.Scale(20);
										ImVec2 vTagPos = { flTagOffset, H::Draw.Scale(4) };

										PushStyleColor(ImGuiCol_Text, IsColorBright(tagColor) ? ImVec4{ 0, 0, 0, 1 } : ImVec4{ 1, 1, 1, 1 });

										GetWindowDrawList()->AddRectFilled(vDrawPos + vTagPos, { vDrawPos.x + vTagPos.x + flTagWidth, vDrawPos.y + vTagPos.y + flTagHeight }, tagColor, H::Draw.Scale(3));
										SetCursorPos({ vTagPos.x + H::Draw.Scale(5), vTagPos.y + H::Draw.Scale(4) });
										FText(pTag->Name.c_str());
										SetCursorPos({ vTagPos.x + flTagWidth - H::Draw.Scale(18), vTagPos.y + H::Draw.Scale(2) });
										if (IconButton(ICON_MD_CANCEL))
											F::PlayerUtils.RemoveTag(player.m_uFriendsID, iID, true, player.m_sName);

										PopStyleColor();

										flTagOffset += flTagWidth + H::Draw.Scale(4);
									}
									PopFont();
								} EndChild();

								//bClicked = IsItemClicked();
								bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;

								SetCursorPos(vOriginalPos);
								/*bClicked = */Button(std::format("##{}", player.m_sName).c_str(), { flWidth, 28 }) || bClicked;
								bClicked = IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Right) || bClicked;
							}

							SetCursorPos(vOriginalPos);
							DebugDummy({ 0, H::Draw.Scale(28) });

							if (bClicked)
								OpenPopup(std::format("Clicked{}", player.m_uFriendsID).c_str());
							else if (bAdd)
								OpenPopup(std::format("Add{}", player.m_uFriendsID).c_str());
							else if (bAlias)
								OpenPopup(std::format("Alias{}", player.m_uFriendsID).c_str());
							else if (bYaw)
								OpenPopup(std::format("Yaw{}", player.m_uFriendsID).c_str());
							else if (bPitch)
								OpenPopup(std::format("Pitch{}", player.m_uFriendsID).c_str());
							else if (bMinwalk)
								OpenPopup(std::format("Minwalk{}", player.m_uFriendsID).c_str());
							else if (bView)
								OpenPopup(std::format("View{}", player.m_uFriendsID).c_str());

							// popups
							if (BeginPopup(std::format("Clicked{}", player.m_uFriendsID).c_str()))
							{
								PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

								if (FSelectable("Profile"))
									I::SteamFriends->ActivateGameOverlayToUser("steamid", CSteamID(player.m_uFriendsID, k_EUniversePublic, k_EAccountTypeIndividual));

								if (FSelectable("History"))
									I::SteamFriends->ActivateGameOverlayToWebPage(std::format("https://steamhistory.net/id/{}", CSteamID(player.m_uFriendsID, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64()).c_str());

								if (FSelectable(F::Spectate.m_iIntendedTarget == player.m_iUserID ? "Unspectate" : "Spectate"))
									F::Spectate.SetTarget(player.m_iUserID);

								if (!player.m_bLocal && FSelectable("Votekick"))
									I::ClientState->SendStringCmd(std::format("callvote kick {}", player.m_iUserID).c_str());

								PopStyleVar();
								EndPopup();
							}
							else if (BeginPopup(std::format("Add{}", player.m_uFriendsID).c_str()))
							{
								PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

								int iID = -1;
								for (auto& tTag : F::PlayerUtils.m_vTags)
								{
									iID++;

									if (!tTag.Assignable || F::PlayerUtils.HasTag(player.m_uFriendsID, iID))
										continue;

									auto imColor = ColorToVec(tTag.Color);
									PushStyleColor(ImGuiCol_Text, imColor);
									imColor.x /= 3; imColor.y /= 3; imColor.z /= 3;
									if (FSelectable(tTag.Name.c_str(), imColor))
										F::PlayerUtils.AddTag(player.m_uFriendsID, iID, true, player.m_sName);
									PopStyleColor();
								}

								PopStyleVar();
								EndPopup();
							}
							else if (BeginPopup(std::format("Alias{}", player.m_uFriendsID).c_str()))
							{
								PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

								FText("Alias");

								bool bHasAlias = F::PlayerUtils.m_mPlayerAliases.contains(player.m_uFriendsID);
								static std::string sInput = "";

								PushStyleVar(ImGuiStyleVar_FramePadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
								PushItemWidth(H::Draw.Scale(150));
								bool bEnter = InputText("##Alias", &sInput, ImGuiInputTextFlags_EnterReturnsTrue);
								if (!IsItemFocused())
									sInput = bHasAlias ? F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID] : "";
								PopItemWidth();
								PopStyleVar();

								if (bEnter)
								{
									if (sInput.empty() && F::PlayerUtils.m_mPlayerAliases.contains(player.m_uFriendsID))
									{
										F::Records.AliasChanged(player.m_sName, "Removed", F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID]);

										auto find = F::PlayerUtils.m_mPlayerAliases.find(player.m_uFriendsID);
										if (find != F::PlayerUtils.m_mPlayerAliases.end())
											F::PlayerUtils.m_mPlayerAliases.erase(find);
										F::PlayerUtils.m_bSavePlayers = true;
									}
									else
									{
										F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID] = sInput;
										F::PlayerUtils.m_bSavePlayers = true;

										F::Records.AliasChanged(player.m_sName, bHasAlias ? "Changed" : "Added", sInput);
									}
								}

								PopStyleVar();
								EndPopup();
							}
							else if (BeginPopup(std::format("Yaw{}", player.m_uFriendsID).c_str()))
							{
								PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

								static std::vector<std::pair<std::string, float>> vYaws = {
									{ "Auto", 0.f },
									{ "Forward", 0.f },
									{ "Left", 90.f },
									{ "Right", -90.f },
									{ "Backwards", 180.f }
								};
								for (auto& [sYaw, flValue] : vYaws)
								{
									if (FSelectable(sYaw.c_str()))
									{
										switch (FNV1A::Hash32(sYaw.c_str()))
										{
										case FNV1A::Hash32Const("Auto"):
											F::Resolver.SetYaw(player.m_iUserID, 0.f, true);
											break;
										default:
											F::Resolver.SetYaw(player.m_iUserID, flValue);
										}
									}
								}

								PopStyleVar();
								EndPopup();
							}
							else if (BeginPopup(std::format("Pitch{}", player.m_uFriendsID).c_str()))
							{
								PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

								static std::vector<std::pair<std::string, float>> vPitches = {
									{ "Auto", 0.f },
									{ "Up", -90.f },
									{ "Down", 90.f },
									{ "Zero", 0.f },
									{ "Inverse", 0.f }
								};
								for (auto& [sPitch, flValue] : vPitches)
								{
									if (FSelectable(sPitch.c_str()))
									{
										switch (FNV1A::Hash32(sPitch.c_str()))
										{
										case FNV1A::Hash32Const("Auto"):
											F::Resolver.SetPitch(player.m_iUserID, 0.f, false, true);
											break;
										case FNV1A::Hash32Const("Inverse"):
											F::Resolver.SetPitch(player.m_iUserID, 0.f, true);
											break;
										default:
											F::Resolver.SetPitch(player.m_iUserID, flValue);
										}
									}
								}

								PopStyleVar();
								EndPopup();
							}
							else if (BeginPopup(std::format("Minwalk{}", player.m_uFriendsID).c_str()))
							{
								PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

								static std::vector<std::pair<std::string, bool>> vPitches = {
									{ "Minwalk on", true },
									{ "Minwalk off", false }
								};
								for (auto& [sPitch, bValue] : vPitches)
								{
									if (FSelectable(sPitch.c_str()))
										F::Resolver.SetMinwalk(player.m_iUserID, bValue);
								}

								PopStyleVar();
								EndPopup();
							}
							else if (BeginPopup(std::format("View{}", player.m_uFriendsID).c_str()))
							{
								PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

								static std::vector<std::pair<std::string, bool>> vPitches = {
									{ "Offset from static view", true },
									{ "Offset from view to local", false }
								};
								for (auto& [sPitch, bValue] : vPitches)
								{
									if (FSelectable(sPitch.c_str()))
										F::Resolver.SetView(player.m_iUserID, bValue);
								}

								PopStyleVar();
								EndPopup();
							}
						};

					// display players
					std::vector<ListPlayer> vBlu, vRed, vOther;
					for (auto& player : playerCache)
					{
						switch (player.m_iTeam)
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
					SetCursorPos({ H::Draw.Scale(18), H::Draw.Scale(39) });
					FText("Not ingame");
					DebugDummy({ 0, H::Draw.Scale(8) });
				}
			} EndSection();
			if (Section("Tags"))
			{
				static int iID = -1;
				static PriorityLabel_t tTag = {};

				{
					ImVec2 vOriginalPos = GetCursorPos();

					SetCursorPos({ 0, vOriginalPos.y - H::Draw.Scale(8) });
					if (BeginChild("Split1", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(64) }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
					{
						FSDropdown("Name", &tTag.Name, {}, FSDropdown_AutoUpdate | FDropdown_Left, -10);
						FColorPicker("Color", &tTag.Color, 0, FColorPicker_Dropdown);

						PushDisabled(iID == DEFAULT_TAG || iID == IGNORED_TAG);
						int iLabel = Disabled ? 0 : tTag.Label;
						FDropdown("Type", &iLabel, { "Priority", "Label" }, {}, FDropdown_Right);
						tTag.Label = iLabel;
						if (Disabled)
							tTag.Label = false;
						PopDisabled();
					} EndChild();

					SetCursorPos({ GetWindowWidth() / 2 - GetStyle().WindowPadding.x / 2, vOriginalPos.y - H::Draw.Scale(8) });
					if (BeginChild("Split2", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(64) }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
					{
						PushTransparent(tTag.Label); // transparent if we want a label, user can still use to sort
						SetCursorPosY(GetCursorPos().y + H::Draw.Scale(12));
						FSlider("Priority", &tTag.Priority, -10, 10, 1, "%i", FSlider_Left);
						PopTransparent();

						// create/modify button
						bool bCreate = false, bClear = false;

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(96), H::Draw.Scale(8) });
						PushDisabled(tTag.Name.empty());
						bCreate = FButton("##CreateButton", FButton_None, { 40, 40 });
						PopDisabled();
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(84), H::Draw.Scale(28) });
						PushTransparent(tTag.Name.empty());
						IconImage(iID != -1 ? ICON_MD_SETTINGS : ICON_MD_ADD);
						PopTransparent();

						// clear button
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(48), H::Draw.Scale(8) });
						bClear = FButton("##ClearButton", FButton_None, { 40, 40 });
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(36), H::Draw.Scale(28) });
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
				}

				auto drawTag = [](std::vector<PriorityLabel_t>::iterator it, PriorityLabel_t& _tTag, int y)
					{
						int _iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);

						bool bClicked = false, bDelete = false;

						ImColor imColor = ColorToVec(_tTag.Color);
						imColor.Value.x /= 3; imColor.Value.y /= 3; imColor.Value.z /= 3;

						ImVec2 vOriginalPos = { !_tTag.Label ? GetStyle().WindowPadding.x : GetWindowWidth() * 2 / 3 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(96 + 36 * y) };

						// background
						float flWidth = GetWindowWidth() * (_tTag.Label ? 1.f / 3 : 2.f / 3) - GetStyle().WindowPadding.x * 1.5f;
						float flHeight = H::Draw.Scale(28);
						ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
						GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, imColor, H::Draw.Scale(3));

						// text
						SetCursorPos({ vOriginalPos.x + H::Draw.Scale(10), vOriginalPos.y + H::Draw.Scale(7) });
						FText(_tTag.Name.c_str());

						if (!_tTag.Label)
						{
							SetCursorPos({ vOriginalPos.x + flWidth / 2, vOriginalPos.y + H::Draw.Scale(7) });
							FText(std::format("{}", _tTag.Priority).c_str());
						}

						// buttons / icons
						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(22), vOriginalPos.y + H::Draw.Scale(5) });
						if (!_tTag.Locked)
							bDelete = IconButton(ICON_MD_DELETE);
						else
						{
							switch (F::PlayerUtils.IndexToTag(_iID))
							{
							//case DEFAULT_TAG: // no image
							case IGNORED_TAG: IconImage(ICON_MD_DO_NOT_DISTURB); break;
							case CHEATER_TAG: IconImage(ICON_MD_FLAG); break;
							case FRIEND_TAG: IconImage(ICON_MD_GROUP); break;
							case PARTY_TAG: IconImage(ICON_MD_GROUPS); break;
							}
						}

						SetCursorPos(vOriginalPos);
						bClicked = Button(std::format("##{}", _tTag.Name).c_str(), { flWidth, flHeight });

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
							FText(std::format("Do you really want to delete '{}'?", _tTag.Name).c_str());

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
				SetCursorPos({ H::Draw.Scale(14), H::Draw.Scale(80) }); FText("Priorities");
				SetCursorPos({ GetWindowWidth() * 2 / 3 + H::Draw.Scale(10), H::Draw.Scale(80) }); FText("Labels");
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
				SetCursorPos({ 0, H::Draw.Scale(60 + 36 * std::max(iPriorities, iLabels)) }); DebugDummy({ 0, H::Draw.Scale(28) });
			} EndSection();
			break;
		// Materials
		case 3:
			if (BeginTable("MaterialsTable", 2))
			{
				static TextEditor TextEditor;
				static std::string CurrentMaterial;
				static bool LockedMaterial;

				/* Column 1 */
				TableNextColumn();
				if (Section("Manager"))
				{
					static std::string newName;
					FSDropdown("Material name", &newName, {}, FSDropdown_AutoUpdate | FDropdown_Left);
					if (FButton("Create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
					{
						F::Materials.AddMaterial(newName.c_str());
						newName.clear();
					}

					if (FButton("Folder", FButton_Fit | FButton_SameLine, { 0, 40 }))
						ShellExecuteA(nullptr, "open", MaterialFolder.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);

					std::vector<Material_t> vMaterials;
					for (auto& [_, mat] : F::Materials.m_mMaterials)
						vMaterials.push_back(mat);

					std::sort(vMaterials.begin(), vMaterials.end(), [&](const auto& a, const auto& b) -> bool
						{
							// override for none material
							if (FNV1A::Hash32(a.m_sName.c_str()) == FNV1A::Hash32Const("None"))
								return true;
							if (FNV1A::Hash32(b.m_sName.c_str()) == FNV1A::Hash32Const("None"))
								return false;

							// keep locked materials higher
							if (a.m_bLocked && !b.m_bLocked)
								return true;
							if (!a.m_bLocked && b.m_bLocked)
								return false;

							return a.m_sName < b.m_sName;
						});

					for (auto& pair : vMaterials)
					{
						ImVec2 vOriginalPos = GetCursorPos();

						SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
						TextColored(pair.m_bLocked ? F::Render.Inactive.Value : F::Render.Active.Value, pair.m_sName.c_str());

						int iOffset = 0;
						if (!pair.m_bLocked)
						{
							SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
							if (IconButton(ICON_MD_DELETE))
								OpenPopup(std::format("Confirmation## DeleteMat{}", pair.m_sName).c_str());
							if (BeginPopupModal(std::format("Confirmation## DeleteMat{}", pair.m_sName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								FText(std::format("Do you really want to delete '{}'?", pair.m_sName).c_str());

								if (FButton("Yes", FButton_Left))
								{
									F::Materials.RemoveMaterial(pair.m_sName.c_str());
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}
						}

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_EDIT))
						{
							CurrentMaterial = pair.m_sName;
							LockedMaterial = pair.m_bLocked;

							TextEditor.SetText(F::Materials.GetVMT(FNV1A::Hash32(CurrentMaterial.c_str())));
							TextEditor.SetReadOnly(LockedMaterial);
						}

						SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
					}
				} EndSection();

				/* Column 2 */
				TableNextColumn();
				if (CurrentMaterial.length())
				{
					auto count = std::ranges::count(TextEditor.GetText(), '\n'); // doesn't account for text editor size otherwise
					if (Section("Editor", H::Draw.Scale(83) + 6 + FCalcTextSize("A", F::Render.FontMono).y * count, true))
					{
						// Toolbar
						if (!LockedMaterial)
						{
							if (FButton("Save", FButton_Fit))
							{
								auto sText = TextEditor.GetText();
								sText.erase(sText.end() - 1, sText.end()); // get rid of random newline
								F::Materials.EditMaterial(CurrentMaterial.c_str(), sText.c_str());
							}
							SameLine();
						}
						if (FButton("Close", FButton_Fit))
							CurrentMaterial = "";
						SameLine(); SetCursorPosY(GetCursorPosY() + H::Draw.Scale(27));
						PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
						FText(std::format("{}: {}", LockedMaterial ? "Viewing" : "Editing", CurrentMaterial).c_str(), FText_Right);
						PopStyleColor();

						// Text editor
						DebugDummy({ 0, H::Draw.Scale(8) });

						PushFont(F::Render.FontMono);
						TextEditor.Render("TextEditor");
						PopFont();
					} EndSection();
				}

				EndTable();
			}
			break;
		}

		PopStyleColor();
	} EndChild();
	PopStyleVar();
}
#pragma endregion

void CMenu::AddDraggable(const char* sTitle, ConfigVar<DragBox_t>& var, bool bShouldDraw)
{
	using namespace ImGui;

	if (!bShouldDraw)
		return;

	static std::unordered_map<const char*, std::pair<DragBox_t, float>> old = {};
	DragBox_t info = FGet(var);
	const float sizeX = H::Draw.Scale(100), sizeY = H::Draw.Scale(40);
	SetNextWindowSize({ sizeX, sizeY }, ImGuiCond_Always);
	if (!old.contains(sTitle) || info != old[sTitle].first || sizeX != old[sTitle].second)
		SetNextWindowPos({ float(info.x - sizeX / 2), float(info.y) }, ImGuiCond_Always);

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, H::Draw.Scale(3));
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, H::Draw.Scale(1));
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { sizeX, sizeY });
	if (Begin(sTitle, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImVec2 vWindowPos = GetWindowPos();

		info.x = vWindowPos.x + sizeX / 2; info.y = vWindowPos.y; old[sTitle] = { info, sizeX };
		FSet(var, info);

		PushFont(F::Render.FontBold);
		auto size = FCalcTextSize(sTitle);
		SetCursorPos({ (sizeX - size.x) * 0.5f, (sizeY - size.y) * 0.5f });
		FText(sTitle);
		PopFont();

		End();
	}
	PopStyleVar(3);
	PopStyleColor(2);
}

void CMenu::DrawBinds()
{
	using namespace ImGui;

	if (IsOpen ? false : !Vars::Menu::ShowBinds.Value || I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible() && !I::EngineClient->IsPlayingDemo())
		return;

	static DragBox_t old = { -2147483648, -2147483648 };
	DragBox_t info = IsOpen ? FGet(Vars::Menu::BindsDisplay) : Vars::Menu::BindsDisplay.Value;
	if (info != old)
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);

	std::vector<std::tuple<bool, const char*, std::string, std::string, int, Bind_t&>> vBinds;
	std::function<void(int)> getBinds = [&](int iParent)
		{
			for (auto it = F::Binds.vBinds.begin(); it != F::Binds.vBinds.end(); it++)
			{
				int iBind = std::distance(F::Binds.vBinds.begin(), it);
				auto& tBind = *it;
				if (iParent != tBind.Parent)
					continue;

				if (tBind.Visible || IsOpen)
				{
					std::string sType; std::string sInfo;
					switch (tBind.Type)
					{
					case BindEnum::Key:
						switch (tBind.Info)
						{
						case BindEnum::KeyEnum::Hold: { sType = "hold"; break; }
						case BindEnum::KeyEnum::Toggle: { sType = "toggle"; break; }
						case BindEnum::KeyEnum::DoubleClick: { sType = "double"; break; }
						}
						sInfo = VK2STR(tBind.Key);
						break;
					case BindEnum::Class:
						sType = "class";
						switch (tBind.Info)
						{
						case BindEnum::ClassEnum::Scout: { sInfo = "scout"; break; }
						case BindEnum::ClassEnum::Soldier: { sInfo = "soldier"; break; }
						case BindEnum::ClassEnum::Pyro: { sInfo = "pyro"; break; }
						case BindEnum::ClassEnum::Demoman: { sInfo = "demoman"; break; }
						case BindEnum::ClassEnum::Heavy: { sInfo = "heavy"; break; }
						case BindEnum::ClassEnum::Engineer: { sInfo = "engineer"; break; }
						case BindEnum::ClassEnum::Medic: { sInfo = "medic"; break; }
						case BindEnum::ClassEnum::Sniper: { sInfo = "sniper"; break; }
						case BindEnum::ClassEnum::Spy: { sInfo = "spy"; break; }
						}
						break;
					case BindEnum::WeaponType:
						sType = "weapon";
						switch (tBind.Info)
						{
						case BindEnum::WeaponTypeEnum::Hitscan: { sInfo = "hitscan"; break; }
						case BindEnum::WeaponTypeEnum::Projectile: { sInfo = "projectile"; break; }
						case BindEnum::WeaponTypeEnum::Melee: { sInfo = "melee"; break; }
						}
						break;
					case BindEnum::ItemSlot:
						sType = "slot";
						sInfo = std::format("{}", tBind.Info + 1);
						break;
					}
					if (tBind.Not)
						sInfo = std::format("not {}", sInfo);

					vBinds.push_back({ tBind.Active, tBind.Name.c_str(), sType, sInfo, iBind, tBind });
				}

				if (tBind.Active || IsOpen)
					getBinds(iBind);
			}
		};
	getBinds(DEFAULT_BIND);

	float flNameWidth = 0, flInfoWidth = 0, flStateWidth = 0;
	if (vBinds.empty())
	{
		if (!IsOpen)
			return;
		else
			flNameWidth = FCalcTextSize("Binds", F::Render.FontLarge).x + H::Draw.Scale(10);
	}
	else
	{
		PushFont(F::Render.FontSmall);
		for (auto& [_, sName, sInfo, sState, iBind, tBind] : vBinds)
		{
			flNameWidth = std::max(flNameWidth, FCalcTextSize(sName).x);
			flInfoWidth = std::max(flInfoWidth, FCalcTextSize(sInfo.c_str()).x);
			flStateWidth = std::max(flStateWidth, FCalcTextSize(sState.c_str()).x);
		}
		PopFont();
		flNameWidth += H::Draw.Scale(9), flInfoWidth += H::Draw.Scale(9), flStateWidth += H::Draw.Scale(9);
	}

	float flWidth = flNameWidth + flInfoWidth + flStateWidth + (IsOpen ? H::Draw.Scale(88) : H::Draw.Scale(14));
	float flHeight = H::Draw.Scale(18 * vBinds.size() + 40);
	SetNextWindowSize({ flWidth, flHeight });
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(40), H::Draw.Scale(40) });
	PushStyleColor(ImGuiCol_WindowBg, F::Render.Background.Value);
	if (Begin("Binds", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImVec2 vWindowPos = GetWindowPos();

		info.x = vWindowPos.x; info.y = vWindowPos.y; old = info;
		if (IsOpen)
			FSet(Vars::Menu::BindsDisplay, info);

		PushFont(F::Render.FontLarge);
		SetCursorPos({ H::Draw.Scale(11), H::Draw.Scale(9) });
		FText("Binds");
		PopFont();

		GetWindowDrawList()->AddRectFilled({ vWindowPos.x + H::Draw.Scale(8), vWindowPos.y + H::Draw.Scale(26) }, { vWindowPos.x + flWidth - H::Draw.Scale(8), vWindowPos.y + H::Draw.Scale(27) }, F::Render.Accent, H::Draw.Scale(3));

		PushFont(F::Render.FontSmall);
		int i = 0; for (auto& [bActive, sName, sInfo, sState, iBind, tBind] : vBinds)
		{
			float flPosX = 0;

			SetCursorPos({ flPosX += H::Draw.Scale(12), H::Draw.Scale(34 + 18 * i) });
			PushStyleColor(ImGuiCol_Text, bActive ? F::Render.Accent.Value : F::Render.Inactive.Value);
			FText(sName);
			PopStyleColor();

			SetCursorPos({ flPosX += flNameWidth, H::Draw.Scale(34 + 18 * i) });
			PushStyleColor(ImGuiCol_Text, bActive ? F::Render.Active.Value : F::Render.Inactive.Value);
			FText(sInfo.c_str());

			SetCursorPos({ flPosX += flInfoWidth, H::Draw.Scale(34 + 18 * i) });
			FText(sState.c_str());
			PopStyleColor();

			if (IsOpen)
			{	// buttons
				SetCursorPos({ flWidth - H::Draw.Scale(25), H::Draw.Scale(33 + 18 * i) });
				bool bDelete = IconButton(ICON_MD_DELETE);

				SetCursorPos({ flWidth - H::Draw.Scale(50), H::Draw.Scale(33 + 18 * i) });
				bool bNot = IconButton(!tBind.Not ? ICON_MD_CODE : ICON_MD_CODE_OFF);

				SetCursorPos({ flWidth - H::Draw.Scale(75), H::Draw.Scale(33 + 18 * i) });
				bool bVisibility = IconButton(tBind.Visible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF);

				PushFont(F::Render.FontRegular);
				PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });

				if (bDelete)
					OpenPopup(std::format("Confirmation## DeleteCond{}", iBind).c_str());
				if (BeginPopupModal(std::format("Confirmation## DeleteCond{}", iBind).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
				{
					FText(std::format("Do you really want to delete '{}'{}?", tBind.Name, F::Binds.HasChildren(iBind) ? " and all of its children" : "").c_str());

					SetCursorPosY(GetCursorPosY() - 8); // stupid and i don't know why this is needed here
					if (FButton("Yes", FButton_Left))
					{
						F::Binds.RemoveBind(iBind);
						CloseCurrentPopup();
					}
					if (FButton("No", FButton_Right | FButton_SameLine))
						CloseCurrentPopup();

					EndPopup();
				}
				if (bNot)
					tBind.Not = !tBind.Not;
				if (bVisibility)
					tBind.Visible = !tBind.Visible;

				PopStyleVar();
				PopFont();
			}

			i++;
		}
		PopFont();

		End();
	}
	PopStyleColor();
	PopStyleVar();
}

/* Window for the camera feature */
void CMenu::DrawCameraWindow()
{
	using namespace ImGui;

	if (!FGet(Vars::Visuals::Simulation::ProjectileCamera))
		return;

	static WindowBox_t old = { -2147483648, -2147483648 };
	WindowBox_t info = FGet(Vars::Visuals::Simulation::ProjectileWindow);
	if (info != old)
	{
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);
		SetNextWindowSize({ float(info.w), float(info.h) }, ImGuiCond_Always);
	}

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, H::Draw.Scale(3));
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, H::Draw.Scale(1));
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(100), H::Draw.Scale(100) });
	if (Begin("Camera", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImVec2 vWindowPos = GetWindowPos();
		ImVec2 vWinSize = GetWindowSize();

		info.x = vWindowPos.x; info.y = vWindowPos.y; info.w = vWinSize.x; info.h = vWinSize.y; old = info;
		FSet(Vars::Visuals::Simulation::ProjectileWindow, info);

		PushFont(F::Render.FontBold);
		auto size = FCalcTextSize("Camera");
		SetCursorPos({ (vWinSize.x - size.x) * 0.5f, (vWinSize.y - size.y) * 0.5f });
		FText("Camera");
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

	static WindowBox_t old = { -2147483648, -2147483648 };
	WindowBox_t info = FGet(Vars::Radar::Main::Window);
	if (info != old)
	{
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);
		SetNextWindowSize({ float(info.w), float(info.w) }, ImGuiCond_Always);
	}

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, H::Draw.Scale(3));
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, H::Draw.Scale(1));
	SetNextWindowSizeConstraints({ H::Draw.Scale(100), H::Draw.Scale(100) }, { H::Draw.Scale(1000), H::Draw.Scale(1000) }, SquareConstraints);
	if (Begin("Radar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		const ImVec2 vWindowPos = GetWindowPos();
		const ImVec2 winSize = GetWindowSize();

		info.x = vWindowPos.x; info.y = vWindowPos.y; info.w = winSize.x; old = info;
		FSet(Vars::Radar::Main::Window, info);

		PushFont(F::Render.FontBold);
		auto size = FCalcTextSize("Radar");
		SetCursorPos({ (winSize.x - size.x) * 0.5f, (winSize.y - size.y) * 0.5f });
		FText("Radar");
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
		DrawMenu();

		DrawCameraWindow();
		DrawRadar();
		AddDraggable("Ticks", Vars::Menu::TicksDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Ticks);
		AddDraggable("Crit hack", Vars::Menu::CritsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::CritHack);
		AddDraggable("Spectators", Vars::Menu::SpectatorsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Spectators);
		AddDraggable("Ping", Vars::Menu::PingDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Ping);
		AddDraggable("Conditions", Vars::Menu::ConditionsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Conditions);
		AddDraggable("Seed prediction", Vars::Menu::SeedPredictionDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::SeedPrediction);

		F::Render.Cursor = GetMouseCursor();
		//vDisabled.clear();
		//vTransparent.clear();
	}
	else
		mActiveMap.clear();

	PopFont();
}