#include "Menu.h"

#include "Components.h"
#include "../../Configs/Configs.h"
#include "../../Binds/Binds.h"
#include "../../Visuals/Groups/Groups.h"
#include "../../Players/PlayerUtils.h"
#include "../../Spectate/Spectate.h"
#include "../../Resolver/Resolver.h"
#include "../../Visuals/Visuals.h"
#include "../../Misc/Misc.h"
#include "../../Output/Output.h"

void CMenu::DrawMenu()
{
	using namespace ImGui;

	static bool bSetPosition = false;
	if (!bSetPosition)
	{
		SetNextWindowPos((GetIO().DisplaySize - ImVec2(H::Draw.Scale(750), H::Draw.Scale(500))) / 2, ImGuiCond_FirstUseEver);
		SetNextWindowSize({ H::Draw.Scale(750), H::Draw.Scale(500) }, ImGuiCond_FirstUseEver);
		bSetPosition = true;
	}

	PushStyleVar(ImGuiStyleVar_WindowMinSize, { H::Draw.Scale(750), H::Draw.Scale(500) });
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

			SetCursorPos({ H::Draw.Scale(flSideSize - 31), H::Draw.Scale(6) });
			if (IconButton(ICON_MD_CANCEL))
				CurrentBind = DEFAULT_BIND;
		}
		else if (!Vars::Menu::CheatTitle.Value.empty()) // title
		{
			flOffset = H::Draw.Scale(36);
			pDrawList->AddRectFilled({ vDrawPos.x, vDrawPos.y + H::Draw.Scale(35) }, { vDrawPos.x + H::Draw.Scale(flSideSize - 1), vDrawPos.y + H::Draw.Scale(36) }, F::Render.Background2);
			
			SetCursorPos({ H::Draw.Scale(12), H::Draw.Scale(11) });
			PushStyleColor(ImGuiCol_Text, F::Render.Accent.Value);
			FText(TruncateText(Vars::Menu::CheatTitle.Value, H::Draw.Scale(flSideSize - 28), F::Render.FontBold).c_str(), 0, F::Render.FontBold);
			PopStyleColor();
		}

		static int iTab = 0, iAimbotTab = 0, iVisualsTab = 0, iMiscTab = 0, iLogsTab = 0, iSettingsTab = 0;
		PushFont(F::Render.FontBold);
		FTabs(
			{
				{ "AIMBOT", "GENERAL", "HVH", "DRAW" },
				{ "VISUALS", "ESP", "CHAMS", "GLOW", "MISC##", "RADAR", "MENU" },
				{ "MISC" },
				{ "LOGS", "PLAYERLIST", "SETTINGS##", "OUTPUT" },
				{ "SETTINGS", "CONFIG", "BINDS", "MATERIALS", "EXTRA" }
			},
			{ &iTab, &iAimbotTab, &iVisualsTab, nullptr, &iLogsTab, &iSettingsTab },
			{ H::Draw.Scale(flSideSize - 16), H::Draw.Scale(36) },
			{ H::Draw.Scale(8), H::Draw.Scale(8) + flOffset },
			FTabsEnum::Vertical | FTabsEnum::HorizontalIcons | FTabsEnum::AlignLeft | FTabsEnum::BarLeft,
			{ { ICON_MD_PERSON }, { ICON_MD_VISIBILITY }, { ICON_MD_ARTICLE }, { ICON_MD_IMPORT_CONTACTS }, { ICON_MD_SETTINGS } },
			{ H::Draw.Scale(10), 0 }, {},
			{}, { H::Draw.Scale(22), 0 }
		);
		PopFont();

		static std::string sSearch = "";
		SetCursorPos({ H::Draw.Scale(8), vWindowSize.y - H::Draw.Scale(37) });
		FInputText("Search...", sSearch, H::Draw.Scale(123), ImGuiInputTextFlags_None);
		bool bSearch = /*IsItemFocused() ||*/ !sSearch.empty();
		if (!bSearch || FCalcTextSize(sSearch.c_str()).x < 86.f)
		{
			SetCursorPos({ H::Draw.Scale(109), vWindowSize.y - H::Draw.Scale(31) });
			IconImage(ICON_MD_SEARCH);
		}
		if (bSearch && IsMouseReleased(ImGuiMouseButton_Left) && IsMouseWithin(vDrawPos.x, vDrawPos.y, H::Draw.Scale(140), vWindowSize.y - H::Draw.Scale(45)))
			sSearch = "";

		SetCursorPos({ H::Draw.Scale(flSideSize), 0 });
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		PushStyleVar(ImGuiStyleVar_WindowPadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
		if (BeginChild("Page", { vWindowSize.x - H::Draw.Scale(flSideSize), vWindowSize.y }, ImGuiChildFlags_AlwaysUseWindowPadding))
		{
			if (!bSearch)
			{
				switch (iTab)
				{
				case 0: MenuAimbot(iAimbotTab); break;
				case 1: MenuVisuals(iVisualsTab); break;
				case 2: MenuMisc(iMiscTab); break;
				case 3: MenuLogs(iLogsTab); break;
				case 4: MenuSettings(iSettingsTab); break;
				}
			}
			else
				MenuSearch(sSearch);
		} EndChild();
		PopStyleVar(2);

		End();
	}
	PopStyleVar();
}

#pragma region Tabs
void CMenu::MenuAimbot(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	// General
	case 0:
	{
		if (BeginTable("AimbotTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("General"))
				{
					FDropdown(Vars::Aimbot::General::AimType, FDropdownEnum::Left);
					FDropdown(Vars::Aimbot::General::TargetSelection, FDropdownEnum::Right);
					FDropdown(Vars::Aimbot::General::Target, FDropdownEnum::Left);
					FDropdown(Vars::Aimbot::General::Ignore, FDropdownEnum::Right);
					FSlider(Vars::Aimbot::General::AimFOV);
					FSlider(Vars::Aimbot::General::MaxTargets, FSliderEnum::Left);
					PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Cloaked));
					{
						FSlider(Vars::Aimbot::General::IgnoreCloak, FSliderEnum::Right);
					}
					PopTransparent();
					FSlider(Vars::Aimbot::General::AssistStrength, FSliderEnum::Left);
					PushTransparent(!(FGet(Vars::Aimbot::General::Ignore) & Vars::Aimbot::General::IgnoreEnum::Unsimulated));
					{
						FSlider(Vars::Aimbot::General::TickTolerance, FSliderEnum::Right);
					}
					PopTransparent();
					FColorPicker(Vars::Colors::FOVCircle);
					FToggle(Vars::Aimbot::General::AutoShoot, FToggleEnum::Left);
					FToggle(Vars::Aimbot::General::FOVCircle, FToggleEnum::Right);
					FToggle(Vars::CritHack::ForceCrits, FToggleEnum::Left);
					FToggle(Vars::CritHack::AvoidRandomCrits, FToggleEnum::Right);
					FToggle(Vars::CritHack::AlwaysMeleeCrit, FToggleEnum::Left);
					FToggle(Vars::Aimbot::General::NoSpread, FToggleEnum::Right);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug Aimbot"))
					{
						FSlider(Vars::Aimbot::General::HitscanPeek);
						FToggle(Vars::Aimbot::General::PeekDTOnly, FToggleEnum::Left, &Hovered); FTooltip("This should probably stay on if you want to be able to target hitboxes other than the highest priority one", Hovered);
						FSlider(Vars::Aimbot::General::NoSpreadOffset);
						FSlider(Vars::Aimbot::General::NoSpreadAverage);
						FSlider(Vars::Aimbot::General::NoSpreadInterval);
						FSlider(Vars::Aimbot::General::NoSpreadBackupInterval);
						FDropdown(Vars::Aimbot::General::AimHoldsFire);
					} EndSection();
				}
				if (Section("Backtrack", 8))
				{
					FSlider(Vars::Backtrack::Latency);
					FSlider(Vars::Backtrack::Interp);
					FSlider(Vars::Backtrack::Window);
					//FToggle(Vars::Backtrack::PreferOnShot);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug Backtrack"))
					{
						FSlider(Vars::Backtrack::Offset);
					} EndSection();
				}
				if (Section("Healing", 8))
				{
					FToggle(Vars::Aimbot::Healing::AutoHeal, FToggleEnum::Left);
					FToggle(Vars::Aimbot::Healing::FriendsOnly, FToggleEnum::Right);
					FToggle(Vars::Aimbot::Healing::AutoVaccinator, FToggleEnum::Left);
					FToggle(Vars::Aimbot::Healing::ActivateOnVoice, FToggleEnum::Right);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug Healing"))
					{
						FSlider(Vars::Aimbot::Healing::AutoVaccinatorBulletScale);
						FSlider(Vars::Aimbot::Healing::AutoVaccinatorBlastScale);
						FSlider(Vars::Aimbot::Healing::AutoVaccinatorFireScale);
					} EndSection();
				}
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Hitscan"))
				{
					FDropdown(Vars::Aimbot::Hitscan::Hitboxes, FDropdownEnum::Left);
					FDropdown(Vars::Aimbot::Hitscan::Modifiers, FDropdownEnum::Right);
					FSlider(Vars::Aimbot::Hitscan::PointScale);
					PushTransparent(!(FGet(Vars::Aimbot::Hitscan::Modifiers) & Vars::Aimbot::Hitscan::ModifiersEnum::Tapfire));
					{
						//FSlider(Vars::Aimbot::Hitscan::TapFireDist);
						FSlider("Tapfire distance", &Vars::Aimbot::Hitscan::TapFireDist[DEFAULT_BIND], 0.f, 1000.f);
					}
					PopTransparent();
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug Hitscan"))
					{
						FSlider(Vars::Aimbot::Hitscan::BoneSizeSubtract);
						FSlider(Vars::Aimbot::Hitscan::BoneSizeMinimumScale);
					} EndSection();
				}
				if (Section("Projectile"))
				{
					FDropdown(Vars::Aimbot::Projectile::StrafePrediction, FDropdownEnum::Left);
					FDropdown(Vars::Aimbot::Projectile::SplashPrediction, FDropdownEnum::Right);
					FDropdown(Vars::Aimbot::Projectile::AutoDetonate, FDropdownEnum::Left);
					FDropdown(Vars::Aimbot::Projectile::AutoAirblast, FDropdownEnum::Right);
					FDropdown(Vars::Aimbot::Projectile::Hitboxes, FDropdownEnum::Left);
					FDropdown(Vars::Aimbot::Projectile::Modifiers, FDropdownEnum::Right);
					FSlider(Vars::Aimbot::Projectile::MaxSimulationTime, FSliderEnum::Left);
					PushTransparent(!FGet(Vars::Aimbot::Projectile::StrafePrediction));
					{
						FSlider(Vars::Aimbot::Projectile::HitChance, FSliderEnum::Right);
					}
					PopTransparent();
					FSlider(Vars::Aimbot::Projectile::AutodetRadius, FSliderEnum::Left);
					FSlider(Vars::Aimbot::Projectile::SplashRadius, FSliderEnum::Right);
					PushTransparent(!FGet(Vars::Aimbot::Projectile::AutoRelease));
					{
						FSlider(Vars::Aimbot::Projectile::AutoRelease);
					}
					PopTransparent();
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug Projectile"))
					{
						FText("Ground");
						FSlider(Vars::Aimbot::Projectile::GroundSamples, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::GroundStraightFuzzyValue, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::GroundLowMinimumSamples, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::GroundHighMinimumSamples, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::GroundLowMinimumDistance, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::GroundHighMinimumDistance, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::GroundMaxChanges, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::GroundMaxChangeTime, FSliderEnum::Right);

						FText("\nAir");
						FSlider(Vars::Aimbot::Projectile::AirSamples, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::AirStraightFuzzyValue, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::AirLowMinimumSamples, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::AirHighMinimumSamples, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::AirLowMinimumDistance, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::AirHighMinimumDistance, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::AirMaxChanges, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::AirMaxChangeTime, FSliderEnum::Right);

						FText("");
						FSlider(Vars::Aimbot::Projectile::VelocityAverageCount, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::VerticalShift, FSliderEnum::Right);

						FSlider(Vars::Aimbot::Projectile::DragOverride, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::TimeOverride, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::HuntsmanLerp, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::HuntsmanLerpLow, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::HuntsmanAdd, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::HuntsmanAddLow, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::HuntsmanClamp, FSliderEnum::Left);
						FToggle(Vars::Aimbot::Projectile::HuntsmanPullPoint, FToggleEnum::Right);
						SetCursorPosY(GetCursorPosY() + 8);

						FSlider(Vars::Aimbot::Projectile::SplashPoints, FSliderEnum::Left);
						FToggle(Vars::Aimbot::Projectile::SplashGrates, FToggleEnum::Right);
						FSlider(Vars::Aimbot::Projectile::SplashRotateX, FSliderEnum::Left, Vars::Aimbot::Projectile::SplashRotateX[DEFAULT_BIND] < 0.f ? "random" : "%g");
						FSlider(Vars::Aimbot::Projectile::SplashRotateY, FSliderEnum::Right, Vars::Aimbot::Projectile::SplashRotateY[DEFAULT_BIND] < 0.f ? "random" : "%g");
						FSlider(Vars::Aimbot::Projectile::SplashNthRoot);
						FSlider(Vars::Aimbot::Projectile::SplashCountDirect, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::SplashCountArc, FSliderEnum::Right);
						FSlider(Vars::Aimbot::Projectile::SplashTraceInterval, FSliderEnum::Left);
						FSlider(Vars::Aimbot::Projectile::SplashNormalSkip, FSliderEnum::Right);
						FDropdown(Vars::Aimbot::Projectile::SplashMode, FDropdownEnum::Left);
						FDropdown(Vars::Aimbot::Projectile::RocketSplashMode, FDropdownEnum::Right, 0, &Hovered); FTooltip("Special splash type for rockets, more expensive", Hovered);
						FSlider(Vars::Aimbot::Projectile::DeltaCount, FSliderEnum::Left);
						FDropdown(Vars::Aimbot::Projectile::DeltaMode, FDropdownEnum::Right);
						FDropdown(Vars::Aimbot::Projectile::MovesimFrictionFlags);
					} EndSection();
				}
				if (Section("Melee", 8))
				{
					FToggle(Vars::Aimbot::Melee::AutoBackstab, FToggleEnum::Left);
					FToggle(Vars::Aimbot::Melee::IgnoreRazorback, FToggleEnum::Right);
					FToggle(Vars::Aimbot::Melee::SwingPrediction, FToggleEnum::Left);
					FToggle(Vars::Aimbot::Melee::WhipTeam, FToggleEnum::Right);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug Melee"))
					{
						FSlider(Vars::Aimbot::Melee::SwingTicks, FSliderEnum::Left);
						FToggle(Vars::Aimbot::Melee::SwingPredictLag, FToggleEnum::Right);
						FToggle(Vars::Aimbot::Melee::BackstabAccountPing, FToggleEnum::Left);
						FToggle(Vars::Aimbot::Melee::BackstabDoubleTest, FToggleEnum::Right);
					} EndSection();
				}
			}
			EndTable();
		}
		break;
	}
	// HvH
	case 1:
	{
		if (BeginTable("HvHTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Doubletap", 8))
				{
					FToggle(Vars::Doubletap::Doubletap, FToggleEnum::Left);
					FToggle(Vars::Doubletap::Warp, FToggleEnum::Right);
					FToggle(Vars::Doubletap::RechargeTicks, FToggleEnum::Left);
					FToggle(Vars::Doubletap::AntiWarp, FToggleEnum::Right);
					FSlider(Vars::Doubletap::TickLimit, FSliderEnum::Left);
					FSlider(Vars::Doubletap::WarpRate, FSliderEnum::Right);
					FSlider(Vars::Doubletap::PassiveRecharge, FSliderEnum::Left);
					FSlider(Vars::Doubletap::RechargeLimit, FSliderEnum::Right);
				} EndSection();
				if (Section("Fakelag"))
				{
					FDropdown(Vars::Fakelag::Fakelag, FSliderEnum::Left);
					FDropdown(Vars::Fakelag::Options, FDropdownEnum::Right);
					PushTransparent(FGet(Vars::Fakelag::Fakelag) != Vars::Fakelag::FakelagEnum::Plain);
					{
						FSlider(Vars::Fakelag::PlainTicks, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(FGet(Vars::Fakelag::Fakelag) != Vars::Fakelag::FakelagEnum::Random);
					{
						FSlider(Vars::Fakelag::RandomTicks, FSliderEnum::Right);
					}
					PopTransparent();
					FToggle(Vars::Fakelag::UnchokeOnAttack, FToggleEnum::Left);
					FToggle(Vars::Fakelag::RetainBlastJump, FToggleEnum::Right);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug"))
					{
						FToggle(Vars::Fakelag::RetainSoldierOnly);
					} EndSection();
				}
				if (Section("Anti-aim", 8))
				{
					FToggle(Vars::AntiAim::Enabled);
					FDropdown(Vars::AntiAim::PitchReal, FDropdownEnum::Left);
					FDropdown(Vars::AntiAim::PitchFake, FDropdownEnum::Right);
					FDropdown(Vars::AntiAim::YawReal, FDropdownEnum::Left);
					FDropdown(Vars::AntiAim::YawFake, FDropdownEnum::Right);
					FDropdown(Vars::AntiAim::RealYawMode, FDropdownEnum::Left);
					FDropdown(Vars::AntiAim::FakeYawMode, FDropdownEnum::Right);
					FSlider(Vars::AntiAim::RealYawOffset, FSliderEnum::Left);
					FSlider(Vars::AntiAim::FakeYawOffset, FSliderEnum::Right);
					PushTransparent(FGet(Vars::AntiAim::YawReal) != Vars::AntiAim::YawEnum::Edge && FGet(Vars::AntiAim::YawReal) != Vars::AntiAim::YawEnum::Jitter);
					{
						FSlider(Vars::AntiAim::RealYawValue, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(FGet(Vars::AntiAim::YawFake) != Vars::AntiAim::YawEnum::Edge && FGet(Vars::AntiAim::YawFake) != Vars::AntiAim::YawEnum::Jitter);
					{
						FSlider(Vars::AntiAim::FakeYawValue, FSliderEnum::Right);
					}
					PopTransparent();
					PushTransparent(FGet(Vars::AntiAim::YawFake) != Vars::AntiAim::YawEnum::Spin && FGet(Vars::AntiAim::YawReal) != Vars::AntiAim::YawEnum::Spin);
					{
						FSlider(Vars::AntiAim::SpinSpeed, FSliderEnum::Left);
					}
					PopTransparent();
					SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetRowPos() + H::Draw.Scale(8) });
					FToggle(Vars::AntiAim::MinWalk, FToggleEnum::Left);
					FToggle(Vars::AntiAim::AntiOverlap, FToggleEnum::Left);
					FToggle(Vars::AntiAim::InvalidShootPitch, FToggleEnum::Right);
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Resolver", 8))
				{
					FToggle(Vars::Resolver::Enabled, FToggleEnum::Left);
					PushTransparent(!FGet(Vars::Resolver::Enabled));
					{
						FToggle(Vars::Resolver::AutoResolve, FToggleEnum::Right);
						PushTransparent(Transparent || !FGet(Vars::Resolver::AutoResolve));
						{
							FToggle(Vars::Resolver::AutoResolveCheatersOnly, FToggleEnum::Left);
							FToggle(Vars::Resolver::AutoResolveHeadshotOnly, FToggleEnum::Right);
							PushTransparent(Transparent || !FGet(Vars::Resolver::AutoResolveYawAmount));
							{
								FSlider(Vars::Resolver::AutoResolveYawAmount, FSliderEnum::Left);
							}
							PopTransparent();
							PushTransparent(Transparent || !FGet(Vars::Resolver::AutoResolvePitchAmount));
							{
								FSlider(Vars::Resolver::AutoResolvePitchAmount, FSliderEnum::Right);
							}
							PopTransparent();
						}
						PopTransparent();
						FSlider(Vars::Resolver::CycleYaw, FSliderEnum::Left);
						FSlider(Vars::Resolver::CyclePitch, FSliderEnum::Right);
						FToggle(Vars::Resolver::CycleView, FToggleEnum::Left);
						FToggle(Vars::Resolver::CycleMinwalk, FToggleEnum::Right);
					}
					PopTransparent();
				} EndSection();
				if (Section("Auto Peek", 8))
				{
					FToggle(Vars::AutoPeek::Enabled);
				} EndSection();
				if (Section("Cheater Detection"))
				{
					FDropdown(Vars::CheaterDetection::Methods);
					PushTransparent(!FGet(Vars::CheaterDetection::DetectionsRequired));
					{
						FSlider(Vars::CheaterDetection::DetectionsRequired);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::CheaterDetection::Methods) & Vars::CheaterDetection::MethodsEnum::PacketChoking));
					{
						FSlider(Vars::CheaterDetection::MinimumChoking);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::CheaterDetection::Methods) & Vars::CheaterDetection::MethodsEnum::AimFlicking));
					{
						FSlider(Vars::CheaterDetection::MinimumFlick, FSliderEnum::Left);
						FSlider(Vars::CheaterDetection::MaximumNoise, FSliderEnum::Right);
					}
					PopTransparent();
				} EndSection();
				if (Section("Speedhack", 8))
				{
					FToggle(Vars::Speedhack::Enabled);
					PushTransparent(!FGet(Vars::Speedhack::Enabled));
					{
						FSlider(Vars::Speedhack::Amount);
					}
					PopTransparent();
				} EndSection();
			}
			EndTable();
		}
		break;
	}
	// Draw
	case 2:
	{
		if (BeginTable("DrawTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Line", 8))
				{
					FColorPicker(Vars::Colors::LineClipped, 0);
					FColorPicker(Vars::Colors::Line, 1);
					FToggle(Vars::Visuals::Line::Enabled);
					FSlider(Vars::Visuals::Line::DrawDuration);
				} EndSection();
				if (Section("Hitbox"))
				{
					FDropdown(Vars::Visuals::Hitbox::BonesEnabled, FDropdownEnum::None, -110);
					FColorPicker(Vars::Colors::TargetHitboxEdge, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::TargetHitboxEdgeClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker(Vars::Colors::TargetHitboxFace, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::TargetHitboxFaceClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker(Vars::Colors::BoneHitboxEdge, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::BoneHitboxEdgeClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker(Vars::Colors::BoneHitboxFace, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::BoneHitboxFaceClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);

					FDropdown(Vars::Visuals::Hitbox::BoundsEnabled, FDropdownEnum::None, -50);
					FColorPicker(Vars::Colors::BoundHitboxEdge, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::BoundHitboxEdgeClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					SameLine(); DebugDummy({ H::Draw.Scale(2), 0 });
					FColorPicker(Vars::Colors::BoundHitboxFace, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::BoundHitboxFaceClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);

					FSlider(Vars::Visuals::Hitbox::DrawDuration);
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Simulation"))
				{
					FDropdown(Vars::Visuals::Simulation::PlayerPath, FDropdownEnum::Left, -20);
					FColorPicker(Vars::Colors::PlayerPath, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::PlayerPathClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FDropdown(Vars::Visuals::Simulation::ProjectilePath, FDropdownEnum::Right, -20);
					FColorPicker(Vars::Colors::ProjectilePath, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::ProjectilePathClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FDropdown(Vars::Visuals::Simulation::TrajectoryPath, FDropdownEnum::Left, -20);
					FColorPicker(Vars::Colors::TrajectoryPath, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::TrajectoryPathClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FDropdown(Vars::Visuals::Simulation::ShotPath, FDropdownEnum::Right, -20);
					FColorPicker(Vars::Colors::ShotPath, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::ShotPathClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FDropdown(Vars::Visuals::Simulation::SplashRadius, FDropdownEnum::None, -20);
					FColorPicker(Vars::Colors::SplashRadius, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FColorPicker(Vars::Colors::SplashRadiusClipped, 0, FColorPickerEnum::Dropdown | FColorPickerEnum::Tooltip);
					FToggle(Vars::Visuals::Simulation::Timed, FToggleEnum::Left);
					FToggle(Vars::Visuals::Simulation::Box, FToggleEnum::Right);
					FToggle(Vars::Visuals::Simulation::ProjectileCamera, FToggleEnum::Left);
					FToggle(Vars::Visuals::Simulation::SwingLines, FToggleEnum::Right);
					PushTransparent(FGet(Vars::Visuals::Simulation::Timed));
					{
						FSlider(Vars::Visuals::Simulation::DrawDuration);
					}
					PopTransparent();
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug Part1"))
					{
						FSlider(Vars::Visuals::Simulation::SeparatorSpacing, FSliderEnum::Left);
						FSlider(Vars::Visuals::Simulation::SeparatorLength, FSliderEnum::Right);
					} EndSection();
					if (Section("##Debug Part2"))
					{
						FToggle(Vars::Visuals::Trajectory::Override);
						FSlider(Vars::Visuals::Trajectory::OffsetX);
						FSlider(Vars::Visuals::Trajectory::OffsetY);
						FSlider(Vars::Visuals::Trajectory::OffsetZ);
						FToggle(Vars::Visuals::Trajectory::Pipes);
						FSlider(Vars::Visuals::Trajectory::Hull);
						FSlider(Vars::Visuals::Trajectory::Speed);
						FSlider(Vars::Visuals::Trajectory::Gravity);
						FSlider(Vars::Visuals::Trajectory::LifeTime);
						FSlider(Vars::Visuals::Trajectory::UpVelocity);
						FSlider(Vars::Visuals::Trajectory::AngularVelocityX);
						FSlider(Vars::Visuals::Trajectory::AngularVelocityY);
						FSlider(Vars::Visuals::Trajectory::AngularVelocityZ);
						FSlider(Vars::Visuals::Trajectory::Drag);
						FSlider(Vars::Visuals::Trajectory::DragX);
						FSlider(Vars::Visuals::Trajectory::DragY);
						FSlider(Vars::Visuals::Trajectory::DragZ);
						FSlider(Vars::Visuals::Trajectory::AngularDragX);
						FSlider(Vars::Visuals::Trajectory::AngularDragY);
						FSlider(Vars::Visuals::Trajectory::AngularDragZ);
						FSlider(Vars::Visuals::Trajectory::MaxVelocity);
						FSlider(Vars::Visuals::Trajectory::MaxAngularVelocity);
					} EndSection();
				}
			}
			EndTable();
		}
		break;
	}
	}
}

void CMenu::MenuVisuals(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	// ESP
	case 0:
	{
		if (BeginTable("VisualsESPTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("ESP"))
				{
					FDropdown(Vars::ESP::Draw);
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Players));
					{
						FDropdown(Vars::ESP::Player);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Buildings));
					{
						FDropdown(Vars::ESP::Building);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Projectiles));
					{
						FDropdown(Vars::ESP::Projectile);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::ESP::Draw) & Vars::ESP::DrawEnum::Objective));
					{
						FDropdown(Vars::ESP::Objective);
					}
					PopTransparent();
				} EndSection();
				if (Section("Out of FOV arrows", 8))
				{
					FToggle(Vars::ESP::FOVArrows::Enabled);
					FSlider(Vars::ESP::FOVArrows::Offset, FSliderEnum::Left);
					FSlider(Vars::ESP::FOVArrows::MaxDistance, FSliderEnum::Right);
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Colors", 8))
				{
					FToggle(Vars::Colors::Relative);
					if (FGet(Vars::Colors::Relative))
					{
						FColorPicker(Vars::Colors::Enemy, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::Team, 0, FColorPickerEnum::Middle);
					}
					else
					{
						FColorPicker(Vars::Colors::TeamRed, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::TeamBlu, 0, FColorPickerEnum::Middle);
					}
					FColorPicker(Vars::Colors::Local, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Target, 0, FColorPickerEnum::Middle);
					FColorPicker(Vars::Colors::Health, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Ammo, 0, FColorPickerEnum::Middle);
					FColorPicker(Vars::Colors::Money, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Powerup, 0, FColorPickerEnum::Middle);
					FColorPicker(Vars::Colors::NPC, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Halloween, 0, FColorPickerEnum::Middle);
				} EndSection();
				if (Section("Dormancy", 8))
				{
					FSlider(Vars::ESP::ActiveAlpha, FSliderEnum::Left);
					FSlider(Vars::ESP::DormantAlpha, FSliderEnum::Right);
					FSlider(Vars::ESP::DormantDuration, FSliderEnum::Left);
					FToggle(Vars::ESP::DormantPriority, FToggleEnum::Right);
				} EndSection();
				if (Section("Other"))
				{
					FDropdown(Vars::ESP::Other::SniperSightlines);
					FToggle(Vars::ESP::Other::PickupTimers);
				} EndSection();
			}
			EndTable();

			/*
			// esp groups system i may or may not go through with. not sure what would be best though with ui/user experience
			static size_t iCurrentGroup = 0;

			/* Column 1 * /
			TableNextColumn();
			{
				if (Section("Groups"))
				{
					static std::string sStaticName;

					PushDisabled(F::Groups.m_vGroups.size() >= sizeof(int) * 8); // for active groups flags
					{
						auto vTable = WidgetTable(2, H::Draw.Scale(48), { GetWindowWidth() - H::Draw.Scale(75) - GetStyle().WindowPadding.x });

						SetCursorPos(vTable[0].m_vPos);
						if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
						{
							FSDropdown("Name", &sStaticName, {}, FSDropdownEnum::AutoUpdate);
						} EndChild();

						SetCursorPos(vTable[1].m_vPos);
						if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
						{
							PushDisabled(Disabled || sStaticName.empty());
							{
								if (FButton("Create", FButtonEnum::Fit, { 0, 40 }))
								{
									F::Groups.m_vGroups.emplace_back(sStaticName);
									sStaticName.clear();
								}
							}
							PopDisabled();
						} EndChild();
					}
					PopDisabled();
					int i;
					FDropdown("Active groups", &i, { "" }); // active groups var for binding/quick access. automatically set bits for this var when adding/removing groups

					PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
					SetCursorPos({ H::Draw.Scale(13), H::Draw.Scale(128) });
					FText("Groups");
					SetCursorPosY(GetCursorPosY() - H::Draw.Scale(8));
					PopStyleColor();

					for (auto it = F::Groups.m_vGroups.begin(); it < F::Groups.m_vGroups.end();)
					{
						int iIndex = std::distance(F::Groups.m_vGroups.begin(), it);
						auto& tGroup = *it;

						ImVec2 vOriginalPos = { H::Draw.Scale(8), GetCursorPosY() - H::Draw.Scale(8) };

						float flWidth = GetWindowWidth() - GetStyle().WindowPadding.x * 2;
						float flHeight = H::Draw.Scale(28);
						ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
						if (iCurrentGroup != iIndex)
							GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, F::Render.Background1p5, H::Draw.Scale(4));
						else
						{
							ImColor tColor = F::Render.Background1p5L;
							GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor, H::Draw.Scale(4));

							tColor = ColorToVec((VecToColor(F::Render.Background1p5)).Lerp({ 127, 127, 127 }, 1.f / 9, LerpEnum::NoAlpha));
							float flInset = H::Draw.Scale(0.5f) - 0.5f;
							GetWindowDrawList()->AddRect({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + flWidth, vDrawPos.y - flInset + flHeight }, tColor, H::Draw.Scale(4), ImDrawFlags_None, H::Draw.Scale());
						}

						float flTextWidth = flWidth - H::Draw.Scale(36);
						SetCursorPos({ vOriginalPos.x + H::Draw.Scale(9), vOriginalPos.y + H::Draw.Scale(7) });
						FText(TruncateText(tGroup.m_sName, flTextWidth).c_str());

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(26), vOriginalPos.y + H::Draw.Scale(2) });
						bool bDelete = IconButton(ICON_MD_DELETE);

						SetCursorPos(vOriginalPos);
						if (Button(std::format("##{}", y).c_str(), { flWidth, flHeight }))
							iCurrentGroup = y;

						if (!bDelete)
							++it;
						else
						{
							it = F::Groups.m_vGroups.erase(it);
							if (iCurrentGroup == y && iCurrentGroup)
								iCurrentGroup--;
						}
					}
				} EndSection();
				if (Section("Colors", 8))
				{
					FToggle(Vars::Colors::Relative);
					if (FGet(Vars::Colors::Relative))
					{
						FColorPicker(Vars::Colors::Enemy, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::Team, 0, FColorPickerEnum::Middle);
					}
					else
					{
						FColorPicker(Vars::Colors::TeamRed, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::TeamBlu, 0, FColorPickerEnum::Middle);
					}
					FColorPicker(Vars::Colors::Local, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Target, 0, FColorPickerEnum::Middle);
					FColorPicker(Vars::Colors::Health, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Ammo, 0, FColorPickerEnum::Middle);
					FColorPicker(Vars::Colors::Money, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Powerup, 0, FColorPickerEnum::Middle);
					FColorPicker(Vars::Colors::NPC, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Colors::Halloween, 0, FColorPickerEnum::Middle);
					// may move these colors over to other spots
					PushTransparent(!FGet(Vars::Colors::Backtrack).a);
					{
						FColorPicker(Vars::Colors::Backtrack, 0, FColorPickerEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Colors::FakeAngle).a);
					{
						FColorPicker(Vars::Colors::FakeAngle, 0, FColorPickerEnum::Middle);
					}
					PopTransparent();
				} EndSection();
				// fake angle/viewmodel chams & glow here?
				if (Section("Other"))
				{
					FDropdown(Vars::ESP::Other::SniperSightlines);
					FToggle(Vars::ESP::Other::PickupTimers);
				} EndSection();
			}
			/* Column 2 * /
			TableNextColumn();
			if (0 <= iCurrentGroup && iCurrentGroup < F::Groups.m_vGroups.size())
			{
				auto& tGroup = F::Groups.m_vGroups[iCurrentGroup];
				if (Section("Target"))
				{
					FDropdown("Targets", &tGroup.m_iTargets, { "Players", "Buildings", "Projectiles", "Ragdolls", "Objective", "NPCs", "Health", "Ammo", "Money", "Powerups", "Bombs", "Spellbook", "Gargoyle" }, {}, FDropdownEnum::Multi);
					if (tGroup.m_iConditions & ConditionsEnum::Relative)
						FDropdown("Conditions", &tGroup.m_iConditions, { "Relative", "Enemy", "Team", "Local", "Friends", "Party", "Priority", "Target", "##Divider", "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" },
							{ ConditionsEnum::Relative, ConditionsEnum::Enemy, ConditionsEnum::Team, ConditionsEnum::Local, ConditionsEnum::Friends, ConditionsEnum::Party, ConditionsEnum::Priority, ConditionsEnum::Target, ConditionsEnum::Scout, ConditionsEnum::Soldier, ConditionsEnum::Pyro, ConditionsEnum::Demoman, ConditionsEnum::Heavy, ConditionsEnum::Engineer, ConditionsEnum::Medic, ConditionsEnum::Sniper, ConditionsEnum::Spy }, FDropdownEnum::Multi);
					else	
						FDropdown("Conditions", &tGroup.m_iConditions, { "Relative", "Enemy", "Team", "BLU", "RED", "Local", "Friends", "Party", "Priority", "Target", "##Divider", "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdownEnum::Multi);
				} EndSection();
				if (Section("ESP"))
				{
					FDropdown("Draw", &tGroup.m_iESP, { "Name", "Box", "Distance", "Bones", "Health bar", "Health text", "Uber bar", "Uber text", "Class icon", "Class text", "Weapon icon", "Weapon text", "Priority", "Labels", "Buffs", "Debuffs", "Misc", "Lag", "Ping", "KDR", "Owner", "Flags", "Level", "Intel return time" }, {}, FDropdownEnum::Multi);
					FSlider("Active alpha", &tGroup.m_iActiveAlpha, 0, 255, 5, "%i", FSliderEnum::Clamp | FSliderEnum::Precision);
					FSlider("Dormant alpha", &tGroup.m_iDormantAlpha, 0, 255, 5, "%i", FSliderEnum::Clamp | FSliderEnum::Precision);
					FSlider("Dormant duration", &tGroup.m_flDormantDuration, 0.015f, 5.0f, 0.1f, "%gs", FSliderEnum::Min | FSliderEnum::Precision);
					Divider();
					FToggle("Out of FOV Arrows", &tGroup.m_bOutOfFOVArrows);
					FSlider("Offset", &tGroup.m_iOutOfFOVArrowsOffset, 0, 500, 25, "%i", FSliderEnum::Left | FSliderEnum::Min | FSliderEnum::Precision);
					FSlider("Max distance", &tGroup.m_flOutOfFOVArrowsMaxDistance, 0.f, 5000.f, 50.f, "%g", FSliderEnum::Right | FSliderEnum::Min | FSliderEnum::Precision);
				} EndSection();
				if (Section("Chams", 8))
				{
					FToggle("Enabled", &tGroup.m_bChams);
					FMDropdown("Visible material", &tGroup.m_tChams.Visible, FDropdownEnum::Left);
					FMDropdown("Occluded material", &tGroup.m_tChams.Occluded, FDropdownEnum::Right);
				} EndSection();
				if (Section("Glow", 8))
				{
					FToggle("Enabled", &tGroup.m_bGlow);
					PushTransparent(!tGroup.m_tBacktrackGlow.Stencil);
					{
						FSlider("Stencil scale", &tGroup.m_tGlow.Stencil, 0.f, 10.f, 1.f, "%.0f", FSliderEnum::Left | FSliderEnum::Min);
					}
					PopTransparent();
					PushTransparent(!tGroup.m_tBacktrackGlow.Blur);
					{
						FSlider("Blur scale", &tGroup.m_tGlow.Blur, 0.f, 10.f, 1.f, "%.0f", FSliderEnum::Right | FSliderEnum::Min);
					}
					PopTransparent();
				} EndSection();
				if (Section("Backtrack", 8))
				{
					FToggle("Enabled", &tGroup.m_bBacktrack, FToggleEnum::Left);
					SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetRowPos() - H::Draw.Scale(8) });
					FDropdown("##Draw", &tGroup.m_iBacktrackDraw, { "Last", "First", "##Divider", "Always", "Ignore team" }, {}, FDropdownEnum::Left | FDropdownEnum::Multi, 0, "All");
					FMDropdown("Material", &tGroup.m_tBacktrackChams.Visible, FDropdownEnum::Left);
					{
						auto& tRow = vRowSizes.front();
						tRow.m_vPos.y += H::Draw.Scale(16), tRow.m_vSize.y -= H::Draw.Scale(16);
						SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetRowPos() });
					}
					FToggle("Ignore Z", &tGroup.m_bBacktrackIgnoreZ, FToggleEnum::Left);
					PushTransparent(!tGroup.m_tBacktrackGlow.Stencil);
					{
						FSlider("Stencil scale## Backtrack", &tGroup.m_tBacktrackGlow.Stencil, 0.f, 10.f, 1.f, "%.0f", FSliderEnum::Left | FSliderEnum::Min);
					}
					PopTransparent();
					PushTransparent(!tGroup.m_tBacktrackGlow.Blur);
					{
						FSlider("Blur scale## Backtrack", &tGroup.m_tBacktrackGlow.Blur, 0.f, 10.f, 1.f, "%.0f", FSliderEnum::Right | FSliderEnum::Min);
					}
					PopTransparent();
				} EndSection();
			}
			EndTable();
			*/
		}
		break;
	}
	// Chams
	case 1:
	{
		if (BeginTable("VisualsChamsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Player", 8))
				{
					FToggle(Vars::Chams::Player::Local, FToggleEnum::Left);
					FToggle(Vars::Chams::Player::Priority, FToggleEnum::Right);
					FToggle(Vars::Chams::Player::Friend, FToggleEnum::Left);
					FToggle(Vars::Chams::Player::Party, FToggleEnum::Right);
					FToggle(Vars::Chams::Player::Target, FToggleEnum::Left);

					FMDropdown(Vars::Chams::Player::Visible, FDropdownEnum::Left);
					FMDropdown(Vars::Chams::Player::Occluded, FDropdownEnum::Right);
				} EndSection();
				bool bRelative = FGet(Vars::Chams::Relative);
				if (Section("##Settings"))
				{
					auto vTable = WidgetTable(3, H::Draw.Scale(24));

					SetCursorPos(vTable[0].m_vPos);
					if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
					{
						FToggle(Vars::Chams::Relative);
					} EndChild();

					PushTransparent(bRelative);
					{
						SetCursorPos(vTable[1].m_vPos);
						if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
						{
							FToggle(Vars::Chams::EnemyChams);
						} EndChild();

						SetCursorPos(vTable[2].m_vPos);
						if (BeginChild(vTable[2].m_sName.c_str(), vTable[2].m_vSize, vTable[2].m_iWindowFlags, vTable[2].m_iChildFlags))
						{
							FToggle(Vars::Chams::TeamChams);
						} EndChild();
					}
					PopTransparent();
				} EndSection();
				if (Section(bRelative ? "Enemy" : "BLU", 8, 28, false, FNV1A::Hash32Const("Enemy")))
				{
					FToggle(Vars::Chams::Enemy::Players, FToggleEnum::Left, nullptr, bRelative ? 1 : 2);
					FToggle(Vars::Chams::Enemy::Ragdolls, FToggleEnum::Right, nullptr, bRelative ? 1 : 2);
					FToggle(Vars::Chams::Enemy::Buildings, FToggleEnum::Left, nullptr, bRelative ? 1 : 2);
					FToggle(Vars::Chams::Enemy::Projectiles, FToggleEnum::Right, nullptr, bRelative ? 1 : 2);

					FMDropdown(Vars::Chams::Enemy::Visible, FDropdownEnum::Left, 0, nullptr, bRelative ? 1 : 2);
					FMDropdown(Vars::Chams::Enemy::Occluded, FDropdownEnum::Right, 0, nullptr, bRelative ? 1 : 2);
				} EndSection();
				if (Section(bRelative ? "Team" : "RED", 8, 28, false, FNV1A::Hash32Const("Team")))
				{
					FToggle(Vars::Chams::Team::Players, FToggleEnum::Left, nullptr, bRelative ? 1 : 2);
					FToggle(Vars::Chams::Team::Ragdolls, FToggleEnum::Right, nullptr, bRelative ? 1 : 2);
					FToggle(Vars::Chams::Team::Buildings, FToggleEnum::Left, nullptr, bRelative ? 1 : 2);
					FToggle(Vars::Chams::Team::Projectiles, FToggleEnum::Right, nullptr, bRelative ? 1 : 2);

					FMDropdown(Vars::Chams::Team::Visible, FDropdownEnum::Left, 0, nullptr, bRelative ? 1 : 2);
					FMDropdown(Vars::Chams::Team::Occluded, FDropdownEnum::Right, 0, nullptr, bRelative ? 1 : 2);
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("World", 8))
				{
					FToggle(Vars::Chams::World::NPCs, FToggleEnum::Left);
					FToggle(Vars::Chams::World::Pickups, FToggleEnum::Right);
					FToggle(Vars::Chams::World::Objective, FToggleEnum::Left);
					FToggle(Vars::Chams::World::Powerups, FToggleEnum::Right);
					FToggle(Vars::Chams::World::Bombs, FToggleEnum::Left);
					FToggle(Vars::Chams::World::Halloween, FToggleEnum::Right);

					FMDropdown(Vars::Chams::World::Visible, FDropdownEnum::Left);
					FMDropdown(Vars::Chams::World::Occluded, FDropdownEnum::Right);
				} EndSection();
				if (Section("Backtrack", 8))
				{
					FToggle(Vars::Chams::Backtrack::Enabled, FToggleEnum::Left);
					FToggle(Vars::Chams::Backtrack::IgnoreZ, FToggleEnum::Right);

					FMDropdown(Vars::Chams::Backtrack::Visible, FDropdownEnum::Left);
					FDropdown(Vars::Chams::Backtrack::Draw, FDropdownEnum::Right);
				} EndSection();
				if (Section("Fake Angle", 8))
				{
					FToggle(Vars::Chams::FakeAngle::Enabled, FToggleEnum::Left);
					FToggle(Vars::Chams::FakeAngle::IgnoreZ, FToggleEnum::Right);

					FMDropdown(Vars::Chams::FakeAngle::Visible);
				} EndSection();
				if (Section("Viewmodel", 8))
				{
					FToggle(Vars::Chams::Viewmodel::Weapon, FToggleEnum::Left);
					FToggle(Vars::Chams::Viewmodel::Hands, FToggleEnum::Right);

					FMDropdown(Vars::Chams::Viewmodel::WeaponMaterial, FDropdownEnum::Left);
					FMDropdown(Vars::Chams::Viewmodel::HandsMaterial, FDropdownEnum::Right);
				} EndSection();
			}
			EndTable();
		}
		break;
	}
	// Glow
	case 2:
	{
		if (BeginTable("VisualsGlowTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Player", 8))
				{
					FToggle(Vars::Glow::Player::Local, FToggleEnum::Left);
					FToggle(Vars::Glow::Player::Priority, FToggleEnum::Right);
					FToggle(Vars::Glow::Player::Friend, FToggleEnum::Left);
					FToggle(Vars::Glow::Player::Party, FToggleEnum::Right);
					FToggle(Vars::Glow::Player::Target, FToggleEnum::Left);

					PushTransparent(!FGet(Vars::Glow::Player::Stencil));
					{
						FSlider(Vars::Glow::Player::Stencil, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Player::Blur));
					{
						FSlider(Vars::Glow::Player::Blur, FSliderEnum::Right);
					}
					PopTransparent();
				} EndSection();
				if (Section("Enemy", 8))
				{
					FToggle(Vars::Glow::Enemy::Players, FToggleEnum::Left);
					FToggle(Vars::Glow::Enemy::Ragdolls, FToggleEnum::Right);
					FToggle(Vars::Glow::Enemy::Buildings, FToggleEnum::Left);
					FToggle(Vars::Glow::Enemy::Projectiles, FToggleEnum::Right);

					PushTransparent(!FGet(Vars::Glow::Enemy::Stencil));
					{
						FSlider(Vars::Glow::Enemy::Stencil, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Enemy::Blur));
					{
						FSlider(Vars::Glow::Enemy::Blur, FSliderEnum::Right);
					}
					PopTransparent();
				} EndSection();
				if (Section("Team", 8))
				{
					FToggle(Vars::Glow::Team::Players, FToggleEnum::Left);
					FToggle(Vars::Glow::Team::Ragdolls, FToggleEnum::Right);
					FToggle(Vars::Glow::Team::Buildings, FToggleEnum::Left);
					FToggle(Vars::Glow::Team::Projectiles, FToggleEnum::Right);

					PushTransparent(!FGet(Vars::Glow::Team::Stencil));
					{
						FSlider(Vars::Glow::Team::Stencil, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Team::Blur));
					{
						FSlider(Vars::Glow::Team::Blur, FSliderEnum::Right);
					}
					PopTransparent();
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("World", 8))
				{
					FToggle(Vars::Glow::World::NPCs, FToggleEnum::Left);
					FToggle(Vars::Glow::World::Pickups, FToggleEnum::Right);
					FToggle(Vars::Glow::World::Objective, FToggleEnum::Left);
					FToggle(Vars::Glow::World::Powerups, FToggleEnum::Right);
					FToggle(Vars::Glow::World::Bombs, FToggleEnum::Left);
					FToggle(Vars::Glow::World::Halloween, FToggleEnum::Right);

					PushTransparent(!FGet(Vars::Glow::World::Stencil));
					{
						FSlider(Vars::Glow::World::Stencil, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::World::Blur));
					{
						FSlider(Vars::Glow::World::Blur, FSliderEnum::Right);
					}
					PopTransparent();
				} EndSection();
				if (Section("Backtrack", 8))
				{
					FToggle(Vars::Glow::Backtrack::Enabled, FToggleEnum::Left);
					PushTransparent(!FGet(Vars::Colors::Backtrack).a);
					{
						FColorPicker(Vars::Colors::Backtrack, 0, FColorPickerEnum::Middle);
					}
					PopTransparent();

					PushTransparent(!FGet(Vars::Glow::Backtrack::Stencil));
					{
						FSlider(Vars::Glow::Backtrack::Stencil, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Backtrack::Blur));
					{
						FSlider(Vars::Glow::Backtrack::Blur, FSliderEnum::Right);
					}
					PopTransparent();
					FDropdown(Vars::Glow::Backtrack::Draw, FDropdownEnum::None);
				} EndSection();
				if (Section("Fake Angle", 8))
				{
					FToggle(Vars::Glow::FakeAngle::Enabled, FToggleEnum::Left);
					PushTransparent(!FGet(Vars::Colors::FakeAngle).a);
					{
						FColorPicker(Vars::Colors::FakeAngle, 0, FColorPickerEnum::Middle);
					}
					PopTransparent();

					PushTransparent(!FGet(Vars::Glow::FakeAngle::Stencil));
					{
						FSlider(Vars::Glow::FakeAngle::Stencil, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::FakeAngle::Blur));
					{
						FSlider(Vars::Glow::FakeAngle::Blur, FSliderEnum::Right);
					}
					PopTransparent();
				} EndSection();
				if (Section("Viewmodel", 8))
				{
					FToggle(Vars::Glow::Viewmodel::Weapon, FToggleEnum::Left);
					FToggle(Vars::Glow::Viewmodel::Hands, FToggleEnum::Right);

					PushTransparent(!FGet(Vars::Glow::Viewmodel::Stencil));
					{
						FSlider(Vars::Glow::Viewmodel::Stencil, FSliderEnum::Left);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Glow::Viewmodel::Blur));
					{
						FSlider(Vars::Glow::Viewmodel::Blur, FSliderEnum::Right);
					}
					PopTransparent();
				} EndSection();
			}
			EndTable();
		}
		break;
	}
	// Misc
	case 3:
	{
		if (BeginTable("VisualsMiscTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Thirdperson", 8))
				{
					FToggle(Vars::Visuals::Thirdperson::Enabled, FToggleEnum::Left);
					FToggle(Vars::Visuals::Thirdperson::Crosshair, FToggleEnum::Right);
					FSlider(Vars::Visuals::Thirdperson::Distance);
					FSlider(Vars::Visuals::Thirdperson::Right);
					FSlider(Vars::Visuals::Thirdperson::Up);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug"))
					{
						FToggle(Vars::Visuals::Thirdperson::Scale, FToggleEnum::Left);
						FToggle(Vars::Visuals::Thirdperson::Collide, FToggleEnum::Right);
					} EndSection();
				}
				if (Section("Effects"))
				{
					// https://developer.valvesoftware.com/wiki/Team_Fortress_2/Particles
					// https://forums.alliedmods.net/showthread.php?t=127111
					FSDropdown(Vars::Visuals::Effects::BulletTracer, FDropdownEnum::Left);
					FSDropdown(Vars::Visuals::Effects::CritTracer, FDropdownEnum::Right);
					FSDropdown(Vars::Visuals::Effects::MedigunBeam, FDropdownEnum::Left);
					FSDropdown(Vars::Visuals::Effects::MedigunCharge, FDropdownEnum::Right);
					FSDropdown(Vars::Visuals::Effects::ProjectileTrail, FDropdownEnum::Left);
					FDropdown(Vars::Visuals::Effects::SpellFootsteps, FDropdownEnum::Right, -10);
					FColorPicker(Vars::Colors::SpellFootstep, 0, FColorPickerEnum::Dropdown);
					FDropdown(Vars::Visuals::Effects::RagdollEffects);
					FToggle(Vars::Visuals::Effects::DrawIconsThroughWalls);
					FToggle(Vars::Visuals::Effects::DrawDamageNumbersThroughWalls);
				} EndSection();
				if (Section("Viewmodel", 8))
				{
					FToggle(Vars::Visuals::Viewmodel::CrosshairAim, FToggleEnum::Left);
					FToggle(Vars::Visuals::Viewmodel::ViewmodelAim, FToggleEnum::Right);
					FSlider(Vars::Visuals::Viewmodel::OffsetX, FSliderEnum::Left);
					FSlider(Vars::Visuals::Viewmodel::Pitch, FSliderEnum::Right);
					FSlider(Vars::Visuals::Viewmodel::OffsetY, FSliderEnum::Left);
					FSlider(Vars::Visuals::Viewmodel::Yaw, FSliderEnum::Right);
					FSlider(Vars::Visuals::Viewmodel::OffsetZ, FSliderEnum::Left);
					FSlider(Vars::Visuals::Viewmodel::Roll, FSliderEnum::Right);
					PushTransparent(!FGet(Vars::Visuals::Viewmodel::SwayScale) || !FGet(Vars::Visuals::Viewmodel::SwayInterp));
					{
						FSlider(Vars::Visuals::Viewmodel::SwayScale, FSliderEnum::Left);
						FSlider(Vars::Visuals::Viewmodel::SwayInterp, FSliderEnum::Right);
					}
					PopTransparent();
					/*
					PushTransparent(!FGet(Vars::Visuals::Viewmodel::FieldOfView));
					{
						FSlider(Vars::Visuals::Viewmodel::FieldOfView);
					}
					PopTransparent();
					*/
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Removals", 8))
				{
					FToggle(Vars::Visuals::Removals::Interpolation, FToggleEnum::Left);
					FToggle(Vars::Visuals::Removals::NoLerp, FToggleEnum::Right);
					FToggle(Vars::Visuals::Removals::Disguises, FToggleEnum::Left);
					FToggle(Vars::Visuals::Removals::Taunts, FToggleEnum::Right);
					FToggle(Vars::Visuals::Removals::Scope, FToggleEnum::Left);
					FToggle(Vars::Visuals::Removals::PostProcessing, FToggleEnum::Right);
					FToggle(Vars::Visuals::Removals::ScreenOverlays, FToggleEnum::Left);
					FToggle(Vars::Visuals::Removals::ScreenEffects, FToggleEnum::Right);
					FToggle(Vars::Visuals::Removals::ViewPunch, FToggleEnum::Left);
					FToggle(Vars::Visuals::Removals::AngleForcing, FToggleEnum::Right);
					FToggle(Vars::Visuals::Removals::Ragdolls, FToggleEnum::Left);
					FToggle(Vars::Visuals::Removals::Gibs, FToggleEnum::Right);
					FToggle(Vars::Visuals::Removals::MOTD, FToggleEnum::Left);
				} EndSection();
				if (Section("UI"))
				{
					FDropdown(Vars::Visuals::UI::StreamerMode, FDropdownEnum::Left);
					FDropdown(Vars::Visuals::UI::ChatTags, FDropdownEnum::Right);
					PushTransparent(!FGet(Vars::Visuals::UI::FieldOfView));
					{
						FSlider(Vars::Visuals::UI::FieldOfView);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Visuals::UI::ZoomFieldOfView));
					{
						FSlider(Vars::Visuals::UI::ZoomFieldOfView);
					}
					PopTransparent();
					PushTransparent(!FGet(Vars::Visuals::UI::AspectRatio));
					{
						FSlider(Vars::Visuals::UI::AspectRatio);
					}
					PopTransparent();
					FToggle(Vars::Visuals::UI::RevealScoreboard, FToggleEnum::Left);
					FToggle(Vars::Visuals::UI::ScoreboardUtility, FToggleEnum::Right);
					FToggle(Vars::Visuals::UI::ScoreboardColors, FToggleEnum::Left);
					FToggle(Vars::Visuals::UI::CleanScreenshots, FToggleEnum::Right);
				} EndSection();
				if (Section("World"))
				{
					FDropdown(Vars::Visuals::World::Modulations);
					FSDropdown(Vars::Visuals::World::WorldTexture, FDropdownEnum::Left);
					FSDropdown(Vars::Visuals::World::SkyboxChanger, FDropdownEnum::Right);
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::World));
					{
						FColorPicker(Vars::Colors::WorldModulation, 0, FColorPickerEnum::Left);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Sky));
					{
						FColorPicker(Vars::Colors::SkyModulation, 0, FColorPickerEnum::Middle);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Prop));
					{
						FColorPicker(Vars::Colors::PropModulation, 0, FColorPickerEnum::Left);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Particle));
					{
						FColorPicker(Vars::Colors::ParticleModulation, 0, FColorPickerEnum::Middle);
					}
					PopTransparent();
					PushTransparent(!(FGet(Vars::Visuals::World::Modulations) & Vars::Visuals::World::ModulationsEnum::Fog));
					{
						FColorPicker(Vars::Colors::FogModulation, 0, FColorPickerEnum::Left);
					}
					PopTransparent();
					FToggle(Vars::Visuals::World::NearPropFade, FToggleEnum::Left);
					FToggle(Vars::Visuals::World::NoPropFade, FToggleEnum::Right);
				} EndSection();
				/*
				if (Section("Other"))
				{
					FSDropdown(Vars::Visuals::Other::LocalDominationOverride, FDropdownEnum::Left);
					FSDropdown(Vars::Visuals::Other::LocalRevengeOverride, FDropdownEnum::Right);
					FSDropdown(Vars::Visuals::Other::DominationOverride, FDropdownEnum::Left);
					FSDropdown(Vars::Visuals::Other::RevengeOverride, FDropdownEnum::Right);
				} EndSection();
				*/
			}
			EndTable();
		}
		break;
	}
	// Radar
	case 4:
	{
		if (BeginTable("VisualsRadarTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Main", 8))
				{
					FToggle(Vars::Radar::Main::Enabled, FToggleEnum::Left);
					FToggle(Vars::Radar::Main::DrawOutOfRange, FToggleEnum::Right);
					FDropdown(Vars::Radar::Main::Style);
					FSlider(Vars::Radar::Main::Range);
					FSlider(Vars::Radar::Main::BackgroundAlpha);
					FSlider(Vars::Radar::Main::LineAlpha);
				} EndSection();
				if (Section("Player", 8))
				{
					FToggle(Vars::Radar::Player::Enabled, FToggleEnum::Left);
					FToggle(Vars::Radar::Player::Background, FToggleEnum::Right);
					FDropdown(Vars::Radar::Player::Draw, FDropdownEnum::Left);
					FDropdown(Vars::Radar::Player::Icon, FDropdownEnum::Right);
					FSlider(Vars::Radar::Player::Size);
					FToggle(Vars::Radar::Player::Health, FToggleEnum::Left);
					FToggle(Vars::Radar::Player::Height, FToggleEnum::Right);
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Building", 8))
				{
					FToggle(Vars::Radar::Building::Enabled, FToggleEnum::Left, nullptr);
					FToggle(Vars::Radar::Building::Background, FToggleEnum::Right, nullptr);
					FDropdown(Vars::Radar::Building::Draw);
					FSlider(Vars::Radar::Building::Size);
					FToggle(Vars::Radar::Building::Health);
				} EndSection();
				if (Section("World", 8))
				{
					FToggle(Vars::Radar::World::Enabled, FToggleEnum::Left);
					FToggle(Vars::Radar::World::Background, FToggleEnum::Right);
					FDropdown(Vars::Radar::World::Draw);
					FSlider(Vars::Radar::World::Size);
				} EndSection();
			}
			EndTable();
		}
		break;
	}
	// Menu
	case 5:
	{
		if (BeginTable("MenuTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Menu", 8))
				{
					FColorPicker(Vars::Menu::Theme::Accent, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Menu::Theme::Background, 0, FColorPickerEnum::Middle);
					FColorPicker(Vars::Menu::Theme::Active, 0, FColorPickerEnum::Left);
					FColorPicker(Vars::Menu::Theme::Inactive, 0, FColorPickerEnum::Middle);

					FSDropdown(Vars::Menu::CheatTitle, FDropdownEnum::Left);
					FSDropdown(Vars::Menu::CheatTag, FDropdownEnum::Right);
					FKeybind(Vars::Menu::MenuPrimaryKey, FButtonEnum::Left, { Vars::Menu::MenuSecondaryKey[DEFAULT_BIND], VK_LBUTTON, VK_RBUTTON });
					FKeybind(Vars::Menu::MenuSecondaryKey, FButtonEnum::Right | FButtonEnum::SameLine, { Vars::Menu::MenuPrimaryKey[DEFAULT_BIND], VK_LBUTTON, VK_RBUTTON });
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Indicators"))
				{
					FDropdown(Vars::Menu::Indicators);
					if (FSlider(Vars::Menu::Scale))
						H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);
					FToggle(Vars::Menu::CheapText);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug"))
					{
						FColorPicker(Vars::Colors::IndicatorGood, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::IndicatorTextGood, 0, FColorPickerEnum::Middle);
						FColorPicker(Vars::Colors::IndicatorBad, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::IndicatorTextBad, 0, FColorPickerEnum::Middle);
						FColorPicker(Vars::Colors::IndicatorMid, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::IndicatorTextMid, 0, FColorPickerEnum::Middle);
						FColorPicker(Vars::Colors::IndicatorMisc, 0, FColorPickerEnum::Left);
						FColorPicker(Vars::Colors::IndicatorTextMisc, 0, FColorPickerEnum::Middle);
					}
					EndSection();
				}
			}
			EndTable();
		}
	}
	}
}

void CMenu::MenuMisc(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	case 0:
	{
		if (BeginTable("MiscTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Movement"))
				{
					FDropdown(Vars::Misc::Movement::AutoStrafe);
					PushTransparent(FGet(Vars::Misc::Movement::AutoStrafe) != Vars::Misc::Movement::AutoStrafeEnum::Directional);
					{
						FSlider(Vars::Misc::Movement::AutoStrafeTurnScale, FSliderEnum::Left);
						FSlider(Vars::Misc::Movement::AutoStrafeMaxDelta, FSliderEnum::Right);
					}
					PopTransparent();
					FToggle(Vars::Misc::Movement::Bunnyhop, FToggleEnum::Left);
					FToggle(Vars::Misc::Movement::EdgeJump, FToggleEnum::Right);
					FToggle(Vars::Misc::Movement::AutoJumpbug, FToggleEnum::Left); // this is unreliable without setups, do not depend on it!
					FToggle(Vars::Misc::Movement::NoPush, FToggleEnum::Right);
					FToggle(Vars::Misc::Movement::AutoRocketJump, FToggleEnum::Left);
					FToggle(Vars::Misc::Movement::AutoCTap, FToggleEnum::Right);
					FToggle(Vars::Misc::Movement::FastStop, FToggleEnum::Left);
					FToggle(Vars::Misc::Movement::FastAccelerate, FToggleEnum::Right);
					FToggle(Vars::Misc::Movement::CrouchSpeed, FToggleEnum::Left);
					FToggle(Vars::Misc::Movement::MovementLock, FToggleEnum::Right);
					FToggle(Vars::Misc::Movement::BreakJump, FToggleEnum::Left);
					FToggle(Vars::Misc::Movement::ShieldTurnRate, FToggleEnum::Right);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug"))
					{
						FSlider(Vars::Misc::Movement::TimingOffset);
						FSlider(Vars::Misc::Movement::ChokeCount);
						FSlider(Vars::Misc::Movement::ApplyAbove);
					} EndSection();
				}
				if (Section("Exploits", 8))
				{
					FToggle(Vars::Misc::Exploits::CheatsBypass, FToggleEnum::Left);
					FToggle(Vars::Misc::Exploits::PureBypass, FToggleEnum::Right);
					FToggle(Vars::Misc::Exploits::EquipRegionUnlock, FToggleEnum::Left);
					FToggle(Vars::Misc::Exploits::BackpackExpander, FToggleEnum::Right);
					FToggle(Vars::Misc::Exploits::PingReducer);
					PushTransparent(!FGet(Vars::Misc::Exploits::PingReducer));
					{
						FSlider(Vars::Misc::Exploits::PingTarget);
					}
					PopTransparent();
				} EndSection();
				if (Section("Automation"))
				{
					FDropdown(Vars::Misc::Automation::AntiBackstab); // pitch/fake _might_ slip up some auto backstabs
					FToggle(Vars::Misc::Automation::AntiAFK, FToggleEnum::Left);
					FToggle(Vars::Misc::Automation::AntiAutobalance, FToggleEnum::Right);
					FToggle(Vars::Misc::Automation::TauntControl, FToggleEnum::Left);
					FToggle(Vars::Misc::Automation::KartControl, FToggleEnum::Right);
					FToggle(Vars::Misc::Automation::AcceptItemDrops);
					FToggle(Vars::Misc::Automation::AutoF2Ignored, FToggleEnum::Left);
					FToggle(Vars::Misc::Automation::AutoF1Priority, FToggleEnum::Right);
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Sound"))
				{
					FDropdown(Vars::Misc::Sound::Block);
					FToggle(Vars::Misc::Sound::HitsoundAlways, FToggleEnum::Left);
					FToggle(Vars::Misc::Sound::RemoveDSP, FToggleEnum::Right);
					FToggle(Vars::Misc::Sound::GiantWeaponSounds);
				} EndSection();
				if (Section("Game", 8))
				{
					FToggle(Vars::Misc::Game::NetworkFix, FToggleEnum::Left);
					FToggle(Vars::Misc::Game::PredictionErrorJitterFix, FToggleEnum::Right);
					FToggle(Vars::Misc::Game::SetupBonesOptimization, FToggleEnum::Left);
					FToggle(Vars::Misc::Game::F2PChatBypass, FToggleEnum::Right);
					FToggle(Vars::Misc::Game::AntiCheatCompatibility);
				} EndSection();
				if (Vars::Debug::Options.Value)
				{
					if (Section("##Debug AntiCheat"))
					{
						FToggle(Vars::Misc::Game::AntiCheatCritHack);
					} EndSection();
				}
				if (Section("Queueing"))
				{
					FDropdown(Vars::Misc::Queueing::ForceRegions);
					FToggle(Vars::Misc::Queueing::FreezeQueue, FToggleEnum::Left);
					FToggle(Vars::Misc::Queueing::AutoCasualQueue, FToggleEnum::Right);
				} EndSection();
				if (Section("Mann vs. Machine", 8))
				{
					FToggle(Vars::Misc::MannVsMachine::InstantRespawn, FToggleEnum::Left);
					FToggle(Vars::Misc::MannVsMachine::InstantRevive, FToggleEnum::Right);
					FToggle(Vars::Misc::MannVsMachine::AllowInspect);
				} EndSection();
				if (Section("Steam RPC", 8))
				{
					FToggle(Vars::Misc::SteamRPC::Enabled, FToggleEnum::Left);
					FToggle(Vars::Misc::SteamRPC::OverrideInMenu, FToggleEnum::Right);
					FDropdown(Vars::Misc::SteamRPC::MatchGroup, FDropdownEnum::Left);
					FSDropdown(Vars::Misc::SteamRPC::MapText, FDropdownEnum::Right);
					FSlider(Vars::Misc::SteamRPC::GroupSize);
				} EndSection();
			}
			EndTable();
		}
	}
	}
}

void CMenu::MenuLogs(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	// PlayerList
	case 0:
	{
		if (Section("Players"))
		{
			if (I::EngineClient->IsInGame())
			{
				std::lock_guard lock(F::PlayerUtils.m_mutex);
				const auto& vPlayers = F::PlayerUtils.m_vPlayerCache;

				std::unordered_map<uint64_t, std::vector<const ListPlayer*>> mParties = {};
				std::unordered_map<uint64_t, float> mHues = {}; // don't just shift based on party id in the case that it will be similar
				for (auto& tPlayer : vPlayers)
				{
					if (tPlayer.m_iParty)
						mParties[tPlayer.m_iParty].push_back(&tPlayer);
				}
				for (auto it = mParties.begin(); it != mParties.end();)
				{
					if (it->second.size() > 1)
						it++;
					else
						it = mParties.erase(it);
				}
				{
					float flParties = mParties.size() + 1 - mParties.contains(1);
					int i = 0; for (auto& [iParty, _] : mParties)
						mHues[iParty] = iParty != 1 ? 360 * ++i / flParties : 0;
				}

				auto getTeamColor = [&](int iTeam, bool bAlive)
					{
						switch (iTeam)
						{
						case 3: return Color_t(100, 150, 200, bAlive ? 255 : 127).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha);
						case 2: return Color_t(255, 100, 100, bAlive ? 255 : 127).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha);
						}
						return Color_t(127, 127, 127, 255).Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha);
					};
				auto drawPlayer = [&](const ListPlayer& tPlayer, int x, int y)
					{
						ImColor tColor = ColorToVec(getTeamColor(tPlayer.m_iTeam, tPlayer.m_bAlive));

						ImVec2 vOriginalPos = { !x ? GetStyle().WindowPadding.x : GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(35 + 36 * y) };

						// background
						float flWidth = GetWindowWidth() / 2 - GetStyle().WindowPadding.x * 1.5f;
						float flHeight = H::Draw.Scale(28);
						ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
						GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor, H::Draw.Scale(4));

						// text + icons
						int lOffset = H::Draw.Scale(10);
						if (tPlayer.m_bLocal)
						{
							lOffset = H::Draw.Scale(29);
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_PERSON);
						}
						else if (F::Spectate.m_iIntendedTarget == tPlayer.m_iUserID)
						{
							lOffset = H::Draw.Scale(29);
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_VISIBILITY);
						}
						else if (tPlayer.m_bFriend)
						{
							lOffset = H::Draw.Scale(29);
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_GROUP);
						}
						else if (tPlayer.m_bParty)
						{
							lOffset = H::Draw.Scale(29);
							SetCursorPos({ vOriginalPos.x + H::Draw.Scale(7), vOriginalPos.y + H::Draw.Scale(6) });
							IconImage(ICON_MD_GROUPS);
						}
						SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y + H::Draw.Scale(7) });
						auto sName = TruncateText(tPlayer.m_sName, flWidth / 2 - lOffset);
						FText(sName.c_str());
						lOffset += FCalcTextSize(sName.c_str()).x + H::Draw.Scale(8);

						// buttons
						bool bClicked = false;

						if (!tPlayer.m_bFake)
						{
							// tag bar
							SetCursorPos({ vOriginalPos.x + lOffset, vOriginalPos.y });
							if (BeginChild(std::format("TagBar{}", tPlayer.m_uFriendsID).c_str(), { flWidth - lOffset - H::Draw.Scale(4), flHeight }, ImGuiWindowFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
							{
								std::vector<PriorityLabel_t> vLabels = {};
								std::vector<std::pair<PriorityLabel_t*, int>> vTags = {};
								if (mHues.contains(tPlayer.m_iParty))
									vLabels.emplace_back("Party", F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)].m_tColor.HueShift(mHues[tPlayer.m_iParty]));
								if (tPlayer.m_bF2P)
									vTags.emplace_back(&F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(F2P_TAG)], 0);
								for (auto& iID : F::PlayerUtils.m_mPlayerTags[tPlayer.m_uFriendsID])
								{
									auto pTag = F::PlayerUtils.GetTag(iID);
									if (pTag)
										vTags.emplace_back(pTag, iID);
								}

								PushFont(F::Render.FontSmall);
								const auto vDrawPos = GetDrawPos();
								float flTagOffset = 0;
								auto drawTag = [&](PriorityLabel_t& tTag, int iID)
									{
										ImColor tTagColor = ColorToVec(tTag.m_tColor);
										float flTagWidth = FCalcTextSize(tTag.m_sName.c_str()).x + H::Draw.Scale(!iID ? 10 : 25);
										float flTagHeight = H::Draw.Scale(20);
										ImVec2 vTagPos = { flTagOffset, H::Draw.Scale(4) };

										GetWindowDrawList()->AddRectFilled(vDrawPos + vTagPos, { vDrawPos.x + vTagPos.x + flTagWidth, vDrawPos.y + vTagPos.y + flTagHeight }, tTagColor, H::Draw.Scale(4));
										SetCursorPos({ vTagPos.x + H::Draw.Scale(5), vTagPos.y + H::Draw.Scale(3) });
										TextColored(IsColorBright(tTagColor) ? ImVec4(0, 0, 0, 1) : ImVec4(1, 1, 1, 1), tTag.m_sName.c_str());
										if (iID)
										{
											SetCursorPos({ vTagPos.x + flTagWidth - H::Draw.Scale(22), vTagPos.y - H::Draw.Scale(2) });
											if (IconButton(ICON_MD_CANCEL))
												F::PlayerUtils.RemoveTag(tPlayer.m_uFriendsID, iID, true, tPlayer.m_sName);
										}

										flTagOffset += flTagWidth + H::Draw.Scale(4);
									};

								for (auto& tTag : vLabels)
									drawTag(tTag, 0);
								for (auto& [pTag, iID] : vTags)
									drawTag(*pTag, iID);
								PopFont();
							} EndChild();

							bClicked = IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right);
						}
						SetCursorPos(vOriginalPos);
						Button(std::format("##{}", tPlayer.m_uFriendsID).c_str(), { flWidth, flHeight });
						bClicked = bClicked || IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right);

						// popups
						if (bClicked)
							OpenPopup(std::format("Clicked{}", tPlayer.m_uFriendsID).c_str());
						if (FBeginPopup(std::format("Clicked{}", tPlayer.m_uFriendsID).c_str()))
						{
							PushStyleVar(ImGuiStyleVar_ItemSpacing, { H::Draw.Scale(8), H::Draw.Scale(8) });

							if (!tPlayer.m_bFake)
							{
								if (FSelectable("Profile"))
									I::SteamFriends->ActivateGameOverlayToUser("steamid", CSteamID(tPlayer.m_uFriendsID, k_EUniversePublic, k_EAccountTypeIndividual));

								if (FSelectable("History"))
									I::SteamFriends->ActivateGameOverlayToWebPage(std::format("https://steamhistory.net/id/{}", CSteamID(tPlayer.m_uFriendsID, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64()).c_str());
							}

							if (FSelectable(F::Spectate.m_iIntendedTarget == tPlayer.m_iUserID ? "Unspectate" : "Spectate"))
								F::Spectate.SetTarget(tPlayer.m_iUserID);

							if (!I::EngineClient->IsPlayingDemo() && FBeginMenu("Votekick"))
							{
								if (FSelectable("No reason"))
									I::ClientState->SendStringCmd(std::format("callvote Kick \"{} other\"", tPlayer.m_iUserID).c_str());

								if (FSelectable("Cheating"))
									I::ClientState->SendStringCmd(std::format("callvote Kick \"{} cheating\"", tPlayer.m_iUserID).c_str());

								if (FSelectable("Idle"))
									I::ClientState->SendStringCmd(std::format("callvote Kick \"{} idle\"", tPlayer.m_iUserID).c_str());

								if (FSelectable("Scamming"))
									I::ClientState->SendStringCmd(std::format("callvote Kick \"{} scamming\"", tPlayer.m_iUserID).c_str());

								ImGui::EndMenu();
							}

							if (!tPlayer.m_bFake)
							{
								if (FBeginMenu("Add tag"))
								{
									for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
									{
										int iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);
										auto& tTag = *it;
										if (!tTag.m_bAssignable || F::PlayerUtils.HasTag(tPlayer.m_uFriendsID, iID))
											continue;

										auto imColor = ColorToVec(tTag.m_tColor);
										PushStyleColor(ImGuiCol_Text, imColor);
										imColor.x /= 3; imColor.y /= 3; imColor.z /= 3;
										if (FSelectable(tTag.m_sName.c_str(), imColor))
											F::PlayerUtils.AddTag(tPlayer.m_uFriendsID, iID, true, tPlayer.m_sName);
										PopStyleColor();
									}

									ImGui::EndMenu();
								}

								if (FBeginMenu("Alias"))
								{
									bool bHasAlias = F::PlayerUtils.m_mPlayerAliases.contains(tPlayer.m_uFriendsID);
									static std::string sInput = "";

									PushStyleVar(ImGuiStyleVar_FramePadding, { H::Draw.Scale(8), H::Draw.Scale(8) });
									PushItemWidth(H::Draw.Scale(150));
									bool bEnter = FInputText("Alias...", sInput, H::Draw.Scale(150), ImGuiInputTextFlags_EnterReturnsTrue);
									if (!IsItemFocused())
										sInput = bHasAlias ? F::PlayerUtils.m_mPlayerAliases[tPlayer.m_uFriendsID] : "";
									PopItemWidth();
									PopStyleVar();

									if (bEnter)
									{
										if (sInput.empty() && bHasAlias)
										{
											F::Output.AliasChanged(tPlayer.m_sName, "Removed", F::PlayerUtils.m_mPlayerAliases[tPlayer.m_uFriendsID]);

											auto it = F::PlayerUtils.m_mPlayerAliases.find(tPlayer.m_uFriendsID);
											if (it != F::PlayerUtils.m_mPlayerAliases.end())
												F::PlayerUtils.m_mPlayerAliases.erase(it);
											F::PlayerUtils.m_bSave = true;
										}
										else if (!sInput.empty())
										{
											F::PlayerUtils.m_mPlayerAliases[tPlayer.m_uFriendsID] = sInput;
											F::PlayerUtils.m_bSave = true;

											F::Output.AliasChanged(tPlayer.m_sName, bHasAlias ? "Changed" : "Added", sInput);
										}
									}

									ImGui::EndMenu();
								}
							}

							if (Vars::Resolver::Enabled.Value && !tPlayer.m_bLocal)
							{
								if (FBeginMenu("Set yaw"))
								{
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
												F::Resolver.SetYaw(tPlayer.m_iUserID, 0.f, true);
												break;
											default:
												F::Resolver.SetYaw(tPlayer.m_iUserID, flValue);
											}
										}
									}

									ImGui::EndMenu();
								}

								if (FBeginMenu("Set pitch"))
								{
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
												F::Resolver.SetPitch(tPlayer.m_iUserID, 0.f, false, true);
												break;
											case FNV1A::Hash32Const("Inverse"):
												F::Resolver.SetPitch(tPlayer.m_iUserID, 0.f, true);
												break;
											default:
												F::Resolver.SetPitch(tPlayer.m_iUserID, flValue);
											}
										}
									}

									ImGui::EndMenu();
								}

								if (FBeginMenu("Set view"))
								{
									static std::vector<std::pair<std::string, bool>> vPitches = {
										{ "Offset from static view", true },
										{ "Offset from view to local", false }
									};
									for (auto& [sPitch, bValue] : vPitches)
									{
										if (FSelectable(sPitch.c_str()))
											F::Resolver.SetView(tPlayer.m_iUserID, bValue);
									}

									ImGui::EndMenu();
								}

								if (FBeginMenu("Set minwalk"))
								{
									static std::vector<std::pair<std::string, bool>> vPitches = {
										{ "Minwalk on", true },
										{ "Minwalk off", false }
									};
									for (auto& [sPitch, bValue] : vPitches)
									{
										if (FSelectable(sPitch.c_str()))
											F::Resolver.SetMinwalk(tPlayer.m_iUserID, bValue);
									}

									ImGui::EndMenu();
								}
							}

							if (mParties.contains(tPlayer.m_iParty))
							{
								Divider(H::Draw.Scale(), H::Draw.Scale(1), 0);

								TextColored(F::Render.Inactive.Value, "Partied:");
								for (auto& pPlayer2 : mParties[tPlayer.m_iParty])
									TextColored(F::Render.Inactive.Value, pPlayer2->m_sName.c_str());
							}

							if (tPlayer.m_iLevel != -2)
							{
								Divider(H::Draw.Scale(), H::Draw.Scale(1), 0);

								std::string sLevel = "T? L?";
								if (tPlayer.m_iLevel != -1)
								{
									int iTier = std::max(std::ceil(tPlayer.m_iLevel / 150.f), 1.f);
									int iLevel = ((tPlayer.m_iLevel - 1) % 150) + 1;
									sLevel = std::format("T{} L{}", iTier, iLevel);
								}
								TextColored(F::Render.Inactive.Value, sLevel.c_str());
							}

							PopStyleVar();
							EndPopup();
						}
					};

				// display players
				std::vector<ListPlayer> vBlu, vRed, vOther;
				for (auto& tPlayer : vPlayers)
				{
					switch (tPlayer.m_iTeam)
					{
					case 3: vBlu.push_back(tPlayer); break;
					case 2: vRed.push_back(tPlayer); break;
					default: vOther.push_back(tPlayer); break;
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
				SetCursorPos({ H::Draw.Scale(15), H::Draw.Scale(40) });
				FText("Not ingame");
				DebugDummy({ 0, H::Draw.Scale(8) });
			}
		} EndSection();
		if (Section("Tags"))
		{
			static int iID = -1;
			static PriorityLabel_t tTag = {};

			auto vTable = WidgetTable(3, H::Draw.Scale(56), { GetWindowWidth() / 2, GetWindowWidth() / 2 - H::Draw.Scale(90) - GetStyle().WindowPadding.x });

			SetCursorPos(vTable[0].m_vPos);
			if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
			{
				FSDropdown("Name", &tTag.m_sName, {}, FDropdownEnum::Left | FSDropdownEnum::AutoUpdate, -10);
				FColorPicker("Color", &tTag.m_tColor, 0, FColorPickerEnum::Dropdown);

				PushDisabled(iID == DEFAULT_TAG || iID == IGNORED_TAG);
				{
					int iLabel = Disabled ? 0 : tTag.m_bLabel;
					FDropdown("Type", &iLabel, { "Priority", "Label" }, {}, FDropdownEnum::Right);
					tTag.m_bLabel = iLabel;
					if (Disabled)
						tTag.m_bLabel = false;
				}
				PopDisabled();
			} EndChild();

			SetCursorPos(vTable[1].m_vPos);
			if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
			{
				PushTransparent(tTag.m_bLabel); // transparent if we want a label, user can still use to sort
				{
					SetCursorPosY(GetCursorPos().y + H::Draw.Scale(12));
					FSlider("Priority", &tTag.m_iPriority, -10, 10);
				}
				PopTransparent();
			} EndChild();

			SetCursorPos(vTable[2].m_vPos);
			if (BeginChild(vTable[2].m_sName.c_str(), vTable[2].m_vSize, vTable[2].m_iWindowFlags, vTable[2].m_iChildFlags))
			{
				// create/modify button
				bool bCreate = false, bClear = false;

				SetCursorPos({ GetWindowWidth() - H::Draw.Scale(95), H::Draw.Scale(8) });
				PushDisabled(tTag.m_sName.empty());
				{
					bCreate = FButton(iID != -1 ? ICON_MD_SETTINGS : ICON_MD_ADD, FButtonEnum::None, { 40, 40 }, 0, F::Render.IconFont);
				}
				PopDisabled();

				// clear button
				SetCursorPos({ GetWindowWidth() - H::Draw.Scale(47), H::Draw.Scale(8) });
				bClear = FButton(ICON_MD_CLEAR, FButtonEnum::None, { 40, 40 }, 0, F::Render.IconFont);

				if (bCreate)
				{
					F::PlayerUtils.m_bSave = true;
					if (iID > -1 || iID < F::PlayerUtils.m_vTags.size())
					{
						F::PlayerUtils.m_vTags[iID].m_sName = tTag.m_sName;
						F::PlayerUtils.m_vTags[iID].m_tColor = tTag.m_tColor;
						F::PlayerUtils.m_vTags[iID].m_iPriority = tTag.m_iPriority;
						F::PlayerUtils.m_vTags[iID].m_bLabel = tTag.m_bLabel;
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

			auto drawTag = [](std::vector<PriorityLabel_t>::iterator it, PriorityLabel_t& _tTag, int y)
				{
					int _iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);

					bool bClicked = false, bDelete = false;

					ImVec2 vOriginalPos = { !_tTag.m_bLabel ? GetStyle().WindowPadding.x : GetWindowWidth() * 2 / 3 + GetStyle().WindowPadding.x / 2, H::Draw.Scale(96 + 36 * y) };

					// background
					float flWidth = GetWindowWidth() * (_tTag.m_bLabel ? 1.f / 3 : 2.f / 3) - GetStyle().WindowPadding.x * 1.5f;
					float flHeight = H::Draw.Scale(28);
					ImColor tColor = ColorToVec(_tTag.m_tColor.Lerp(Vars::Menu::Theme::Background.Value, 0.5f, LerpEnum::NoAlpha));
					ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
					if (iID != _iID)
						GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor, H::Draw.Scale(4));
					else
					{
						ImColor tColor2 = { tColor.Value.x * 1.1f, tColor.Value.y * 1.1f, tColor.Value.z * 1.1f, tColor.Value.w };
						GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor2, H::Draw.Scale(4));

						tColor2 = ColorToVec(_tTag.m_tColor.Lerp(Vars::Menu::Theme::Background.Value, 0.25f, LerpEnum::NoAlpha));
						float flInset = H::Draw.Scale(0.5f) - 0.5f;
						GetWindowDrawList()->AddRect({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + flWidth, vDrawPos.y - flInset + flHeight }, tColor2, H::Draw.Scale(4), ImDrawFlags_None, H::Draw.Scale());
					}

					// text
					SetCursorPos({ vOriginalPos.x + H::Draw.Scale(9), vOriginalPos.y + H::Draw.Scale(7) });
					FText(TruncateText(_tTag.m_sName, _tTag.m_bLabel ? flWidth - H::Draw.Scale(38) : flWidth / 2 - H::Draw.Scale(20)).c_str());

					if (!_tTag.m_bLabel)
					{
						SetCursorPos({ vOriginalPos.x + flWidth / 2, vOriginalPos.y + H::Draw.Scale(7) });
						FText(std::format("{}", _tTag.m_iPriority).c_str());
					}

					// buttons / icons
					if (!_tTag.m_bLocked)
					{
						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(26), vOriginalPos.y + H::Draw.Scale(2) });
						bDelete = IconButton(ICON_MD_DELETE);
					}
					else
					{
						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(22), vOriginalPos.y + H::Draw.Scale(6) });
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
					bClicked = Button(std::format("##{}", _tTag.m_sName).c_str(), { flWidth, flHeight });

					if (bClicked)
					{
						iID = _iID;
						tTag.m_sName = _tTag.m_sName;
						tTag.m_tColor = _tTag.m_tColor;
						tTag.m_iPriority = _tTag.m_iPriority;
						tTag.m_bLabel = _tTag.m_bLabel;
					}
					if (bDelete)
						OpenPopup(std::format("Confirmation## DeleteTag{}", _iID).c_str());
					if (FBeginPopupModal(std::format("Confirmation## DeleteTag{}", _iID).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText(std::format("Do you really want to delete '{}'?", _tTag.m_sName).c_str());

						if (FButton("Yes", FButtonEnum::Left))
						{
							F::PlayerUtils.m_vTags.erase(it);
							F::PlayerUtils.m_bSave = F::PlayerUtils.m_bSave = true;

							for (auto& [_, vTags] : F::PlayerUtils.m_mPlayerTags)
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
						if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
				};

			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			SetCursorPos({ H::Draw.Scale(13), H::Draw.Scale(80) }); FText("Priorities");
			SetCursorPos({ GetWindowWidth() * 2 / 3 + H::Draw.Scale(9), H::Draw.Scale(80) }); FText("Labels");
			PopStyleColor();

			std::vector<std::pair<std::vector<PriorityLabel_t>::iterator, PriorityLabel_t>> vPriorities = {}, vLabels = {};
			for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
			{
				auto& _tTag = *it;

				if (!_tTag.m_bLabel)
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
					if (a.second.m_iPriority != b.second.m_iPriority)
						return a.second.m_iPriority > b.second.m_iPriority;

					return a.second.m_sName < b.second.m_sName;
				});
			std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
				{
					// sort by priority if unequal
					if (a.second.m_iPriority != b.second.m_iPriority)
						return a.second.m_iPriority > b.second.m_iPriority;

					return a.second.m_sName < b.second.m_sName;
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
				SetCursorPosY(GetCursorPosY() - H::Draw.Scale(8));
				if (FButton(ICON_MD_SYNC, FButtonEnum::None, { 30, 30 }, 0, F::Render.IconFont))
					F::PlayerUtils.m_bLoad = true;

				if (FButton("Export", FButtonEnum::Fit | FButtonEnum::SameLine))
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

					if (FButton("Import", FButtonEnum::Fit | FButtonEnum::SameLine))
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
										if (auto getValue = it.second.get_optional<std::string>("Name")) { tTag.m_sName = *getValue; }
										if (const auto getChild = it.second.get_child_optional("Color")) { F::Configs.TreeToColor(*getChild, tTag.m_tColor); }
										if (auto getValue = it.second.get_optional<int>("Priority")) { tTag.m_iPriority = *getValue; }
										if (auto getValue = it.second.get_optional<bool>("Label")) { tTag.m_bLabel = *getValue; }

										int iID = -1;
										try
										{	// new id based indexing
											iID = std::stoi(it.first);
											iID = F::PlayerUtils.TagToIndex(iID);
										}
										catch (...) {}

										if (iID > -1 && iID < vTags.size())
										{
											vTags[iID].m_sName = tTag.m_sName;
											vTags[iID].m_tColor = tTag.m_tColor;
											vTags[iID].m_iPriority = tTag.m_iPriority;
											vTags[iID].m_bLabel = tTag.m_bLabel;
										}
										else
											vTags.push_back(tTag);
									}
								}

								if (auto tagTree = readTree.get_child_optional("Tags"))
								{
									for (auto& player : *tagTree)
									{
										uint32_t uFriendsID = std::stoi(player.first);

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
											if (!pTag || !pTag->m_bAssignable)
												continue;

											if (!F::PlayerUtils.HasTag(uFriendsID, iID, mPlayerTags))
												F::PlayerUtils.AddTag(uFriendsID, iID, false, "", mPlayerTags);
										}
									}
								}

								if (auto aliasTree = readTree.get_child_optional("Aliases"))
								{
									for (auto& player : *aliasTree)
									{
										uint32_t uFriendsID = std::stoi(player.first);
										std::string sAlias = player.second.data();

										if (!sAlias.empty())
											mPlayerAliases[uFriendsID] = player.second.data();
									}
								}
							}

							//std::filesystem::remove(F::Configs.m_sCorePath + "Import.json");

							for (int i = 0; i < vTags.size(); i++)
							{
								if (vTags[i].m_bAssignable)
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
							SDK::Output("Amalgam", "Failed to import playerlist", { 175, 150, 255, 127 }, true, true, true);
						}
					}

					SetNextWindowSize({ H::Draw.Scale(300), 0 });
					if (FBeginPopupModal("Import playerlist", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText("Import");
						FText("As", FTextEnum::Right | FTextEnum::SameLine);

						for (int i = 0; i < vTags.size(); i++)
						{
							if (!vTags[i].m_bAssignable)
								continue;

							auto& iIDTo = mAs[i];

							ImVec2 vOriginalPos = GetCursorPos();
							PushStyleColor(ImGuiCol_Text, ColorToVec(vTags[i].m_tColor));
							SetCursorPos(vOriginalPos + ImVec2(H::Draw.Scale(8), H::Draw.Scale(5)));
							FText(vTags[i].m_sName.c_str());
							PopStyleColor();
							SetCursorPos(vOriginalPos - ImVec2(0, H::Draw.Scale(8))); DebugDummy({ GetWindowWidth() - GetStyle().WindowPadding.x * 2, H::Draw.Scale(32) });

							std::vector<const char*> vEntries = { "None" };
							std::vector<int> vValues = { 0 };
							for (int i = 0; i < F::PlayerUtils.m_vTags.size(); i++)
							{
								if (F::PlayerUtils.m_vTags[i].m_bAssignable)
								{
									vEntries.push_back(F::PlayerUtils.m_vTags[i].m_sName.c_str());
									vValues.push_back(i + 1);
								}
							}
							PushTransparent(iIDTo == -1);
							{
								int iTo = iIDTo + 1;
								FDropdown(std::format("##{}", i).c_str(), &iTo, vEntries, vValues, FSliderEnum::Right);
								iIDTo = iTo - 1;
							}
							PopTransparent();
						}

						if (FButton("Import", FButtonEnum::Left))
						{
							for (auto& [uFriendsID, vTags] : mPlayerTags)
							{
								for (auto& iTag : vTags)
								{
									int iID = mAs.contains(iTag) ? mAs[iTag] : -1;
									if (iID != -1 && !F::PlayerUtils.HasTag(uFriendsID, iID))
										F::PlayerUtils.AddTag(uFriendsID, iID, false);
								}
							}
							for (auto& [uFriendsID, sAlias] : mPlayerAliases)
							{
								if (!F::PlayerUtils.m_mPlayerAliases.contains(uFriendsID))
									F::PlayerUtils.m_mPlayerAliases[uFriendsID] = sAlias;
							}

							F::PlayerUtils.m_bSave = true;
							SDK::Output("Amalgam", "Imported playerlist", { 175, 150, 255 }, true, true, true);

							CloseCurrentPopup();
						}
						if (FButton("Cancel", FButtonEnum::Right | FButtonEnum::SameLine))
							CloseCurrentPopup();

						EndPopup();
					}
				}

				if (FButton("Backup", FButtonEnum::Fit | FButtonEnum::SameLine))
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
						SDK::Output("Amalgam", "Failed to backup playerlist", { 175, 150, 255, 127 }, true, true, true);
					}
				}
			}
			PopDisabled();

			if (FButton("Folder", FButtonEnum::Fit | FButtonEnum::SameLine))
				ShellExecuteA(NULL, NULL, F::Configs.m_sCorePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		break;
	}
	// Settings
	case 1:
	{
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			{
				if (Section("Logging"))
				{
					FDropdown(Vars::Logging::Logs);
					FDropdown(Vars::Logging::NotificationPosition);
					FSlider(Vars::Logging::Lifetime);
				} EndSection();
				if (Section("Vote Start"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteStart));
					{
						FDropdown(Vars::Logging::VoteStart::LogTo);
					}
					PopTransparent();
				} EndSection();
				if (Section("Vote Cast"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::VoteCast));
					{
						FDropdown(Vars::Logging::VoteCast::LogTo);
					}
					PopTransparent();
				} EndSection();
				if (Section("Class Change"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::ClassChanges));
					{
						FDropdown(Vars::Logging::ClassChange::LogTo);
					}
					PopTransparent();
				} EndSection();
			}
			/* Column 2 */
			TableNextColumn();
			{
				if (Section("Damage"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Damage));
					{
						FDropdown(Vars::Logging::Damage::LogTo);
					}
					PopTransparent();
				} EndSection();
				if (Section("Cheat Detection"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::CheatDetection));
					{
						FDropdown(Vars::Logging::CheatDetection::LogTo);
					}
					PopTransparent();
				} EndSection();
				if (Section("Tags"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Tags));
					{
						FDropdown(Vars::Logging::Tags::LogTo);
					}
					PopTransparent();
				} EndSection();
				if (Section("Aliases"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Aliases));
					{
						FDropdown(Vars::Logging::Aliases::LogTo);
					}
					PopTransparent();
				} EndSection();
				if (Section("Resolver"))
				{
					PushTransparent(!(FGet(Vars::Logging::Logs) & Vars::Logging::LogsEnum::Resolver));
					{
						FDropdown(Vars::Logging::Resolver::LogTo);
					}
					PopTransparent();
				} EndSection();
			}
			EndTable();
		}
		break;
	}
	// Output
	case 2:
	{
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

				if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
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
}

void CMenu::MenuSettings(int iTab)
{
	using namespace ImGui;

	switch (iTab)
	{
	// Settings
	case 0:
	{
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/*
			if (Section("Config"))
			{
				static int iCurrentType = 0;
				PushFont(F::Render.FontBold);
				FTabs({ "GENERAL", "VISUAL", }, &iCurrentType, { H::Draw.Scale(20), H::Draw.Scale(28) }, { GetWindowWidth(), 0 }, FTabsEnum::AlignReverse | FTabsEnum::Fit);
				SetCursorPosY(GetCursorPosY() - H::Draw.Scale());
				PopFont();

				switch (iCurrentType)
			*/

			/* Column 1 */
			TableNextColumn();
			if (Section("Config"))
			{
				static std::string sStaticName;

				auto vTable = WidgetTable(2, H::Draw.Scale(48), { GetWindowWidth() - H::Draw.Scale(154) - GetStyle().WindowPadding.x });

				SetCursorPos(vTable[0].m_vPos);
				if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
				{
					FSDropdown("Name", &sStaticName, {}, FSDropdownEnum::AutoUpdate);
				} EndChild();

				SetCursorPos(vTable[1].m_vPos);
				if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
				{
					PushDisabled(sStaticName.empty());
					{
						if (FButton("Create", FButtonEnum::Fit, { 0, 40 }))
						{
							if (!std::filesystem::exists(F::Configs.m_sConfigPath + sStaticName))
								F::Configs.SaveConfig(sStaticName);
							sStaticName.clear();
						}
					}
					PopDisabled();
					if (FButton("Folder", FButtonEnum::Fit | FButtonEnum::SameLine, { 0, 40 }))
						ShellExecuteA(NULL, NULL, F::Configs.m_sConfigPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				} EndChild();

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

					SetCursorPos({ vOriginalPos.x + H::Draw.Scale(2), vOriginalPos.y + H::Draw.Scale(9) });
					if (IconButton(bCurrentConfig ? ICON_MD_REFRESH : ICON_MD_DOWNLOAD))
						F::Configs.LoadConfig(sConfigName);

					SetCursorPos({ H::Draw.Scale(43), vOriginalPos.y + H::Draw.Scale(14) });
					TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, TruncateText(sConfigName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(80)).c_str());

					int iOffset = 9;
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
					if (IconButton(ICON_MD_DELETE))
						OpenPopup(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str());

					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
					if (IconButton(ICON_MD_SAVE))
					{
						if (!bCurrentConfig || F::Configs.m_sCurrentVisuals.length())
							OpenPopup(std::format("Confirmation## SaveConfig{}", sConfigName).c_str());
						else
							F::Configs.SaveConfig(sConfigName);
					}

					if (FBeginPopupModal(std::format("Confirmation## SaveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText(std::format("Do you really want to override '{}'?", sConfigName).c_str());

						if (FButton("Yes, override", FButtonEnum::Left))
						{
							F::Configs.SaveConfig(sConfigName);
							CloseCurrentPopup();
						}
						if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
							CloseCurrentPopup();

						EndPopup();
					}

					if (FBeginPopupModal(std::format("Confirmation## RemoveConfig{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						FText(std::format("Do you really want to remove '{}'?", sConfigName).c_str());

						PushDisabled(FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"));
						{
							if (FButton("Yes, delete", FButtonEnum::Fit))
							{
								F::Configs.DeleteConfig(sConfigName);
								CloseCurrentPopup();
							}
						}
						PopDisabled();
						if (FButton("Yes, reset", FButtonEnum::Fit | FButtonEnum::SameLine))
						{
							F::Configs.ResetConfig(sConfigName);
							CloseCurrentPopup();
						}
						if (FButton("No", FButtonEnum::Fit | FButtonEnum::SameLine))
							CloseCurrentPopup();

						EndPopup();
					}

					SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
				}
				DebugDummy({ 0, H::Draw.Scale(7) });
			} EndSection();
			SetCursorPosX(GetCursorPosX() + 8);
			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			FText("Built @ " __DATE__ ", " __TIME__ ", " __CONFIGURATION__);

			PopStyleColor();

			/* Column 2 */
			TableNextColumn();
			if (Section("Visuals"))
			{
				static std::string sStaticName;

				auto vTable = WidgetTable(2, H::Draw.Scale(48), { GetWindowWidth() - H::Draw.Scale(154) - GetStyle().WindowPadding.x });

				SetCursorPos(vTable[0].m_vPos);
				if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
				{
					FSDropdown("Name", &sStaticName, {}, FSDropdownEnum::AutoUpdate);
				} EndChild();

				SetCursorPos(vTable[1].m_vPos);
				if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
				{
					PushDisabled(sStaticName.empty());
					{
						if (FButton("Create", FButtonEnum::Fit, { 0, 40 }))
						{
							if (!std::filesystem::exists(F::Configs.m_sVisualsPath + sStaticName))
								F::Configs.SaveVisual(sStaticName);
							sStaticName.clear();
						}
					}
					PopDisabled();
					if (FButton("Folder", FButtonEnum::Fit | FButtonEnum::SameLine, { 0, 40 }))
						ShellExecuteA(NULL, NULL, F::Configs.m_sVisualsPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				} EndChild();

				for (auto& entry : std::filesystem::directory_iterator(F::Configs.m_sVisualsPath))
				{
					if (!entry.is_regular_file() || entry.path().extension() != F::Configs.m_sConfigExtension)
						continue;

					std::string sConfigName = entry.path().filename().string();
					sConfigName.erase(sConfigName.end() - F::Configs.m_sConfigExtension.size(), sConfigName.end());

					bool bCurrentConfig = FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32(F::Configs.m_sCurrentVisuals.c_str());
					ImVec2 vOriginalPos = GetCursorPos();

					SetCursorPos({ vOriginalPos.x + H::Draw.Scale(2), vOriginalPos.y + H::Draw.Scale(9) });
					if (IconButton(bCurrentConfig ? ICON_MD_REFRESH : ICON_MD_DOWNLOAD))
						F::Configs.LoadVisual(sConfigName);

					SetCursorPos({ H::Draw.Scale(43), vOriginalPos.y + H::Draw.Scale(14) });
					TextColored(bCurrentConfig ? F::Render.Active.Value : F::Render.Inactive.Value, TruncateText(sConfigName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(80)).c_str());

					int iOffset = 9;
					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
					if (IconButton(ICON_MD_DELETE))
						OpenPopup(std::format("Confirmation## DeleteVisual{}", sConfigName).c_str());

					SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
					if (IconButton(ICON_MD_SAVE))
					{
						if (!bCurrentConfig)
							OpenPopup(std::format("Confirmation## SaveVisual{}", sConfigName).c_str());
						else
							F::Configs.SaveVisual(sConfigName);
					}

					// Dialogs
					{
						// Save config dialog
						if (FBeginPopupModal(std::format("Confirmation## SaveVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("Do you really want to override '{}'?", sConfigName).c_str());

							if (FButton("Yes, override", FButtonEnum::Left))
							{
								F::Configs.SaveVisual(sConfigName);
								CloseCurrentPopup();
							}
							if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
								CloseCurrentPopup();

							EndPopup();
						}

						// Delete config dialog
						if (FBeginPopupModal(std::format("Confirmation## DeleteVisual{}", sConfigName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("Do you really want to delete '{}'?", sConfigName).c_str());

							if (FButton("Yes, delete", FButtonEnum::Left))
							{
								F::Configs.DeleteVisual(sConfigName);
								CloseCurrentPopup();
							}
							if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
								CloseCurrentPopup();

							EndPopup();
						}
					}

					SetCursorPos(vOriginalPos); DebugDummy({ 0, H::Draw.Scale(28) });
				}
				DebugDummy({ 0, H::Draw.Scale(7) });
			} EndSection();

			EndTable();
		}
		break;
	}
	// Binds
	case 1:
	{
		if (Section("Settings", 8))
		{
			auto vTable = WidgetTable(3, H::Draw.Scale(24));

			SetCursorPos(vTable[0].m_vPos);
			if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
			{
				FToggle(Vars::Menu::BindWindow);
			} EndChild();

			SetCursorPos(vTable[1].m_vPos);
			if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
			{
				FToggle(Vars::Menu::BindWindowTitle);
			} EndChild();

			SetCursorPos(vTable[2].m_vPos);
			if (BeginChild(vTable[2].m_sName.c_str(), vTable[2].m_vSize, vTable[2].m_iWindowFlags, vTable[2].m_iChildFlags))
			{
				FToggle(Vars::Menu::MenuShowsBinds);
			} EndChild();
		} EndSection();
		if (Section("Binds"))
		{
			static int iBind = DEFAULT_BIND;
			static Bind_t tBind = {};

			static int bParent = false;
			if (bParent)
				SetMouseCursor(ImGuiMouseCursor_Hand);

			auto vTable = WidgetTable(2, H::Draw.Scale(104));

			SetCursorPos(vTable[0].m_vPos);
			if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
			{
				FSDropdown("Name", &tBind.m_sName, {}, FDropdownEnum::Left | FSDropdownEnum::AutoUpdate);
				{
					auto sParent = bParent ? "..." : tBind.m_iParent != DEFAULT_BIND && tBind.m_iParent < F::Binds.m_vBinds.size() ? F::Binds.m_vBinds[tBind.m_iParent].m_sName : "None";
					if (FButton(std::format("Parent: {}", sParent).c_str(), FButtonEnum::Right | FButtonEnum::SameLine | FButtonEnum::NoUpper, { 0, 40 }))
						bParent = 2;
				}
				FDropdown("Type", &tBind.m_iType, { "Key", "Class", "Weapon type", "Item slot" }, {}, FDropdownEnum::Left);
				switch (tBind.m_iType)
				{
				case BindEnum::Key: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 2); FDropdown("Behavior", &tBind.m_iInfo, { "Hold", "Toggle", "Double click" }, {}, FDropdownEnum::Right); break;
				case BindEnum::Class: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 8); FDropdown("Class", &tBind.m_iInfo, { "Scout", "Soldier", "Pyro", "Demoman", "Heavy", "Engineer", "Medic", "Sniper", "Spy" }, {}, FDropdownEnum::Right); break;
				case BindEnum::WeaponType: tBind.m_iInfo = std::clamp(tBind.m_iInfo, 0, 2); FDropdown("Weapon type", &tBind.m_iInfo, { "Hitscan", "Projectile", "Melee" }, {}, FDropdownEnum::Right); break;
				case BindEnum::ItemSlot: tBind.m_iInfo = std::max(tBind.m_iInfo, 0); FDropdown("Item slot", &tBind.m_iInfo, { "1", "2", "3", "4", "5", "6", "7", "8", "9" }, {}, FDropdownEnum::Right); break;
				}
			} EndChild();

			SetCursorPos(vTable[1].m_vPos);
			if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
			{
				int iNot = tBind.m_bNot;
				FDropdown("While", &iNot, { "Active", "Not active" }, {}, FDropdownEnum::Left);
				tBind.m_bNot = iNot;
				FDropdown("Visibility", &tBind.m_iVisibility, { "Always", "While active", "Hidden" }, {}, FDropdownEnum::Right);
				if (tBind.m_iType == 0)
					FKeybind("Key", tBind.m_iKey, FButtonEnum::None, { Vars::Menu::MenuPrimaryKey[DEFAULT_BIND], Vars::Menu::MenuSecondaryKey[DEFAULT_BIND] }, { 0, 40 }, -96);

				// create/modify button
				bool bCreate = false, bClear = false, bParent = true;
				if (tBind.m_iParent != DEFAULT_BIND)
					bParent = F::Binds.m_vBinds.size() > tBind.m_iParent;

				SetCursorPos({ GetWindowWidth() - H::Draw.Scale(96), H::Draw.Scale(56) });
				PushDisabled(!bParent || !(tBind.m_iType == BindEnum::Key ? tBind.m_iKey : true));
				{
					bool bMatch = iBind != DEFAULT_BIND && F::Binds.m_vBinds.size() > iBind;
					bCreate = FButton(bMatch ? ICON_MD_SETTINGS : ICON_MD_ADD, FButtonEnum::None, { 40, 40 }, 0, F::Render.IconFont);
				}
				PopDisabled();

				// clear button
				SetCursorPos({ GetWindowWidth() - H::Draw.Scale(48), H::Draw.Scale(56) });
				bClear = FButton(ICON_MD_CLEAR, FButtonEnum::None, { 40, 40 }, 0, F::Render.IconFont);

				if (bCreate)
					F::Binds.AddBind(iBind, tBind);
				if (bCreate || bClear)
				{
					iBind = DEFAULT_BIND;
					tBind = {};
				}
			} EndChild();

			PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
			SetCursorPos({ H::Draw.Scale(13), H::Draw.Scale(128) });
			FText("Binds");
			SetCursorPosY(GetCursorPosY() - H::Draw.Scale(5));
			PopStyleColor();

			std::unordered_map<int, bool> mBinds = {};
			std::function<void(int, int)> getBinds = [&](int iParent, int x)
				{
					for (auto it = F::Binds.m_vBinds.begin(); it < F::Binds.m_vBinds.end(); it++)
					{
						int _iBind = std::distance(F::Binds.m_vBinds.begin(), it);
						auto& _tBind = *it;
						if (iParent != DEFAULT_BIND - 1 && iParent != _tBind.m_iParent || mBinds.contains(_iBind))
							continue;

						mBinds[_iBind] = true;

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

						ImVec2 vOriginalPos = { H::Draw.Scale(8) + H::Draw.Scale(28) * std::min(x, 3), GetCursorPosY() + H::Draw.Scale(8) };

						// background
						float flWidth = GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(28) * std::min(x, 3);
						float flHeight = H::Draw.Scale(28);
						ImVec2 vDrawPos = GetDrawPos() + vOriginalPos;
						if (iBind != _iBind)
							GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, F::Render.Background1p5, H::Draw.Scale(4));
						else
						{
							ImColor tColor = F::Render.Background1p5L;
							GetWindowDrawList()->AddRectFilled(vDrawPos, { vDrawPos.x + flWidth, vDrawPos.y + flHeight }, tColor, H::Draw.Scale(4));

							tColor = ColorToVec((VecToColor(F::Render.Background1p5)).Lerp({ 127, 127, 127 }, 1.f / 9, LerpEnum::NoAlpha));
							float flInset = H::Draw.Scale(0.5f) - 0.5f;
							GetWindowDrawList()->AddRect({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + flWidth, vDrawPos.y - flInset + flHeight }, tColor, H::Draw.Scale(4), ImDrawFlags_None, H::Draw.Scale());
						}

						// text
						if (x > 3)
						{	// don't indent too much
							auto sText = std::format("-> {}", x);
							SetCursorPos({ vOriginalPos.x - FCalcTextSize(sText.c_str()).x - H::Draw.Scale(10), vOriginalPos.y + H::Draw.Scale(7) });
							FText(sText.c_str());
						}

						float flTextWidth = flWidth - H::Draw.Scale(127);
						PushTransparent(!F::Binds.WillBeEnabled(_iBind), true);

						SetCursorPos({ vOriginalPos.x + H::Draw.Scale(9), vOriginalPos.y + H::Draw.Scale(7) });
						FText(TruncateText(_tBind.m_sName, flTextWidth * (1.f / 3) - H::Draw.Scale(20)).c_str());

						SetCursorPos({ vOriginalPos.x + flTextWidth * (1.f / 3), vOriginalPos.y + H::Draw.Scale(7) });
						FText(sType.c_str());

						SetCursorPos({ vOriginalPos.x + flTextWidth * (2.f / 3), vOriginalPos.y + H::Draw.Scale(7) });
						FText(sInfo.c_str());

						// buttons
						int iOffset = 1;

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(2) });
						bool bDelete = IconButton(ICON_MD_DELETE);

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(2) });
						if (IconButton(ICON_MD_EDIT))
							CurrentBind = CurrentBind != _iBind ? _iBind : DEFAULT_BIND;

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(2) });
						if (IconButton(!_tBind.m_bNot ? ICON_MD_CODE : ICON_MD_CODE_OFF))
							_tBind.m_bNot = !_tBind.m_bNot;

						PushTransparent(Transparent || _tBind.m_iVisibility == BindVisibilityEnum::Hidden, true);
						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(2) });
						if (IconButton(_tBind.m_iVisibility == BindVisibilityEnum::Always ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF))
							_tBind.m_iVisibility = (_tBind.m_iVisibility + 1) % 3;
						PopTransparent(1, 1);

						SetCursorPos({ vOriginalPos.x + flWidth - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(2) });
						if (IconButton(_tBind.m_bEnabled ? ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF))
							_tBind.m_bEnabled = !_tBind.m_bEnabled;

						SetCursorPos(vOriginalPos);
						bool bClicked = Button(std::format("##{}", _iBind).c_str(), { flWidth, flHeight });

						PopTransparent(1, 1);

						if (bClicked)
						{
							if (bParent)
							{
								bParent = false;
								tBind.m_iParent = _iBind;

								// make sure bind can't be parented to itself or any of its children
								int _iBind2 = _iBind;
								Bind_t _tBind2;
								while (F::Binds.GetBind(_iBind2, &_tBind2))
								{
									if (_iBind2 == iBind)
										tBind.m_iParent = DEFAULT_BIND;
									_iBind2 = _tBind2.m_iParent;
								}
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

							if (FButton("Yes", FButtonEnum::Left))
							{
								F::Binds.RemoveBind(_iBind);
								CloseCurrentPopup();

								iBind = DEFAULT_BIND;
								tBind = {};
							}
							if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
								CloseCurrentPopup();

							EndPopup();
						}

						if (iParent != DEFAULT_BIND - 1)
							getBinds(_iBind, x + 1);
					}
				};
			getBinds(DEFAULT_BIND, 0);

			// this should ideally never happen, but failsafe
			if (F::Binds.m_vBinds.size() > mBinds.size())
			{
				PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
				SetCursorPos({ H::Draw.Scale(13), GetCursorPosY() + H::Draw.Scale(5) });
				FText("Dangling");
				SetCursorPosY(GetCursorPosY() - H::Draw.Scale(5));
				PopStyleColor();

				getBinds(DEFAULT_BIND - 1, 0);
			}

			if (bParent == 2) // dumb
				bParent = 1;
			else if (bParent && IsMouseReleased(ImGuiMouseButton_Left))
			{
				bParent = false;
				tBind.m_iParent = DEFAULT_BIND;
			}
		} EndSection();
		break;
	}
	// Materials
	case 2:
	{
		static TextEditor TextEditor;
		static std::string CurrentMaterial;
		static bool LockedMaterial;

		bool bTable = false;
		if (!CurrentMaterial.empty())
			bTable = BeginTable("MaterialsTable", 2);
		{
			if (bTable)
			{
				TableSetupColumn("MaterialsTable1", ImGuiTableColumnFlags_WidthFixed, H::Draw.Scale(288));
				TableSetupColumn("MaterialsTable2", ImGuiTableColumnFlags_WidthFixed, GetWindowWidth());

				/* Column 1 */
				TableNextColumn();
			}

			if (Section("Manager"))
			{
				static std::string sStaticName;

				auto vTable = WidgetTable(2, H::Draw.Scale(48), { GetWindowWidth() - H::Draw.Scale(154) - GetStyle().WindowPadding.x });

				SetCursorPos(vTable[0].m_vPos);
				if (BeginChild(vTable[0].m_sName.c_str(), vTable[0].m_vSize, vTable[0].m_iWindowFlags, vTable[0].m_iChildFlags))
				{
					FSDropdown("Name", &sStaticName, {}, FSDropdownEnum::AutoUpdate);
				} EndChild();

				SetCursorPos(vTable[1].m_vPos);
				if (BeginChild(vTable[1].m_sName.c_str(), vTable[1].m_vSize, vTable[1].m_iWindowFlags, vTable[1].m_iChildFlags))
				{
					PushDisabled(sStaticName.empty());
					{
						if (FButton("Create", FButtonEnum::Fit, { 0, 40 }))
						{
							F::Materials.AddMaterial(sStaticName.c_str());
							sStaticName.clear();
						}
					}
					PopDisabled();
					if (FButton("Folder", FButtonEnum::Fit | FButtonEnum::SameLine, { 0, 40 }))
						ShellExecuteA(NULL, NULL, F::Configs.m_sMaterialsPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				} EndChild();

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

					SetCursorPos({ H::Draw.Scale(17), vOriginalPos.y + H::Draw.Scale(14) });
					TextColored(tMaterial.m_bLocked ? F::Render.Inactive.Value : F::Render.Active.Value, TruncateText(tMaterial.m_sName, GetWindowWidth() - GetStyle().WindowPadding.x * 2 - H::Draw.Scale(56)).c_str());

					int iOffset = 9;
					if (!tMaterial.m_bLocked)
					{
						SetCursorPos({ GetWindowWidth() - H::Draw.Scale(iOffset += 25), vOriginalPos.y + H::Draw.Scale(9) });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteMat{}", tMaterial.m_sName).c_str());
						if (FBeginPopupModal(std::format("Confirmation## DeleteMat{}", tMaterial.m_sName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							FText(std::format("Do you really want to delete '{}'?", tMaterial.m_sName).c_str());

							if (FButton("Yes", FButtonEnum::Left))
							{
								F::Materials.RemoveMaterial(tMaterial.m_sName.c_str());
								CloseCurrentPopup();
							}
							if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
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
				DebugDummy({ 0, H::Draw.Scale(7) });
			}
			else
				SetScrollY(0);
			EndSection();

			/* Column 2 */
			if (bTable)
			{
				TableNextColumn();
				if (CurrentMaterial.length())
				{
					SetCursorPosY(GetScrollY() + GetStyle().WindowPadding.y);
					if (Section("Editor", 0, GetWindowHeight() - GetStyle().WindowPadding.y * 2, true))
					{
						// Toolbar
						if (!LockedMaterial)
						{
							if (FButton("Save", FButtonEnum::Fit))
							{
								auto sText = TextEditor.GetText();
								F::Materials.EditMaterial(CurrentMaterial.c_str(), sText.c_str());
							}
							SameLine();
						}
						if (FButton("Close", FButtonEnum::Fit))
							CurrentMaterial = "";
						SetCursorPosY(H::Draw.Scale(52));
						PushStyleColor(ImGuiCol_Text, F::Render.Inactive.Value);
						FText(std::format("{}: {}", LockedMaterial ? "Viewing" : "Editing", CurrentMaterial).c_str(), FTextEnum::Right);
						PopStyleColor();

						// Text editor
						DebugDummy({ 0, H::Draw.Scale(8) });

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
						
						PushFont(F::Render.FontMono);
						ImVec2 vDrawPos = GetDrawPos() + GetCursorPos();
						TextEditor.Render("TextEditor");
						ImVec2 vSize = GetItemRectSize();
						float flInset = H::Draw.Scale(0.5f) - 0.5f;
						GetWindowDrawList()->AddRect({ vDrawPos.x + flInset, vDrawPos.y + flInset }, { vDrawPos.x - flInset + vSize.x, vDrawPos.y - flInset + vSize.y }, F::Render.Background2, H::Draw.Scale(4), ImDrawFlags_None, H::Draw.Scale());
						PopFont();
					} EndSection();
				}

				EndTable();
			}
		}
		break;
	}
	// Extra
	case 3:
	{
		if (Section("Debug", 8))
		{
			FToggle(Vars::Debug::Info, FToggleEnum::Left);
			FToggle(Vars::Debug::Logging, FToggleEnum::Right);
			FToggle(Vars::Debug::Options, FToggleEnum::Left);
			FToggle(Vars::Debug::DrawServerHitboxes, FToggleEnum::Right, &Hovered); FTooltip("Only localhost servers", Hovered);
			FToggle(Vars::Debug::AntiAimLines, FToggleEnum::Left);
			FToggle(Vars::Debug::CrashLogging, FToggleEnum::Right);
#ifdef DEBUG_TRACES
			FToggle(Vars::Debug::VisualizeTraces, FToggleEnum::Left);
			FToggle(Vars::Debug::VisualizeTraceHits, FToggleEnum::Right);
#endif
		} EndSection();
		if (Section("Extra"))
		{
			if (FButton("cl_fullupdate", FButtonEnum::Left))
				I::EngineClient->ClientCmd_Unrestricted("cl_fullupdate");
			if (FButton("retry", FButtonEnum::Right | FButtonEnum::SameLine))
				I::EngineClient->ClientCmd_Unrestricted("retry");
			if (FButton("Console", FButtonEnum::Left))
				I::EngineClient->ClientCmd_Unrestricted("toggleconsole");
			if (FButton("Fix materials", FButtonEnum::Right | FButtonEnum::SameLine) && F::Materials.m_bLoaded)
				F::Materials.ReloadMaterials();

			if (!I::EngineClient->IsConnected())
			{
				if (FButton("Unlock achievements", FButtonEnum::Left))
					OpenPopup("UnlockAchievements");
				if (FButton("Lock achievements", FButtonEnum::Right | FButtonEnum::SameLine))
					OpenPopup("LockAchievements");

				if (FBeginPopupModal("UnlockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
				{
					FText("Do you really want to unlock all achievements?");

					if (FButton("Yes, unlock", FButtonEnum::Left))
					{
						F::Misc.UnlockAchievements();
						CloseCurrentPopup();
					}
					if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
						CloseCurrentPopup();

					EndPopup();
				}
				if (FBeginPopupModal("LockAchievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
				{
					FText("Do you really want to lock all achievements?");

					if (FButton("Yes, lock", FButtonEnum::Left))
					{
						F::Misc.LockAchievements();
						CloseCurrentPopup();
					}
					if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
						CloseCurrentPopup();

					EndPopup();
				}
			}

		} EndSection();
		if (Vars::Debug::Options.Value && I::EngineClient->IsConnected())
		{
			if (Section("##Debug", -8))
			{
				if (FButton("Restore lines", FButtonEnum::Left))
				{
					for (auto& tLine : G::LineStorage)
						tLine.m_flTime = I::GlobalVars->curtime + 60.f;
				}
				if (FButton("Restore paths", FButtonEnum::Right | FButtonEnum::SameLine))
				{
					for (auto& tPath : G::PathStorage)
						tPath.m_flTime = I::GlobalVars->curtime + 60.f;
				}
				if (FButton("Restore boxes", FButtonEnum::Left))
				{
					for (auto& tBox : G::BoxStorage)
						tBox.m_flTime = I::GlobalVars->curtime + 60.f;
				}
				if (FButton("Clear visuals", FButtonEnum::Right | FButtonEnum::SameLine))
				{
					G::LineStorage.clear();
					G::PathStorage.clear();
					G::BoxStorage.clear();
					G::SphereStorage.clear();
					G::SweptStorage.clear();
				}
			} EndSection();
		}
		/*
		if (Vars::Debug::Options.Value)
		{
			if (Section("Convar spoofer"))
			{
				static std::string sName = "", sValue = "";

				FSDropdown("Convar", &sName, {}, FDropdownEnum::Left);
				FSDropdown("Value", &sValue, {}, FDropdownEnum::Right);
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
		*/
#ifdef DEBUG_HOOKS
		if (Section("Hooks", 8))
		{
			int i = 0; for (auto& pVar : G::Vars)
			{
				if (pVar->m_sName.find("Vars::Hooks::") == std::string::npos)
					continue;

				FToggle(*pVar->As<bool>(), !(i % 2) ? FToggleEnum::Left : FToggleEnum::Right);
				i++;
			}
		} EndSection();
#endif
	}
	}
}

void CMenu::MenuSearch(std::string sSearch)
{
	using namespace ImGui;

	if (sSearch.empty())
		return;

	static std::vector<BaseVar*> vVars = {}; // don't string search every single frame

	static uint32_t uStaticHash = 0;
	const uint32_t uLastHash = uStaticHash;
	const uint32_t uCurrHash = uStaticHash = FNV1A::Hash32(sSearch.c_str());

	if (uCurrHash != uLastHash)
	{
		std::string sSearch2 = sSearch;
		std::transform(sSearch2.begin(), sSearch2.end(), sSearch2.begin(), ::tolower);

		vVars.clear();
		for (auto pVar : G::Vars)
		{
			if (!Vars::Debug::Options[DEFAULT_BIND] && pVar->m_iFlags & DEBUGVAR)
				continue;

			std::vector<const char*> vSearch = { pVar->m_sName.c_str(), pVar->m_sSection };
			vSearch.insert(vSearch.end(), pVar->m_vTitle.begin(), pVar->m_vTitle.end());
			vSearch.insert(vSearch.end(), pVar->m_vValues.begin(), pVar->m_vValues.end());
			for (auto pSearch : vSearch)
			{
				std::string sSearch3 = pSearch;
				if (auto iFind = sSearch3.find("Vars::"); iFind != std::string::npos)
					sSearch3 = sSearch3.replace(iFind, strlen("Vars::"), "");
				if (auto iFind = sSearch3.find("##"); iFind != std::string::npos)
					sSearch3 = sSearch3.replace(iFind, strlen("##"), "");
				std::transform(sSearch3.begin(), sSearch3.end(), sSearch3.begin(), ::tolower);
				if (sSearch3.find(sSearch2) != std::string::npos)
				{
					vVars.push_back(pVar);
					break;
				}
			}
		}
	}

	if (vVars.empty())
		return;

	uint32_t uLastSection = 0;
	int i = 0; for (auto pBase : vVars) // possibly implement tablelike visuals, do away with left right, just switch if current side is higher than other?
	{
		int iWidgetEnum = WidgetEnum::Invalid, iTypeEnum = WidgetEnum::Invalid;
		if (auto pVar = pBase->As<bool>())
			iWidgetEnum = iTypeEnum = WidgetEnum::FToggle;
		else if (auto pVar = pBase->As<int>())
		{
			if (!pVar->m_vValues.empty())
				iWidgetEnum = iTypeEnum = WidgetEnum::FDropdown;
			else if (pVar->m_sExtra)
				iWidgetEnum = WidgetEnum::FISlider, iTypeEnum = WidgetEnum::FSlider;
			else
				iWidgetEnum = iTypeEnum = WidgetEnum::FKeybind;
		}
		else if (auto pVar = pBase->As<float>())
			iWidgetEnum = WidgetEnum::FFSlider, iTypeEnum = WidgetEnum::FSlider;
		else if (auto pVar = pBase->As<IntRange_t>())
			iWidgetEnum = WidgetEnum::FIRSlider, iTypeEnum = WidgetEnum::FSlider;
		else if (auto pVar = pBase->As<FloatRange_t>())
			iWidgetEnum = WidgetEnum::FFRSlider, iTypeEnum = WidgetEnum::FSlider;
		else if (auto pVar = pBase->As<std::string>())
			iWidgetEnum = WidgetEnum::FSDropdown, iTypeEnum = WidgetEnum::FDropdown;
		else if (auto pVar = pBase->As<std::vector<std::pair<std::string, Color_t>>>())
			iWidgetEnum = WidgetEnum::FMDropdown, iTypeEnum = WidgetEnum::FDropdown;
		else if (auto pVar = pBase->As<Color_t>())
			iWidgetEnum = WidgetEnum::FColorPicker, iTypeEnum = WidgetEnum::FToggle;
		else if (auto pVar = pBase->As<Gradient_t>())
			iWidgetEnum = WidgetEnum::FGColorPicker, iTypeEnum = WidgetEnum::FToggle;
		else
			continue;

		uint32_t uSection = FNV1A::Hash32(pBase->m_sSection);
		if (uSection != uLastSection)
		{
			if (uLastSection)
				EndSection();
			const char* sSection = pBase->m_sSection;
			switch (FNV1A::Hash32(sSection))
			{
			case FNV1A::Hash32Const("Enemy Chams"):
				if (!FGet(Vars::Chams::Relative))
					sSection = "BLU Chams";
				break;
			case FNV1A::Hash32Const("Team Chams"):
				if (!FGet(Vars::Chams::Relative))
					sSection = "RED Chams";
				break;
			}
			Section(std::format("{}## {}", pBase->m_sSection, pBase->m_sName).c_str());
			i = 0;
		}
		uLastSection = uSection;

		static int iLastEnum = WidgetEnum::Invalid;
		if (!i || iTypeEnum != iLastEnum)
		{
			if (!i)
			{
				switch (iWidgetEnum)
				{
				case WidgetEnum::FToggle:
				case WidgetEnum::FISlider:
				case WidgetEnum::FFSlider:
				case WidgetEnum::FIRSlider:
				case WidgetEnum::FFRSlider:
				case WidgetEnum::FColorPicker:
				case WidgetEnum::FGColorPicker:
					DebugDummy({ 0, H::Draw.Scale(8) });
				}
				i = 2;
			}
			else if (iTypeEnum == WidgetEnum::FToggle && iLastEnum == WidgetEnum::FSlider && (i % 2))
			{
				SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetRowPos() + H::Draw.Scale(8) });
				i = 0;
			}
			else if (iTypeEnum == WidgetEnum::FSlider && iLastEnum == WidgetEnum::FDropdown && (i % 2) && !vRowSizes.empty())
			{
				auto& tRow = vRowSizes.front();
				tRow.m_vPos.y += H::Draw.Scale(13), tRow.m_vSize.y -= H::Draw.Scale(13);
				SetCursorPos({ GetWindowWidth() / 2 + GetStyle().WindowPadding.x / 2, GetRowPos() });
				i = 0;
			}
			else
				i = 2;
		}
		iLastEnum = iTypeEnum;

		int iOverride = -1;
		switch (iWidgetEnum)
		{
		case WidgetEnum::FToggle:
		{
			auto pVar = pBase->As<bool>();
			bool bTransparent = false;
			switch (FNV1A::Hash32(pVar->m_sName.c_str()))
			{
			case FNV1A::Hash32Const("Vars::Chams::Enemy::Players"):
			case FNV1A::Hash32Const("Vars::Chams::Enemy::Ragdolls"):
			case FNV1A::Hash32Const("Vars::Chams::Enemy::Buildings"):
			case FNV1A::Hash32Const("Vars::Chams::Enemy::Projectiles"):
			case FNV1A::Hash32Const("Vars::Chams::Team::Players"):
			case FNV1A::Hash32Const("Vars::Chams::Team::Ragdolls"):
			case FNV1A::Hash32Const("Vars::Chams::Team::Buildings"):
			case FNV1A::Hash32Const("Vars::Chams::Team::Projectiles"):
				iOverride = FGet(Vars::Chams::Relative) ? 1 : 2;
				break;
			case FNV1A::Hash32Const("Vars::Chams::EnemyChams"):
			case FNV1A::Hash32Const("Vars::Chams::TeamChams"):
				bTransparent = FGet(Vars::Chams::Relative);
			}
			PushTransparent(bTransparent);
			if (FToggle(*pVar, !(i % 2) ? FToggleEnum::Left : FToggleEnum::Right, nullptr, iOverride/*, iOverride*/))
			{
				if (FNV1A::Hash32(pVar->m_sName.c_str()) == FNV1A::Hash32Const("Vars::Debug::Options"))
					uStaticHash = 0;
			}
			PopTransparent();
			break;
		}
		case WidgetEnum::FISlider:
		{
			auto pVar = pBase->As<int>();
			FSlider(*pVar, !(i % 2) ? FSliderEnum::Left : FSliderEnum::Right, nullptr, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FFSlider:
		{
			auto pVar = pBase->As<float>();
			const char* sFormat = pVar->m_sExtra;
			switch (FNV1A::Hash32(pVar->m_sName.c_str()))
			{
			case FNV1A::Hash32Const("Vars::Aimbot::Projectile::SplashRotateX"):
			case FNV1A::Hash32Const("Vars::Aimbot::Projectile::SplashRotateY"):
				if (pVar->Map[DEFAULT_BIND] < 0.f)
					sFormat = "random";
			}
			FSlider(*pVar, !(i % 2) ? FSliderEnum::Left : FSliderEnum::Right, sFormat, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FIRSlider:
		{
			auto pVar = pBase->As<IntRange_t>();
			FSlider(*pVar, !(i % 2) ? FSliderEnum::Left : FSliderEnum::Right, nullptr, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FFRSlider:
		{
			auto pVar = pBase->As<FloatRange_t>();
			FSlider(*pVar, !(i % 2) ? FSliderEnum::Left : FSliderEnum::Right, nullptr, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FDropdown:
		{
			auto pVar = pBase->As<int>();
			FDropdown(*pVar, !(i % 2) ? FDropdownEnum::Left : FDropdownEnum::Right, 0, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FSDropdown:
		{
			auto pVar = pBase->As<std::string>();
			FSDropdown(*pVar, !(i % 2) ? FDropdownEnum::Left : FDropdownEnum::Right, 0, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FMDropdown:
		{
			auto pVar = pBase->As<std::vector<std::pair<std::string, Color_t>>>();
			FMDropdown(*pVar, !(i % 2) ? FDropdownEnum::Left : FDropdownEnum::Right, 0, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FColorPicker:
		{
			auto pVar = pBase->As<Color_t>();
			switch (FNV1A::Hash32(pVar->m_sName.c_str()))
			{
			case FNV1A::Hash32Const("Vars::Colors::Backtrack"):
			case FNV1A::Hash32Const("Vars::Colors::FakeAngle"):
				iOverride = int(pBase->m_vTitle.size() - 1);
				break;
			case FNV1A::Hash32Const("Vars::Colors::Enemy"):
			case FNV1A::Hash32Const("Vars::Colors::Team"):
				if (!FGet(Vars::Colors::Relative))
					iOverride = -2;
				break;
			case FNV1A::Hash32Const("Vars::Colors::TeamRed"):
			case FNV1A::Hash32Const("Vars::Colors::TeamBlu"):
				if (FGet(Vars::Colors::Relative))
					iOverride = -2;
			}
			if (iOverride == -2)
				break;
			FColorPicker(*pVar, 0, !(i % 2) ? FColorPickerEnum::Left : FColorPickerEnum::Middle, nullptr, iOverride, iOverride);
			break;
		}
		case WidgetEnum::FGColorPicker:
		{
			auto pVar = pBase->As<Gradient_t>();
			FColorPicker(*pVar, true, 0, !(i % 2) ? FColorPickerEnum::Left : FColorPickerEnum::Middle, nullptr, iOverride/*, iOverride*/);
			FColorPicker(*pVar, false, 0, !(++i % 2) ? FColorPickerEnum::Left : FColorPickerEnum::Middle, nullptr, iOverride/*, iOverride*/);
			break;
		}
		case WidgetEnum::FKeybind:
		{
			auto pVar = pBase->As<int>();
			std::vector<int> vIgnore;
			switch (FNV1A::Hash32(pVar->m_sName.c_str()))
			{
			case FNV1A::Hash32Const("Vars::Menu::MenuPrimaryKey"):
				vIgnore = { Vars::Menu::MenuSecondaryKey[DEFAULT_BIND], VK_LBUTTON, VK_RBUTTON };
				break;
			case FNV1A::Hash32Const("Vars::Menu::MenuSecondaryKey"):
				vIgnore = { Vars::Menu::MenuPrimaryKey[DEFAULT_BIND], VK_LBUTTON, VK_RBUTTON };
				break;
			default:
				vIgnore = { Vars::Menu::MenuPrimaryKey[DEFAULT_BIND], Vars::Menu::MenuSecondaryKey[DEFAULT_BIND] };
			}
			FKeybind(iOverride != -1 ? pVar->m_vTitle[iOverride] : pVar->m_vTitle.front(), pVar->Map[DEFAULT_BIND], !(i % 2) ? FButtonEnum::Left : FButtonEnum::Right | FButtonEnum::SameLine, vIgnore);
			break;
		}
		}

		if (iOverride != -2)
			i += i > 1 ? 1 : 2;
	}
	if (uLastSection)
		EndSection();
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

	if (m_bIsOpen ? false : !Vars::Menu::BindWindow.Value || I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible() && !I::EngineClient->IsPlayingDemo())
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

				if (tBind.m_iVisibility == BindVisibilityEnum::Always || tBind.m_iVisibility == BindVisibilityEnum::WhileActive && tBind.m_bActive || m_bIsOpen)
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
	if (Begin("Binds", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing))
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
				PushTransparent(!F::Binds.WillBeEnabled(iBind), true);

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
				SetCursorPos({ flWidth - H::Draw.Scale(26), H::Draw.Scale(iListStart - 2 + 18 * i) });
				bool bDelete = IconButton(ICON_MD_DELETE, H::Draw.Scale(18));

				SetCursorPos({ flWidth - H::Draw.Scale(51), H::Draw.Scale(iListStart - 2 + 18 * i) });
				if (IconButton(!tBind.m_bNot ? ICON_MD_CODE : ICON_MD_CODE_OFF, H::Draw.Scale(18)))
					tBind.m_bNot = !tBind.m_bNot;

				PushTransparent(Transparent || tBind.m_iVisibility == BindVisibilityEnum::Hidden, true);
				SetCursorPos({ flWidth - H::Draw.Scale(76), H::Draw.Scale(iListStart - 2 + 18 * i) });
				if (IconButton(tBind.m_iVisibility == BindVisibilityEnum::Always ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF, H::Draw.Scale(18)))
					tBind.m_iVisibility = (tBind.m_iVisibility + 1) % 3;
				PopTransparent(1, 1);

				SetCursorPos({ flWidth - H::Draw.Scale(101), H::Draw.Scale(iListStart - 2 + 18 * i) });
				if (IconButton(tBind.m_bEnabled ? ICON_MD_TOGGLE_ON : ICON_MD_TOGGLE_OFF, H::Draw.Scale(18)))
					tBind.m_bEnabled = !tBind.m_bEnabled;

				PopTransparent(1, 1);

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
					if (FButton("Yes", FButtonEnum::Left))
					{
						F::Binds.RemoveBind(iBind);
						CloseCurrentPopup();
					}
					if (FButton("No", FButtonEnum::Right | FButtonEnum::SameLine))
						CloseCurrentPopup();

					EndPopup();
				}

				PopStyleVar();
				PopFont();
			}

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

	if (!F::Configs.m_bConfigLoaded || !(ImGui::GetIO().DisplaySize.x > 160.f && ImGui::GetIO().DisplaySize.y > 28.f))
		return;

	m_bInKeybind = m_bWindowHovered = false;
	if (m_bIsOpen)
	{
		for (int iKey = 0; iKey < 256; iKey++)
			U::KeyHandler.StoreKey(iKey);
	}
	else
	{
		U::KeyHandler.StoreKey(Vars::Menu::MenuPrimaryKey.Value);
		U::KeyHandler.StoreKey(Vars::Menu::MenuSecondaryKey.Value);
		U::KeyHandler.StoreKey(VK_F11);
	}
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
		m_bWindowHovered = IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

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