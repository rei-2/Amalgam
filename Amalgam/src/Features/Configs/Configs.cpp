#include "Configs.h"

#include "../Conditions/Conditions.h"
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



void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, bool val)
{
	mapTree.put(name, val);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, int val)
{
	mapTree.put(name, val);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, float val)
{
	mapTree.put(name, val);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const IntRange_t& val)
{
	boost::property_tree::ptree rangeTree;
	rangeTree.put("Min", val.Min);
	rangeTree.put("Max", val.Max);

	mapTree.put_child(name, rangeTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const FloatRange_t& val)
{
	boost::property_tree::ptree rangeTree;
	rangeTree.put("Min", val.Min);
	rangeTree.put("Max", val.Max);

	mapTree.put_child(name, rangeTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const std::string& val)
{
	mapTree.put(name, val);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const std::vector<std::string>& val)
{
	boost::property_tree::ptree vectorTree;
	for (const auto& sMat : val)
	{
		boost::property_tree::ptree child; child.put("", sMat);
		vectorTree.push_back(std::make_pair("", child));
	}
	mapTree.put_child(name, vectorTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const Color_t& val)
{
	mapTree.put_child(name, ColorToTree(val));
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const Gradient_t& val)
{
	boost::property_tree::ptree gradientTree;
	gradientTree.put_child("StartColor", ColorToTree(val.StartColor));
	gradientTree.put_child("EndColor", ColorToTree(val.EndColor));

	mapTree.put_child(name, gradientTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const DragBox_t& val)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", val.x);
	dragBoxTree.put("y", val.y);

	mapTree.put_child(name, dragBoxTree);
}

void CConfigs::SaveJson(boost::property_tree::ptree& mapTree, const char* name, const WindowBox_t& val)
{
	boost::property_tree::ptree dragBoxTree;
	dragBoxTree.put("x", val.x);
	dragBoxTree.put("y", val.y);
	dragBoxTree.put("w", val.w);
	dragBoxTree.put("h", val.h);

	mapTree.put_child(name, dragBoxTree);
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, bool& val)
{
	if (auto getValue = mapTree.get_optional<bool>(name))
	{
		val = *getValue;
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, int& val)
{
	if (auto getValue = mapTree.get_optional<int>(name))
	{
		val = *getValue;
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, float& val)
{
	if (auto getValue = mapTree.get_optional<float>(name))
	{
		val = *getValue;
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, IntRange_t& val)
{
	if (const auto getChild = mapTree.get_child_optional(name))
	{
		if (auto getValue = getChild->get_optional<int>("Min")) { val.Min = *getValue; }
		if (auto getValue = getChild->get_optional<int>("Max")) { val.Max = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, FloatRange_t& val)
{
	if (const auto getChild = mapTree.get_child_optional(name))
	{
		if (auto getValue = getChild->get_optional<int>("Min")) { val.Min = *getValue; }
		if (auto getValue = getChild->get_optional<int>("Max")) { val.Max = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, std::string& val)
{
	if (auto getValue = mapTree.get_optional<std::string>(name))
	{
		val = *getValue;
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, std::vector<std::string>& val)
{
	auto getMaterials = [](std::vector<std::string>& val, const boost::optional<boost::property_tree::ptree&> getVector)
		{
			if (!getVector)
				return;

			val.clear();
			for (auto& [_, mat] : *getVector)
			{
				std::string sMat = mat.data();

				bool bFound = false; // ensure no duplicates are assigned
				for (auto& str : val)
				{
					if (str == sMat)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
					val.push_back(mat.data());
			}

			// remove invalid materials
			for (auto it = val.begin(); it != val.end();)
			{
				if (FNV1A::Hash(it->c_str()) == FNV1A::HashConst("None") || FNV1A::Hash(it->c_str()) != FNV1A::HashConst("Original") && !F::Materials.mChamMaterials.contains(*it))
					it = val.erase(it);
				else
					++it;
			}
		};

	getMaterials(val, mapTree.get_child_optional(name));
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, Color_t& val)
{
	if (const auto getChild = mapTree.get_child_optional(name))
	{
		TreeToColor(*getChild, val);
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, Gradient_t& val)
{
	if (const auto getChild = mapTree.get_child_optional(name))
	{
		if (const auto getStartColor = getChild->get_child_optional("StartColor"))
			TreeToColor(*getStartColor, val.StartColor);
		if (const auto endColor = getChild->get_child_optional("EndColor"))
			TreeToColor(*endColor, val.EndColor);
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, DragBox_t& val)
{
	if (const auto getChild = mapTree.get_child_optional(name))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { val.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { val.y = *getValue; }
	}
}

void CConfigs::LoadJson(boost::property_tree::ptree& mapTree, const char* name, WindowBox_t& val)
{
	if (const auto getChild = mapTree.get_child_optional(name))
	{
		if (auto getValue = getChild->get_optional<int>("x")) { val.x = *getValue; }
		if (auto getValue = getChild->get_optional<int>("y")) { val.y = *getValue; }
		if (auto getValue = getChild->get_optional<int>("w")) { val.w = *getValue; }
		if (auto getValue = getChild->get_optional<int>("h")) { val.h = *getValue; }
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
#define SaveType(type, tree)\
{\
	boost::property_tree::ptree mapTree;\
	for (const auto& [cond, value] : var->GetVar<type>()->Map)\
		SaveJson(mapTree, cond.c_str(), value);\
	tree.put_child(var->m_sName.c_str(), mapTree);\
}
#define SaveT(type, tree) if (IsType(type)) SaveType(type, tree)
#define LoadType(type, tree)\
{\
	var->GetVar<type>()->Map = { { "default", var->GetVar<type>()->Default } };\
	if (const auto mapTree = tree.get_child_optional(var->m_sName.c_str()))\
	{\
		for (auto& it : *mapTree)\
		{\
			if ((!F::Conditions.mConditions.contains(it.first) || var->GetVar<type>()->m_iFlags & NOCOND) && FNV1A::Hash(it.first.c_str()) != FNV1A::HashConst("default"))\
				continue;\
			LoadJson(*mapTree, it.first.c_str(), var->GetVar<type>()->Map[it.first]);\
		}\
	}\
}
#define LoadT(type, tree) if (IsType(type)) LoadType(type, tree)

bool CConfigs::SaveConfig(const std::string& configName, bool bNotify)
{
	try
	{
		boost::property_tree::ptree writeTree;

		boost::property_tree::ptree condTree;
		for (const auto& sCond : F::Conditions.vConditions)
		{
			auto& tCond = F::Conditions.mConditions[sCond];

			boost::property_tree::ptree condTree2;
			condTree2.put("Type", tCond.Type);
			condTree2.put("Info", tCond.Info);
			condTree2.put("Key", tCond.Key);
			condTree2.put("Not", tCond.Not);
			condTree2.put("Active", tCond.Active);
			condTree2.put("Visible", tCond.Visible);
			condTree2.put("Parent", tCond.Parent);

			condTree.put_child(sCond, condTree2);
		}
		writeTree.put_child("Conditions", condTree);

		boost::property_tree::ptree varTree;
		for (const auto var : g_Vars)
		{
			if (var->m_iFlags & NOSAVE)
				continue;

			SaveT(bool, varTree)
			else SaveT(int, varTree)
			else SaveT(float, varTree)
			else SaveT(IntRange_t, varTree)
			else SaveT(FloatRange_t, varTree)
			else SaveT(std::string, varTree)
			else SaveT(std::vector<std::string>, varTree)
			else SaveT(Color_t, varTree)
			else SaveT(Gradient_t, varTree)
			else SaveT(DragBox_t, varTree)
			else SaveT(WindowBox_t, varTree)
		}
		writeTree.put_child("ConVars", varTree);

		write_json(sConfigPath + "\\" + configName + sConfigExtension, writeTree);
		sCurrentConfig = configName; sCurrentVisuals = "";
		if (bNotify)
			F::Notifications.Add("Config " + configName + " saved");
	}
	catch (...)
	{
		SDK::Output("F::Configs::SaveConfig()", "Failed", { 175, 150, 255, 255 });
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
		boost::property_tree::ptree readTree;
		read_json(sConfigPath + "\\" + configName + sConfigExtension, readTree);

		if (const auto condTree = readTree.get_child_optional("Conditions"))
		{
			F::Conditions.mConditions.clear();
			F::Conditions.vConditions.clear();

			for (auto& it : *condTree)
			{
				if (FNV1A::Hash(it.first.c_str()) == FNV1A::HashConst("default"))
					continue;

				Condition_t tCond = {};
				if (auto getValue = it.second.get_optional<int>("Type")) { tCond.Type = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Info")) { tCond.Info = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Key")) { tCond.Key = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Not")) { tCond.Not = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Active")) { tCond.Active = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Visible")) { tCond.Visible = *getValue; }
				if (auto getValue = it.second.get_optional<std::string>("Parent")) { tCond.Parent = *getValue; }

				F::Conditions.mConditions[it.first] = tCond;
				F::Conditions.vConditions.push_back(it.first);
			}
		}

		if (const auto conVars = readTree.get_child_optional("ConVars"))
		{
			auto& varTree = *conVars;
			for (const auto var : g_Vars)
			{
				if (var->m_iFlags & NOSAVE)
					continue;

				LoadT(bool, varTree)
				else LoadT(int, varTree)
				else LoadT(float, varTree)
				else LoadT(IntRange_t, varTree)
				else LoadT(FloatRange_t, varTree)
				else LoadT(std::string, varTree)
				else LoadT(std::vector<std::string>, varTree)
				else LoadT(Color_t, varTree)
				else LoadT(Gradient_t, varTree)
				else LoadT(DragBox_t, varTree)
				else LoadT(WindowBox_t, varTree)
			}
		}

		H::Fonts.Reload();

		sCurrentConfig = configName; sCurrentVisuals = "";
		if (bNotify)
			F::Notifications.Add("Config " + configName + " loaded");
	}
	catch (...)
	{
		SDK::Output("F::Configs::LoadConfig()", "Failed", { 175, 150, 255, 255 });
		return false;
	}

	return true;
}

bool CConfigs::SaveVisual(const std::string& configName, bool bNotify)
{
	try
	{
		boost::property_tree::ptree writeTree;

		for (const auto var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || var->m_iFlags & NOSAVE)
				continue;

			SaveT(bool, writeTree)
			else SaveT(int, writeTree)
			else SaveT(float, writeTree)
			else SaveT(IntRange_t, writeTree)
			else SaveT(FloatRange_t, writeTree)
			else SaveT(std::string, writeTree)
			else SaveT(std::vector<std::string>, writeTree)
			else SaveT(Color_t, writeTree)
			else SaveT(Gradient_t, writeTree)
			else SaveT(DragBox_t, writeTree)
			else SaveT(WindowBox_t, writeTree)
		}

		write_json(sConfigPath + "\\Visuals\\" + configName + sConfigExtension, writeTree);
		if (bNotify)
			F::Notifications.Add("Visual config " + configName + " saved");
	}
	catch (...)
	{
		SDK::Output("F::Configs::SaveVisual()", "Failed", { 175, 150, 255, 255 });
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
		boost::property_tree::ptree readTree;
		read_json(sConfigPath + "\\Visuals\\" + configName + sConfigExtension, readTree);

		for (const auto var : g_Vars)
		{
			if (!(var->m_iFlags & VISUAL) || var->m_iFlags & NOSAVE)
				continue;

			LoadT(bool, readTree)
			else LoadT(int, readTree)
			else LoadT(float, readTree)
			else LoadT(IntRange_t, readTree)
			else LoadT(FloatRange_t, readTree)
			else LoadT(std::string, readTree)
			else LoadT(std::vector<std::string>, readTree)
			else LoadT(Color_t, readTree)
			else LoadT(Gradient_t, readTree)
			else LoadT(DragBox_t, readTree)
			else LoadT(WindowBox_t, readTree)
		}

		H::Fonts.Reload();

		sCurrentVisuals = configName;
		if (bNotify)
			F::Notifications.Add("Visual config " + configName + " loaded");
	}
	catch (...)
	{
		SDK::Output("F::Configs::LoadVisual()", "Failed", { 175, 150, 255, 255 });
		return false;
	}
	return true;
}

#define ResetType(type) var->GetVar<type>()->Map = { { "default", var->GetVar<type>()->Default } };
#define ResetT(type) if (IsType(type)) ResetType(type)

void CConfigs::RemoveConfig(const std::string& configName)
{
	if (FNV1A::Hash(configName.c_str()) != FNV1A::HashConst("default"))
		std::filesystem::remove(sConfigPath + "\\" + configName + sConfigExtension);
	else
	{
		F::Conditions.mConditions.clear();
		F::Conditions.vConditions.clear();

		for (const auto var : g_Vars)
		{
			if (var->m_iFlags & NOSAVE)
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