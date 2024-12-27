#include "Configs.h"

#include "../Binds/Binds.h"
#include "../Visuals/Notifications/Notifications.h"
#include "../Visuals/Materials/Materials.h"

boost::property_tree::ptree CConfigs::ColorToTree(const Color_t& color)
{
	boost::property_tree::ptree colorTree;
	colorTree.put("r", color.r);
	colorTree.put("g", color.g);
	colorTree.put("b", color.b);
	colorTree.put("a", color.a);

	return colorTree;
}

void CConfigs::TreeToColor(const boost::property_tree::ptree& tree, Color_t& out)
{
	if (auto v = tree.get_optional<byte>("r")) { out.r = *v; }
	if (auto v = tree.get_optional<byte>("g")) { out.g = *v; }
	if (auto v = tree.get_optional<byte>("b")) { out.b = *v; }
	if (auto v = tree.get_optional<byte>("a")) { out.a = *v; }
}



void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, bool bVal)
{
	mapTree.put(sName, bVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, int iVal)
{
	mapTree.put(sName, iVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, float flVal)
{
	mapTree.put(sName, flVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const IntRange_t& irVal)
{
	boost::property_tree::ptree rangeTree;
	rangeTree.put("Min", irVal.Min);
	rangeTree.put("Max", irVal.Max);

	mapTree.put_child(sName, rangeTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const FloatRange_t& frVal)
{
	boost::property_tree::ptree rangeTree;
	rangeTree.put("Min", frVal.Min);
	rangeTree.put("Max", frVal.Max);

	mapTree.put_child(sName, rangeTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const std::string& sVal)
{
	mapTree.put(sName, sVal);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const std::vector<std::pair<std::string, Color_t>>& vVal)
{
	boost::property_tree::ptree vectorTree;
	for (auto& pair : vVal)
	{
		boost::property_tree::ptree materialTree;
		materialTree.put("Material", pair.first);
		materialTree.put_child("Color", ColorToTree(pair.second));
		vectorTree.push_back(std::make_pair("", materialTree));
	}
	mapTree.put_child(sName, vectorTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const Color_t& tVal)
{
	mapTree.put_child(sName, ColorToTree(tVal));
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const Gradient_t& tVal)
{
	boost::property_tree::ptree gradientTree;
	gradientTree.put_child("StartColor", ColorToTree(tVal.StartColor));
	gradientTree.put_child("EndColor", ColorToTree(tVal.EndColor));

	mapTree.put_child(sName, gradientTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const DragBox_t& tVal)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", tVal.x);
	dragBoxTree.put("y", tVal.y);

	mapTree.put_child(sName, dragBoxTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, std::string sName, const WindowBox_t& tVal)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", tVal.x);
	dragBoxTree.put("y", tVal.y);
	dragBoxTree.put("w", tVal.w);
	dragBoxTree.put("h", tVal.h);

	mapTree.put_child(sName, dragBoxTree);
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, bool& bVal)
{
	if (auto getValue = mapTree.get_optional<bool>(sName))
		bVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, int& iVal)
{
	if (auto getValue = mapTree.get_optional<int>(sName))
		iVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, float& flVal)
{
	if (auto getValue = mapTree.get_optional<float>(sName))
		flVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, IntRange_t& irVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("Min")) { irVal.Min = *getValue; }
		if (auto getValue = getChild->get_optional<int>("Max")) { irVal.Max = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, FloatRange_t& frVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("Min")) { frVal.Min = *getValue; }
		if (auto getValue = getChild->get_optional<int>("Max")) { frVal.Max = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, std::string& sVal)
{
	if (auto getValue = mapTree.get_optional<std::string>(sName))
		sVal = *getValue;
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, std::vector<std::pair<std::string, Color_t>>& vVal)
{
	auto getMaterials = [&](std::vector<std::pair<std::string, Color_t>>& val, const boost::optional<boost::property_tree::ptree&> getVector)
		{
			if (!getVector)
				return;

			val.clear();
			for (auto& [_, tree] : *getVector)
			{
				auto getValue = tree.get_optional<std::string>("Material");
				if (!getValue)
					continue;

				std::string sMat = *getValue;
				Color_t tColor;

				if (const auto getChild = tree.get_child_optional("Color"))
					TreeToColor(*getChild, tColor);

				bool bFound = false; // ensure no duplicates are assigned
				for (auto& pair : val)
				{
					if (pair.first == sMat)
					{
						bFound = true;
						break;
					}
				}
				if (bFound)
					continue;

				val.push_back({ sMat, tColor });
			}

			// remove invalid materials
			for (auto it = val.begin(); it != val.end();)
			{
				if (FNV1A::Hash32(it->first.c_str()) == FNV1A::Hash32Const("None") || FNV1A::Hash32(it->first.c_str()) != FNV1A::Hash32Const("Original") && !F::Materials.m_mMaterials.contains(FNV1A::Hash32(it->first.c_str())))
					it = val.erase(it);
				else
					++it;
			}
		};

	getMaterials(vVal, mapTree.get_child_optional(sName));
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, Color_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
		TreeToColor(*getChild, tVal);
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, Gradient_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (const auto getStartColor = getChild->get_child_optional("StartColor"))
			TreeToColor(*getStartColor, tVal.StartColor);
		if (const auto endColor = getChild->get_child_optional("EndColor"))
			TreeToColor(*endColor, tVal.EndColor);
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, DragBox_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { tVal.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { tVal.y = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, std::string sName, WindowBox_t& tVal)
{
	if (const auto getChild = mapTree.get_child_optional(sName))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { tVal.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { tVal.y = *getValue; }
		if (auto getValue = getChild->get_optional<int>("w")) { tVal.w = *getValue; }
		if (auto getValue = getChild->get_optional<int>("h")) { tVal.h = *getValue; }
	}
}

CConfigs::CConfigs()
{
	sConfigPath = std::filesystem::current_path().string() + "\\Amalgam";
	sVisualsPath = sConfigPath + "\\Visuals";

	if (!std::filesystem::exists(sConfigPath))
		std::filesystem::create_directory(sConfigPath);

	if (!std::filesystem::exists(sVisualsPath))
		std::filesystem::create_directory(sVisualsPath);

	// Create 'Core' folder for Attribute-Changer & Playerlist
	if (!std::filesystem::exists(sConfigPath + "\\Core"))
		std::filesystem::create_directory(sConfigPath + "\\Core");

	// Create 'Materials' folder for custom materials
	if (!std::filesystem::exists(sConfigPath + "\\Materials"))
		std::filesystem::create_directory(sConfigPath + "\\Materials");
}

#define IsType(type) var->m_iType == typeid(type).hash_code()

#define SaveCond(type, tree)\
{\
	boost::property_tree::ptree mapTree;\
	for (auto& [iBind, tValue] : var->As<type>()->Map)\
		SaveJson(mapTree, std::to_string(iBind), tValue);\
	tree.put_child(var->m_sName.c_str(), mapTree);\
}
#define SaveMain(type, tree) if (IsType(type)) SaveCond(type, tree)
#define LoadCond(type, tree)\
{\
	var->As<type>()->Map = { { DEFAULT_BIND, var->As<type>()->Default } };\
	if (const auto mapTree = tree.get_child_optional(var->m_sName.c_str()))\
	{\
		for (auto& it : *mapTree)\
		{\
			if (!bLegacy)\
			{\
				int iBind = std::stoi(it.first);\
				if ((F::Binds.vBinds.size() <= iBind || var->As<type>()->m_iFlags & NOBIND) && iBind != DEFAULT_BIND)\
					continue;\
				LoadJson(*mapTree, it.first, var->As<type>()->Map[iBind]);\
			}\
			else\
			{\
				int iBind = -2; /*invalid bind*/ \
				auto uHash = FNV1A::Hash32(it.first.c_str());\
				if (uHash == FNV1A::Hash32Const("default"))\
					iBind = DEFAULT_BIND;\
				else\
				{\
					for (auto it2 = F::Binds.vBinds.begin(); it2 != F::Binds.vBinds.end(); it2++)\
					{\
						if (uHash == FNV1A::Hash32(it2->Name.c_str()))\
						{\
							iBind = std::distance(F::Binds.vBinds.begin(), it2);\
							break;\
						}\
					}\
				}\
				if (iBind == -2 || (F::Binds.vBinds.size() <= iBind || var->As<type>()->m_iFlags & NOBIND) && iBind != DEFAULT_BIND)\
					continue;\
				LoadJson(*mapTree, it.first, var->As<type>()->Map[iBind]);\
			}\
		}\
	}\
}
#define LoadMain(type, tree) if (IsType(type)) LoadCond(type, tree)

bool CConfigs::SaveConfig(const std::string& configName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree writeTree;

		boost::property_tree::ptree bindTree;
		for (auto it = F::Binds.vBinds.begin(); it != F::Binds.vBinds.end(); it++)
		{
			auto& tBind = *it;

			boost::property_tree::ptree bindTree2;
			bindTree2.put("Name", tBind.Name);
			bindTree2.put("Type", tBind.Type);
			bindTree2.put("Info", tBind.Info);
			bindTree2.put("Key", tBind.Key);
			bindTree2.put("Not", tBind.Not);
			bindTree2.put("Active", tBind.Active);
			bindTree2.put("Visible", tBind.Visible);
			bindTree2.put("Parent", tBind.Parent);

			bindTree.push_back(std::make_pair("", bindTree2));
		}
		writeTree.put_child("Binds", bindTree);

		boost::property_tree::ptree varTree;
		for (auto& var : g_Vars)
		{
			if (!bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			SaveMain(bool, varTree)
			else SaveMain(int, varTree)
			else SaveMain(float, varTree)
			else SaveMain(IntRange_t, varTree)
			else SaveMain(FloatRange_t, varTree)
			else SaveMain(std::string, varTree)
			else SaveMain(VA_LIST(std::vector<std::pair<std::string, Color_t>>), varTree)
			else SaveMain(Color_t, varTree)
			else SaveMain(Gradient_t, varTree)
			else SaveMain(DragBox_t, varTree)
			else SaveMain(WindowBox_t, varTree)
		}
		writeTree.put_child("ConVars", varTree);

		write_json(sConfigPath + "\\" + configName + sConfigExtension, writeTree);
		sCurrentConfig = configName; sCurrentVisuals = "";
		if (bNotify)
			F::Notifications.Add("Config " + configName + " saved");
	}
	catch (...)
	{
		SDK::Output("SaveConfig", "Failed", { 175, 150, 255, 255 });
		return false;
	}

	return true;
}

bool CConfigs::LoadConfig(const std::string& configName, bool bNotify)
{
	// Check if the config exists
	if (!std::filesystem::exists(sConfigPath + "\\" + configName + sConfigExtension))
	{
		// Save default config if one doesn't yet exist
		if (configName == std::string("default"))
			SaveConfig("default", false);

		return false;
	}

	// Read ptree from json
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree readTree;
		read_json(sConfigPath + "\\" + configName + sConfigExtension, readTree);
		
		bool bLegacy = false;
		if (const auto condTree = readTree.get_child_optional("Binds"))
		{
			F::Binds.vBinds.clear();

			for (auto& it : *condTree)
			{
				Bind_t tBind = {};
				if (auto getValue = it.second.get_optional<std::string>("Name")) { tBind.Name = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Type")) { tBind.Type = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Info")) { tBind.Info = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Key")) { tBind.Key = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Not")) { tBind.Not = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Active")) { tBind.Active = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Visible")) { tBind.Visible = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Parent")) { tBind.Parent = *getValue; }

				F::Binds.vBinds.push_back(tBind);
			}
		}
		else if (const auto condTree = readTree.get_child_optional("Conditions"))
		{	// support old string based indexing
			bLegacy = true;

			F::Binds.vBinds.clear();

			for (auto& it : *condTree)
			{
				if (FNV1A::Hash32(it.first.c_str()) == FNV1A::Hash32Const("default"))
					continue;

				Bind_t tBind = { it.first };
				if (auto getValue = it.second.get_optional<int>("Type")) { tBind.Type = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Info")) { tBind.Info = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Key")) { tBind.Key = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Not")) { tBind.Not = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Active")) { tBind.Active = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Visible")) { tBind.Visible = *getValue; }
				if (auto getValue = it.second.get_optional<std::string>("Parent"))
				{
					auto uHash = FNV1A::Hash32(getValue->c_str());
					for (auto it = F::Binds.vBinds.begin(); it != F::Binds.vBinds.end(); it++)
					{
						if (FNV1A::Hash32(it->Name.c_str()) == uHash)
						{
							tBind.Parent = std::distance(F::Binds.vBinds.begin(), it);
							break;
						}
					}
				}

				F::Binds.vBinds.push_back(tBind);
			}
		}

		if (const auto conVars = readTree.get_child_optional("ConVars"))
		{
			auto& varTree = *conVars;
			for (auto& var : g_Vars)
			{
				if (!bLoadNosave && var->m_iFlags & NOSAVE)
					continue;

				LoadMain(bool, varTree)
				else LoadMain(int, varTree)
				else LoadMain(float, varTree)
				else LoadMain(IntRange_t, varTree)
				else LoadMain(FloatRange_t, varTree)
				else LoadMain(std::string, varTree)
				else LoadMain(VA_LIST(std::vector<std::pair<std::string, Color_t>>), varTree)
				else LoadMain(Color_t, varTree)
				else LoadMain(Gradient_t, varTree)
				else LoadMain(DragBox_t, varTree)
				else LoadMain(WindowBox_t, varTree)
			}
		}

		H::Fonts.Reload(Vars::Menu::Scale.Map[DEFAULT_BIND]);

		sCurrentConfig = configName; sCurrentVisuals = "";
		if (bNotify)
			F::Notifications.Add("Config " + configName + " loaded");
	}
	catch (...)
	{
		SDK::Output("LoadConfig", "Failed", { 175, 150, 255, 255 });
		return false;
	}

	return true;
}

#define SaveRegular(type, tree) SaveJson(tree, var->m_sName.c_str(), var->As<type>()->Map[DEFAULT_BIND])
#define SaveMisc(type, tree) if (IsType(type)) SaveRegular(type, tree);
#define LoadRegular(type, tree) LoadJson(tree, var->m_sName.c_str(), var->As<type>()->Map[DEFAULT_BIND])
#define LoadMisc(type, tree) if (IsType(type)) LoadRegular(type, tree);

bool CConfigs::SaveVisual(const std::string& configName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree writeTree;

		for (auto& var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || !bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			SaveMisc(bool, writeTree)
			else SaveMisc(int, writeTree)
			else SaveMisc(float, writeTree)
			else SaveMisc(IntRange_t, writeTree)
			else SaveMisc(FloatRange_t, writeTree)
			else SaveMisc(std::string, writeTree)
			else SaveMisc(VA_LIST(std::vector<std::pair<std::string, Color_t>>), writeTree)
			else SaveMisc(Color_t, writeTree)
			else SaveMisc(Gradient_t, writeTree)
			else SaveMisc(DragBox_t, writeTree)
			else SaveMisc(WindowBox_t, writeTree)
		}

		write_json(sConfigPath + "\\Visuals\\" + configName + sConfigExtension, writeTree);
		if (bNotify)
			F::Notifications.Add("Visual config " + configName + " saved");
	}
	catch (...)
	{
		SDK::Output("SaveVisual", "Failed", { 175, 150, 255, 255 });
		return false;
	}
	return true;
}

bool CConfigs::LoadVisual(const std::string& configName, bool bNotify)
{
	// Check if the visual config exists
	if (!std::filesystem::exists(sVisualsPath + "\\" + configName + sConfigExtension))
	{
		//if (configName == std::string("default"))
		//	SaveVisual("default");
		return false;
	}

	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree readTree;
		read_json(sConfigPath + "\\Visuals\\" + configName + sConfigExtension, readTree);

		for (auto& var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || !bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			LoadMisc(bool, readTree)
			else LoadMisc(int, readTree)
			else LoadMisc(float, readTree)
			else LoadMisc(IntRange_t, readTree)
			else LoadMisc(FloatRange_t, readTree)
			else LoadMisc(std::string, readTree)
			else LoadMisc(VA_LIST(std::vector<std::pair<std::string, Color_t>>), readTree)
			else LoadMisc(Color_t, readTree)
			else LoadMisc(Gradient_t, readTree)
			else LoadMisc(DragBox_t, readTree)
			else LoadMisc(WindowBox_t, readTree)
		}

		sCurrentVisuals = configName;
		if (bNotify)
			F::Notifications.Add("Visual config " + configName + " loaded");
	}
	catch (...)
	{
		SDK::Output("LoadVisual", "Failed", { 175, 150, 255, 255 });
		return false;
	}
	return true;
}

#define ResetType(type) var->As<type>()->Map = { { DEFAULT_BIND, var->As<type>()->Default } };
#define ResetT(type) if (IsType(type)) ResetType(type)

void CConfigs::RemoveConfig(const std::string& configName)
{
	if (FNV1A::Hash32(configName.c_str()) != FNV1A::Hash32Const("default"))
		std::filesystem::remove(sConfigPath + "\\" + configName + sConfigExtension);
	else
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		F::Binds.vBinds.clear();

		for (auto& var : g_Vars)
		{
			if (!bLoadNosave && var->m_iFlags & NOSAVE)
				continue;

			ResetT(bool)
			else ResetT(int)
			else ResetT(float)
			else ResetT(IntRange_t)
			else ResetT(FloatRange_t)
			else ResetT(std::string)
			else ResetT(std::vector<std::string>)
			else ResetT(Color_t)
			else ResetT(Gradient_t)
			else ResetT(DragBox_t)
			else ResetT(WindowBox_t)
		}

		SaveConfig("default", false);
	}
}

void CConfigs::RemoveVisual(const std::string& configName)
{
	std::filesystem::remove(sVisualsPath + "\\" + configName + sConfigExtension);
}