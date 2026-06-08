#include "Debug.h"

#ifdef DEBUG_INFO

#define PAIR(x) { x, #x }
void CDebug::Draw(CTFPlayer* pLocal)
{
#ifdef DEBUG_TEXT
	if (!Vars::Debug::Info.Value && m_vDebugText.empty())
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
		auto pCmd = !I::EngineClient->IsPlayingDemo() ? G::LastUserCmd : I::Input->GetUserCmd(I::ClientState->lastoutgoingcommand);

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
				}()
					).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Tickcount: {}, Command: {}", pCmd->tick_count, pCmd->command_number).c_str());
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
	if (!m_vDebugText.empty())
	{
		if (Vars::Debug::Info.Value)
			y += nTall;
		for (auto it = m_vDebugText.begin(); it != m_vDebugText.end();)
		{
			if (it->m_flTime && it->m_flTime < I::GlobalVars->curtime)
				it = m_vDebugText.erase(it);
			else
				++it;
		}
		for (auto& [sString, tColor, _, vPosition2D, vPosition3D] : m_vDebugText)
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
#endif
}
#undef PAIR



#ifdef DEBUG_TEXT
const std::vector<DebugText_t>& CDebug::GetText()
{
	return m_vDebugText;
}

void CDebug::ClearText(byte iType)
{
	if (iType == TextEnum::All)
		m_vDebugText.clear();
	else for (auto it = m_vDebugText.begin(); it != m_vDebugText.end();)
	{
		if ((iType == TextEnum::Manual) == (it->vPosition2D || it->vPosition3D))
			it = m_vDebugText.erase(it);
		else
			++it;
	}
}

void CDebug::PopText(byte iType)
{
	if (!m_vDebugText.empty())
	{
		if (iType == TextEnum::All)
			m_vDebugText.erase(m_vDebugText.begin());
		else for (auto it = m_vDebugText.begin(); it != m_vDebugText.end();)
		{
			if ((iType == TextEnum::Manual) == (it->vPosition2D || it->vPosition3D))
			{
				it = m_vDebugText.erase(it);
				break;
			}
		}
	}
}

void CDebug::AddText(const DebugText_t& sText)
{
	m_vDebugText.push_back(sText);
}

void CDebug::AddText(const std::string& sString, Color_t tColor)
{
	m_vDebugText.emplace_back(sString, tColor, 0.f, std::nullopt, std::nullopt);
}

void CDebug::AddText(const std::string& sString, const Vec2& vPosition, Color_t tColor)
{
	m_vDebugText.emplace_back(sString, tColor, 0.f, vPosition, std::nullopt);
}

void CDebug::AddText(const std::string& sString, const Vec3& vPosition, Color_t tColor)
{
	m_vDebugText.emplace_back(sString, tColor, 0.f, std::nullopt, vPosition);
}

void CDebug::AddText(const std::string& sString, float flTime, Color_t tColor)
{
	m_vDebugText.emplace_back(sString, tColor, flTime, std::nullopt, std::nullopt);
}

void CDebug::AddText(const std::string& sString, const Vec2& vPosition, float flTime, Color_t tColor)
{
	m_vDebugText.emplace_back(sString, tColor, flTime, vPosition, std::nullopt);
}

void CDebug::AddText(const std::string& sString, const Vec3& vPosition, float flTime, Color_t tColor)
{
	m_vDebugText.emplace_back(sString, tColor, flTime, std::nullopt, vPosition);
}
#endif

#endif