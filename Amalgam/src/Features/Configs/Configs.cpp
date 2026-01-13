#include "Configs.h"

#include "../Binds/Binds.h"
#include "../Visuals/Groups/Groups.h"
#include "../Visuals/Materials/Materials.h"

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, bool v)
{
	t.put(s, v);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, byte v)
{
	t.put(s, v);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, int v)
{
	t.put(s, v);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, float v)
{
	t.put(s, v);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const std::string& v)
{
	t.put(s, v);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const IntRange_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "Min", v.Min);
	SaveJson(tChild, "Max", v.Max);

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const FloatRange_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "Min", v.Min);
	SaveJson(tChild, "Max", v.Max);

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const std::vector<std::pair<std::string, Color_t>>& v)
{
	boost::property_tree::ptree tChild;
	for (auto& [m, c] : v)
	{
		boost::property_tree::ptree tLayer;
		SaveJson(tLayer, "Material", m);
		SaveJson(tLayer, "Color", c);

		tChild.push_back({ "", tLayer });
	}

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const Color_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "r", v.r);
	SaveJson(tChild, "g", v.g);
	SaveJson(tChild, "b", v.b);
	SaveJson(tChild, "a", v.a);

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const Gradient_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "StartColor", v.StartColor);
	SaveJson(tChild, "EndColor", v.EndColor);

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const DragBox_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "x", v.x);
	SaveJson(tChild, "y", v.y);

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const WindowBox_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "x", v.x);
	SaveJson(tChild, "y", v.y);
	SaveJson(tChild, "w", v.w);
	SaveJson(tChild, "h", v.h);

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const Chams_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "Visible", v.Visible);
	SaveJson(tChild, "Occluded", v.Occluded);

	t.put_child(s, tChild);
}

void CConfigs::SaveJson(boost::property_tree::ptree& t, const std::string& s, const Glow_t& v)
{
	boost::property_tree::ptree tChild;
	SaveJson(tChild, "Stencil", v.Stencil);
	SaveJson(tChild, "Blur", v.Blur);

	t.put_child(s, tChild);
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, bool& v)
{
	if (auto o = t.get_optional<bool>(s))
		v = *o;
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, byte& v)
{
	if (auto o = t.get_optional<byte>(s))
		v = *o;
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, int& v)
{
	if (auto o = t.get_optional<int>(s))
		v = *o;
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, float& v)
{
	if (auto o = t.get_optional<float>(s))
		v = *o;
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, std::string& v)
{
	if (auto o = t.get_optional<std::string>(s))
		v = *o;
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, IntRange_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "Min", v.Min);
		LoadJson(*tChild, "Max", v.Max);
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, FloatRange_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "Min", v.Min);
		LoadJson(*tChild, "Max", v.Max);
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, std::vector<std::pair<std::string, Color_t>>& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		v.clear();

		for (auto& [_, tLayer] : *tChild)
		{
			auto o = tLayer.get_optional<std::string>("Material");
			if (!o)
				continue;

			std::string& m = *o;
			Color_t c; LoadJson(tLayer, "Color", c);

			bool bFound = false; // ensure no duplicates are assigned
			for (auto& [sMat, _] : v)
			{
				if (FNV1A::Hash32(sMat.c_str()) == FNV1A::Hash32(m.c_str()))
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
				v.emplace_back(m, c);
		}

		// remove invalid materials
		for (auto it = v.begin(); it != v.end();)
		{
			auto uHash = FNV1A::Hash32(it->first.c_str());
			if (uHash == FNV1A::Hash32Const("None")
				|| uHash != FNV1A::Hash32Const("Original") && !F::Materials.m_mMaterials.contains(uHash))
				it = v.erase(it);
			else
				++it;
		}
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, Color_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "r", v.r);
		LoadJson(*tChild, "g", v.g);
		LoadJson(*tChild, "b", v.b);
		LoadJson(*tChild, "a", v.a);
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, Gradient_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "StartColor", v.StartColor);
		LoadJson(*tChild, "EndColor", v.EndColor);
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, DragBox_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "x", v.x);
		LoadJson(*tChild, "y", v.y);
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, WindowBox_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "x", v.x);
		LoadJson(*tChild, "y", v.y);
		LoadJson(*tChild, "w", v.w);
		LoadJson(*tChild, "h", v.h);
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, Chams_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "Visible", v.Visible);
		LoadJson(*tChild, "Occluded", v.Occluded);
	}
}

void CConfigs::LoadJson(const boost::property_tree::ptree& t, const std::string& s, Glow_t& v)
{
	if (auto tChild = t.get_child_optional(s))
	{
		LoadJson(*tChild, "Stencil", v.Stencil);
		LoadJson(*tChild, "Blur", v.Blur);
	}
}



CConfigs::CConfigs()
{
	m_sConfigPath = std::filesystem::current_path().string() + "\\Amalgam\\";
	m_sVisualsPath = m_sConfigPath + "Visuals\\";
	m_sCorePath = m_sConfigPath + "Core\\";
	m_sMaterialsPath = m_sConfigPath + "Materials\\";

	if (!std::filesystem::exists(m_sConfigPath))
		std::filesystem::create_directory(m_sConfigPath);

	if (!std::filesystem::exists(m_sVisualsPath))
		std::filesystem::create_directory(m_sVisualsPath);

	if (!std::filesystem::exists(m_sCorePath))
		std::filesystem::create_directory(m_sCorePath);

	if (!std::filesystem::exists(m_sMaterialsPath))
		std::filesystem::create_directory(m_sMaterialsPath);
}

#define IsType(t) pBase->m_iType == typeid(t).hash_code()

template <class T>
static inline void SaveMain(BaseVar*& pBase, boost::property_tree::ptree& tTree)
{
	auto pVar = pBase->As<T>();

	boost::property_tree::ptree tMap;
	for (auto& [iBind, tValue] : pVar->Map)
		F::Configs.SaveJson(tMap, std::to_string(iBind), tValue);
	tTree.put_child(pVar->m_sName, tMap);
}
#define Save(t, j) if (IsType(t)) SaveMain<t>(pBase, j);

template <class T>
static inline void LoadMain(BaseVar*& pBase, boost::property_tree::ptree& tTree)
{
	auto pVar = pBase->As<T>();

	pVar->Map = { { DEFAULT_BIND, pVar->Default } };
	if (auto tMap = tTree.get_child_optional(pVar->m_sName))
	{
		for (auto& [sKey, _] : *tMap)
		{
			int iBind = std::stoi(sKey);
			if (iBind == DEFAULT_BIND || F::Binds.m_vBinds.size() > iBind && !(pVar->m_iFlags & NOBIND))
			{
				F::Configs.LoadJson(*tMap, sKey, pVar->Map[iBind]);
				if (iBind != DEFAULT_BIND)
					std::next(F::Binds.m_vBinds.begin(), iBind)->m_vVars.push_back(pVar);
			}
		}
	}
	else if (!(pVar->m_iFlags & NOSAVE))
		SDK::Output("Amalgam", std::format("{} not found", pVar->m_sName).c_str(), ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
}
#define Load(t, j) if (IsType(t)) LoadMain<t>(pBase, j);

bool CConfigs::SaveConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		boost::property_tree::ptree tWrite;

		{
			boost::property_tree::ptree tSub;
			for (int iID = 0; iID < F::Binds.m_vBinds.size(); iID++)
			{
				auto& tBind = F::Binds.m_vBinds[iID];

				boost::property_tree::ptree tChild;
				SaveJson(tChild, "Name", tBind.m_sName);
				SaveJson(tChild, "Type", tBind.m_iType);
				SaveJson(tChild, "Info", tBind.m_iInfo);
				SaveJson(tChild, "Key", tBind.m_iKey);
				SaveJson(tChild, "Enabled", tBind.m_bEnabled);
				SaveJson(tChild, "Visibility", tBind.m_iVisibility);
				SaveJson(tChild, "Not", tBind.m_bNot);
				SaveJson(tChild, "Active", tBind.m_bActive);
				SaveJson(tChild, "Parent", tBind.m_iParent);

				tSub.put_child(std::to_string(iID), tChild);
			}
			tWrite.put_child("Binds", tSub);
		}

		{
			boost::property_tree::ptree tSub;
			bool bNoSave = GetAsyncKeyState(VK_SHIFT) & 0x8000;
			for (auto& pBase : G::Vars)
			{
				if (!bNoSave && pBase->m_iFlags & NOSAVE)
					continue;

				Save(bool, tSub)
				else Save(int, tSub)
				else Save(float, tSub)
				else Save(IntRange_t, tSub)
				else Save(FloatRange_t, tSub)
				else Save(std::string, tSub)
				else Save(VA_LIST(std::vector<std::pair<std::string, Color_t>>), tSub)
				else Save(Color_t, tSub)
				else Save(Gradient_t, tSub)
				else Save(DragBox_t, tSub)
				else Save(WindowBox_t, tSub)
			}
			tWrite.put_child("Vars", tSub);
		}

		{
			boost::property_tree::ptree tSub;
			for (int iID = 0; iID < F::Groups.m_vGroups.size(); iID++)
			{
				auto& tGroup = F::Groups.m_vGroups[iID];

				boost::property_tree::ptree tChild;
				SaveJson(tChild, "Name", tGroup.m_sName);
				SaveJson(tChild, "Color", tGroup.m_tColor);
				SaveJson(tChild, "TagsOverrideColor", tGroup.m_bTagsOverrideColor);
				SaveJson(tChild, "Targets", tGroup.m_iTargets);
				SaveJson(tChild, "Conditions", tGroup.m_iConditions);
				SaveJson(tChild, "Players", tGroup.m_iPlayers);
				SaveJson(tChild, "Buildings", tGroup.m_iBuildings);
				SaveJson(tChild, "Projectiles", tGroup.m_iProjectiles);
				SaveJson(tChild, "ESP", tGroup.m_iESP);
				SaveJson(tChild, "Chams", tGroup.m_tChams);
				SaveJson(tChild, "Glow", tGroup.m_tGlow);
				SaveJson(tChild, "OffscreenArrows", tGroup.m_bOffscreenArrows);
				SaveJson(tChild, "OffscreenArrowsOffset", tGroup.m_iOffscreenArrowsOffset);
				SaveJson(tChild, "OffscreenArrowsMaxDistance", tGroup.m_flOffscreenArrowsMaxDistance);
				SaveJson(tChild, "PickupTimer", tGroup.m_bPickupTimer);
				SaveJson(tChild, "Backtrack", tGroup.m_iBacktrack);
				SaveJson(tChild, "BacktrackChams", tGroup.m_vBacktrackChams);
				SaveJson(tChild, "BacktrackGlow", tGroup.m_tBacktrackGlow);
				SaveJson(tChild, "Trajectory", tGroup.m_iTrajectory);
				SaveJson(tChild, "Sightlines", tGroup.m_iSightlines);

				tSub.put_child(std::to_string(iID), tChild);
				if (F::Groups.m_vGroups.size() >= sizeof(int) * 8)
					break;
			}
			tWrite.put_child("Groups", tSub);
		}

		write_json(m_sConfigPath + sConfigName + m_sConfigExtension, tWrite);

		m_sCurrentConfig = sConfigName; m_sCurrentVisuals = "";
		if (bNotify)
			SDK::Output("Amalgam", std::format("Config {} saved", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Save config failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
		return false;
	}

	return true;
}

bool CConfigs::LoadConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		if (!std::filesystem::exists(m_sConfigPath + sConfigName + m_sConfigExtension))
		{
			if (sConfigName == std::string("default"))
			{
				SaveConfig("default", false);

				H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);
			}
			return false;
		}

		boost::property_tree::ptree tRead;
		read_json(m_sConfigPath + sConfigName + m_sConfigExtension, tRead);

		F::Binds.m_vBinds.clear();
		F::Groups.m_vGroups.clear();

		if (auto tSub = tRead.get_child_optional("Binds"))
		{
			for (const auto& [_, tChild] : *tSub)
			{
				Bind_t tBind = {};
				LoadJson(tChild, "Name", tBind.m_sName);
				LoadJson(tChild, "Type", tBind.m_iType);
				LoadJson(tChild, "Info", tBind.m_iInfo);
				LoadJson(tChild, "Key", tBind.m_iKey);
				LoadJson(tChild, "Enabled", tBind.m_bEnabled);
				LoadJson(tChild, "Visibility", tBind.m_iVisibility);
				LoadJson(tChild, "Not", tBind.m_bNot);
				LoadJson(tChild, "Active", tBind.m_bActive);
				LoadJson(tChild, "Parent", tBind.m_iParent);
				if (F::Binds.m_vBinds.size() == tBind.m_iParent)
					tBind.m_iParent = DEFAULT_BIND - 1; // prevent infinite loop

				F::Binds.m_vBinds.push_back(tBind);
			}
		}
		else
			SDK::Output("Amalgam", "Config binds not found", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);

		if (auto tSub = tRead.get_child_optional("Vars");
			tSub || (tSub = tRead.get_child_optional("ConVars")))
		{
			bool bNoSave = GetAsyncKeyState(VK_SHIFT) & 0x8000;
			for (auto& pBase : G::Vars)
			{
				if (!bNoSave && pBase->m_iFlags & NOSAVE)
					continue;

				Load(bool, *tSub)
				else Load(int, *tSub)
				else Load(float, *tSub)
				else Load(IntRange_t, *tSub)
				else Load(FloatRange_t, *tSub)
				else Load(std::string, *tSub)
				else Load(VA_LIST(std::vector<std::pair<std::string, Color_t>>), *tSub)
				else Load(Color_t, *tSub)
				else Load(Gradient_t, *tSub)
				else Load(DragBox_t, *tSub)
				else Load(WindowBox_t, *tSub)
			}
		}
		else
			SDK::Output("Amalgam", "Config vars not found", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);

		if (auto tSub = tRead.get_child_optional("Groups"))
		{
			for (auto& [_, tChild] : *tSub)
			{
				Group_t tGroup = {};
				LoadJson(tChild, "Name", tGroup.m_sName);
				LoadJson(tChild, "Color", tGroup.m_tColor);
				LoadJson(tChild, "TagsOverrideColor", tGroup.m_bTagsOverrideColor);
				LoadJson(tChild, "Targets", tGroup.m_iTargets);
				LoadJson(tChild, "Conditions", tGroup.m_iConditions);
				LoadJson(tChild, "Players", tGroup.m_iPlayers);
				LoadJson(tChild, "Buildings", tGroup.m_iBuildings);
				LoadJson(tChild, "Projectiles", tGroup.m_iProjectiles);
				LoadJson(tChild, "ESP", tGroup.m_iESP);
				LoadJson(tChild, "Chams", tGroup.m_tChams);
				LoadJson(tChild, "Glow", tGroup.m_tGlow);
				LoadJson(tChild, "OffscreenArrows", tGroup.m_bOffscreenArrows);
				LoadJson(tChild, "OffscreenArrowsOffset", tGroup.m_iOffscreenArrowsOffset);
				LoadJson(tChild, "OffscreenArrowsMaxDistance", tGroup.m_flOffscreenArrowsMaxDistance);
				LoadJson(tChild, "PickupTimer", tGroup.m_bPickupTimer);
				LoadJson(tChild, "Backtrack", tGroup.m_iBacktrack);
				LoadJson(tChild, "BacktrackChams", tGroup.m_vBacktrackChams);
				LoadJson(tChild, "BacktrackGlow", tGroup.m_tBacktrackGlow);
				LoadJson(tChild, "Trajectory", tGroup.m_iTrajectory);
				LoadJson(tChild, "Sightlines", tGroup.m_iSightlines);

				F::Groups.m_vGroups.push_back(tGroup);
			}
		}
		else
			SDK::Output("Amalgam", "Config groups not found", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);

		F::Binds.SetVars(nullptr, nullptr, false);
		H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);

		m_sCurrentConfig = sConfigName; m_sCurrentVisuals = "";
		if (bNotify)
			SDK::Output("Amalgam", std::format("Config {} loaded", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Load config failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
		return false;
	}

	return true;
}

template <class T>
static inline void SaveMiscMain(BaseVar*& pBase, boost::property_tree::ptree& tTree)
{
	F::Configs.SaveJson(tTree, pBase->m_sName, pBase->As<T>()->Map[DEFAULT_BIND]);
}
#define SaveMisc(t, j) if (IsType(t)) SaveMiscMain<t>(pBase, j);

template <class T>
static inline void LoadMiscMain(BaseVar*& pBase, boost::property_tree::ptree& tTree)
{
	F::Configs.LoadJson(tTree, pBase->m_sName, pBase->As<T>()->Map[DEFAULT_BIND]);
}
#define LoadMisc(t, j) if (IsType(t)) LoadMiscMain<t>(pBase, j);

bool CConfigs::SaveVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		boost::property_tree::ptree tWrite;

		{
			boost::property_tree::ptree tSub;
			bool bNoSave = GetAsyncKeyState(VK_SHIFT) & 0x8000;
			for (auto& pBase : G::Vars)
			{
				if (!(pBase->m_iFlags & VISUAL) || !bNoSave && pBase->m_iFlags & NOSAVE)
					continue;

				SaveMisc(bool, tSub)
				else SaveMisc(int, tSub)
				else SaveMisc(float, tSub)
				else SaveMisc(IntRange_t, tSub)
				else SaveMisc(FloatRange_t, tSub)
				else SaveMisc(std::string, tSub)
				else SaveMisc(VA_LIST(std::vector<std::pair<std::string, Color_t>>), tSub)
				else SaveMisc(Color_t, tSub)
				else SaveMisc(Gradient_t, tSub)
				else SaveMisc(DragBox_t, tSub)
				else SaveMisc(WindowBox_t, tSub)
			}
			tWrite.put_child("Vars", tSub);
		}

		{
			boost::property_tree::ptree tSub;
			for (int iID = 0; iID < F::Groups.m_vGroups.size(); iID++)
			{
				auto& tGroup = F::Groups.m_vGroups[iID];

				boost::property_tree::ptree tChild;
				SaveJson(tChild, "Name", tGroup.m_sName);
				SaveJson(tChild, "Color", tGroup.m_tColor);
				SaveJson(tChild, "TagsOverrideColor", tGroup.m_bTagsOverrideColor);
				SaveJson(tChild, "Targets", tGroup.m_iTargets);
				SaveJson(tChild, "Conditions", tGroup.m_iConditions);
				SaveJson(tChild, "Players", tGroup.m_iPlayers);
				SaveJson(tChild, "Buildings", tGroup.m_iBuildings);
				SaveJson(tChild, "Projectiles", tGroup.m_iProjectiles);
				SaveJson(tChild, "ESP", tGroup.m_iESP);
				SaveJson(tChild, "Chams", tGroup.m_tChams);
				SaveJson(tChild, "Glow", tGroup.m_tGlow);
				SaveJson(tChild, "OffscreenArrows", tGroup.m_bOffscreenArrows);
				SaveJson(tChild, "OffscreenArrowsOffset", tGroup.m_iOffscreenArrowsOffset);
				SaveJson(tChild, "OffscreenArrowsMaxDistance", tGroup.m_flOffscreenArrowsMaxDistance);
				SaveJson(tChild, "PickupTimer", tGroup.m_bPickupTimer);
				SaveJson(tChild, "Backtrack", tGroup.m_iBacktrack);
				SaveJson(tChild, "BacktrackChams", tGroup.m_vBacktrackChams);
				SaveJson(tChild, "BacktrackGlow", tGroup.m_tBacktrackGlow);
				SaveJson(tChild, "Trajectory", tGroup.m_iTrajectory);
				SaveJson(tChild, "Sightlines", tGroup.m_iSightlines);

				tSub.put_child(std::to_string(iID), tChild);
				if (F::Groups.m_vGroups.size() >= sizeof(int) * 8)
					break;
			}
			tWrite.put_child("Groups", tSub);
		}

		write_json(m_sVisualsPath + sConfigName + m_sConfigExtension, tWrite);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} saved", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Save visuals failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
		return false;
	}
	return true;
}

bool CConfigs::LoadVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		if (!std::filesystem::exists(m_sVisualsPath + sConfigName + m_sConfigExtension))
			return false;

		boost::property_tree::ptree tRead;
		read_json(m_sVisualsPath + sConfigName + m_sConfigExtension, tRead);

		F::Groups.m_vGroups.clear();

		if (auto tSub = tRead.get_child_optional("Vars");
			tSub || (tSub = tRead))
		{
			bool bNoSave = GetAsyncKeyState(VK_SHIFT) & 0x8000;
			for (auto& pBase : G::Vars)
			{
				if (!(pBase->m_iFlags & VISUAL) || !bNoSave && pBase->m_iFlags & NOSAVE)
					continue;

				LoadMisc(bool, *tSub)
				else LoadMisc(int, *tSub)
				else LoadMisc(float, *tSub)
				else LoadMisc(IntRange_t, *tSub)
				else LoadMisc(FloatRange_t, *tSub)
				else LoadMisc(std::string, *tSub)
				else LoadMisc(VA_LIST(std::vector<std::pair<std::string, Color_t>>), *tSub)
				else LoadMisc(Color_t, *tSub)
				else LoadMisc(Gradient_t, *tSub)
				else LoadMisc(DragBox_t, *tSub)
				else LoadMisc(WindowBox_t, *tSub)
			}
		}
		else
			SDK::Output("Amalgam", "Config vars not found", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);

		if (auto tSub = tRead.get_child_optional("Groups"))
		{
			for (auto& [_, tChild] : *tSub)
			{
				Group_t tGroup = {};
				LoadJson(tChild, "Name", tGroup.m_sName);
				LoadJson(tChild, "Color", tGroup.m_tColor);
				LoadJson(tChild, "TagsOverrideColor", tGroup.m_bTagsOverrideColor);
				LoadJson(tChild, "Targets", tGroup.m_iTargets);
				LoadJson(tChild, "Conditions", tGroup.m_iConditions);
				LoadJson(tChild, "Players", tGroup.m_iPlayers);
				LoadJson(tChild, "Buildings", tGroup.m_iBuildings);
				LoadJson(tChild, "Projectiles", tGroup.m_iProjectiles);
				LoadJson(tChild, "ESP", tGroup.m_iESP);
				LoadJson(tChild, "Chams", tGroup.m_tChams);
				LoadJson(tChild, "Glow", tGroup.m_tGlow);
				LoadJson(tChild, "OffscreenArrows", tGroup.m_bOffscreenArrows);
				LoadJson(tChild, "OffscreenArrowsOffset", tGroup.m_iOffscreenArrowsOffset);
				LoadJson(tChild, "OffscreenArrowsMaxDistance", tGroup.m_flOffscreenArrowsMaxDistance);
				LoadJson(tChild, "PickupTimer", tGroup.m_bPickupTimer);
				LoadJson(tChild, "Backtrack", tGroup.m_iBacktrack);
				LoadJson(tChild, "BacktrackChams", tGroup.m_vBacktrackChams);
				LoadJson(tChild, "BacktrackGlow", tGroup.m_tBacktrackGlow);
				LoadJson(tChild, "Trajectory", tGroup.m_iTrajectory);
				LoadJson(tChild, "Sightlines", tGroup.m_iSightlines);

				F::Groups.m_vGroups.push_back(tGroup);
			}
		}
		else
			SDK::Output("Amalgam", "Config groups not found", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);

		F::Binds.SetVars(nullptr, nullptr, false);

		m_sCurrentVisuals = sConfigName;
		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} loaded", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Load visuals failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
		return false;
	}
	return true;
}

template <class T>
static inline void ResetMain(BaseVar*& pBase)
{
	auto pVar = pBase->As<T>();

	pVar->Map = { { DEFAULT_BIND, pVar->Default } };
}
#define Reset(t) if (IsType(t)) ResetMain<t>(pBase);

void CConfigs::DeleteConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		if (FNV1A::Hash32(sConfigName.c_str()) == FNV1A::Hash32Const("default"))
		{
			ResetConfig(sConfigName);
			return;
		}

		std::filesystem::remove(m_sConfigPath + sConfigName + m_sConfigExtension);
		if (FNV1A::Hash32(m_sCurrentConfig.c_str()) == FNV1A::Hash32(sConfigName.c_str()))
			LoadConfig("default", false);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Config {} deleted", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Remove config failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
	}
}

void CConfigs::ResetConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		F::Binds.m_vBinds.clear();
		F::Groups.m_vGroups.clear();

		bool bNoSave = GetAsyncKeyState(VK_SHIFT) & 0x8000;
		for (auto& pBase : G::Vars)
		{
			if (!bNoSave && pBase->m_iFlags & NOSAVE)
				continue;

			Reset(bool)
			else Reset(int)
			else Reset(float)
			else Reset(IntRange_t)
			else Reset(FloatRange_t)
			else Reset(std::string)
			else Reset(std::vector<std::string>)
			else Reset(Color_t)
			else Reset(Gradient_t)
			else Reset(DragBox_t)
			else Reset(WindowBox_t)
		}

		SaveConfig(sConfigName, false);
		F::Binds.SetVars(nullptr, nullptr, false);
		H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Config {} reset", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Reset config failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
	}
}

void CConfigs::DeleteVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		std::filesystem::remove(m_sVisualsPath + sConfigName + m_sConfigExtension);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} deleted", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Remove visuals failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
	}
}

void CConfigs::ResetVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		F::Groups.m_vGroups.clear();

		bool bNoSave = GetAsyncKeyState(VK_SHIFT) & 0x8000;
		for (auto& pBase : G::Vars)
		{
			if (!(pBase->m_iFlags & VISUAL) || !bNoSave && pBase->m_iFlags & NOSAVE)
				continue;

			Reset(bool)
			else Reset(int)
			else Reset(float)
			else Reset(IntRange_t)
			else Reset(FloatRange_t)
			else Reset(std::string)
			else Reset(std::vector<std::string>)
			else Reset(Color_t)
			else Reset(Gradient_t)
			else Reset(DragBox_t)
			else Reset(WindowBox_t)
		}

		SaveVisual(sConfigName, false);
		F::Binds.SetVars(nullptr, nullptr, false);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} reset", sConfigName).c_str(), DEFAULT_COLOR, OUTPUT_CONSOLE | OUTPUT_TOAST | OUTPUT_MENU | OUTPUT_DEBUG);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Reset visuals failed", ALTERNATE_COLOR, OUTPUT_CONSOLE | OUTPUT_MENU | OUTPUT_DEBUG);
	}
}