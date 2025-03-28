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
#include "../../Output/Output.h"
#include "../../Spectate/Spectate.h"

void CMenu::DrawMenu()
{
	using namespace ImGui;

	SetNextWindowSize({ 750, 500 }, ImGuiCond_FirstUseEver);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(750), H::Draw.Scale(500) });
	PushStyleVar(ImGuiStyleVar_ChildRounding, H::Draw.Scale(3));
	if (Begin("Main", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground))
	{
		ImVec2 vWindowPos = GetWindowPos();
		ImVec2 vWindowSize = GetWindowSize();
		float flSideSize = 140.f;

		PushClipRect({ 0, 0 }, { ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y }, false);
		RenderTwoToneBackground(H::Draw.Scale(flSideSize), F::Render.Background0, F::Render.Background1, F::Render.Background2, 0.f, false);
		PopClipRect();

		ImVec2 vDrawPos = GetDrawPos();
		auto pDrawList = GetWindowDrawList();
		
		float flOffset = 0.f;
		Bind_t tBind;
		if (!F::Binds.GetBind(CurrentBind, &tBind))
			CurrentBind = DEFAULT_BIND;

		if (CurrentBind != DEFAULT_BIND) // bind
		{
			flOffset = H::Draw.Scale(60);
			pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y + H::Draw.Scale(59) }, { vDrawPos.x + H::Draw.Scale(flSideSize - 1), vDrawPos.y + H::Draw.Scale(60) }, F::Render.Background2);

			SetCursorPos({ H::Draw.Scale(12), H::Draw.Scale(11) });
			FText("Editing bind", 0, F::Render.FontRegular);
			SetCursorPos({ H::Draw.Scale(12), H::Draw.Scale(35) });
			PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
			FText(TruncateText(tBind.m_sName, H::Draw.Scale(flSideSize - 28)).c_str(), 0, F::Render.FontRegular);
			PopStyleColor();

			SetCursorPos({ H::Draw.Scale(flSideSize - 27), H::Draw.Scale(10) });
			if (IconButton(ICON_MD_CANCEL))
				CurrentBind = DEFAULT_BIND;
		}
		else if (!Vars::Menu::CheatName.Value.empty()) // title
		{
			flOffset = H::Draw.Scale(36);
			pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y + H::Draw.Scale(35) }, { vDrawPos.x + H::Draw.Scale(flSideSize - 1), vDrawPos.y + H::Draw.Scale(36) }, F::Render.Background2);
			
			SetCursorPos({ H::Draw.Scale(12), H::Draw.Scale(11) });
			PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
			FText(TruncateText(Vars::Menu::CheatName.Value, H::Draw.Scale(flSideSize - 28), F::Render.FontBold).c_str(), 0, F::Render.FontBold);
			PopStyleColor();
		}

		static int iTab = 0, iAimbotTab = 0, iVisualsTab = 0, iMiscTab = 0, iLogsTab = 0, iSettingsTab = 0;
		PushFont(F::Render.FontBold);
		FTabs(
			{
				{ "AIMBOT", "GENERAL", "HVH" },
				{ "VISUALS", "ESP", "CHAMS", "GLOW", "MISC##", "RADAR", "MENU" },
				{ "MISC" },
				{ "LOGS", "SETTINGS##", "OUTPUT" },
				{ "SETTINGS", "CONFIG", "BINDS", "PLAYERLIST", "MATERIALS" }
			},
			{ &iTab, &iAimbotTab, &iVisualsTab, nullptr, &iLogsTab, &iSettingsTab },
			{ H::Draw.Scale(flSideSize - 16), H::Draw.Scale(36) },
			{ H::Draw.Scale(8), H::Draw.Scale(8) + flOffset },
			FTabs_Vertical | FTabs_HorizontalIcons | FTabs_AlignLeft | FTabs_BarLeft,
			{ { ICON_MD_PERSON }, { ICON_MD_VISIBILITY }, { ICON_MD_MORE_HORIZ }, { ICON_MD_IMPORT_CONTACTS }, { ICON_MD_SETTINGS } },
			{ H::Draw.Scale(10), 0 }, {},
			{}, { H::Draw.Scale(22), 0 }
		);
		PopFont();

		SetCursorPos({ H::Draw.Scale(flSideSize), 0 });
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
		if (BeginChild("Page", { vWindowSize.x - H::Draw.Scale(flSideSize), vWindowSize.y }, ImGuiChildFlags_AlwaysUseWindowPadding))
		{
			switch (iTab)
			{
			case 0: MenuAimbot(iAimbotTab); break;
			case 1: MenuVisuals(iVisualsTab); break;
			case 2: MenuMisc(iMiscTab); break;
			case 3: MenuLogs(iLogsTab); break;
			case 4: MenuSettings(iSettingsTab); break;
			}
		} EndChild();
		PopStyleVar(2);

		End();
	}
	PopStyleVar(2);
}

#pragma region Tabs
void CMenu::MenuAimbot(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	// General
	case 0:
		if (BeginTable("AimbotTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("General"))
			{
				FDropdown("Aim type", Vars::Aimbot::General::AimType, { "Off", "Plain", "Smooth", "Silent", "Locking", "Assistive" }, {}, FDropdown_Left);
				FDropdown("Target selection", Vars::Aimbot::General::TargetSelection, { "FOV", "Distance" }, {}, FDropdown_Right);
				FDropdown("Target", Vars::Aimbot::General::Target, { "Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs" }, {}, FDropdown_Left | FDropdown_Multi);
				FDropdown("Ignore", Vars::Aimbot::General::Ignore, { "Friends", "Party", "Invulnerable", "Cloaked", "Unsimulated players", "Dead Ringer", "Vaccinator", "Disguised", "Taunting" }, {}, FDropdown_Right | FDropdown_Multi);
				FSlider("Aim FOV", Vars::Aimbot::General::AimFOV, 0.f, 180.f, 1.f, "%g", FSlider_Clamp | FSlider_Precision);
				FSlider("Max targets", Vars::Aimbot::General::MaxTargets, 1, 6, 1, "%i", FSlider_Left | FSlider_Min);
				PushTransparent(FGet(Vars::Aimbot::General::AimType) != Vars::Aimbot::General::AimTypeEnum::Smooth && FGet(Vars::Aimbot::General::AimType) != Vars::Aimbot::General::AimTypeEnum::Assistive);
				{
					FSlider("Assist strength", Vars::Aimbot::General::AssistStrength, 0.f, 100.f, 1.f, "%g%%", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Cloaked));
				{
					FSlider("Ignore cloak", Vars::Aimbot::General::IgnoreCloakPercentage, 0, 100, 10, "%d%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Unsimulated));
				{
					FSlider("Tick tolerance", Vars::Aimbot::General::TickTolerance, 0, 21, 1, "%i", FSlider_Right | FSlider_Clamp);
				}
				PopTransparent();
				FColorPicker("FOV circle", Vars::Colors::FOVCircle, 0, FColorPicker_None, nullptr, "FOV circle color");
				FToggle("Autoshoot", Vars::Aimbot::General::AutoShoot, FToggle_Left);
				FToggle("FOV Circle", Vars::Aimbot::General::FOVCircle, FToggle_Right);
				FToggle("Force crits", Vars::CritHack::ForceCrits, FToggle_Left);
				FToggle("Avoid random crits", Vars::CritHack::AvoidRandom, FToggle_Right);
				FToggle("Always melee crit", Vars::CritHack::AlwaysMeleeCrit, FToggle_Left);
				FToggle("No spread", Vars::Aimbot::General::NoSpread, FToggle_Right);
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Aimbot", true))
				{
					FSlider("Hitscan peek", Vars::Aimbot::General::HitscanPeek, 0, 5);
					FToggle("Peek DT only", Vars::Aimbot::General::PeekDTOnly);
					FTooltip("This should probably stay on if you want to be able to\ntarget hitboxes other than the highest priority one");
					FSlider("No spread offset", Vars::Aimbot::General::NoSpreadOffset, -1.f, 1.f, 0.1f, "%g", FSlider_Precision);
					FSlider("No spread average", Vars::Aimbot::General::NoSpreadAverage, 1, 25, 1, "%d", FSlider_Min);
					FSlider("No spread interval", Vars::Aimbot::General::NoSpreadInterval, 0.05f, 5.f, 0.1f, "%gs", FSlider_Min);
					FSlider("No spread backup", Vars::Aimbot::General::NoSpreadBackupInterval, 2.f, 10.f, 0.1f, "%gs", FSlider_Min);
					FDropdown("Aim holds fire", Vars::Aimbot::General::AimHoldsFire, { "False", "Minigun only", "Always" });
				} EndSection();
			}
			if (Section("Backtrack", true))
			{
				FToggle("Enabled", Vars::Backtrack::Enabled, FToggle_Left, nullptr, "Backtrack enabled");
				FToggle("Prefer on shot", Vars::Backtrack::PreferOnShot, FToggle_Right);
				FSlider("Fake latency", Vars::Backtrack::Latency, 0, F::Backtrack.m_flMaxUnlag * 1000, 5, "%i", FSlider_Clamp);
				FSlider("Fake interp", Vars::Backtrack::Interp, 0, F::Backtrack.m_flMaxUnlag * 1000, 5, "%i", FSlider_Clamp | FSlider_Precision);
				FSlider("Window", Vars::Backtrack::Window, 1, 200, 5, "%i", FSlider_Clamp, nullptr, "Backtrack window");
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Backtrack", true))
				{
					FSlider("Offset", Vars::Backtrack::Offset, -1, 1);
				} EndSection();
			}
			if (Section("Healing", true))
			{
				FToggle("Auto heal", Vars::Aimbot::Healing::AutoHeal, FToggle_Left);
				FToggle("Friends only", Vars::Aimbot::Healing::FriendsOnly, FToggle_Right, nullptr, "Heal friends only");
				FToggle("Activate on voice", Vars::Aimbot::Healing::ActivateOnVoice);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Hitscan"))
			{
				FDropdown("Hitboxes## Hitscan", Vars::Aimbot::Hitscan::Hitboxes, { "Head", "Body", "Pelvis", "Arms", "Legs", "##Divider", "Bodyaim if lethal" }, {}, FDropdown_Left | FDropdown_Multi, 0, nullptr, "Hitscan hitboxes");
				FDropdown("Modifiers## Hitscan", Vars::Aimbot::Hitscan::Modifiers, { "Tapfire", "Wait for headshot", "Wait for charge", "Scoped only", "Auto scope", "Auto rev minigun", "Extinguish team" }, {}, FDropdown_Right | FDropdown_Multi, 0, nullptr, "Hitscan modifiers");
				FSlider("Point scale", Vars::Aimbot::Hitscan::PointScale, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				PushTransparent(!(FGet(Vars::Aimbot::Hitscan::Modifiers) & Vars::Aimbot::Hitscan::ModifiersEnum::Tapfire));
				{
					FSlider("Tapfire distance", Vars::Aimbot::Hitscan::TapFireDist, 250.f, 1000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
				}
				PopTransparent();
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Hitscan", true))
				{
					FSlider("Bone size subtract", Vars::Aimbot::Hitscan::BoneSizeSubtract, 0.f, 4.f, 0.25f, "%g", FSlider_Min);
					FSlider("Bone size minimum scale", Vars::Aimbot::Hitscan::BoneSizeMinimumScale, 0.f, 1.f, 0.1f, "%g", FSlider_Clamp);
				} EndSection();
			}
			if (Section("Projectile"))
			{
				FDropdown("Predict", Vars::Aimbot::Projectile::StrafePrediction, { "Air strafing", "Ground strafing" }, {}, FDropdown_Left | FDropdown_Multi);
				FDropdown("Splash", Vars::Aimbot::Projectile::SplashPrediction, { "Off", "Include", "Prefer", "Only" }, {}, FDropdown_Right);
				FDropdown("Auto detonate", Vars::Aimbot::Projectile::AutoDetonate, { "Stickies", "Flares", "##Divider", "Prevent self damage", "Ignore cloak" }, {}, FDropdown_Left | FDropdown_Multi);
				FDropdown("Auto airblast", Vars::Aimbot::Projectile::AutoAirblast, { "Enabled", "##Divider", "Redirect simple", "Redirect advanced", "##Divider", "Respect FOV"}, {}, FDropdown_Right | FDropdown_Multi); // todo: finish redirect advanced!!
				FDropdown("Hitboxes## Projectile", Vars::Aimbot::Projectile::Hitboxes, { "Auto", "##Divider", "Head", "Body", "Feet", "##Divider", "Bodyaim if lethal", "Aim blast at feet" }, {}, FDropdown_Left | FDropdown_Multi, 0, nullptr, "Projectile hitboxes");
				FDropdown("Modifiers## Projectile", Vars::Aimbot::Projectile::Modifiers, { "Charge shot", "Cancel charge", "Use prime time" }, {}, FDropdown_Right | FDropdown_Multi, 0, nullptr, "Projectile modifiers");
				FSlider("Max simulation time", Vars::Aimbot::Projectile::PredictionTime, 0.1f, 2.5f, 0.25f, "%gs", FSlider_Left | FSlider_Min | FSlider_Precision);
				PushTransparent(!FGet(Vars::Aimbot::Projectile::StrafePrediction));
				{
					FSlider("Hit chance", Vars::Aimbot::Projectile::Hitchance, 0.f, 100.f, 10.f, "%g%%", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				FSlider("Autodet radius", Vars::Aimbot::Projectile::AutodetRadius, 0.f, 100.f, 10.f, "%g%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				FSlider("Splash radius", Vars::Aimbot::Projectile::SplashRadius, 0.f, 100.f, 10.f, "%g%%", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				PushTransparent(!FGet(Vars::Aimbot::Projectile::AutoRelease));
				{
					FSlider("Auto release", Vars::Aimbot::Projectile::AutoRelease, 0.f, 100.f, 5.f, "%g%%", FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Projectile", true))
				{
					FText("Ground");
					FSlider("Samples## Ground", Vars::Aimbot::Projectile::GroundSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("Straight fuzzy value## Ground", Vars::Aimbot::Projectile::GroundStraightFuzzyValue, 0.f, 500.f, 25.f, "%g", FSlider_Right | FSlider_Precision);
					FSlider("Low min samples## Ground", Vars::Aimbot::Projectile::GroundLowMinimumSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("High min samples## Ground", Vars::Aimbot::Projectile::GroundHighMinimumSamples, 3, 66, 1, "%i", FSlider_Right);
					FSlider("Low min distance## Ground", Vars::Aimbot::Projectile::GroundLowMinimumDistance, 0.f, 2500.f, 100.f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("High min distance## Ground", Vars::Aimbot::Projectile::GroundHighMinimumDistance, 0.f, 2500.f, 100.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					FSlider("Max changes## Ground", Vars::Aimbot::Projectile::GroundMaxChanges, 0, 5, 1, "%i", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("Max change time## Ground", Vars::Aimbot::Projectile::GroundMaxChangeTime, 0, 66, 1, "%i", FSlider_Right | FSlider_Min | FSlider_Precision);

					FText("Air");
					FSlider("Samples## Air", Vars::Aimbot::Projectile::AirSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("Straight fuzzy value## Air", Vars::Aimbot::Projectile::AirStraightFuzzyValue, 0.f, 500.f, 25.f, "%g", FSlider_Right | FSlider_Precision);
					FSlider("Low min samples## Air", Vars::Aimbot::Projectile::AirLowMinimumSamples, 3, 66, 1, "%i", FSlider_Left);
					FSlider("High min samples## Air", Vars::Aimbot::Projectile::AirHighMinimumSamples, 3, 66, 1, "%i", FSlider_Right);
					FSlider("Low min distance## Air", Vars::Aimbot::Projectile::AirLowMinimumDistance, 0.f, 2500.f, 100.f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("High min distance## Air", Vars::Aimbot::Projectile::AirHighMinimumDistance, 0.f, 2500.f, 100.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					FSlider("Max changes## Air", Vars::Aimbot::Projectile::AirMaxChanges, 0, 5, 1, "%i", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("Max change time## Air", Vars::Aimbot::Projectile::AirMaxChangeTime, 0, 66, 1, "%i", FSlider_Right | FSlider_Min | FSlider_Precision);

					FText("");
					FSlider("Velocity average count", Vars::Aimbot::Projectile::VelocityAverageCount, 1, 10, 1, "%i", FSlider_Left);
					FSlider("Vertical shift", Vars::Aimbot::Projectile::VerticalShift, 0.f, 10.f, 0.5f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);

					FSlider("Drag override", Vars::Aimbot::Projectile::DragOverride, 0.f, 1.f, 0.01f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision);
					FSlider("Time override", Vars::Aimbot::Projectile::TimeOverride, 0.f, 2.f, 0.01f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision);
					FSlider("Huntsman lerp", Vars::Aimbot::Projectile::HuntsmanLerp, 0.f, 100.f, 1.f, "%g%%", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FSlider("Huntsman lerp low", Vars::Aimbot::Projectile::HuntsmanLerpLow, 0.f, 100.f, 1.f, "%g%%", FSlider_Right | FSlider_Clamp | FSlider_Precision);
					FSlider("Huntsman add", Vars::Aimbot::Projectile::HuntsmanAdd, 0.f, 20.f, 1.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FSlider("Huntsman add low", Vars::Aimbot::Projectile::HuntsmanAddLow, 0.f, 20.f, 1.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
					FSlider("Huntsman clamp", Vars::Aimbot::Projectile::HuntsmanClamp, 0.f, 10.f, 0.5f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FToggle("Huntsman pull point", Vars::Aimbot::Projectile::HuntsmanPullPoint, FToggle_Right);
					SetCursorPosY(GetCursorPosY() + 8);

					FSlider("Splash points", Vars::Aimbot::Projectile::SplashPoints, 1, 400, 5, "%i", FSlider_Left | FSlider_Min);
					FToggle("Splash grates", Vars::Aimbot::Projectile::SplashGrates, FToggle_Right);
					FSlider("Splash Nth root", Vars::Aimbot::Projectile::SplashNthRoot, 0.5f, 2.f, 0.1f, "%g", FSlider_Left | FSlider_Min);
					FSlider("Splash rotate", Vars::Aimbot::Projectile::SplashRotate, 0.f, 360.f, 1.f, "%g%%", FSlider_Right | FSlider_Min);
					FSlider("Direct splash count", Vars::Aimbot::Projectile::SplashCountDirect, 1, 100, 1, "%i", FSlider_Left | FSlider_Min);
					FSlider("Arc splash count", Vars::Aimbot::Projectile::SplashCountArc, 1, 100, 1, "%i", FSlider_Right | FSlider_Min);
					FSlider("Splash trace interval", Vars::Aimbot::Projectile::SplashTraceInterval, 1, 10, 1, "%i", FSlider_Left);
					bool bHovered;
					FDropdown("Splash mode", Vars::Aimbot::Projectile::SplashMode, { "Multi", "Single" }, {}, FDropdown_Left, 0, & bHovered);
					FTooltip("Debug option to test performance,\nleave on multi if you want splash to work properly", bHovered);
					FDropdown("Rocket splash mode", Vars::Aimbot::Projectile::RocketSplashMode, { "Regular", "Special light", "Special heavy" }, {}, FDropdown_Right, 0, &bHovered);
					FTooltip("Special splash type for rockets, more expensive", bHovered);
					FSlider("Delta count", Vars::Aimbot::Projectile::DeltaCount, 1, 5, 1, "%i", FSlider_Left);
					FDropdown("Delta mode", Vars::Aimbot::Projectile::DeltaMode, { "Average", "Max" }, {}, FDropdown_Right);
				} EndSection();
			}
			if (Section("Melee", true))
			{
				FToggle("Auto backstab", Vars::Aimbot::Melee::AutoBackstab, FToggle_Left);
				FToggle("Ignore razorback", Vars::Aimbot::Melee::IgnoreRazorback, FToggle_Right);
				FToggle("Swing prediction", Vars::Aimbot::Melee::SwingPrediction, FToggle_Left);
				FToggle("Whip teammates", Vars::Aimbot::Melee::WhipTeam, FToggle_Right);
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Melee", true))
				{
					FSlider("Swing ticks", Vars::Aimbot::Melee::SwingTicks, 10, 14, 1, "%i", FSlider_Left);
					FToggle("Swing predict lag", Vars::Aimbot::Melee::SwingPredictLag, FToggle_Right);
					FToggle("Backstab account ping", Vars::Aimbot::Melee::BackstabAccountPing, FToggle_Left);
					FToggle("Backstab double test", Vars::Aimbot::Melee::BackstabDoubleTest, FToggle_Right);
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
			if (Section("Doubletap", true))
			{
				FToggle("Doubletap", Vars::CL_Move::Doubletap::Doubletap, FToggle_Left);
				FToggle("Warp", Vars::CL_Move::Doubletap::Warp, FToggle_Right);
				FToggle("Recharge ticks", Vars::CL_Move::Doubletap::RechargeTicks, FToggle_Left);
				FToggle("Anti-warp", Vars::CL_Move::Doubletap::AntiWarp, FToggle_Right);
				FSlider("Tick limit", Vars::CL_Move::Doubletap::TickLimit, 2, 22, 1, "%i", FSlider_Left | FSlider_Clamp);
				FSlider("Warp rate", Vars::CL_Move::Doubletap::WarpRate, 2, 22, 1, "%i", FSlider_Right | FSlider_Clamp);
				FSlider("Passive recharge", Vars::CL_Move::Doubletap::PassiveRecharge, 0, 67, 1, "%i", FSlider_Left | FSlider_Clamp);
				FSlider("Recharge limit", Vars::CL_Move::Doubletap::RechargeLimit, 1, 24, 1, "%i", FSlider_Right | FSlider_Clamp);
			} EndSection();
			if (Section("Fakelag"))
			{
				FDropdown("Fakelag", Vars::CL_Move::Fakelag::Fakelag, { "Off", "Plain", "Random", "Adaptive" }, {}, FSlider_Left);
				FDropdown("Options", Vars::CL_Move::Fakelag::Options, { "Only moving", "On unduck", "Not airborne" }, {}, FDropdown_Right | FDropdown_Multi, 0, nullptr, "Fakelag options");
				PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != Vars::CL_Move::Fakelag::FakelagEnum::Plain);
				{
					FSlider("Plain ticks", Vars::CL_Move::Fakelag::PlainTicks, 1, 22, 1, "%i", FSlider_Clamp | FSlider_Left);
				}
				PopTransparent();
				PushTransparent(FGet(Vars::CL_Move::Fakelag::Fakelag) != Vars::CL_Move::Fakelag::FakelagEnum::Random);
				{
					FSlider("Random ticks", Vars::CL_Move::Fakelag::RandomTicks, 1, 22, 1, "%i - %i", FSlider_Clamp | FSlider_Right);
				}
				PopTransparent();
				FToggle("Unchoke on attack", Vars::CL_Move::Fakelag::UnchokeOnAttack, FToggle_Left);
				FToggle("Retain blastjump", Vars::CL_Move::Fakelag::RetainBlastJump, FToggle_Right);
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug", true))
				{
					FToggle("Retain blastjump soldier only", Vars::CL_Move::Fakelag::RetainSoldierOnly);
				} EndSection();
			}
			if (Section("Anti Aim", true))
			{
				FToggle("Enabled", Vars::AntiHack::AntiAim::Enabled, FToggle_None, nullptr, "Antiaim enabled");
				FDropdown("Real pitch", Vars::AntiHack::AntiAim::PitchReal, { "None", "Up", "Down", "Zero", "Jitter", "Reverse jitter" }, {}, FDropdown_Left);
				FDropdown("Fake pitch", Vars::AntiHack::AntiAim::PitchFake, { "None", "Up", "Down", "Jitter", "Reverse jitter" }, {}, FDropdown_Right);
				FDropdown("Real yaw", Vars::AntiHack::AntiAim::YawReal, { "Forward", "Left", "Right", "Backwards", "Edge", "Jitter", "Spin" }, {}, FDropdown_Left);
				FDropdown("Fake yaw", Vars::AntiHack::AntiAim::YawFake, { "Forward", "Left", "Right", "Backwards", "Edge", "Jitter", "Spin" }, {}, FDropdown_Right);
				FDropdown("Real offset", Vars::AntiHack::AntiAim::RealYawMode, { "View", "Target" }, {}, FDropdown_Left);
				FDropdown("Fake offset", Vars::AntiHack::AntiAim::FakeYawMode, { "View", "Target" }, {}, FDropdown_Right);
				FSlider("Real offset## Offset", Vars::AntiHack::AntiAim::RealYawOffset, -180, 180, 5, "%i", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				FSlider("Fake offset## Offset", Vars::AntiHack::AntiAim::FakeYawOffset, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Edge && FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Jitter);
				{
					FSlider("Real value", Vars::AntiHack::AntiAim::RealYawValue, -180, 180.f, 5.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Edge && FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Jitter);
				{
					FSlider("Fake value", Vars::AntiHack::AntiAim::FakeYawValue, -180.f, 180.f, 5.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				PushTransparent(FGet(Vars::AntiHack::AntiAim::YawFake) != Vars::AntiHack::AntiAim::YawEnum::Spin && FGet(Vars::AntiHack::AntiAim::YawReal) != Vars::AntiHack::AntiAim::YawEnum::Spin);
				{
					FSlider("Spin speed", Vars::AntiHack::AntiAim::SpinSpeed, -30.f, 30.f, 1.f, "%g", FSlider_Left | FSlider_Precision);
				}
				PopTransparent();
				SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetRowPos() + H::Draw.Scale(8) });
				FToggle("Minwalk", Vars::AntiHack::AntiAim::MinWalk, FToggle_Left);
				FToggle("Anti-overlap", Vars::AntiHack::AntiAim::AntiOverlap, FToggle_Left);
				FToggle("Hide pitch on shot", Vars::AntiHack::AntiAim::InvalidShootPitch, FToggle_Right);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Resolver", true))
			{
				FToggle("Enabled", Vars::AntiHack::Resolver::Enabled, FToggle_Left, nullptr, "Resolver enabled");
				PushTransparent(!FGet(Vars::AntiHack::Resolver::Enabled));
				{
					FToggle("Auto resolve", Vars::AntiHack::Resolver::AutoResolve, FToggle_Right);
					PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolve));
					{
						FToggle("Auto resolve cheaters only", Vars::AntiHack::Resolver::AutoResolveCheatersOnly, FToggle_Left);
						FToggle("Auto resolve headshot only", Vars::AntiHack::Resolver::AutoResolveHeadshotOnly, FToggle_Right);
						PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolveYawAmount));
						{
							FSlider("Auto resolve yaw", Vars::AntiHack::Resolver::AutoResolveYawAmount, -180.f, 180.f, 45.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
						}
						PopTransparent();
						PushTransparent(Transparent || !FGet(Vars::AntiHack::Resolver::AutoResolvePitchAmount));
						{
							FSlider("Auto resolve pitch", Vars::AntiHack::Resolver::AutoResolvePitchAmount, -180.f, 180.f, 90.f, "%g", FSlider_Right | FSlider_Clamp);
						}
						PopTransparent();
					}
					PopTransparent();
					FSlider("Cycle yaw", Vars::AntiHack::Resolver::CycleYaw, -180.f, 180.f, 45.f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FSlider("Cycle pitch", Vars::AntiHack::Resolver::CyclePitch, -180.f, 180.f, 90.f, "%g", FSlider_Right | FSlider_Clamp);
					FToggle("Cycle view", Vars::AntiHack::Resolver::CycleView, FToggle_Left);
					FToggle("Cycle minwalk", Vars::AntiHack::Resolver::CycleMinwalk, FToggle_Right);
				}
				PopTransparent();
			} EndSection();
			if (Section("Auto Peek", true))
			{
				FToggle("Auto peek", Vars::CL_Move::AutoPeek);
			} EndSection();
			if (Section("Cheater Detection"))
			{
				PushTransparent(!FGet(Vars::CheaterDetection::Methods));
				{
					FDropdown("Detection methods", Vars::CheaterDetection::Methods, { "Invalid pitch", "Packet choking", "Aim flicking", "Duck Speed" }, {}, FDropdown_Multi);
					PushTransparent(Transparent || !FGet(Vars::CheaterDetection::DetectionsRequired));
					{
						FSlider("Detections required", Vars::CheaterDetection::DetectionsRequired, 0, 50, 1);
					}
					PopTransparent();

					PushTransparent(Transparent || !(FGet(Vars::CheaterDetection::Methods) & Vars::CheaterDetection::MethodsEnum::PacketChoking));
					{
						FSlider("Minimum choking", Vars::CheaterDetection::MinimumChoking, 4, 22, 1);
					}
					PopTransparent();

					PushTransparent(Transparent || !(FGet(Vars::CheaterDetection::Methods) & Vars::CheaterDetection::MethodsEnum::AimFlicking));
					{
						FSlider("Minimum flick angle", Vars::CheaterDetection::MinimumFlick, 10.f, 30.f, 1.f, "%.0f", FSlider_Left);
						FSlider("Maximum flick noise", Vars::CheaterDetection::MaximumNoise, 1.f, 10.f, 1.f, "%.0f", FSlider_Right);
					}
					PopTransparent();
				}
				PopTransparent();
			} EndSection();
			if (Section("Speedhack", true))
			{
				FToggle("Speedhack", Vars::CL_Move::SpeedEnabled);
				PushTransparent(!FGet(Vars::CL_Move::SpeedEnabled));
				{
					FSlider("SpeedHack factor", Vars::CL_Move::SpeedFactor, 1, 50, 1);
				}
				PopTransparent();
			} EndSection();

			EndTable();
		}
		break;
	}
}

void CMenu::MenuVisuals(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	// ESP
	case 0:
		if (BeginTable("VisualsESPTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("ESP"))
			{
				FDropdown("Draw", Vars::ESP::Draw, { "Players", "Buildings", "Projectiles", "Objective", "NPCs", "Health", "Ammo", "Money", "Powerups", "Bombs", "Spellbook", "Gargoyle" }, {}, FDropdown_Multi, 0, nullptr, "Draw ESP");
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Players));
				{
					FDropdown("Player", Vars::ESP::Player, { "Enemy", "Team", "Local", "Prioritized", "Friends", "Party", "##Divider", "Name", "Box", "Distance", "Bones", "Health bar", "Health text", "Uber bar", "Uber text", "Class icon", "Class text", "Weapon icon", "Weapon text", "Priority", "Labels", "Buffs", "Debuffs", "Misc", "Lag compensation", "Ping", "KDR" }, {}, FDropdown_Multi, 0, nullptr, "Player ESP");
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Buildings));
				{
					FDropdown("Building", Vars::ESP::Building, { "Enemy", "Team", "Local", "Prioritized", "Friends", "Party", "##Divider", "Name", "Box", "Distance", "Health bar", "Health text", "Owner", "Level", "Flags" }, {}, FDropdown_Multi, 0, nullptr, "Building ESP");
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Projectiles));
				{
					FDropdown("Projectile", Vars::ESP::Projectile, { "Enemy", "Team", "Local", "Prioritized", "Friends", "Party", "##Divider", "Name", "Box", "Distance", "Owner", "Flags" }, {}, FDropdown_Multi, 0, nullptr, "Projectile ESP");
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Objective));
				{
					FDropdown("Objective", Vars::ESP::Objective, { "Enemy", "Team", "##Divider", "Name", "Box", "Distance", "Flags", "Intel return time" }, {}, FDropdown_Multi, 0, nullptr, "Objective ESP");
				}
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Colors", true))
			{
				FToggle("Relative colors", Vars::Colors::Relative);
				if (FGet(Vars::Colors::Relative))
				{
					FColorPicker("Enemy color", Vars::Colors::Enemy, 0, FColorPicker_Left);
					FColorPicker("Team color", Vars::Colors::Team, 0, FColorPicker_Middle);
				}
				else
				{
					FColorPicker("RED color", Vars::Colors::TeamRed, 0, FColorPicker_Left);
					FColorPicker("BLU color", Vars::Colors::TeamBlu, 0, FColorPicker_Middle);
				}

				FColorPicker("Local color", Vars::Colors::Local, 0, FColorPicker_Left);
				FColorPicker("Target color", Vars::Colors::Target, 0, FColorPicker_Middle);

				FColorPicker("Healthpack color", Vars::Colors::Health, 0, FColorPicker_Left);
				FColorPicker("Ammopack color", Vars::Colors::Ammo, 0, FColorPicker_Middle);
				FColorPicker("Money color", Vars::Colors::Money, 0, FColorPicker_Left);
				FColorPicker("Powerup color", Vars::Colors::Powerup, 0, FColorPicker_Middle);
				FColorPicker("NPC color", Vars::Colors::NPC, 0, FColorPicker_Left);
				FColorPicker("Halloween color", Vars::Colors::Halloween, 0, FColorPicker_Middle);
			} EndSection();
			if (Section("Dormancy", true))
			{
				FSlider("Active alpha", Vars::ESP::ActiveAlpha, 0, 255, 5, "%i", FSlider_Left | FSlider_Clamp);
				FSlider("Dormant alpha", Vars::ESP::DormantAlpha, 0, 255, 5, "%i", FSlider_Right | FSlider_Clamp);
				FSlider("Dormant decay", Vars::ESP::DormantTime, 0.015f, 5.0f, 0.1f, "%gs", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				FToggle("Dormant priority only", Vars::ESP::DormantPriority, FToggle_Right);
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
			if (Section("Friendly", true))
			{
				FToggle("Players", Vars::Chams::Friendly::Players, FToggle_Left, nullptr, "Friendly player chams");
				FToggle("Ragdolls", Vars::Chams::Friendly::Ragdolls, FToggle_Right, nullptr, "Friendly ragdoll chams");
				FToggle("Buildings", Vars::Chams::Friendly::Buildings, FToggle_Left, nullptr, "Friendly building chams");
				FToggle("Projectiles", Vars::Chams::Friendly::Projectiles, FToggle_Right, nullptr, "Friendly projectile chams");

				FMDropdown("Visible material", Vars::Chams::Friendly::Visible, FDropdown_Left, 0, nullptr, "Friendly visible material");
				FMDropdown("Occluded material", Vars::Chams::Friendly::Occluded, FDropdown_Right, 0, nullptr, "Friendly occluded material");
			} EndSection();
			if (Section("Enemy", true))
			{
				FToggle("Players", Vars::Chams::Enemy::Players, FToggle_Left, nullptr, "Enemy player chams");
				FToggle("Ragdolls", Vars::Chams::Enemy::Ragdolls, FToggle_Right, nullptr, "Enemy ragdoll chams");
				FToggle("Buildings", Vars::Chams::Enemy::Buildings, FToggle_Left, nullptr, "Enemy building chams");
				FToggle("Projectiles", Vars::Chams::Enemy::Projectiles, FToggle_Right, nullptr, "Enemy projectile chams");

				FMDropdown("Visible material", Vars::Chams::Enemy::Visible, FDropdown_Left, 0, nullptr, "Enemy visible material");
				FMDropdown("Occluded material", Vars::Chams::Enemy::Occluded, FDropdown_Right, 0, nullptr, "Enemy occluded material");
			} EndSection();
			if (Section("World", true))
			{
				FToggle("NPCs", Vars::Chams::World::NPCs, FToggle_Left, nullptr, "NPC chams");
				FToggle("Pickups", Vars::Chams::World::Pickups, FToggle_Right, nullptr, "Pickup chams");
				FToggle("Objective", Vars::Chams::World::Objective, FToggle_Left, nullptr, "Objective chams");
				FToggle("Powerups", Vars::Chams::World::Powerups, FToggle_Right, nullptr, "Powerup chams");
				FToggle("Bombs", Vars::Chams::World::Bombs, FToggle_Left, nullptr, "Bomb chams");
				FToggle("Halloween", Vars::Chams::World::Halloween, FToggle_Right, nullptr, "Halloween chams");

				FMDropdown("Visible material", Vars::Chams::World::Visible, FDropdown_Left, 0, nullptr, "World visible material");
				FMDropdown("Occluded material", Vars::Chams::World::Occluded, FDropdown_Right, 0, nullptr, "World occluded material");
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Player", true))
			{
				FToggle("Local", Vars::Chams::Player::Local, FToggle_Left, nullptr, "Local chams");
				FToggle("Priority", Vars::Chams::Player::Priority, FToggle_Right, nullptr, "Priority chams");
				FToggle("Friend", Vars::Chams::Player::Friend, FToggle_Left, nullptr, "Friend chams");
				FToggle("Party", Vars::Chams::Player::Party, FToggle_Right, nullptr, "Party chams");
				FToggle("Target", Vars::Chams::Player::Target, FToggle_Left, nullptr, "Target chams");

				FMDropdown("Visible material", Vars::Chams::Player::Visible, FDropdown_Left, 0, nullptr, "Player visible material");
				FMDropdown("Occluded material", Vars::Chams::Player::Occluded, FDropdown_Right, 0, nullptr, "Player occluded material");
			} EndSection();
			if (Section("Backtrack", true))
			{
				FToggle("Enabled", Vars::Chams::Backtrack::Enabled, FToggle_Left, nullptr, "Backtrack chams");

				FMDropdown("Material", Vars::Chams::Backtrack::Visible, FDropdown_Left, 0, nullptr, "Backtrack material");
				FDropdown("Draw", Vars::Chams::Backtrack::Draw, { "Last", "Last + first", "All" }, {}, FDropdown_Right, 0, nullptr, "Backtrack chams mode");
			} EndSection();
			if (Section("Fake Angle", true))
			{
				FToggle("Enabled", Vars::Chams::FakeAngle::Enabled, FToggle_None, nullptr, "Fake angle chams");

				FMDropdown("Material", Vars::Chams::FakeAngle::Visible, FDropdown_None, 0, nullptr, "Fake angle material");
			} EndSection();
			if (Section("Viewmodel", true))
			{
				FToggle("Weapon", Vars::Chams::Viewmodel::Weapon, FToggle_Left, nullptr, "Weapon chams");
				FToggle("Hands", Vars::Chams::Viewmodel::Hands, FToggle_Right, nullptr, "Hands chams");

				FMDropdown("Weapon material", Vars::Chams::Viewmodel::WeaponVisible, FDropdown_Left, 0, nullptr, "Weapon material");
				FMDropdown("Hands material", Vars::Chams::Viewmodel::HandsVisible, FDropdown_Right, 0, nullptr, "Hands material");
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
			if (Section("Friendly", true))
			{
				FToggle("Players", Vars::Glow::Friendly::Players, FToggle_Left, nullptr, "Friendly player glow");
				FToggle("Ragdolls", Vars::Glow::Friendly::Ragdolls, FToggle_Right, nullptr, "Friendly ragdoll glow");
				FToggle("Buildings", Vars::Glow::Friendly::Buildings, FToggle_Left, nullptr, "Friendly building glow");
				FToggle("Projectiles", Vars::Glow::Friendly::Projectiles, FToggle_Right, nullptr, "Friendly projectile glow");

				PushTransparent(!FGet(Vars::Glow::Friendly::Stencil));
				{
					FSlider("Stencil scale## Friendly", Vars::Glow::Friendly::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "Friendly stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Friendly::Blur));
				{
					FSlider("Blur scale## Friendly", Vars::Glow::Friendly::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "Friendly blur scale");
				}
				PopTransparent();
			} EndSection();
			if (Section("Enemy", true))
			{
				FToggle("Players", Vars::Glow::Enemy::Players, FToggle_Left, nullptr, "Enemy player glow");
				FToggle("Ragdolls", Vars::Glow::Enemy::Ragdolls, FToggle_Right, nullptr, "Enemy ragdoll glow");
				FToggle("Buildings", Vars::Glow::Enemy::Buildings, FToggle_Left, nullptr, "Enemy building glow");
				FToggle("Projectiles", Vars::Glow::Enemy::Projectiles, FToggle_Right, nullptr, "Enemy projectile glow");

				PushTransparent(!FGet(Vars::Glow::Enemy::Stencil));
				{
					FSlider("Stencil scale## Enemy", Vars::Glow::Enemy::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "Enemy stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Enemy::Blur));
				{
					FSlider("Blur scale## Enemy", Vars::Glow::Enemy::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "Enemy blur scale");
				}
				PopTransparent();
			} EndSection();
			if (Section("World", true))
			{
				FToggle("NPCs", Vars::Glow::World::NPCs, FToggle_Left, nullptr, "NPC glow");
				FToggle("Pickups", Vars::Glow::World::Pickups, FToggle_Right, nullptr, "Pickup glow");
				FToggle("Objective", Vars::Glow::World::Objective, FToggle_Left, nullptr, "Objective glow");
				FToggle("Powerups", Vars::Glow::World::Powerups, FToggle_Right, nullptr, "Powerup glow");
				FToggle("Bombs", Vars::Glow::World::Bombs, FToggle_Left, nullptr, "Bomb glow");
				FToggle("Halloween", Vars::Glow::World::Halloween, FToggle_Right, nullptr, "Halloween glow");

				PushTransparent(!FGet(Vars::Glow::World::Stencil));
				{
					FSlider("Stencil scale## World", Vars::Glow::World::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "World stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::World::Blur));
				{
					FSlider("Blur scale## World", Vars::Glow::World::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "World blur scale");
				}
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Player", true))
			{
				FToggle("Local", Vars::Glow::Player::Local, FToggle_Left, nullptr, "Local glow");
				FToggle("Priority", Vars::Glow::Player::Priority, FToggle_Right, nullptr, "Priority glow");
				FToggle("Friend", Vars::Glow::Player::Friend, FToggle_Left, nullptr, "Friend glow");
				FToggle("Party", Vars::Glow::Player::Party, FToggle_Right, nullptr, "Party glow");
				FToggle("Target", Vars::Glow::Player::Target, FToggle_Left, nullptr, "Target glow");

				PushTransparent(!FGet(Vars::Glow::Player::Stencil));
				{
					FSlider("Stencil scale## Player", Vars::Glow::Player::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "Player stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Player::Blur));
				{
					FSlider("Blur scale## Player", Vars::Glow::Player::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "Player blur scale");
				}
				PopTransparent();
			} EndSection();
			if (Section("Backtrack", true))
			{
				FToggle("Enabled", Vars::Glow::Backtrack::Enabled, FToggle_Left, nullptr, "Backtrack glow");

				PushTransparent(!FGet(Vars::Glow::Backtrack::Stencil));
				{
					FSlider("Stencil scale## Backtrack", Vars::Glow::Backtrack::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "Backtrack stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Backtrack::Blur));
				{
					FSlider("Blur scale## Backtrack", Vars::Glow::Backtrack::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "Backtrack blur scale");
				}
				PopTransparent();
				FDropdown("Draw", Vars::Glow::Backtrack::Draw, { "Last", "Last + first", "All" }, {}, FDropdown_None, 0, nullptr, "Backtrack glow mode");
			} EndSection();
			if (Section("Fake Angle", true))
			{
				FToggle("Enabled", Vars::Glow::FakeAngle::Enabled, FToggle_None, nullptr, "Fake angle glow");

				PushTransparent(!FGet(Vars::Glow::FakeAngle::Stencil));
				{
					FSlider("Stencil scale## FakeAngle", Vars::Glow::FakeAngle::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "Fake angle stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::FakeAngle::Blur));
				{
					FSlider("Blur scale## FakeAngle", Vars::Glow::FakeAngle::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "Fake angle blur scale");
				}
				PopTransparent();
			} EndSection();
			if (Section("Viewmodel", true))
			{
				FToggle("Weapon", Vars::Glow::Viewmodel::Weapon, FToggle_Left, nullptr, "Weapon glow");
				FToggle("Hands", Vars::Glow::Viewmodel::Hands, FToggle_Right, nullptr, "Hands glow");

				PushTransparent(!FGet(Vars::Glow::Viewmodel::Stencil));
				{
					FSlider("Stencil scale## Viewmodel", Vars::Glow::Viewmodel::Stencil, 0, 10, 1, "%i", FSlider_Left | FSlider_Min, nullptr, "Viewmodel stencil scale");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Glow::Viewmodel::Blur));
				{
					FSlider("Blur scale## Viewmodel", Vars::Glow::Viewmodel::Blur, 0, 10, 1, "%i", FSlider_Right | FSlider_Min, nullptr, "Viewmodel blur scale");
				}
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
			if (Section("Removals", true))
			{
				FToggle("Scope", Vars::Visuals::Removals::Scope, FToggle_Left, nullptr, "Scope removal");
				FToggle("Interpolation", Vars::Visuals::Removals::Interpolation, FToggle_Right, nullptr, "Interpolation removal");
				FToggle("Disguises", Vars::Visuals::Removals::Disguises, FToggle_Left, nullptr, "Disguises removal");
				FToggle("Screen overlays", Vars::Visuals::Removals::ScreenOverlays, FToggle_Right, nullptr, "Screen overlays removal");
				FToggle("Taunts", Vars::Visuals::Removals::Taunts, FToggle_Left, nullptr, "Taunts removal");
				FToggle("Screen effects", Vars::Visuals::Removals::ScreenEffects, FToggle_Right, nullptr, "Screen effects removal");
				FToggle("View punch", Vars::Visuals::Removals::ViewPunch, FToggle_Left, nullptr, "View punch removal");
				FToggle("Angle forcing", Vars::Visuals::Removals::AngleForcing, FToggle_Right, nullptr, "Angle forcing removal");
				FToggle("Post processing", Vars::Visuals::Removals::PostProcessing, FToggle_Left, nullptr, "Post processing removal");
				FToggle("MOTD", Vars::Visuals::Removals::MOTD, FToggle_Right, nullptr, "MOTD removal");
			} EndSection();
			if (Section("UI"))
			{
				FDropdown("Streamer mode", Vars::Visuals::UI::StreamerMode, { "Off", "Local", "Friends", "Party", "All" }, {}, FDropdown_Left);
				FDropdown("Chat tags", Vars::Visuals::UI::ChatTags, { "Local", "Friends", "Party", "Assigned" }, {}, FDropdown_Right | FDropdown_Multi);
				PushTransparent(!FGet(Vars::Visuals::UI::FieldOfView));
				{
					FSlider("Field of view", Vars::Visuals::UI::FieldOfView, 0.f, 160.f, 1.f, "%g", FSlider_Min);
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Visuals::UI::ZoomFieldOfView));
				{
					FSlider("Zoomed field of view", Vars::Visuals::UI::ZoomFieldOfView, 0.f, 160.f, 1.f, "%g", FSlider_Min);
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Visuals::UI::AspectRatio));
				{
					FSlider("Aspect ratio", Vars::Visuals::UI::AspectRatio, 0.f, 5.f, 0.01f, "%g", FSlider_Min | FSlider_Precision);
				}
				PopTransparent();
				FToggle("Reveal scoreboard", Vars::Visuals::UI::RevealScoreboard, FToggle_Left);
				FToggle("Scoreboard utility", Vars::Visuals::UI::ScoreboardUtility, FToggle_Right);
				FToggle("Scoreboard colors", Vars::Visuals::UI::ScoreboardColors, FToggle_Left);
				FToggle("Clean screenshots", Vars::Visuals::UI::CleanScreenshots, FToggle_Right);
				FToggle("Sniper sightlines", Vars::Visuals::UI::SniperSightlines, FToggle_Left);
				FToggle("Pickup timers", Vars::Visuals::UI::PickupTimers, FToggle_Right);
			} EndSection();
			if (Section("Viewmodel", true))
			{
				FToggle("Crosshair aim position", Vars::Visuals::Viewmodel::CrosshairAim, FToggle_Left);
				FToggle("Viewmodel aim position", Vars::Visuals::Viewmodel::ViewmodelAim, FToggle_Right);
				FSlider("Offset X", Vars::Visuals::Viewmodel::OffsetX, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision, nullptr, "Viewmodel offset x");
				FSlider("Pitch", Vars::Visuals::Viewmodel::Pitch, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel pitch");
				FSlider("Offset Y", Vars::Visuals::Viewmodel::OffsetY, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision, nullptr, "Viewmodel offset y");
				FSlider("Yaw", Vars::Visuals::Viewmodel::Yaw, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel yaw");
				FSlider("Offset Z", Vars::Visuals::Viewmodel::OffsetZ, -45, 45, 5, "%i", FSlider_Left | FSlider_Precision, nullptr, "Viewmodel offset z");
				FSlider("Roll", Vars::Visuals::Viewmodel::Roll, -180, 180, 5, "%i", FSlider_Right | FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel roll");
				PushTransparent(!FGet(Vars::Visuals::Viewmodel::FieldOfView));
				{
					FSlider("Field of view## Viewmodel", Vars::Visuals::Viewmodel::FieldOfView, 0.f, 180.f, 1.f, "%.0f", FSlider_Clamp | FSlider_Precision, nullptr, "Viewmodel field of view");
				}
				PopTransparent();
				PushTransparent(!FGet(Vars::Visuals::Viewmodel::SwayScale) || !FGet(Vars::Visuals::Viewmodel::SwayInterp));
				{
					FSlider("Sway scale", Vars::Visuals::Viewmodel::SwayScale, 0.f, 5.f, 0.5f, "%g", FSlider_Left | FSlider_Min | FSlider_Precision, nullptr, "Viewmodel sway scale");
					FSlider("Sway interp", Vars::Visuals::Viewmodel::SwayInterp, 0.f, 1.f, 0.1f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision, nullptr, "Viewmodel sway interp");
				}
				PopTransparent();
			} EndSection();
			if (Section("Particles"))
			{
				// https://developer.valvesoftware.com/wiki/Team_Fortress_2/Particles
				// https://forums.alliedmods.net/showthread.php?t=127111
				FSDropdown("Bullet trail", Vars::Visuals::Particles::BulletTrail, { "Off", "Big nasty", "Distortion trail", "Machina", "Sniper rail", "Short circuit", "C.A.P.P.E.R", "Merasmus ZAP", "Merasmus ZAP 2", "Black ink", "Line", "Clipped line", "Beam" }, FDropdown_Left | FSDropdown_Custom);
				FSDropdown("Crit trail", Vars::Visuals::Particles::CritTrail, { "Off", "Big nasty", "Distortion trail", "Machina", "Sniper rail", "Short circuit", "C.A.P.P.E.R", "Merasmus ZAP", "Merasmus ZAP 2", "Black ink", "Line", "Clipped line", "Beam" }, FDropdown_Right | FSDropdown_Custom);
				FSDropdown("Medigun beam", Vars::Visuals::Particles::MedigunBeam, { "Off", "None", "Uber", "Dispenser", "Passtime", "Bombonomicon", "White", "Orange" }, FDropdown_Left | FSDropdown_Custom);
				FSDropdown("Medigun charge", Vars::Visuals::Particles::MedigunCharge, { "Off", "None", "Electrocuted", "Halloween", "Fireball", "Teleport", "Burning", "Scorching", "Purple energy", "Green energy", "Nebula", "Purple stars", "Green stars", "Sunbeams", "Spellbound", "Purple sparks", "Yellow sparks", "Green zap", "Yellow zap", "Plasma", "Frostbite", "Time warp", "Purple souls", "Green souls", "Bubbles", "Hearts" }, FDropdown_Right | FSDropdown_Custom);
				FSDropdown("Projectile trail", Vars::Visuals::Particles::ProjectileTrail, { "Off", "None", "Rocket", "Critical", "Energy", "Charged", "Ray", "Fireball", "Teleport", "Fire", "Flame", "Sparks", "Flare", "Trail", "Health", "Smoke", "Bubbles", "Halloween", "Monoculus", "Sparkles", "Rainbow" }, FDropdown_Left | FSDropdown_Custom);
				FDropdown("Spell footsteps", Vars::Visuals::Particles::SpellFootsteps, { "Off", "Color", "Team", "Halloween" }, {}, FDropdown_Right, -10);
				FColorPicker("Spell footstep", Vars::Colors::SpellFootstep, 0, FColorPicker_Dropdown, nullptr, "Spell footstep color");
				FToggle("Draw icons through walls", Vars::Visuals::Particles::DrawIconsThroughWalls);
				FToggle("Draw damage numbers through walls", Vars::Visuals::Particles::DrawDamageNumbersThroughWalls);
			} EndSection();
			if (Section("Ragdolls", true))
			{
				FToggle("No ragdolls", Vars::Visuals::Ragdolls::NoRagdolls, FToggle_Left);
				FToggle("No gibs", Vars::Visuals::Ragdolls::NoGib, FToggle_Right);
				FToggle("Mods", Vars::Visuals::Ragdolls::Enabled, FToggle_Left, nullptr, "Ragdoll mods");
				PushTransparent(!FGet(Vars::Visuals::Ragdolls::Enabled));
				{
					FToggle("Enemy only", Vars::Visuals::Ragdolls::EnemyOnly, FToggle_Right, nullptr, "Ragdoll mods enemy only");
					FDropdown("Ragdoll effects", Vars::Visuals::Ragdolls::Effects, { "Burning", "Electrocuted", "Ash", "Dissolve" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Ragdoll model", Vars::Visuals::Ragdolls::Type, { "None", "Gold", "Ice" }, {}, FDropdown_Right);
					FSlider("Ragdoll force", Vars::Visuals::Ragdolls::Force, -10.f, 10.f, 0.5f, "%g", FSlider_Precision);
					FSlider("Horizontal force", Vars::Visuals::Ragdolls::ForceHorizontal, -10.f, 10.f, 0.5f, "%g", FSlider_Precision, nullptr, "Ragdoll horizontal force");
					FSlider("Vertical force", Vars::Visuals::Ragdolls::ForceVertical, -10.f, 10.f, 0.5f, "%g", FSlider_Precision, nullptr, "Ragdoll vertical force");
				}
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Line", true))
			{
				FColorPicker("Line tracer clipped", Vars::Colors::LineClipped, 0, FColorPicker_None, nullptr, "Line tracer clipped color");
				FColorPicker("Line tracer", Vars::Colors::Line, 1, FColorPicker_None, nullptr, "Line tracer color");
				FToggle("Line tracers", Vars::Visuals::Line::Enabled);
				FSlider("Draw duration## Line", Vars::Visuals::Line::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision, nullptr, "Line draw duration");
			} EndSection();
			if (Section("Simulation"))
			{
				FDropdown("Player path", Vars::Visuals::Simulation::PlayerPath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Left, -20);
				FColorPicker("Player path", Vars::Colors::PlayerPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Player path color");
				FColorPicker("Player path clipped", Vars::Colors::PlayerPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Player path clipped color");
				FDropdown("Projectile path", Vars::Visuals::Simulation::ProjectilePath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Right, -20);
				FColorPicker("Projectile path", Vars::Colors::ProjectilePath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Projectile path color");
				FColorPicker("Projectile path clipped", Vars::Colors::ProjectilePathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Projectile path clipped color");
				FDropdown("Trajectory path", Vars::Visuals::Simulation::TrajectoryPath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Left, -20);
				FColorPicker("Trajectory path", Vars::Colors::TrajectoryPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Trajectory path color");
				FColorPicker("Trajectory path clipped", Vars::Colors::TrajectoryPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Trajectory path clipped color");
				FDropdown("Shot path", Vars::Visuals::Simulation::ShotPath, { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" }, {}, FDropdown_Right, -20);
				FColorPicker("Shot path", Vars::Colors::ShotPath, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Shot path color");
				FColorPicker("Shot path clipped", Vars::Colors::ShotPathClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Shot path clipped color");
				FDropdown("Splash radius", Vars::Visuals::Simulation::SplashRadius, { "Simulation", "##Divider", "Priority", "Enemy", "Team", "Local", "Friends", "Party", "##Divider", "Rockets", "Stickies", "Pipes", "Scorch shot", "##Divider", "Trace" }, {}, FDropdown_Multi, -20);
				FColorPicker("Splash radius", Vars::Colors::SplashRadius, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Splash radius color");
				FColorPicker("Splash radius clipped", Vars::Colors::SplashRadiusClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Splash radius color");
				FToggle("Timed", Vars::Visuals::Simulation::Timed, FToggle_Left, nullptr, "Timed path");
				FToggle("Box", Vars::Visuals::Simulation::Box, FToggle_Right, nullptr, "Path box");
				FToggle("Projectile camera", Vars::Visuals::Simulation::ProjectileCamera, FToggle_Left);
				FToggle("Swing prediction lines", Vars::Visuals::Simulation::SwingLines, FToggle_Right);
				PushTransparent(FGet(Vars::Visuals::Simulation::Timed));
				{
					FSlider("Draw duration## Simulation", Vars::Visuals::Simulation::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision, nullptr, "Simulation draw duration");
				}
				PopTransparent();
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Part1", true))
				{
					FSlider("Seperator spacing", Vars::Visuals::Simulation::SeparatorSpacing, 1, 16, 1, "%d", FSlider_Left);
					FSlider("Seperator length", Vars::Visuals::Simulation::SeparatorLength, 2, 16, 1, "%d", FSlider_Right);
				} EndSection();
				if (Section("Debug## Part2", true))
				{
					FToggle("Simulation override", Vars::Visuals::Trajectory::Override);
					FSlider("Offset X", Vars::Visuals::Trajectory::OffX, -25.f, 25.f, 0.5f, "%g", FSlider_Precision);
					FSlider("Offset Y", Vars::Visuals::Trajectory::OffY, -25.f, 25.f, 0.5f, "%g", FSlider_Precision);
					FSlider("Offset Z", Vars::Visuals::Trajectory::OffZ, -25.f, 25.f, 0.5f, "%g", FSlider_Precision);
					FToggle("Pipes", Vars::Visuals::Trajectory::Pipes);
					FSlider("Hull", Vars::Visuals::Trajectory::Hull, 0.f, 10.f, 0.5f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("Speed", Vars::Visuals::Trajectory::Speed, 0.f, 5000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("Gravity", Vars::Visuals::Trajectory::Gravity, 0.f, 1.f, 0.1f, "%g", FSlider_Precision);
					FToggle("No spin", Vars::Visuals::Trajectory::NoSpin);
					FSlider("Lifetime", Vars::Visuals::Trajectory::LifeTime, 0.f, 10.f, 0.1f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("Up velocity", Vars::Visuals::Trajectory::UpVelocity, 0.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("Angular velocity X", Vars::Visuals::Trajectory::AngVelocityX, -1000.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("Angular velocity Y", Vars::Visuals::Trajectory::AngVelocityY, -1000.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("Angular velocity Z", Vars::Visuals::Trajectory::AngVelocityZ, -1000.f, 1000.f, 50.f, "%g", FSlider_Precision);
					FSlider("Drag", Vars::Visuals::Trajectory::Drag, 0.f, 2.f, 0.1f, "%g", FSlider_Precision);
					FSlider("Drag X", Vars::Visuals::Trajectory::DragBasisX, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("Drag Y", Vars::Visuals::Trajectory::DragBasisY, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("Drag Z", Vars::Visuals::Trajectory::DragBasisZ, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("Angular drag X", Vars::Visuals::Trajectory::AngDragBasisX, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("Angular drag Y", Vars::Visuals::Trajectory::AngDragBasisY, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("Angular drag Z", Vars::Visuals::Trajectory::AngDragBasisZ, 0.f, 0.1f, 0.01f, "%.15g", FSlider_Precision);
					FSlider("Max velocity", Vars::Visuals::Trajectory::MaxVelocity, 0.f, 4000.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
					FSlider("Max angular velocity", Vars::Visuals::Trajectory::MaxAngularVelocity, 0.f, 7200.f, 50.f, "%g", FSlider_Min | FSlider_Precision);
				} EndSection();
			}
			if (Section("Hitbox"))
			{
				FDropdown("Bones enabled", Vars::Visuals::Hitbox::BonesEnabled, { "On shot", "On hit" }, {}, FDropdown_Multi, -110, nullptr, "Hitbox bones enabled");
				FColorPicker("Target edge", Vars::Colors::TargetHitboxEdge, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Target edge color");
				FColorPicker("Target edge clipped", Vars::Colors::TargetHitboxEdgeClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Target edge clipped color");
				SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
				FColorPicker("Target face", Vars::Colors::TargetHitboxFace, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Target face color");
				FColorPicker("Target face clipped", Vars::Colors::TargetHitboxFaceClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Target face clipped color");
				SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
				FColorPicker("Bone edge", Vars::Colors::BoneHitboxEdge, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bone edge color");
				FColorPicker("Bone edge clipped", Vars::Colors::BoneHitboxEdgeClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bone edge clipped color");
				SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
				FColorPicker("Bone face", Vars::Colors::BoneHitboxFace, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bone face color");
				FColorPicker("Bone face clipped", Vars::Colors::BoneHitboxFaceClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bone face clipped color");

				FDropdown("Bounds enabled", Vars::Visuals::Hitbox::BoundsEnabled, { "On shot", "On hit", "Aim point" }, {}, FDropdown_Multi, -50, nullptr, "Hitbox bounds enabled");
				FColorPicker("Bound edge", Vars::Colors::BoundHitboxEdge, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bound edge color");
				FColorPicker("Bound edge clipped", Vars::Colors::BoundHitboxEdgeClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bound edge clipped color");
				SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
				FColorPicker("Bound face", Vars::Colors::BoundHitboxFace, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bound face color");
				FColorPicker("Bound face clipped", Vars::Colors::BoundHitboxFaceClipped, 0, FColorPicker_Dropdown | FColorPicker_Tooltip, nullptr, "Bound face clipped color");

				FSlider("Draw duration## Hitbox", Vars::Visuals::Hitbox::DrawDuration, 0.f, 10.f, 1.f, "%g", FSlider_Min | FSlider_Precision, nullptr, "Hitbox draw duration");
			} EndSection();
			if (Section("Thirdperson", true))
			{
				FToggle("Thirdperson", Vars::Visuals::ThirdPerson::Enabled, FToggle_Left);
				FToggle("Thirdperson crosshair", Vars::Visuals::ThirdPerson::Crosshair, FToggle_Right);
				FSlider("Thirdperson distance", Vars::Visuals::ThirdPerson::Distance, 0.f, 500.f, 5.f, "%g", FSlider_Precision);
				FSlider("Thirdperson right", Vars::Visuals::ThirdPerson::Right, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
				FSlider("Thirdperson up", Vars::Visuals::ThirdPerson::Up, -500.f, 500.f, 5.f, "%g", FSlider_Precision);
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug", true))
				{
					FToggle("Thirdperson scales", Vars::Visuals::ThirdPerson::Scale, FToggle_Left);
					FToggle("Thirdperson collides", Vars::Visuals::ThirdPerson::Collide, FToggle_Right);
				} EndSection();
			}
			if (Section("Out of FOV arrows", true))
			{
				FToggle("Enabled", Vars::Visuals::FOVArrows::Enabled, FToggle_None, nullptr, "Out of FOV arrows enabled");
				FSlider("Offset", Vars::Visuals::FOVArrows::Offset, 0, 500, 25, "%i", FSlider_Left | FSlider_Min | FSlider_Precision, nullptr, "Out of FOV arrows offset");
				FSlider("Max distance", Vars::Visuals::FOVArrows::MaxDist, 0.f, 5000.f, 50.f, "%g", FSlider_Right | FSlider_Min | FSlider_Precision, nullptr, "Out of FOV arrows max distance");
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
				{
					FColorPicker("World modulation", Vars::Colors::WorldModulation, 0, FColorPicker_Left);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Sky));
				{
					FColorPicker("Sky modulation", Vars::Colors::SkyModulation, 0, FColorPicker_Middle);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Prop));
				{
					FColorPicker("Prop modulation", Vars::Colors::PropModulation, 0, FColorPicker_Left);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Particle));
				{
					FColorPicker("Particle modulation", Vars::Colors::ParticleModulation, 0, FColorPicker_Middle);
				}
				PopTransparent();
				PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Fog));
				{
					FColorPicker("Fog modulation", Vars::Colors::FogModulation, 0, FColorPicker_Left);
				}
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
			if (Section("Main", true))
			{
				FToggle("Enabled", Vars::Radar::Main::Enabled, FToggle_Left, nullptr, "Radar enabled");
				FToggle("Draw out of range", Vars::Radar::Main::AlwaysDraw, FToggle_Right);
				FDropdown("Style", Vars::Radar::Main::Style, { "Circle", "Rectangle" }, {}, FDropdown_None, 0, nullptr, "Radar style");
				FSlider("Range", Vars::Radar::Main::Range, 50, 3000, 50, "%i", FSlider_Precision, nullptr, "Radar range");
				FSlider("Background alpha", Vars::Radar::Main::BackAlpha, 0, 255, 1, "%i", FSlider_Clamp, nullptr, "Radar background alpha");
				FSlider("Line alpha", Vars::Radar::Main::LineAlpha, 0, 255, 1, "%i", FSlider_Clamp, nullptr, "Radar line alpha");
			} EndSection();
			if (Section("Player", true))
			{
				FToggle("Enabled", Vars::Radar::Players::Enabled, FToggle_Left, nullptr, "Radar player enabled");
				FToggle("Background", Vars::Radar::Players::Background, FToggle_Right, nullptr, "Radar player background");
				FDropdown("Draw", Vars::Radar::Players::Draw, { "Local", "Enemy", "Team", "Friends", "Party", "Prioritized", "Cloaked" }, {}, FDropdown_Left | FDropdown_Multi, 0, nullptr, "Radar player draw");
				FDropdown("Icon", Vars::Radar::Players::IconType, { "Icons", "Portraits", "Avatar" }, {}, FDropdown_Right, 0, nullptr, "Radar player icon");
				FSlider("Icon size## Player", Vars::Radar::Players::IconSize, 12, 30, 2, "%i", FSlider_None, nullptr, "Radar player icon size");
				FToggle("Health bar", Vars::Radar::Players::Health, FToggle_Left, nullptr, "Radar player health bar");
				FToggle("Height indicator", Vars::Radar::Players::Height, FToggle_Right, nullptr, "Radar player height indicator");
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Building", true))
			{
				FToggle("Enabled", Vars::Radar::Buildings::Enabled, FToggle_Left, nullptr, "Radar building enabled");
				FToggle("Background", Vars::Radar::Buildings::Background, FToggle_Right, nullptr, "Radar building background");
				FDropdown("Draw", Vars::Radar::Buildings::Draw, { "Local", "Enemy", "Team", "Friends", "Party", "Prioritized" }, {}, FDropdown_Multi, 0, nullptr, "Radar building draw");
				FSlider("Icon size## Building", Vars::Radar::Buildings::IconSize, 12, 30, 2, "%i", FSlider_None, nullptr, "Radar building icon size");
				FToggle("Health bar", Vars::Radar::Buildings::Health, FToggle_None, nullptr, "Radar building health bar");
			} EndSection();
			if (Section("World", true))
			{
				FToggle("Enabled", Vars::Radar::World::Enabled, FToggle_Left, nullptr, "Radar world enabled");
				FToggle("Background", Vars::Radar::World::Background, FToggle_Right, nullptr, "Radar world background");
				FDropdown("Draw", Vars::Radar::World::Draw, { "Health", "Ammo", "Money", "Bombs", "Powerup", "Spellbook", "Gargoyle" }, {}, FDropdown_Multi, 0, nullptr, "Radar world draw");
				FSlider("Icon size## World", Vars::Radar::World::IconSize, 12, 30, 2, "%i", FSlider_None, nullptr, "Radar world icon size");
			} EndSection();

			EndTable();
		}
		break;
	// Menu
	case 5:
		if (BeginTable("MenuTable", 2))
		{
			/* Column 1 */
			TableNextColumn();

			if (Section("General", true))
			{
				FColorPicker("Accent color", Vars::Menu::Theme::Accent, 0, FColorPicker_Left);
				FColorPicker("Background color", Vars::Menu::Theme::Background, 0, FColorPicker_Middle);
				FColorPicker("Active color", Vars::Menu::Theme::Active, 0, FColorPicker_Left);
				FColorPicker("Inactive color", Vars::Menu::Theme::Inactive, 0, FColorPicker_Middle);

				FSDropdown("Cheat title", Vars::Menu::CheatName, {}, FDropdown_Left | FSDropdown_AutoUpdate);
				FSDropdown("Chat info prefix", Vars::Menu::CheatPrefix, {}, FDropdown_Right);
				FKeybind("Primary key", Vars::Menu::MenuPrimaryKey.Map[DEFAULT_BIND], FButton_Left | FKeybind_AllowMenu);
				FKeybind("Secondary key", Vars::Menu::MenuSecondaryKey.Map[DEFAULT_BIND], FButton_Right | FButton_SameLine | FKeybind_AllowMenu);
				switch (Vars::Menu::MenuPrimaryKey.Map[DEFAULT_BIND])
				{
				case VK_LBUTTON:
				case VK_RBUTTON:
					Vars::Menu::MenuPrimaryKey.Map[DEFAULT_BIND] = Vars::Menu::MenuPrimaryKey.Default;
				}
				switch (Vars::Menu::MenuSecondaryKey.Map[DEFAULT_BIND])
				{
				case VK_LBUTTON:
				case VK_RBUTTON:
					Vars::Menu::MenuSecondaryKey.Map[DEFAULT_BIND] = Vars::Menu::MenuSecondaryKey.Default;
				}
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
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug", true))
				{
					FColorPicker("Indicator good", Vars::Colors::IndicatorGood, 0, FColorPicker_Left);
					FColorPicker("Indicator text good", Vars::Colors::IndicatorTextGood, 0, FColorPicker_Middle);
					FColorPicker("Indicator bad", Vars::Colors::IndicatorBad, 0, FColorPicker_Left);
					FColorPicker("Indicator text bad", Vars::Colors::IndicatorTextBad, 0, FColorPicker_Middle);
					FColorPicker("Indicator mid", Vars::Colors::IndicatorMid, 0, FColorPicker_Left);
					FColorPicker("Indicator text mid", Vars::Colors::IndicatorTextMid, 0, FColorPicker_Middle);
					FColorPicker("Indicator misc", Vars::Colors::IndicatorMisc, 0, FColorPicker_Left);
					FColorPicker("Indicator text misc", Vars::Colors::IndicatorTextMisc, 0, FColorPicker_Middle);
				}
				EndSection();
			}

			EndTable();
		}
	}
}

void CMenu::MenuMisc(int iTab)
{
	using namespace ImGui;

	switch (iTab)
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
				{
					FSlider("Auto strafe turn scale", Vars::Misc::Movement::AutoStrafeTurnScale, 0.f, 1.f, 0.1f, "%g", FSlider_Left | FSlider_Clamp | FSlider_Precision);
					FSlider("Auto strafe max delta", Vars::Misc::Movement::AutoStrafeMaxDelta, 0.f, 180.f, 1.f, "%g", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				}
				PopTransparent();
				FToggle("Bunnyhop", Vars::Misc::Movement::Bunnyhop, FToggle_Left);
				FToggle("Edge jump", Vars::Misc::Movement::EdgeJump, FToggle_Right);
				FToggle("Auto jumpbug", Vars::Misc::Movement::AutoJumpbug, FToggle_Left); // this is unreliable without setups, do not depend on it!
				FToggle("No push", Vars::Misc::Movement::NoPush, FToggle_Right);
				FToggle("Auto rocketjump", Vars::Misc::Movement::AutoRocketJump, FToggle_Left);
				FToggle("Auto ctap", Vars::Misc::Movement::AutoCTap, FToggle_Right);
				FToggle("Fast stop", Vars::Misc::Movement::FastStop, FToggle_Left);
				FToggle("Fast accelerate", Vars::Misc::Movement::FastAccel, FToggle_Right);
				FToggle("Crouch speed", Vars::Misc::Movement::CrouchSpeed, FToggle_Left);
				FToggle("Movement lock", Vars::Misc::Movement::MovementLock, FToggle_Right);
				FToggle("Break jump", Vars::Misc::Movement::BreakJump, FToggle_Left);
				FToggle("Shield turn rate", Vars::Misc::Movement::ShieldTurnRate, FToggle_Right);
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug", true))
				{
					FSlider("Timing offset", Vars::Misc::Movement::TimingOffset, 0, 3);
					FSlider("Choke count", Vars::Misc::Movement::ChokeCount, 0, 3);
					FSlider("Apply timing offset above", Vars::Misc::Movement::ApplyAbove, 0, 8);
				} EndSection();
			}
			if (Section("Exploits", true))
			{
				FToggle("Cheats bypass", Vars::Misc::Exploits::CheatsBypass, FToggle_Left);
				FToggle("Pure bypass", Vars::Misc::Exploits::BypassPure, FToggle_Right);
				FToggle("Ping reducer", Vars::Misc::Exploits::PingReducer, FToggle_Left);
				FToggle("Equip region unlock", Vars::Misc::Exploits::EquipRegionUnlock, FToggle_Right);
				PushTransparent(!FGet(Vars::Misc::Exploits::PingReducer));
				{
					FSlider("cl_cmdrate", Vars::Misc::Exploits::PingTarget, 1, 66, 1, "%i", FSlider_Clamp);
				}
				PopTransparent();
			} EndSection();
			if (Vars::Debug::Options.Value)
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
				FToggle("Taunt control", Vars::Misc::Automation::TauntControl, FToggle_Left);
				FToggle("Kart control", Vars::Misc::Automation::KartControl, FToggle_Right);
				FToggle("Backpack expander", Vars::Misc::Automation::BackpackExpander, FToggle_Left);
				FToggle("Auto accept item drops", Vars::Misc::Automation::AcceptItemDrops, FToggle_Right);
				FToggle("Auto F2 ignored", Vars::Misc::Automation::AutoF2Ignored, FToggle_Left);
				FToggle("Auto F1 priority", Vars::Misc::Automation::AutoF1Priority, FToggle_Right);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Sound"))
			{
				FDropdown("Block", Vars::Misc::Sound::Block, { "Footsteps", "Noisemaker", "Frying pan", "Water" }, {}, FDropdown_Multi, 0, nullptr, "Sound block");
				FToggle("Hitsound always", Vars::Misc::Sound::HitsoundAlways, FToggle_Left);
				FToggle("Remove DSP", Vars::Misc::Sound::RemoveDSP, FToggle_Right);
				FToggle("Giant weapon sounds", Vars::Misc::Sound::GiantWeaponSounds);
			} EndSection();
			if (Section("Game", true))
			{
				FToggle("Network fix", Vars::Misc::Game::NetworkFix, FToggle_Left);
				FToggle("Prediction error jitter fix", Vars::Misc::Game::PredictionErrorJitterFix, FToggle_Right);
				FToggle("Bones optimization", Vars::Misc::Game::SetupBonesOptimization, FToggle_Left);
				FToggle("F2P chat bypass", Vars::Misc::Game::F2PChatBypass, FToggle_Right);
				FToggle("Anti cheat compatibility", Vars::Misc::Game::AntiCheatCompatibility);
			} EndSection();
			if (Vars::Debug::Options.Value)
			{
				if (Section("Debug## Anti cheat", true))
				{
					FToggle("Anti cheat crit hack", Vars::Misc::Game::AntiCheatCritHack);
				} EndSection();
			}
			if (Section("Queueing"))
			{
				FDropdown("Force regions", Vars::Misc::Queueing::ForceRegions,
					{ "Atlanta", "Chicago", "Texas", "Los Angeles", "Moses Lake", "New York", "Seattle", "Virginia", "##Divider", "Amsterdam", "Frankfurt", "Helsinki", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "##Divider", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "##Divider", "Bombay", "Chennai", "Dubai", "Hong Kong", "Madras", "Mumbai", "Seoul", "Singapore", "Tokyo", "Sydney", "##Divider", "Johannesburg" },
					{}, FDropdown_Multi
				);
				FToggle("Freeze queue", Vars::Misc::Queueing::FreezeQueue, FToggle_Left);
				FToggle("Auto queue", Vars::Misc::Queueing::AutoCasualQueue, FToggle_Right);
			} EndSection();
			if (Section("Mann vs. Machine", true))
			{
				FToggle("Instant respawn", Vars::Misc::MannVsMachine::InstantRespawn, FToggle_Left);
				FToggle("Instant revive", Vars::Misc::MannVsMachine::InstantRevive, FToggle_Right);
				FToggle("Allow MVM inspect", Vars::Misc::MannVsMachine::AllowInspect);
			} EndSection();
			if (Section("Steam RPC", true))
			{
				FToggle("Steam RPC", Vars::Misc::Steam::EnableRPC, FToggle_Left);
				FToggle("Override in menu", Vars::Misc::Steam::OverrideMenu, FToggle_Right);
				FDropdown("Match group", Vars::Misc::Steam::MatchGroup, { "Special Event", "MvM Mann Up", "Competitive", "Casual", "MvM Boot Camp" }, {}, FDropdown_Left);
				FSDropdown("Map text", Vars::Misc::Steam::MapText, {}, FDropdown_Right | FSDropdown_Custom);
				FSlider("Group size", Vars::Misc::Steam::GroupSize, 0, 6);
			} EndSection();

			EndTable();
		}
	}
}

void CMenu::MenuLogs(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	// Settings
	case 0:
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
				{
					FDropdown("Log to", Vars::Logging::VoteStart::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Vote start log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("Vote Cast"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteCast));
				{
					FDropdown("Log to", Vars::Logging::VoteCast::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Vote cast log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("Class Change"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::ClassChanges));
				{
					FDropdown("Log to", Vars::Logging::ClassChange::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Class change log to");
				}
				PopTransparent();
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Damage"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Damage));
				{
					FDropdown("Log to", Vars::Logging::Damage::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Damage log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("Cheat Detection"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::CheatDetection));
				{
					FDropdown("Log to", Vars::Logging::CheatDetection::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Cheat detection log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("Tags"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Tags));
				{
					FDropdown("Log to", Vars::Logging::Tags::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Tags log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("Aliases"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Aliases));
				{
					FDropdown("Log to", Vars::Logging::Aliases::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Aliases log to");
				}
				PopTransparent();
			} EndSection();
			if (Section("Resolver"))
			{
				PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Resolver));
				{
					FDropdown("Log to", Vars::Logging::Resolver::LogTo, { "Toasts", "Chat", "Party", "Console", "Menu", "Debug" }, {}, FDropdown_Multi, 0, nullptr, "Resolver log to");
				}
				PopTransparent();
			} EndSection();

			EndTable();
		}
		break;
	// Output
	case 1:
		if (Section("##Output", false, GetWindowHeight() - GetStyle().WindowPadding.y * 2))
		{
			for (auto& tOutput : m_vOutput)
			{
				ImVec2 vOriginalPos = GetCursorPos();
				size_t iLines = 1;

				float flWidth = GetWindowWidth() - GetStyle().WindowPadding.x * 2;
				if (tOutput.m_sFunction != "")
				{
					float flTitleWidth = 0.f;

					PushStyleColor(ImGuiCol_Text, ColorToVec(tOutput.tAccent));

					auto vWrapped = WrapText(tOutput.m_sFunction, flWidth);
					for (size_t i = 0; i < vWrapped.size(); i++)
					{
						FText(vWrapped[i].c_str());
						if (i == vWrapped.size() - 1)
							flTitleWidth = FCalcTextSize(vWrapped[i].c_str()).x + H::Draw.Scale(4);
					}
					iLines = vWrapped.size();

					PopStyleColor();
					
					vWrapped = WrapText(tOutput.m_sLog, flWidth - flTitleWidth);
					if (!vWrapped.empty())
					{
						SameLine(flTitleWidth + GetStyle().WindowPadding.x);
						FText(vWrapped.front().c_str());

						if (vWrapped.size() > 1)
						{
							std::string sLog = "";
							for (size_t i = 1; i < vWrapped.size(); i++)
							{
								sLog += vWrapped[i].c_str();
								if (i != vWrapped.size() - 1)
									sLog += " ";
							}
							vWrapped = WrapText(sLog, flWidth);
							for (size_t i = 0; i < vWrapped.size(); i++)
							{
								FText(vWrapped[i].c_str());
								if (i == vWrapped.size() - 1)
									flTitleWidth = FCalcTextSize(vWrapped[i].c_str()).x + H::Draw.Scale(4);
							}
							iLines += vWrapped.size();
						}
					}
				}
				else
				{
					PushStyleColor(ImGuiCol_Text, ColorToVec(tOutput.tAccent));

					auto vWrapped = WrapText(tOutput.m_sLog, flWidth);
					for (size_t i = 0; i < vWrapped.size(); i++)
						FText(vWrapped[i].c_str());
					iLines = vWrapped.size();

					PopStyleColor();
				}

				SetCursorPos(vOriginalPos); DebugDummy({ flWidth, H::Draw.Scale(13) * iLines + GetStyle().WindowPadding.y });

				if (IsItemHovered() && IsMouseDown(ImGuiMouseButton_Right))
					OpenPopup(std::format("Output{}", tOutput.m_iID).c_str());
				if (FBeginPopup(std::format("Output{}", tOutput.m_iID).c_str()))
				{
					PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

					if (FSelectable("Copy"))
						SDK::SetClipboard(std::format("{}{}{}", tOutput.m_sFunction, tOutput.m_sFunction != "" ? " " : "", tOutput.m_sLog));

					PopStyleVar();
					EndPopup();
				}
			}
		} EndSection();
	}
}

void CMenu::MenuSettings(int iTab)
{
	using namespace ImGui;

	switch (iTab)
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
					ShellExecuteA(NULL, NULL, F::Configs.m_sConfigPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				if (FButton("Visuals folder", FButton_Right | FButton_SameLine))
					ShellExecuteA(NULL, NULL, F::Configs.m_sVisualsPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

				static int iCurrentType = 0;
				FTabs({ "GENERAL", "VISUALS", }, &iCurrentType, { GetColumnWidth() / 2, H::Draw.Scale(40) }, { H::Draw.Scale(8), GetCursorPos().y }, false);

				switch (iCurrentType)
				{
				// General
				case 0:
				{
					static std::string newName;
					FSDropdown("Config name", &newName, {}, FDropdown_Left | FSDropdown_AutoUpdate);
					if (FButton("Create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
					{
						if (!std::filesystem::exists(F::Configs.m_sConfigPath + newName))
							F::Configs.SaveConfig(newName);
						newName.clear();
					}

					std::vector<std::pair<std::filesystem::directory_entry, std::string>> vConfigs = {};
					bool bDefaultFound = false;
					for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sConfigPath))
					{
						if (!entry.is_regular_file() || entry.path().extension() != F::Configs.m_sConfigExtension)
							continue;

						std::string sConfigName = entry.path().filename().string();
						sConfigName.erase(sConfigName.end() - F::Configs.m_sConfigExtension.size(), sConfigName.end());
						if (FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"))
							bDefaultFound = true;

						vConfigs.emplace_back(entry, sConfigName);
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
						bool bCurrentConfig = FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32(F::Configs.m_sCurrentConfig.c_str());
						ImVec2 vOriginalPos = GetCursorPos();

						SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
						TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, TruncateText(sConfigName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(80)).c_str());

						int iOffset = 0;
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str());

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_SAVE))
						{
							if (!bCurrentConfig || F::Configs.m_sCurrentVisuals.length())
								OpenPopup(std::format("Confirmation## SaveConfig{}", sConfigName).c_str());
							else
								F::Configs.SaveConfig(sConfigName);
						}

						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(++iOffset) * 25, vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DOWNLOAD))
							F::Configs.LoadConfig(sConfigName);

						if (FBeginPopupModal(std::format("Confirmation## SaveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
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

						if (FBeginPopupModal(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("Do you really want to remove '{}'?", sConfigName).c_str());

							PushDisabled(FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"));
							{
								if (FButton("Yes, delete", FButton_Fit))
								{
									F::Configs.RemoveConfig(sConfigName);
									CloseCurrentPopup();
								}
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
					FSDropdown("Config name", &newName, {}, FDropdown_Left | FSDropdown_AutoUpdate);
					if (FButton("Create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
					{
						if (!std::filesystem::exists(F::Configs.m_sVisualsPath + newName))
							F::Configs.SaveVisual(newName);
						newName.clear();
					}

					for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sVisualsPath))
					{
						if (!entry.is_regular_file() || entry.path().extension() != F::Configs.m_sConfigExtension)
							continue;

						std::string sConfigName = entry.path().filename().string();
						sConfigName.erase(sConfigName.end() - F::Configs.m_sConfigExtension.size(), sConfigName.end());

						bool bCurrentConfig = FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32(F::Configs.m_sCurrentVisuals.c_str());
						ImVec2 vOriginalPos = GetCursorPos();

						SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
						TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, TruncateText(sConfigName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(80)).c_str());

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
							if (FBeginPopupModal(std::format("Confirmation## SaveVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
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
							if (FBeginPopupModal(std::format("Confirmation## DeleteVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
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
			if (Section("Debug", true))
			{
				FToggle("Debug info", Vars::Debug::Info, FToggle_Left);
				FToggle("Debug logging", Vars::Debug::Logging, FToggle_Right);
				FToggle("Debug options", Vars::Debug::Options, FToggle_Left);
				FToggle("Show server hitboxes", Vars::Debug::ServerHitbox, FToggle_Right); FTooltip("Only localhost servers");
				FToggle("Anti aim lines", Vars::Debug::AntiAimLines, FToggle_Left);
				FToggle("Crash logging", Vars::Debug::CrashLogging, FToggle_Right);
#ifdef DEBUG_TRACES
				FToggle("Visualize traces", Vars::Debug::VisualizeTraces, FToggle_Left);
				FToggle("Visualize trace hits", Vars::Debug::VisualizeTraceHits, FToggle_Right);
#endif
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

				if (Vars::Debug::Options.Value && I::EngineClient->IsConnected())
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
			if (!I::EngineClient->IsConnected())
			{
				if (Section("Achievements"))
				{
					if (FButton("Unlock achievements"))
						OpenPopup("UnlockAchievements");
					if (FButton("Lock achievements"))
						OpenPopup("LockAchievements");

					if (FBeginPopupModal("UnlockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
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
					if (FBeginPopupModal("LockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
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
				} EndSection();
			}

			EndTable();

#ifdef DEBUG_HOOKS
			if (Vars::Debug::Options.Value)
			{
				if (Section("Hooks", true))
				{
					int i = 0;
					for (auto& var : g_Vars)
					{
						if (var->m_sName.find("Vars::Hooks::") == std::string::npos)
							continue;

						auto sName = var->m_sName;
						FToggle(sName.replace(0, strlen("Vars::Hooks::"), "").c_str(), *var->As<bool>(), !(i % 2) ? FToggle_Left : FToggle_Right);
						i++;
					}
				} EndSection();
			}
#endif
		}
		break;
	// Binds
	case 1:
		if (Section("Settings", true))
		{
			FToggle("Bind window title", Vars::Menu::BindWindowTitle, FToggle_Left);
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
					FSDropdown("Name", &tBind.m_sName, {}, FDropdown_Left | FSDropdown_AutoUpdate);
					{
						auto sParent = bParent ? "..." : tBind.m_iParent != DEFAULT_BIND && tBind.m_iParent < F::Binds.m_vBinds.size() ? F::Binds.m_vBinds[tBind.m_iParent].m_sName : "None";
						if (FButton(std::format("Parent: {}", sParent).c_str(), FButton_Right | FButton_SameLine | FButton_NoUpper, { 0, 40 }))
							bParent = 2;
					}
					FDropdown("Type", &tBind.m_iType, { "Key", "Class", "Weapon type", "Item slot" }, {}, FDropdown_Left);
					switch (tBind.m_iType)
					{
					case BindEnum::Key: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 2); FDropdown("Behavior", &tBind.m_iInfo, { "Hold", "Toggle", "Double click" }, {}, FDropdown_Right); break;
					case BindEnum::Class: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 8); FDropdown("Class", &tBind.m_iInfo, { "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdown_Right); break;
					case BindEnum::WeaponType: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 2); FDropdown("Weapon type", &tBind.m_iInfo, { "Hitscan", "Projectile", "Melee" }, {}, FDropdown_Right); break;
					case BindEnum::ItemSlot: tBind.m_iInfo = std::max(tBind.m_iInfo, 0); FDropdown("Item slot", &tBind.m_iInfo, { "1", "2", "3", "4", "5", "6", "7", "8", "9" }, {}, FDropdown_Right); break;
					}
				} EndChild();

				SetCursorPos({ GetWindowWidth() / 2 - GetStyle().WindowPadding.x / 2, vOriginalPos.y - H::Draw.Scale(8) });
				if (BeginChild("Split2", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(112) }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoBackground))
				{
					int iNot = tBind.m_bNot;
					FDropdown("While", &iNot, { "Active", "Not active" }, {}, FDropdown_Left);
					tBind.m_bNot = iNot;
					int iVisible = tBind.m_bVisible;
					FDropdown("Visibility", &iVisible, { "Visible", "Hidden" }, { 1, 0 }, FDropdown_Right);
					tBind.m_bVisible = iVisible;
					if (tBind.m_iType == 0)
						FKeybind("Key", tBind.m_iKey, FButton_None, { 0, 40 }, -96);

					// create/modify button
					bool bCreate = false, bClear = false, bParent = true;
					if (tBind.m_iParent != DEFAULT_BIND)
						bParent = F::Binds.m_vBinds.size() > tBind.m_iParent;

					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(96), H::Draw.Scale(56) });
					PushDisabled(!(bParent && (tBind.m_iType == BindEnum::Key ? tBind.m_iKey : true)));
					{
						bCreate = FButton("##CreateButton", FButton_None, { 40, 40 });
					}
					PopDisabled();
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(84), H::Draw.Scale(76) });
					bool bMatch = iBind != DEFAULT_BIND && F::Binds.m_vBinds.size() > iBind;
					if (bParent && (tBind.m_iType == BindEnum::Key ? tBind.m_iKey : true))
						IconImage(bMatch ? ICON_MD_SETTINGS : ICON_MD_ADD);
					else
					{
						PushTransparent(true);
							IconImage(bMatch ? ICON_MD_SETTINGS : ICON_MD_ADD);
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

			std::function<int(int, int, int)> getBinds = [&](int iParent, int x, int y)
				{
					for (auto it = F::Binds.m_vBinds.begin(); it < F::Binds.m_vBinds.end(); it++)
					{
						int _iBind = std::distance(F::Binds.m_vBinds.begin(), it);
						auto& _tBind = *it;
						if (iParent != _tBind.m_iParent)
							continue;

						y++;

						std::string sType; std::string sInfo;
						switch (_tBind.m_iType)
						{
						case BindEnum::Key:
							switch (_tBind.m_iInfo)
							{
							case BindEnum::KeyEnum::Hold: { sType = "hold"; break; }
							case BindEnum::KeyEnum::Toggle: { sType = "toggle"; break; }
							case BindEnum::KeyEnum::DoubleClick: { sType = "double"; break; }
							}
							sInfo = VK2STR(_tBind.m_iKey);
							break;
						case BindEnum::Class:
							sType = "class";
							switch (_tBind.m_iInfo)
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
							switch (_tBind.m_iInfo)
							{
							case BindEnum::WeaponTypeEnum::Hitscan: { sInfo = "hitscan"; break; }
							case BindEnum::WeaponTypeEnum::Projectile: { sInfo = "projectile"; break; }
							case BindEnum::WeaponTypeEnum::Melee: { sInfo = "melee"; break; }
							}
							break;
						case BindEnum::ItemSlot:
							sType = "slot";
							sInfo = std::format("{}", _tBind.m_iInfo + 1);
							break;
						}
						if (_tBind.m_bNot && (_tBind.m_iType != BindEnum::Key || _tBind.m_iInfo == BindEnum::KeyEnum::Hold))
							sType = std::format("not {}", sType);

						ImVec2 vOriginalPos = { H::Draw.Scale(8) + H::Draw.Scale(28) * x, H::Draw.Scale(108) + H::Draw.Scale(36) * y };

						// background
						float flWidth = GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(28) * x;
						float flHeight = H::Draw.Scale(28);
						ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
						GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, F::Render.Background1p5, H::Draw.Scale(3));

						// text
						float flTextWidth = flWidth - H::Draw.Scale(127);
						PushStyleVar(ImGuiStyleVar_Alpha, F::Binds.WillBeEnabled(_iBind) ? 1.f : 0.5f);

						SetCursorPos({ vOriginalPos.x + H::Draw.Scale(10), vOriginalPos.y + H::Draw.Scale(7) });
						FText(TruncateText(_tBind.m_sName, flTextWidth* (1.f / 3) - H::Draw.Scale(20)).c_str());

						SetCursorPos({ vOriginalPos.x + flTextWidth * (1.f / 3), vOriginalPos.y + H::Draw.Scale(7) });
						FText(sType.c_str());

						SetCursorPos({ vOriginalPos.x + flTextWidth * (2.f / 3), vOriginalPos.y + H::Draw.Scale(7) });
						FText(sInfo.c_str());

						// buttons
						int iOffset = -3;

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(6) });
						bool bDelete = IconButton(ICON_MD_DELETE);

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(6) });
						bool bEdit = IconButton(ICON_MD_EDIT);

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(6) });
						bool bNot = IconButton(!_tBind.m_bNot ? ICON_MD_CODE : ICON_MD_CODE_OFF);

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(6) });
						bool bVisibility = IconButton(_tBind.m_bVisible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF);

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(6) });
						bool bEnable = IconButton(_tBind.m_bEnabled ? ICON_MD_RADIO_BUTTON_ON : ICON_MD_RADIO_BUTTON_OFF);

						SetCursorPos(vOriginalPos);
						bool bClicked = Button(std::format("##{}", _iBind).c_str(), { flWidth, flHeight });

						PopStyleVar();

						if (bClicked)
						{
							if (bParent)
							{
								bParent = false;
								tBind.m_iParent = _iBind;
							}
							else
							{
								iBind = _iBind;
								tBind = _tBind;
							}
						}
						if (bDelete)
						{
							if (U::KeyHandler.Down(VK_SHIFT)) // allow user to quickly remove binds
								F::Binds.RemoveBind(_iBind);
							else
								OpenPopup(std::format("Confirmation## DeleteBind{}", _iBind).c_str());
						}
						if (FBeginPopupModal(std::format("Confirmation## DeleteBind{}", _iBind).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("Do you really want to delete '{}'{}?", _tBind.m_sName, F::Binds.HasChildren(_iBind) ? " and all of its children" : "").c_str());

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
						if (bEnable)
							_tBind.m_bEnabled = !_tBind.m_bEnabled;
						if (bVisibility)
							_tBind.m_bVisible = !_tBind.m_bVisible;
						if (bNot)
							_tBind.m_bNot = !_tBind.m_bNot;

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
				tBind.m_iParent = DEFAULT_BIND;
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

				auto getTeamColor = [&](int team, bool alive)
					{
						switch (team)
						{
						case 3: return Color_t(100, 150, 200, alive ? 255 : 127).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, false);
						case 2: return Color_t(255, 100, 100, alive ? 255 : 127).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, false);
						}
						return Color_t(127, 127, 127, 255).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, false);
					};
				auto drawPlayer = [getTeamColor](const ListPlayer& player, int x, int y)
					{
						const Color_t teamColor = getTeamColor(player.m_iTeam, player.m_bAlive);
						const ImColor imColor = ColorToVec(teamColor);

						ImVec2 vOriginalPos = { !x ? GetStyle().WindowPadding.x : GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(36 + 36 * y) };

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
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_PERSON);
						}
						else if (F::Spectate.m_iIntendedTarget == player.m_iUserID)
						{
							lOffset = H::Draw.Scale(29);
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_VISIBILITY);
						}
						SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y + H::Draw.Scale(7) });
						FText(player.m_sName.c_str());
						lOffset += FCalcTextSize(player.m_sName.c_str()).x + H::Draw.Scale(8);

						// buttons
						bool bClicked = false, bAdd = false, bAlias = false, bPitch = false, bYaw = false, bMinwalk = false, bView = false;
						
						int iOffset = 2;

						if (!player.m_bFake)
						{
							// right
							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bAlias = IconButton(ICON_MD_EDIT);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bAdd = IconButton(ICON_MD_ADD);
						}

						bool bResolver = Vars::AntiHack::Resolver::Enabled.Value && !player.m_bLocal;
						if (bResolver)
						{
							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bView = IconButton(ICON_MD_NEAR_ME);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bMinwalk = IconButton(ICON_MD_DIRECTIONS_WALK);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bYaw = IconButton(ICON_MD_ARROW_FORWARD);

							SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 20), vOriginalPos.y + H::Draw.Scale(6) });
							bPitch = IconButton(ICON_MD_ARROW_UPWARD);
						}

						if (!player.m_bFake)
						{
							// tag bar
							SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y });
							if (BeginChild(std::format("TagBar{}", player.m_uFriendsID).c_str(), { flWidth - lOffset - H::Draw.Scale(bResolver ? 128 : 48), flHeight }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
							{
								std::vector<std::pair<PriorityLabel_t*, int>> vTags = {};
								if (player.m_bFriend)
									vTags.emplace_back(&F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)], 0);
								if (player.m_bParty)
									vTags.emplace_back(&F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)], 0);
								if (player.m_bF2P)
									vTags.emplace_back(&F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(F2P_TAG)], 0);
								for (auto& iID : F::PlayerUtils.m_mPlayerTags[player.m_uFriendsID])
								{
									auto pTag = F::PlayerUtils.GetTag(iID);
									if (pTag)
										vTags.emplace_back(pTag, iID);
								}

								PushFont(F::Render.FontSmall);
								const auto vDrawPos = GetDrawPos();
								float flTagOffset = 0;
								for (auto& [pTag, iID] : vTags)
								{
									ImColor tagColor = ColorToVec(pTag->Color);
									float flTagWidth = FCalcTextSize(pTag->Name.c_str()).x + H::Draw.Scale(!iID ? 10 : 25);
									float flTagHeight = H::Draw.Scale(20);
									ImVec2 vTagPos = { flTagOffset, H::Draw.Scale(4) };

									PushStyleColor(ImGuiCol_Text, IsColorDark(tagColor) ? ImVec4{ 1, 1, 1, 1 } : ImVec4{ 0, 0, 0, 1 });

									GetWindowDrawList()->AddRectFilled(vDrawPos + vTagPos, { vDrawPos.x + vTagPos.x + flTagWidth, vDrawPos.y + vTagPos.y + flTagHeight }, tagColor, H::Draw.Scale(3));
									SetCursorPos({ vTagPos.x + H::Draw.Scale(5), vTagPos.y + H::Draw.Scale(4) });
									FText(pTag->Name.c_str());
									SetCursorPos({ vTagPos.x + flTagWidth - H::Draw.Scale(18), vTagPos.y + H::Draw.Scale(2) });
									if (iID && IconButton(ICON_MD_CANCEL))
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
						if (FBeginPopup(std::format("Clicked{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							if (player.m_iLevel != -2)
							{
								std::string sLevel = "T? L?";
								if (player.m_iLevel != -1)
								{
									int iTier = std::ceil(player.m_iLevel / 150.f);
									int iLevel = ((player.m_iLevel - 1) % 150) + 1;
									sLevel = std::format("T{} L{}", iTier, iLevel);
								}
								PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
								FText(sLevel.c_str());
								PopStyleColor();
							}

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
						else if (FBeginPopup(std::format("Add{}", player.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
							{
								int iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);
								auto& tTag = *it;
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
						else if (FBeginPopup(std::format("Alias{}", player.m_uFriendsID).c_str()))
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
									F::Output.AliasChanged(player.m_sName, "Removed", F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID]);

									auto find = F::PlayerUtils.m_mPlayerAliases.find(player.m_uFriendsID);
									if (find != F::PlayerUtils.m_mPlayerAliases.end())
										F::PlayerUtils.m_mPlayerAliases.erase(find);
									F::PlayerUtils.m_bSave = true;
								}
								else
								{
									F::PlayerUtils.m_mPlayerAliases[player.m_uFriendsID] = sInput;
									F::PlayerUtils.m_bSave = true;

									F::Output.AliasChanged(player.m_sName, bHasAlias ? "Changed" : "Added", sInput);
								}
							}

							PopStyleVar();
							EndPopup();
						}
						else if (FBeginPopup(std::format("Yaw{}", player.m_uFriendsID).c_str()))
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
						else if (FBeginPopup(std::format("Pitch{}", player.m_uFriendsID).c_str()))
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
						else if (FBeginPopup(std::format("Minwalk{}", player.m_uFriendsID).c_str()))
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
						else if (FBeginPopup(std::format("View{}", player.m_uFriendsID).c_str()))
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
				if (BeginChild("Split1", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(64) }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
				{
					FSDropdown("Name", &tTag.Name, {}, FDropdown_Left | FSDropdown_AutoUpdate, -10);
					FColorPicker("Color", &tTag.Color, 0, FColorPicker_Dropdown);

					PushDisabled(iID == DEFAULT_TAG || iID == IGNORED_TAG);
					{
						int iLabel = Disabled ? 0 : tTag.Label;
						FDropdown("Type", &iLabel, { "Priority", "Label" }, {}, FDropdown_Right);
						tTag.Label = iLabel;
						if (Disabled)
							tTag.Label = false;
					}
					PopDisabled();
				} EndChild();

				SetCursorPos({ GetWindowWidth() / 2 - GetStyle().WindowPadding.x / 2, vOriginalPos.y - H::Draw.Scale(8) });
				if (BeginChild("Split2", { GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(64) }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
				{
					PushTransparent(tTag.Label); // transparent if we want a label, user can still use to sort
					{
						SetCursorPosY(GetCursorPos().y + H::Draw.Scale(12));
						FSlider("Priority", &tTag.Priority, -10, 10, 1, "%i", FSlider_Left);
					}
					PopTransparent();

					// create/modify button
					bool bCreate = false, bClear = false;

					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(96), H::Draw.Scale(8) });
					PushDisabled(tTag.Name.empty());
					{
						bCreate = FButton("##CreateButton", FButton_None, { 40, 40 });
					}
					PopDisabled();
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(84), H::Draw.Scale(28) });
					PushTransparent(tTag.Name.empty());
					{
						IconImage(iID != -1 ? ICON_MD_SETTINGS : ICON_MD_ADD);
					}
					PopTransparent();

					// clear button
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(48), H::Draw.Scale(8) });
					bClear = FButton("##ClearButton", FButton_None, { 40, 40 });
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(36), H::Draw.Scale(28) });
					IconImage(ICON_MD_CLEAR);

					if (bCreate)
					{
						F::PlayerUtils.m_bSave = true;
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

					ImVec2 vOriginalPos = { !_tTag.Label ? GetStyle().WindowPadding.x : GetWindowWidth() * 2 / 3 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(96 + 36 * y) };

					// background
					float flWidth = GetWindowWidth() * (_tTag.Label ? 1.f / 3 : 2.f / 3) - GetStyle().WindowPadding.x * 1.5f;
					float flHeight = H::Draw.Scale(28);
					ImColor imColor = ColorToVec(_tTag.Color.Lerp(Vars::Menu::Theme::Background.Value, 0.5f, false));
					ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
					GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, imColor, H::Draw.Scale(3));

					// text
					SetCursorPos({ vOriginalPos.x + H::Draw.Scale(10), vOriginalPos.y + H::Draw.Scale(7) });
					FText(TruncateText(_tTag.Name, _tTag.Label ? flWidth - H::Draw.Scale(38) : flWidth / 2 - H::Draw.Scale(20)).c_str());

					if (!_tTag.Label)
					{
						SetCursorPos({ vOriginalPos.x + flWidth / 2, vOriginalPos.y + H::Draw.Scale(7) });
						FText(std::format("{}", _tTag.Priority).c_str());
					}

					// buttons / icons
					SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(22), vOriginalPos.y + H::Draw.Scale(6) });
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
						case F2P_TAG: IconImage(ICON_MD_MONEY_OFF); break;
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
					if (FBeginPopupModal(std::format("Confirmation## DeleteTag{}", _iID).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText(std::format("Do you really want to delete '{}'?", _tTag.Name).c_str());

						if (FButton("Yes", FButton_Left))
						{
							F::PlayerUtils.m_vTags.erase(it);
							F::PlayerUtils.m_bSave = F::PlayerUtils.m_bSave = true;

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
					vPriorities.emplace_back(it, _tTag);
				else
					vLabels.emplace_back(it, _tTag);
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
		{
			PushDisabled(F::PlayerUtils.m_bLoad);
			{
				ImVec2 vOriginal = GetCursorPos();
				SetCursorPosY(GetCursorPosY() - 8);
				if (FButton("##RefreshButton", FButton_None, { 30, 30 }))
					F::PlayerUtils.m_bLoad = true;
				SetCursorPos(vOriginal + ImVec2(7, 7));
				IconImage(ICON_MD_SYNC);

				SetCursorPos(vOriginal + ImVec2(38, -8));
				if (FButton("Export", FButton_Fit))
				{
					// this should be up2date anyways
					std::ifstream file;
					file.open(F::Configs.m_sCorePath + "Players.json", std::ios_base::app);
					if (file.is_open())
					{
						std::string sString;
						{
							std::string line;
							while (std::getline(file, line))
								sString += line + "\n";
							if (!sString.empty())
								sString.pop_back();
						}
						file.close();

						SDK::SetClipboard(sString);
						SDK::Output("Amalgam", "Copied playerlist to clipboard", { 175, 150, 255 }, true, true, true);
					}
				}

				{
					static std::vector<PriorityLabel_t> vTags = {};
					static std::unordered_map<uint32_t, std::vector<int>> mPlayerTags = {};
					static std::unordered_map<uint32_t, std::string> mPlayerAliases = {};
					static std::unordered_map<int, int> mAs = {};

					if (FButton("Import", FButton_Fit | FButton_SameLine))
					{
						// just put it in a file so it will be easier to use with boost
						std::ofstream outStream(F::Configs.m_sCorePath + "Import.json");
						outStream << SDK::GetClipboard();
						outStream.close();

						try
						{
							// will not directly support older tag systems
							if (std::filesystem::exists(F::Configs.m_sCorePath + "Import.json"))
							{
								boost::property_tree::ptree readTree;
								read_json(F::Configs.m_sCorePath + "Import.json", readTree);

								mPlayerTags.clear();
								mPlayerAliases.clear();
								mAs.clear();
								vTags = {
									{ "Default", { 200, 200, 200, 255 }, 0, false, false, true },
									{ "Ignored", { 200, 200, 200, 255 }, -1, false, true, true },
									{ "Cheater", { 255, 100, 100, 255 }, 1, false, true, true },
									{ "Friend", { 100, 255, 100, 255 }, 0, true, false, true },
									{ "Party", { 100, 100, 255, 255 }, 0, true, false, true },
									{ "F2P", { 255, 255, 255, 255 }, 0, true, false, true }
								};

								if (auto configTree = readTree.get_child_optional("Config"))
								{
									for (auto& it : *configTree)
									{
										PriorityLabel_t tTag = {};
										if (auto getValue = it.second.get_optional<std::string>("Name")) { tTag.Name = *getValue; }
										if (const auto getChild = it.second.get_child_optional("Color")) { F::Configs.TreeToColor(*getChild, tTag.Color); }
										if (auto getValue = it.second.get_optional<int>("Priority")) { tTag.Priority = *getValue; }
										if (auto getValue = it.second.get_optional<bool>("Label")) { tTag.Label = *getValue; }

										int iID = -1;
										try
										{	// new id based indexing
											iID = std::stoi(it.first);
											iID = F::PlayerUtils.TagToIndex(iID);
										}
										catch (...) {}

										if (iID > -1 && iID < vTags.size())
										{
											vTags[iID].Name = tTag.Name;
											vTags[iID].Color = tTag.Color;
											vTags[iID].Priority = tTag.Priority;
											vTags[iID].Label = tTag.Label;
										}
										else
											vTags.push_back(tTag);
									}
								}

								if (auto tagTree = readTree.get_child_optional("Tags"))
								{
									for (auto& player : *tagTree)
									{
										uint32_t friendsID = std::stoi(player.first);

										for (auto& tag : player.second)
										{
											std::string sTag = tag.second.data();

											int iID = -1;
											try
											{	// new id based indexing
												iID = std::stoi(sTag);
												iID = F::PlayerUtils.TagToIndex(iID);
											}
											catch (...) {}
											if (iID == -1)
												continue;

											auto pTag = F::PlayerUtils.GetTag(iID);
											if (!pTag || !pTag->Assignable)
												continue;

											if (!F::PlayerUtils.HasTag(friendsID, iID, mPlayerTags))
												F::PlayerUtils.AddTag(friendsID, iID, false, "", mPlayerTags);
										}
									}
								}

								if (auto aliasTree = readTree.get_child_optional("Aliases"))
								{
									for (auto& player : *aliasTree)
									{
										uint32_t friendsID = std::stoi(player.first);
										std::string sAlias = player.second.data();

										if (!sAlias.empty())
											mPlayerAliases[friendsID] = player.second.data();
									}
								}
							}

							//std::filesystem::remove(F::Configs.m_sCorePath + "Import.json");

							for (int i = 0; i < vTags.size(); i++)
							{
								if (vTags[i].Assignable)
								{
									if (F::PlayerUtils.IndexToTag(i) <= 0)
										mAs[i] = i;
									else
										mAs[i] = -1;
								}
							}
							OpenPopup("Import playerlist");
						}
						catch (...)
						{
							SDK::Output("Amalgam", "Failed to import playerlist", { 175, 150, 255 }, true, true, true);
						}
					}

					SetNextWindowSize({ H::Draw.Scale(300), 0 });
					if (FBeginPopupModal("Import playerlist", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText("Import");
						FText("As", FText_Right | FText_SameLine);

						for (int i = 0; i < vTags.size(); i++)
						{
							if (!vTags[i].Assignable)
								continue;

							auto& iIDTo = mAs[i];

							ImVec2 vOriginalPos = GetCursorPos();
							PushStyleColor(ImGuiCol_Text, ColorToVec(vTags[i].Color));
							SetCursorPos(vOriginalPos + ImVec2(H::Draw.Scale(8), H::Draw.Scale(5)));
							FText(vTags[i].Name.c_str());
							PopStyleColor();
							SetCursorPos(vOriginalPos - ImVec2(0, H::Draw.Scale(8))); DebugDummy({ GetWindowWidth() - GetStyle().WindowPadding.x * 2, H::Draw.Scale(32) });

							std::vector<const char*> vEntries = { "None" };
							std::vector<int> vValues = { 0 };
							for (int i = 0; i < F::PlayerUtils.m_vTags.size(); i++)
							{
								if (F::PlayerUtils.m_vTags[i].Assignable)
								{
									vEntries.push_back(F::PlayerUtils.m_vTags[i].Name.c_str());
									vValues.push_back(i + 1);
								}
							}
							PushTransparent(iIDTo == -1);
							{
								int iTo = iIDTo + 1;
								FDropdown(std::format("##{}", i).c_str(), &iTo, vEntries, vValues, FSlider_Right);
								iIDTo = iTo - 1;
							}
							PopTransparent();
						}

						if (FButton("Import", FButton_Left))
						{
							for (auto& [friendsID, vTags] : mPlayerTags)
							{
								for (auto& iTag : vTags)
								{
									int iID = mAs.contains(iTag) ? mAs[iTag] : -1;
									if (iID != -1 && !F::PlayerUtils.HasTag(friendsID, iID))
										F::PlayerUtils.AddTag(friendsID, iID, false);
								}
							}
							for (auto& [friendsID, sAlias] : mPlayerAliases)
							{
								if (!F::PlayerUtils.m_mPlayerAliases.contains(friendsID))
									F::PlayerUtils.m_mPlayerAliases[friendsID] = sAlias;
							}
							
							F::PlayerUtils.m_bSave = true;
							SDK::Output("Amalgam", "Imported playerlist", { 175, 150, 255 }, true, true, true);

							CloseCurrentPopup();
						}
						if (FButton("Cancel", FButton_Right | FButton_SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
				}

				if (FButton("Backup", FButton_Fit | FButton_SameLine))
				{
					try
					{
						int iBackupCount = 0;
						for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sCorePath))
						{
							if (!entry.is_regular_file() || entry.path().extension() != F::Configs.m_sConfigExtension)
								continue;

							std::string sConfigName = entry.path().filename().string();
							sConfigName.erase(sConfigName.end() - F::Configs.m_sConfigExtension.size(), sConfigName.end());
							if (sConfigName.find("Backup") != std::string::npos)
								iBackupCount++;
						}
						std::filesystem::copy(
							F::Configs.m_sCorePath + "Players.json",
							F::Configs.m_sCorePath + std::format("Backup{}.json", iBackupCount + 1),
							std::filesystem::copy_options::overwrite_existing
						);
						SDK::Output("Amalgam", "Saved backup playerlist", { 175, 150, 255 }, true, true, true);
					}
					catch (...)
					{
						SDK::Output("Amalgam", "Failed to backup playerlist", { 175, 150, 255 }, true, true, true);
					}
				}
			}
			PopDisabled();

			if (FButton("Folder", FButton_Fit | FButton_SameLine))
				ShellExecuteA(NULL, NULL, F::Configs.m_sCorePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
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
				FSDropdown("Material name", &newName, {}, FDropdown_Left | FSDropdown_AutoUpdate);
				if (FButton("Create", FButton_Fit | FButton_SameLine, { 0, 40 }) && newName.length() > 0)
				{
					F::Materials.AddMaterial(newName.c_str());
					newName.clear();
				}

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

				for (auto& tMaterial : vMaterials)
				{
					ImVec2 vOriginalPos = GetCursorPos();

					SetCursorPos({ H::Draw.Scale(14), vOriginalPos.y + H::Draw.Scale(11) });
					TextColored(tMaterial.m_bLocked ? F::Render.Inactive.Value : F::Render.Active.Value, TruncateText(tMaterial.m_sName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(56)).c_str());

					int iOffset = 0;
					if (!tMaterial.m_bLocked)
					{
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteMat{}", tMaterial.m_sName).c_str());
						if (FBeginPopupModal(std::format("Confirmation## DeleteMat{}", tMaterial.m_sName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("Do you really want to delete '{}'?", tMaterial.m_sName).c_str());

							if (FButton("Yes", FButton_Left))
							{
								F::Materials.RemoveMaterial(tMaterial.m_sName.c_str());
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
						CurrentMaterial = tMaterial.m_sName;
						LockedMaterial = tMaterial.m_bLocked;

						TextEditor.SetText(F::Materials.GetVMT(FNV1A::Hash32(CurrentMaterial.c_str())));
						TextEditor.SetReadOnlyEnabled(LockedMaterial);
					}

					SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
				}
			} EndSection();
			SetCursorPosY(GetCursorPosY() - 8);
			if (FButton("Folder", FButton_Fit))
				ShellExecuteA(NULL, NULL, F::Configs.m_sMaterialsPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

			/* Column 2 */
			TableNextColumn();
			if (CurrentMaterial.length())
			{
				SetCursorPosY(GetScrollY() + GetStyle().WindowPadding.y);
				if (Section("Editor", false, GetWindowHeight() - GetStyle().WindowPadding.y * 2, true))
				{
					// Toolbar
					if (!LockedMaterial)
					{
						if (FButton("Save", FButton_Fit))
						{
							auto sText = TextEditor.GetText();
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
					TextEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cpp);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::Background, F::Render.Background1);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::Default, F::Render.Active);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::Identifier, F::Render.Active);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::Cursor, F::Render.Active);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::LineNumber, F::Render.Inactive);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::Comment, F::Render.Inactive);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::MultiLineComment, F::Render.Inactive);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::Punctuation, F::Render.Inactive);
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::ControlCharacter, ImColor(F::Render.Inactive.Value.x, F::Render.Inactive.Value.y, F::Render.Inactive.Value.z, 0.1f));
					TextEditor.SetPaletteIndex(TextEditor::PaletteIndex::String, F::Render.Accent);
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

struct DragBoxStorage_t
{
	DragBox_t m_tDragBox;
	float m_flScale;
};
static std::unordered_map<uint32_t, DragBoxStorage_t> mDragBoxStorage = {};
void CMenu::AddDraggable(const char* sLabel, ConfigVar<DragBox_t>& var, bool bShouldDraw, ImVec2 vSize)
{
	using namespace ImGui;

	if (!bShouldDraw)
		return;

	auto tDragBox = FGet(var, true);
	auto uHash = FNV1A::Hash32(sLabel);

	bool bContains = mDragBoxStorage.contains(uHash);
	auto& tStorage = mDragBoxStorage[uHash];

	SetNextWindowSize(vSize, ImGuiCond_Always);
	if (!bContains || tDragBox != tStorage.m_tDragBox || H::Draw.Scale() != tStorage.m_flScale)
		SetNextWindowPos({ float(tDragBox.x - vSize.x / 2), float(tDragBox.y) }, ImGuiCond_Always);

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, H::Draw.Scale(3));
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, H::Draw.Scale(1));
	PushStyleVar(ImGuiStyleVar_WindowMinSize, vSize);
	if (Begin(sLabel, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImVec2 vWindowPos = GetWindowPos();

		tDragBox.x = vWindowPos.x + vSize.x / 2, tDragBox.y = vWindowPos.y;
		tStorage = { tDragBox, H::Draw.Scale() };
		FSet(var, tDragBox);

		PushFont(F::Render.FontBold);
		ImVec2 vTextSize = FCalcTextSize(sLabel);
		SetCursorPos({ (vSize.x - vTextSize.x) * 0.5f, (vSize.y - vTextSize.y) * 0.5f });
		FText(sLabel);
		PopFont();

		End();
	}
	PopStyleVar(3);
	PopStyleColor(2);
}

struct WindowBoxStorage_t
{
	WindowBox_t m_tWindowBox;
	float m_flScale;
};
static std::unordered_map<uint32_t, WindowBoxStorage_t> mWindowBoxStorage = {};
void CMenu::AddResizableDraggable(const char* sLabel, ConfigVar<WindowBox_t>& var, bool bShouldDraw, ImVec2 vMinSize, ImVec2 vMaxSize, ImGuiSizeCallback fCustomCallback)
{
	using namespace ImGui;

	if (!bShouldDraw)
		return;

	auto tWindowBox = FGet(var, true);
	auto uHash = FNV1A::Hash32(sLabel);

	bool bContains = mWindowBoxStorage.contains(uHash);
	auto& tStorage = mWindowBoxStorage[uHash];

	SetNextWindowSizeConstraints(vMinSize, vMaxSize, fCustomCallback);
	if (!bContains || tWindowBox != tStorage.m_tWindowBox || H::Draw.Scale() != tStorage.m_flScale)
	{
		SetNextWindowPos({ float(tWindowBox.x - tWindowBox.w / 2), float(tWindowBox.y) }, ImGuiCond_Always);
		SetNextWindowSize({ float(tWindowBox.w), float(tWindowBox.h) }, ImGuiCond_Always);
	}

	PushStyleColor(ImGuiCol_WindowBg, {});
	PushStyleColor(ImGuiCol_Border, F::Render.Active.Value);
	PushStyleVar(ImGuiStyleVar_WindowRounding, H::Draw.Scale(3));
	PushStyleVar(ImGuiStyleVar_WindowBorderSize, H::Draw.Scale(1));
	if (Begin(sLabel, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImVec2 vWindowPos = GetWindowPos();
		ImVec2 vWinSize = GetWindowSize();

		tWindowBox.w = vWinSize.x, tWindowBox.h = vWinSize.y;
		tWindowBox.x = vWindowPos.x + tWindowBox.w / 2, tWindowBox.y = vWindowPos.y;
		tStorage = { tWindowBox, H::Draw.Scale() };
		FSet(var, tWindowBox);

		PushFont(F::Render.FontBold);
		ImVec2 vTextSize = FCalcTextSize(sLabel);
		SetCursorPos({ (vWinSize.x - vTextSize.x) * 0.5f, (vWinSize.y - vTextSize.y) * 0.5f });
		FText(sLabel);
		PopFont();

		End();
	}
	PopStyleVar(2);
	PopStyleColor(2);
}

struct BindInfo_t
{
	const char* sName;
	std::string sInfo;
	std::string sState;

	int iBind;
	Bind_t& tBind;
};
void CMenu::DrawBinds()
{
	using namespace ImGui;

	if (m_bIsOpen ? false : I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible() && !I::EngineClient->IsPlayingDemo())
		return;

	std::vector<BindInfo_t> vInfo;
	std::function<void(int)> getBinds = [&](int iParent)
		{
			for (auto it = F::Binds.m_vBinds.begin(); it < F::Binds.m_vBinds.end(); it++)
			{
				int iBind = std::distance(F::Binds.m_vBinds.begin(), it);
				auto& tBind = *it;
				if (iParent != tBind.m_iParent || !tBind.m_bEnabled && !m_bIsOpen)
					continue;

				if (tBind.m_bVisible || m_bIsOpen)
				{
					std::string sType; std::string sInfo;
					switch (tBind.m_iType)
					{
					case BindEnum::Key:
						switch (tBind.m_iInfo)
						{
						case BindEnum::KeyEnum::Hold: { sType = "hold"; break; }
						case BindEnum::KeyEnum::Toggle: { sType = "toggle"; break; }
						case BindEnum::KeyEnum::DoubleClick: { sType = "double"; break; }
						}
						sInfo = VK2STR(tBind.m_iKey);
						break;
					case BindEnum::Class:
						sType = "class";
						switch (tBind.m_iInfo)
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
						switch (tBind.m_iInfo)
						{
						case BindEnum::WeaponTypeEnum::Hitscan: { sInfo = "hitscan"; break; }
						case BindEnum::WeaponTypeEnum::Projectile: { sInfo = "projectile"; break; }
						case BindEnum::WeaponTypeEnum::Melee: { sInfo = "melee"; break; }
						}
						break;
					case BindEnum::ItemSlot:
						sType = "slot";
						sInfo = std::format("{}", tBind.m_iInfo + 1);
						break;
					}
					if (tBind.m_bNot && (tBind.m_iType != BindEnum::Key || tBind.m_iInfo == BindEnum::KeyEnum::Hold))
						sInfo = std::format("not {}", sInfo);

					vInfo.emplace_back(tBind.m_sName.c_str(), sType, sInfo, iBind, tBind);
				}

				if (tBind.m_bActive || m_bIsOpen)
					getBinds(iBind);
			}
		};
	getBinds(DEFAULT_BIND);
	if (vInfo.empty())
		return;

	static DragBox_t old = { -2147483648, -2147483648 };
	DragBox_t info = m_bIsOpen ? FGet(Vars::Menu::BindsDisplay, true) : Vars::Menu::BindsDisplay.Value;
	if (info != old)
		SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);

	float flNameWidth = 0, flInfoWidth = 0, flStateWidth = 0;
	PushFont(F::Render.FontSmall);
	for (auto& [sName, sInfo, sState, iBind, tBind] : vInfo)
	{
		flNameWidth = std::max(flNameWidth, FCalcTextSize(sName).x);
		flInfoWidth = std::max(flInfoWidth, FCalcTextSize(sInfo.c_str()).x);
		flStateWidth = std::max(flStateWidth, FCalcTextSize(sState.c_str()).x);
	}
	PopFont();
	flNameWidth += H::Draw.Scale(9), flInfoWidth += H::Draw.Scale(9), flStateWidth += H::Draw.Scale(9);

	float flWidth = flNameWidth + flInfoWidth + flStateWidth + (m_bIsOpen ? H::Draw.Scale(113) : H::Draw.Scale(14));
	float flHeight = H::Draw.Scale(18 * vInfo.size() + (Vars::Menu::BindWindowTitle.Value ? 42 : 12));
	SetNextWindowSize({ flWidth, flHeight });
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(40), H::Draw.Scale(40) });
	if (Begin("Binds", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImVec2 vWindowPos = GetWindowPos();

		if (Vars::Menu::BindWindowTitle.Value)
			RenderTwoToneBackground(H::Draw.Scale(28), F::Render.Background0, F::Render.Background0p5, F::Render.Background2);
		else
			RenderBackground(F::Render.Background0p5, F::Render.Background2);

		info.x = vWindowPos.x; info.y = vWindowPos.y; old = info;
		if (m_bIsOpen)
			FSet(Vars::Menu::BindsDisplay, info);

		int iListStart = 8;
		if (Vars::Menu::BindWindowTitle.Value)
		{
			SetCursorPos({ H::Draw.Scale(8), H::Draw.Scale(6) });
			IconImage(ICON_MD_KEYBOARD);
			PushFont(F::Render.FontLarge);
			SetCursorPos({ H::Draw.Scale(30), H::Draw.Scale(7) });
			FText("Binds");
			PopFont();

			iListStart = 36;
		}

		PushFont(F::Render.FontSmall);
		int i = 0; for (auto& [sName, sInfo, sState, iBind, tBind] : vInfo)
		{
			float flPosX = 0;

			if (m_bIsOpen)
				PushStyleVar(ImGuiStyleVar_Alpha, F::Binds.WillBeEnabled(iBind) ? 1.f : 0.5f);

			SetCursorPos({ flPosX += H::Draw.Scale(12), H::Draw.Scale(iListStart + 18 * i) });
			PushStyleColor(ImGuiCol_Text, tBind.m_bActive ? F::Render.Accent.Value : F::Render.Inactive.Value);
			FText(sName);
			PopStyleColor();

			SetCursorPos({ flPosX += flNameWidth, H::Draw.Scale(iListStart + 18 * i) });
			PushStyleColor(ImGuiCol_Text, tBind.m_bActive ? F::Render.Active.Value : F::Render.Inactive.Value);
			FText(sInfo.c_str());

			SetCursorPos({ flPosX += flInfoWidth, H::Draw.Scale(iListStart + 18 * i) });
			FText(sState.c_str());
			PopStyleColor();

			if (m_bIsOpen)
			{	// buttons
				SetCursorPos({ flWidth - H::Draw.Scale(25), H::Draw.Scale(iListStart - 1 + 18 * i) });
				bool bDelete = IconButton(ICON_MD_DELETE);

				SetCursorPos({ flWidth - H::Draw.Scale(50), H::Draw.Scale(iListStart - 1 + 18 * i) });
				bool bNot = IconButton(!tBind.m_bNot ? ICON_MD_CODE : ICON_MD_CODE_OFF);

				SetCursorPos({ flWidth - H::Draw.Scale(75), H::Draw.Scale(iListStart - 1 + 18 * i) });
				bool bVisibility = IconButton(tBind.m_bVisible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF);

				SetCursorPos({ flWidth - H::Draw.Scale(100), H::Draw.Scale(iListStart - 1 + 18 * i) });
				bool bEnable = IconButton(tBind.m_bEnabled ? ICON_MD_RADIO_BUTTON_ON : ICON_MD_RADIO_BUTTON_OFF);

				PushFont(F::Render.FontRegular);
				PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });

				if (bDelete)
				{
					if (U::KeyHandler.Down(VK_SHIFT)) // allow user to quickly remove binds
						F::Binds.RemoveBind(iBind);
					else
						OpenPopup(std::format("Confirmation## DeleteBind{}", iBind).c_str());
				}
				if (FBeginPopupModal(std::format("Confirmation## DeleteBind{}", iBind).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
				{
					FText(std::format("Do you really want to delete '{}'{}?", tBind.m_sName, F::Binds.HasChildren(iBind) ? " and all of its children" : "").c_str());

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
				if (bEnable)
					tBind.m_bEnabled = !tBind.m_bEnabled;
				if (bVisibility)
					tBind.m_bVisible = !tBind.m_bVisible;
				if (bNot)
					tBind.m_bNot = !tBind.m_bNot;

				PopStyleVar();
				PopFont();
			}

			if (m_bIsOpen)
				PopStyleVar();

			i++;
		}
		PopFont();

		End();
	}
	PopStyleVar();
}

static inline void SquareConstraints(ImGuiSizeCallbackData* data)
{
	//data->DesiredSize.x = data->DesiredSize.y = std::max(data->DesiredSize.x, data->DesiredSize.y);
	data->DesiredSize.x = data->DesiredSize.y = (data->DesiredSize.x + data->DesiredSize.y) / 2;
}

void CMenu::Render()
{
	using namespace ImGui;

	for (int iKey = 0; iKey < 256; iKey++)
		U::KeyHandler.StoreKey(iKey);

	if (!F::Configs.m_bConfigLoaded || !(ImGui::GetIO().DisplaySize.x > 160.f && ImGui::GetIO().DisplaySize.y > 28.f))
		return;

	m_bInKeybind = false;
	if (U::KeyHandler.Pressed(Vars::Menu::MenuPrimaryKey.Value) || U::KeyHandler.Pressed(Vars::Menu::MenuSecondaryKey.Value))
		I::MatSystemSurface->SetCursorAlwaysVisible(m_bIsOpen = !m_bIsOpen);

	PushFont(F::Render.FontRegular);

	DrawBinds();
	if (m_bIsOpen)
	{
		DrawMenu();

		AddDraggable("Ticks", Vars::Menu::TicksDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Ticks);
		AddDraggable("Crit hack", Vars::Menu::CritsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::CritHack);
		AddDraggable("Spectators", Vars::Menu::SpectatorsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Spectators);
		AddDraggable("Ping", Vars::Menu::PingDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Ping);
		AddDraggable("Conditions", Vars::Menu::ConditionsDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::Conditions);
		AddDraggable("Seed prediction", Vars::Menu::SeedPredictionDisplay, FGet(Vars::Menu::Indicators) & Vars::Menu::IndicatorsEnum::SeedPrediction);
		AddResizableDraggable("Camera", Vars::Visuals::Simulation::ProjectileWindow, FGet(Vars::Visuals::Simulation::ProjectileCamera));
		AddResizableDraggable("Radar", Vars::Radar::Main::Window, FGet(Vars::Radar::Main::Enabled), { H::Draw.Scale(100), H::Draw.Scale(100) }, { H::Draw.Scale(1000), H::Draw.Scale(1000) }, SquareConstraints);

		F::Render.Cursor = GetMouseCursor();

		if (!vDisabled.empty())
		{
			IM_ASSERT_USER_ERROR(0, "Calling PopDisabled() too little times: stack overflow.");
			Disabled = false;
			vDisabled.clear();
		}
		if (!vTransparent.empty())
		{
			IM_ASSERT_USER_ERROR(0, "Calling PopTransparent() too little times: stack overflow.");
			Transparent = false;
			vTransparent.clear();
		}
	}
	else
		mActiveMap.clear();

	PopFont();
}

void CMenu::AddOutput(const std::string& sFunction, const std::string& sLog, const Color_t& tColor)
{
	static size_t iID = 0;

	m_vOutput.emplace_back(sFunction, sLog, iID++, tColor);
	while (m_vOutput.size() > m_iMaxOutputSize)
		m_vOutput.pop_front();
}