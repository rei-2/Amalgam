#include "Debug.h"

#ifdef DEBUG_INFO

#define PAIR(x) { x, #x }
void CDebug::Draw(CTFPlayer* pLocal)
{
#ifdef DEBUG_TEXT
	if (!Vars::Debug::Info.Value && m_mDebugText.empty())
		return;
#else
	if (!Vars::Debug::Info.Value)
		return;
#endif

	int x = 10, y = 10;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(1);
	y -= nTall;

	if (Vars::Debug::Info.Value)
	{
		auto pWeapon = H::Entities.GetWeapon();
		auto pCmd = !I::EngineClient->IsPlayingDemo() ? G::CurrentUserCmd : I::Input->GetUserCmd(I::ClientState->lastoutgoingcommand);

		if (pCmd)
		{
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("View: ({:.3f}, {:.3f}, {:.3f})", pCmd->viewangles.x, pCmd->viewangles.y, pCmd->viewangles.z).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Move: ({}, {}, {})", pCmd->forwardmove, pCmd->sidemove, pCmd->upmove).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Buttons: {:#034b} ({})", pCmd->buttons,
				[&]()
				{
					std::string sReturn = "";
					if (pCmd->buttons)
					{
						static std::vector<std::pair<int, const char*>> vFlags = {
							PAIR(IN_ATTACK),
							PAIR(IN_ATTACK2),
							PAIR(IN_ATTACK3),
							PAIR(IN_FORWARD),
							PAIR(IN_BACK),
							PAIR(IN_MOVELEFT),
							PAIR(IN_MOVERIGHT),
							PAIR(IN_JUMP),
							PAIR(IN_DUCK),
							PAIR(IN_RELOAD),
							PAIR(IN_LEFT),
							PAIR(IN_RIGHT),
							PAIR(IN_SCORE),
							/*
							PAIR(IN_USE),
							PAIR(IN_CANCEL),
							PAIR(IN_RUN),
							PAIR(IN_ALT1),
							PAIR(IN_ALT2),
							PAIR(IN_SPEED),
							PAIR(IN_WALK),
							PAIR(IN_ZOOM),
							PAIR(IN_WEAPON1),
							PAIR(IN_WEAPON2),
							PAIR(IN_BULLRUSH),
							PAIR(IN_GRENADE1),
							PAIR(IN_GRENADE2),
							*/
						};

						for (int i = 0; i < vFlags.size(); i++)
						{
							auto& paFlag = vFlags[i];
							if (pCmd->buttons & paFlag.first)
							{
								if (!sReturn.empty())
									sReturn += " | ";
								sReturn += paFlag.second;
							}
						}
					}
					return sReturn.empty() ? "NONE" : sReturn;
				}()).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("TickCount: {}, Command: {}", pCmd->tick_count, pCmd->command_number).c_str());
			//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("WeaponSelect: {}, WeaponSubtype: {}", pCmd->weaponselect, pCmd->weaponsubtype, pCmd->impulse).c_str());
			//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("RandomSeed: {}, Impulse: {}", pCmd->random_seed, pCmd->impulse).c_str());
			//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("MouseDx: {}, MouseDy: {}", pCmd->random_seed, pCmd->impulse).c_str());
			//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("HasBeenPredicted: {}", pCmd->hasbeenpredicted).c_str());
		}
		Vec3 vOrigin = pLocal->m_vecOrigin();
		H::Draw.StringOutlined(fFont, x, y += nTall * 2, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Origin: ({:.3f}, {:.3f}, {:.3f})", vOrigin.x, vOrigin.y, vOrigin.z).c_str());
		Vec3 vVelocity = pLocal->m_vecVelocity();
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Velocity: {:.3f}, {:.3f} ({:.3f}, {:.3f}, {:.3f})", vVelocity.Length(), vVelocity.Length2D(), vVelocity.x, vVelocity.y, vVelocity.z).c_str());
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Tickbase: {}", pLocal->m_nTickBase()).c_str());
		//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Choke: {}, {}", G::Choking, I::ClientState->chokedcommands).c_str());
		//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Ticks: {}, {}", F::Ticks.m_iShiftedTicks, F::Ticks.m_iShiftedGoal).c_str());
		//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Round state: {}, {}, {}", SDK::GetRoundState(), SDK::GetWinningTeam(), I::EngineClient->IsPlayingDemo()).c_str());
		//H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Entities: {} ({}, {})", I::ClientEntityList->GetMaxEntities(), I::ClientEntityList->GetHighestEntityIndex(), I::ClientEntityList->NumberOfEntities(false)).c_str());

		/*
		if (pWeapon)
		{
			float flTime = TICKS_TO_TIME(pLocal->m_nTickBase());
			float flPrimaryAttack = pWeapon->m_flNextPrimaryAttack();
			float flSecondaryAttack = pWeapon->m_flNextSecondaryAttack();
			float flAttack = pLocal->m_flNextAttack();

			y += nTall;
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Weapon: {}, {}", pWeapon->GetSlot(), pWeapon->GetWeaponID()).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Attacking: {}", G::Attacking).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("CanPrimaryAttack: {} ([{:.3f} | {:.3f}] <= {:.3f})", G::CanPrimaryAttack, flPrimaryAttack, flAttack, flTime).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("CanSecondaryAttack: {} ([{:.3f} | {:.3f}] <= {:.3f})", G::CanSecondaryAttack, flSecondaryAttack, flAttack, flTime).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Attack: {:.3f}, {:.3f}; {:.3f}", flTime - flPrimaryAttack, flTime - flSecondaryAttack, flTime - flAttack).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Reload: {} ({} || {} != 0)", G::Reloading, pWeapon->m_bInReload(), pWeapon->m_iReloadMode()).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Throw: {}, Smack: {}", G::Throwing, pWeapon->m_flSmackTime()).c_str());
		}
		*/
	}

#ifdef DEBUG_TEXT
	if (!m_mDebugText.empty())
	{
		if (Vars::Debug::Info.Value)
			y += nTall;
		for (auto& [uType, vDebugText] : m_mDebugText)
		{
			for (auto it = vDebugText.begin(); it != vDebugText.end();)
			{
				if (it->m_flTime && it->m_flTime < I::GlobalVars->curtime)
					it = vDebugText.erase(it);
				else
					++it;
			}
			if (vDebugText.empty())
			{
				m_mDebugText.erase(uType);
				continue;
			}
			for (auto& [sString, flTime, vPosition2D, vPosition3D, tColor] : vDebugText)
			{
				if (vPosition3D)
				{
					if (Vec3 vScreen; SDK::W2S(*vPosition3D, vScreen))
						H::Draw.StringOutlined(fFont, vScreen.x, vScreen.y, tColor, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, sString.c_str());
				}
				else if (vPosition2D)
					H::Draw.StringOutlined(fFont, vPosition2D->x, vPosition2D->y, tColor, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, sString.c_str());
				else
					H::Draw.StringOutlined(fFont, x, y += nTall, tColor, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, sString.c_str());
			}
		}
	}
#endif
}
#undef PAIR



#ifdef DEBUG_TEXT
const std::map<int, std::vector<DebugText_t>>& CDebug::GetText()
{
	return m_mDebugText;
}
const std::vector<DebugText_t>& CDebug::GetText(uint32_t uType)
{
	return m_mDebugText[uType];
}

void CDebug::ClearText(uint32_t uType)
{
	if (!uType)
		m_mDebugText.clear();
	else if (m_mDebugText.contains(uType))
		m_mDebugText.erase(uType);
}

void CDebug::PopText(uint32_t uType)
{
	if (m_mDebugText.contains(uType))
		return;

	auto& vDebugText = m_mDebugText[uType];
	if (!vDebugText.empty())
		return;

	vDebugText.erase(vDebugText.begin());
}

void CDebug::AddText(const DebugText_t& sText)
{
	m_mDebugText[0].push_back(sText);
}

void CDebug::AddText(const DebugText_t& sText, uint32_t uType)
{
	m_mDebugText[uType].push_back(sText);
}

void CDebug::AddText(const std::string& sString, Color_t tColor)
{
	m_mDebugText[0].emplace_back(sString, 0.f, std::nullopt, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, const Vec2& vPosition, Color_t tColor)
{
	m_mDebugText[0].emplace_back(sString, 0.f, vPosition, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, const Vec3& vPosition, Color_t tColor)
{
	m_mDebugText[0].emplace_back(sString, 0.f, std::nullopt, vPosition, tColor);
}

void CDebug::AddText(const std::string& sString, uint32_t uType, Color_t tColor)
{
	m_mDebugText[uType].emplace_back(sString, 0.f, std::nullopt, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, uint32_t uType, const Vec2& vPosition, Color_t tColor)
{
	m_mDebugText[uType].emplace_back(sString, 0.f, vPosition, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, uint32_t uType, const Vec3& vPosition, Color_t tColor)
{
	m_mDebugText[uType].emplace_back(sString, 0.f, std::nullopt, vPosition, tColor);
}

void CDebug::AddText(const std::string& sString, float flTime, Color_t tColor)
{
	m_mDebugText[0].emplace_back(sString, flTime, std::nullopt, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, float flTime, const Vec2& vPosition, Color_t tColor)
{
	m_mDebugText[0].emplace_back(sString, flTime, vPosition, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, float flTime, const Vec3& vPosition, Color_t tColor)
{
	m_mDebugText[0].emplace_back(sString, flTime, std::nullopt, vPosition, tColor);
}

void CDebug::AddText(const std::string& sString, float flTime, uint32_t uType, Color_t tColor)
{
	m_mDebugText[uType].emplace_back(sString, flTime, std::nullopt, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, float flTime, uint32_t uType, const Vec2& vPosition, Color_t tColor)
{
	m_mDebugText[uType].emplace_back(sString, flTime, vPosition, std::nullopt, tColor);
}

void CDebug::AddText(const std::string& sString, float flTime, uint32_t uType, const Vec3& vPosition, Color_t tColor)
{
	m_mDebugText[uType].emplace_back(sString, flTime, std::nullopt, vPosition, tColor);
}
#endif

#endif