#include "Configs.h"

#include "../Binds/Binds.h"
#include "../Visuals/Materials/Materials.h"
#include "../Chat/Chat.h"

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
					if (FNV1A::Hash32(pair.first.c_str()) == FNV1A::Hash32(sMat.c_str()))
					{
						bFound = true;
						break;
					}
				}
				if (bFound)
					continue;

				val.emplace_back(sMat, tColor);
			}

			// remove invalid materials
			for (auto it = val.begin(); it != val.end();)
			{
				auto uHash = FNV1A::Hash32(it->first.c_str());
				if (uHash == FNV1A::Hash32Const("None")
					|| uHash != FNV1A::Hash32Const("Original") && !F::Materials.m_mMaterials.contains(uHash))
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

#define IsType(type) pVar->m_iType == typeid(type).hash_code()

#define SaveCond(type, tree)\
{\
	boost::property_tree::ptree mapTree;\
	for (auto& [iBind, tValue] : pVar->As<type>()->Map)\
		SaveJson(mapTree, std::to_string(iBind), tValue);\
	tree.put_child(pVar->m_sName, mapTree);\
}
#define SaveMain(type, tree) if (IsType(type)) SaveCond(type, tree)
#define LoadCond(type, tree)\
{\
	auto currentValue = pVar->As<type>()->Map.find(DEFAULT_BIND) != pVar->As<type>()->Map.end() ? pVar->As<type>()->Map[DEFAULT_BIND] : pVar->As<type>()->Default;\
	pVar->As<type>()->Map = { { DEFAULT_BIND, currentValue } };\
	if (const auto mapTree = tree.get_child_optional(pVar->m_sName))\
	{\
		for (auto& it : *mapTree)\
		{\
			if (!bLegacy)\
			{\
				int iBind = std::stoi(it.first);\
				if (iBind == DEFAULT_BIND || F::Binds.m_vBinds.size() > iBind && !(pVar->As<type>()->m_iFlags & NOBIND))\
				{\
					LoadJson(*mapTree, it.first, pVar->As<type>()->Map[iBind]);\
					if (iBind != DEFAULT_BIND)\
						std::next(F::Binds.m_vBinds.begin(), iBind)->m_vVars.push_back(pVar);\
				}\
			}\
			else\
			{\
				int iBind = -2; /*invalid bind*/ \
				auto uHash = FNV1A::Hash32(it.first.c_str());\
				if (uHash == FNV1A::Hash32Const("default"))\
					iBind = DEFAULT_BIND;\
				else\
				{\
					for (auto it2 = F::Binds.m_vBinds.begin(); it2 != F::Binds.m_vBinds.end(); it2++)\
					{\
						if (uHash == FNV1A::Hash32(it2->m_sName.c_str()))\
						{\
							iBind = std::distance(F::Binds.m_vBinds.begin(), it2);\
							break;\
						}\
					}\
				}\
				if (iBind != -2 && (iBind == DEFAULT_BIND || F::Binds.m_vBinds.size() > iBind && !(pVar->As<type>()->m_iFlags & NOBIND)))\
				{\
					LoadJson(*mapTree, it.first, pVar->As<type>()->Map[iBind]);\
					if (iBind != DEFAULT_BIND)\
						std::next(F::Binds.m_vBinds.begin(), iBind)->m_vVars.push_back(pVar);\
				}\
			}\
		}\
	}\
	else if (!(pVar->m_iFlags & NOSAVE))\
	{\
		/* Suppress warnings for new Competitive variables to reduce console spam */\
		bool bSuppressWarning = pVar->m_sName.find("Competitive::") == 0;\
		if (!bSuppressWarning)\
			SDK::Output("Amalgam", std::format("{} not found", pVar->m_sName).c_str(), { 175, 150, 255, 127 }, true, true);\
	}\
}
#define LoadMain(type, tree) if (IsType(type)) LoadCond(type, tree)

bool CConfigs::SaveConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree writeTree;

		boost::property_tree::ptree bindTree;
		for (auto it = F::Binds.m_vBinds.begin(); it != F::Binds.m_vBinds.end(); it++)
		{
			int iID = std::distance(F::Binds.m_vBinds.begin(), it);
			auto& tBind = *it;

			boost::property_tree::ptree bindTree2;
			bindTree2.put("Name", tBind.m_sName);
			bindTree2.put("Type", tBind.m_iType);
			bindTree2.put("Info", tBind.m_iInfo);
			bindTree2.put("Key", tBind.m_iKey);
			bindTree2.put("Enabled", tBind.m_bEnabled);
			bindTree2.put("Visibility", tBind.m_iVisibility);
			bindTree2.put("Not", tBind.m_bNot);
			bindTree2.put("Active", tBind.m_bActive);
			bindTree2.put("Parent", tBind.m_iParent);

			bindTree.put_child(std::to_string(iID), bindTree2);
		}
		writeTree.put_child("Binds", bindTree);

		boost::property_tree::ptree varTree;
		for (auto& pVar : G::Vars)
		{
			if (!bLoadNosave && pVar->m_iFlags & NOSAVE)
				continue;

			// Special handling for Chat credentials - encrypt password and clear others when SaveCredentials is disabled
			if ((pVar->m_sName == "Vars::Chat::Password" || pVar->m_sName == "Vars::Chat::Username" || 
				 pVar->m_sName == "Vars::Chat::Email" || pVar->m_sName == "Vars::Chat::Server" ||
				 pVar->m_sName == "Vars::Chat::Space" || pVar->m_sName == "Vars::Chat::Room") && pVar->m_iType == typeid(std::string).hash_code())
			{
				auto stringVar = pVar->As<std::string>();
				std::string originalValue;
				
				// Get the current value
				if (stringVar->Map.find(DEFAULT_BIND) != stringVar->Map.end())
					originalValue = stringVar->Map[DEFAULT_BIND];
				
				// Check if SaveCredentials is enabled
				bool shouldSaveCredentials = false;
				for (auto& pCheckVar : G::Vars)
				{
					if (pCheckVar->m_sName == "Vars::Chat::SaveCredentials" && pCheckVar->m_iType == typeid(bool).hash_code())
					{
						auto saveCredsVar = pCheckVar->As<bool>();
						if (saveCredsVar->Map.find(DEFAULT_BIND) != saveCredsVar->Map.end())
							shouldSaveCredentials = saveCredsVar->Map[DEFAULT_BIND];
						break;
					}
				}
				
				if (pVar->m_sName == "Vars::Chat::Password")
				{
					// Special password handling - encrypt if SaveCredentials is enabled
					if (shouldSaveCredentials && !originalValue.empty())
					{
						// Encrypt the password
						std::string encryptedPassword = F::Chat.EncryptPassword(originalValue);
						if (!encryptedPassword.empty())
						{
							// Temporarily replace with encrypted version
							stringVar->Map[DEFAULT_BIND] = encryptedPassword;
							SaveMain(std::string, varTree);
							// Restore original password
							stringVar->Map[DEFAULT_BIND] = originalValue;
						}
						else
						{
							// Encryption failed, save empty string
							stringVar->Map[DEFAULT_BIND] = "";
							SaveMain(std::string, varTree);
							stringVar->Map[DEFAULT_BIND] = originalValue;
						}
					}
					else
					{
						// SaveCredentials disabled or password empty, save empty string
						stringVar->Map[DEFAULT_BIND] = "";
						SaveMain(std::string, varTree);
						stringVar->Map[DEFAULT_BIND] = originalValue;
					}
				}
				else
				{
					// Other chat credentials - save if SaveCredentials is enabled, otherwise save empty
					if (shouldSaveCredentials)
					{
						SaveMain(std::string, varTree);
					}
					else
					{
						// SaveCredentials disabled, save empty string
						std::string tempValue = stringVar->Map[DEFAULT_BIND];
						stringVar->Map[DEFAULT_BIND] = "";
						SaveMain(std::string, varTree);
						stringVar->Map[DEFAULT_BIND] = tempValue;
					}
				}
			}
			else
			{
				// Standard variable saving
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
		}
		writeTree.put_child("ConVars", varTree);

		write_json(m_sConfigPath + sConfigName + m_sConfigExtension, writeTree);
		m_sCurrentConfig = sConfigName; m_sCurrentVisuals = "";
		if (bNotify)
			SDK::Output("Amalgam", std::format("Config {} saved", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Save config failed", { 175, 150, 255, 127 }, true, true);
		return false;
	}

	return true;
}

bool CConfigs::LoadConfig(const std::string& sConfigName, bool bNotify)
{
	// Check if the config exists
	if (!std::filesystem::exists(m_sConfigPath + sConfigName + m_sConfigExtension))
	{
		// Save default config if one doesn't yet exist
		if (sConfigName == std::string("default"))
			SaveConfig("default", false);

		return false;
	}

	// Read ptree from json
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree readTree;
		read_json(m_sConfigPath + sConfigName + m_sConfigExtension, readTree);
		
		bool bLegacy = false;
		if (const auto condTree = readTree.get_child_optional("Binds"))
		{
			F::Binds.m_vBinds.clear();

			for (auto& it : *condTree)
			{
				Bind_t tBind = {};
				if (auto getValue = it.second.get_optional<std::string>("Name")) { tBind.m_sName = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Type")) { tBind.m_iType = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Info")) { tBind.m_iInfo = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Key")) { tBind.m_iKey = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Enabled")) { tBind.m_bEnabled = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Visibility")) { tBind.m_iVisibility = *getValue; }
				else if (auto getValue = it.second.get_optional<bool>("Visible")) { tBind.m_iVisibility = *getValue ? BindVisibilityEnum::Always : BindVisibilityEnum::Hidden; }
				if (auto getValue = it.second.get_optional<bool>("Not")) { tBind.m_bNot = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Active")) { tBind.m_bActive = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Parent"))
				{
					tBind.m_iParent = *getValue;
					if (F::Binds.m_vBinds.size() == tBind.m_iParent)
						tBind.m_iParent = DEFAULT_BIND - 1; // prevent infinite loop
				}

				F::Binds.m_vBinds.push_back(tBind);
			}
		}
		else if (const auto condTree = readTree.get_child_optional("Conditions"))
		{	// support old string based indexing
			bLegacy = true;

			F::Binds.m_vBinds.clear();

			for (auto& it : *condTree)
			{
				if (FNV1A::Hash32(it.first.c_str()) == FNV1A::Hash32Const("default"))
					continue;

				Bind_t tBind = { it.first };
				if (auto getValue = it.second.get_optional<int>("Type")) { tBind.m_iType = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Info")) { tBind.m_iInfo = *getValue; }
				if (auto getValue = it.second.get_optional<int>("Key")) { tBind.m_iKey = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Visible")) { tBind.m_iVisibility = *getValue ? BindVisibilityEnum::Always : BindVisibilityEnum::Hidden; }
				if (auto getValue = it.second.get_optional<bool>("Not")) { tBind.m_bNot = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Active")) { tBind.m_bActive = *getValue; }
				if (auto getValue = it.second.get_optional<std::string>("Parent"))
				{
					auto uHash = FNV1A::Hash32(getValue->c_str());
					for (auto it = F::Binds.m_vBinds.begin(); it != F::Binds.m_vBinds.end(); it++)
					{
						if (FNV1A::Hash32(it->m_sName.c_str()) == uHash)
						{
							tBind.m_iParent = std::distance(F::Binds.m_vBinds.begin(), it);
							break;
						}
					}
				}

				F::Binds.m_vBinds.push_back(tBind);
			}
		}

		if (const auto conVars = readTree.get_child_optional("ConVars"))
		{
			auto& varTree = *conVars;
			
			// First pass: Load all variables normally
			for (auto& pVar : G::Vars)
			{
				if (!bLoadNosave && pVar->m_iFlags & NOSAVE)
					continue;

				// Standard variable loading for all variables (including chat credentials)
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
			
			// Second pass: Handle chat credential decryption and clearing
			bool shouldLoadCredentials = false;
			
			// Get SaveCredentials setting (now that all variables are loaded)
			for (auto& pCheckVar : G::Vars)
			{
				if (pCheckVar->m_sName == "Vars::Chat::SaveCredentials" && pCheckVar->m_iType == typeid(bool).hash_code())
				{
					auto saveCredsVar = pCheckVar->As<bool>();
					if (saveCredsVar && saveCredsVar->Map.find(DEFAULT_BIND) != saveCredsVar->Map.end())
						shouldLoadCredentials = saveCredsVar->Map[DEFAULT_BIND];
					break;
				}
			}
			
			// Apply chat credential handling
			for (auto& pVar : G::Vars)
			{
				if ((pVar->m_sName == "Vars::Chat::Password" || pVar->m_sName == "Vars::Chat::Username" || 
					 pVar->m_sName == "Vars::Chat::Email" || pVar->m_sName == "Vars::Chat::Server" ||
					 pVar->m_sName == "Vars::Chat::Space" || pVar->m_sName == "Vars::Chat::Room") && 
					 pVar->m_iType == typeid(std::string).hash_code())
				{
					auto stringVar = pVar->As<std::string>();
					if (!stringVar) continue;
					
					std::string loadedValue;
					if (stringVar->Map.find(DEFAULT_BIND) != stringVar->Map.end())
						loadedValue = stringVar->Map[DEFAULT_BIND];
					
					if (pVar->m_sName == "Vars::Chat::Password")
					{
						// Special password handling - decrypt if SaveCredentials is enabled
						if (shouldLoadCredentials && !loadedValue.empty())
						{
							// Try to decrypt the password
							std::string decryptedPassword = F::Chat.DecryptPassword(loadedValue);
							if (!decryptedPassword.empty())
							{
								// Successfully decrypted, replace the encrypted value
								stringVar->Map[DEFAULT_BIND] = decryptedPassword;
								stringVar->Value = decryptedPassword;
							}
							// If decryption fails, keep the loaded value (might be legacy plain text)
						}
						else
						{
							// SaveCredentials disabled, clear the password
							stringVar->Map[DEFAULT_BIND] = "";
							stringVar->Value = "";
						}
					}
					else
					{
						// Other chat credentials - clear if SaveCredentials is disabled
						if (!shouldLoadCredentials)
						{
							stringVar->Map[DEFAULT_BIND] = "";
							stringVar->Value = "";
						}
						else
						{
							// If SaveCredentials is enabled, ensure Value field is updated
							stringVar->Value = stringVar->Map[DEFAULT_BIND];
						}
					}
				}
			}
		}

		H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);

		m_sCurrentConfig = sConfigName; m_sCurrentVisuals = "";
		if (bNotify)
			SDK::Output("Amalgam", std::format("Config {} loaded", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Load config failed", { 175, 150, 255, 127 }, true, true);
		return false;
	}

	return true;
}

bool CConfigs::SaveChatCredentials(const std::string& sConfigName)
{
	try
	{
		
		// Ensure default values are set in runtime variables if they're empty
		for (auto& pVar : G::Vars)
		{
			if (pVar->m_iType == typeid(std::string).hash_code())
			{
				auto stringVar = pVar->As<std::string>();
				if (stringVar->Map.find(DEFAULT_BIND) != stringVar->Map.end())
				{
					std::string& currentValue = stringVar->Map[DEFAULT_BIND];
					
					// Set default values if empty
					if (currentValue.empty())
					{
						if (pVar->m_sName == "Vars::Chat::Server")
							currentValue = "matrix.org";
						else if (pVar->m_sName == "Vars::Chat::Space")
							currentValue = "amalgam-comp";
						else if (pVar->m_sName == "Vars::Chat::Room")
							currentValue = "chat";
					}
				}
			}
		}
		
		// Read existing config if it exists
		boost::property_tree::ptree readTree;
		if (std::filesystem::exists(m_sConfigPath + sConfigName + m_sConfigExtension))
		{
			read_json(m_sConfigPath + sConfigName + m_sConfigExtension, readTree);
		}
		
		// Get or create the ConVars section
		boost::property_tree::ptree conVars;
		if (auto existingConVars = readTree.get_child_optional("ConVars"))
		{
			conVars = *existingConVars;
		}
		
		// Update only chat credential variables
		std::vector<std::string> chatVars = {
			"Vars::Chat::Server", "Vars::Chat::Username", "Vars::Chat::Password", 
			"Vars::Chat::Email", "Vars::Chat::Space", "Vars::Chat::Room", "Vars::Chat::SaveCredentials"
		};
		
		for (auto& pVar : G::Vars)
		{
			// Only process chat credential variables
			if (std::find(chatVars.begin(), chatVars.end(), pVar->m_sName) == chatVars.end())
				continue;
			
			// Debug: Show we found a chat variable  
			SDK::Output("Amalgam", std::format("Found chat var: {}", pVar->m_sName).c_str(), { 255, 255, 0 }, true, true);
			
				
			if (pVar->m_iType != typeid(std::string).hash_code() && pVar->m_iType != typeid(bool).hash_code())
				continue;
			
			// Debug: Show current value in this variable
			if (pVar->m_iType == typeid(std::string).hash_code())
			{
				auto stringVar = pVar->As<std::string>();
				if (stringVar->Map.find(DEFAULT_BIND) != stringVar->Map.end())
				{
					SDK::Output("Amalgam", std::format("  {} current value: '{}'", pVar->m_sName, stringVar->Map[DEFAULT_BIND]).c_str(), { 0, 255, 255 }, true, true);
				}
			}
			else if (pVar->m_iType == typeid(bool).hash_code())
			{
				auto boolVar = pVar->As<bool>();
				if (boolVar->Map.find(DEFAULT_BIND) != boolVar->Map.end())
				{
					SDK::Output("Amalgam", std::format("  {} current value: {}", pVar->m_sName, boolVar->Map[DEFAULT_BIND] ? "true" : "false").c_str(), { 0, 255, 255 }, true, true);
				}
			}
				
			// Special handling for Chat credentials - encrypt password and clear others when SaveCredentials is disabled
			if ((pVar->m_sName == "Vars::Chat::Password" || pVar->m_sName == "Vars::Chat::Username" || 
				 pVar->m_sName == "Vars::Chat::Email" || pVar->m_sName == "Vars::Chat::Server" ||
				 pVar->m_sName == "Vars::Chat::Space" || pVar->m_sName == "Vars::Chat::Room") && pVar->m_iType == typeid(std::string).hash_code())
			{
				auto stringVar = pVar->As<std::string>();
				std::string originalValue;
				
				// Get the current value
				if (stringVar->Map.find(DEFAULT_BIND) != stringVar->Map.end())
					originalValue = stringVar->Map[DEFAULT_BIND];
				
				// Check if SaveCredentials is enabled
				bool shouldSaveCredentials = false;
				for (auto& pCheckVar : G::Vars)
				{
					if (pCheckVar->m_sName == "Vars::Chat::SaveCredentials" && pCheckVar->m_iType == typeid(bool).hash_code())
					{
						auto saveCredsVar = pCheckVar->As<bool>();
						if (saveCredsVar->Map.find(DEFAULT_BIND) != saveCredsVar->Map.end())
							shouldSaveCredentials = saveCredsVar->Map[DEFAULT_BIND];
						break;
					}
				}
				
				boost::property_tree::ptree mapTree;
				
				if (pVar->m_sName == "Vars::Chat::Password")
				{
					// Special password handling - save if SaveCredentials is enabled
					if (shouldSaveCredentials && !originalValue.empty())
					{
						// "Encrypt" the password (currently plaintext for simplicity)
						std::string savedPassword = F::Chat.EncryptPassword(originalValue);
						mapTree.put(std::to_string(DEFAULT_BIND), savedPassword);
					}
					else
					{
						// SaveCredentials disabled or password empty, save empty string
						mapTree.put(std::to_string(DEFAULT_BIND), "");
					}
				}
				else
				{
					// Other chat credentials handling
					if (pVar->m_sName == "Vars::Chat::Username" || pVar->m_sName == "Vars::Chat::Email")
					{
						// Username and Email: save if SaveCredentials is enabled, otherwise save empty
						if (shouldSaveCredentials)
						{
							mapTree.put(std::to_string(DEFAULT_BIND), originalValue);
						}
						else
						{
							// SaveCredentials disabled, save empty string
							mapTree.put(std::to_string(DEFAULT_BIND), "");
						}
					}
					else
					{
						// Server, Space, Room: always save current value, but restore defaults if empty
						std::string valueToSave = originalValue;
						
						// Restore default values if empty
						if (valueToSave.empty())
						{
							if (pVar->m_sName == "Vars::Chat::Server")
								valueToSave = "matrix.org";
							else if (pVar->m_sName == "Vars::Chat::Space")
								valueToSave = "amalgam-comp";
							else if (pVar->m_sName == "Vars::Chat::Room")
								valueToSave = "chat";
						}
						
						mapTree.put(std::to_string(DEFAULT_BIND), valueToSave);
					}
				}
				
				conVars.put_child(pVar->m_sName, mapTree);
			}
			else if (pVar->m_sName == "Vars::Chat::SaveCredentials" && pVar->m_iType == typeid(bool).hash_code())
			{
				// Always save the SaveCredentials setting
				auto boolVar = pVar->As<bool>();
				boost::property_tree::ptree mapTree;
				
				bool value = false;
				if (boolVar->Map.find(DEFAULT_BIND) != boolVar->Map.end())
					value = boolVar->Map[DEFAULT_BIND];
				
				mapTree.put(std::to_string(DEFAULT_BIND), value);
				conVars.put_child(pVar->m_sName, mapTree);
			}
		}
		
		// Update the ConVars section in the tree
		readTree.put_child("ConVars", conVars);
		
		// Write the updated config back
		write_json(m_sConfigPath + sConfigName + m_sConfigExtension, readTree);
		
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool CConfigs::SaveChatCredentials(const std::string& sConfigName, const std::string& username, const std::string& password, const std::string& email)
{
	try
	{
		// Temporarily set the Vars values to the provided parameters
		std::string oldUsername = Vars::Chat::Username.Value;
		std::string oldPassword = Vars::Chat::Password.Value;
		std::string oldEmail = Vars::Chat::Email.Value;
		
		// Set both .Value and the actual Map storage that config system uses
		Vars::Chat::Username.Value = username;
		Vars::Chat::Password.Value = password;
		Vars::Chat::Email.Value = email;
		
		// Also set the Map storage directly (this is what the config system actually reads)
		for (auto& pVar : G::Vars)
		{
			if (pVar->m_sName == "Vars::Chat::Username" && pVar->m_iType == typeid(std::string).hash_code())
			{
				auto stringVar = pVar->As<std::string>();
				stringVar->Map[DEFAULT_BIND] = username;
			}
			else if (pVar->m_sName == "Vars::Chat::Password" && pVar->m_iType == typeid(std::string).hash_code())
			{
				auto stringVar = pVar->As<std::string>();
				stringVar->Map[DEFAULT_BIND] = password;
			}
			else if (pVar->m_sName == "Vars::Chat::Email" && pVar->m_iType == typeid(std::string).hash_code())
			{
				auto stringVar = pVar->As<std::string>();
				stringVar->Map[DEFAULT_BIND] = email;
			}
		}
		
		// Call the regular SaveChatCredentials function
		bool result = SaveChatCredentials(sConfigName);
		
		// Restore the original values (though they should be the same now)
		Vars::Chat::Username.Value = oldUsername;
		Vars::Chat::Password.Value = oldPassword;
		Vars::Chat::Email.Value = oldEmail;
		
		return result;
	}
	catch (...)
	{
		return false;
	}
}

#define SaveRegular(type, tree) SaveJson(tree, pVar->m_sName, pVar->As<type>()->Map[DEFAULT_BIND])
#define SaveMisc(type, tree) if (IsType(type)) SaveRegular(type, tree);
#define LoadRegular(type, tree) LoadJson(tree, pVar->m_sName, pVar->As<type>()->Map[DEFAULT_BIND])
#define LoadMisc(type, tree) if (IsType(type)) LoadRegular(type, tree);

bool CConfigs::SaveVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree writeTree;

		for (auto& pVar : G::Vars)
		{
			if (!(pVar->m_iFlags & VISUAL) || !bLoadNosave && pVar->m_iFlags & NOSAVE)
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

		write_json(m_sVisualsPath + sConfigName + m_sConfigExtension, writeTree);
		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} saved", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Save visuals failed", { 175, 150, 255, 127 }, true, true);
		return false;
	}
	return true;
}

bool CConfigs::LoadVisual(const std::string& sConfigName, bool bNotify)
{
	// Check if the visual config exists
	if (!std::filesystem::exists(m_sVisualsPath + sConfigName + m_sConfigExtension))
	{
		//if (sConfigName == std::string("default"))
		//	SaveVisual("default");
		return false;
	}

	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		boost::property_tree::ptree readTree;
		read_json(m_sVisualsPath + sConfigName + m_sConfigExtension, readTree);

		for (auto& pVar : G::Vars)
		{
			if (!(pVar->m_iFlags & VISUAL) || !bLoadNosave && pVar->m_iFlags & NOSAVE)
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

		m_sCurrentVisuals = sConfigName;
		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} loaded", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Load visuals failed", { 175, 150, 255, 127 }, true, true);
		return false;
	}
	return true;
}

#define ResetType(type) pVar->As<type>()->Map = { { DEFAULT_BIND, pVar->As<type>()->Default } };
#define ResetT(type) if (IsType(type)) ResetType(type)

void CConfigs::DeleteConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		if (FNV1A::Hash32(sConfigName.c_str()) != FNV1A::Hash32Const("default"))
		{
			std::filesystem::remove(m_sConfigPath + sConfigName + m_sConfigExtension);

			if (FNV1A::Hash32(m_sCurrentConfig.c_str()) == FNV1A::Hash32(sConfigName.c_str()))
				LoadConfig("default", false);

			if (bNotify)
				SDK::Output("Amalgam", std::format("Config {} deleted", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
		}
		else
			ResetConfig(sConfigName);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Remove config failed", { 175, 150, 255, 127 }, true, true);
	}
}

void CConfigs::ResetConfig(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		F::Binds.m_vBinds.clear();

		for (auto& pVar : G::Vars)
		{
			if (!bLoadNosave && pVar->m_iFlags & NOSAVE)
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

		SaveConfig(sConfigName, false);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Config {} reset", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Reset config failed", { 175, 150, 255, 127 }, true, true);
	}
}

void CConfigs::DeleteVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		std::filesystem::remove(m_sVisualsPath + sConfigName + m_sConfigExtension);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} deleted", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Remove visuals failed", { 175, 150, 255, 127 }, true, true);
	}
}

void CConfigs::ResetVisual(const std::string& sConfigName, bool bNotify)
{
	try
	{
		const bool bLoadNosave = GetAsyncKeyState(VK_SHIFT) & 0x8000;

		for (auto& pVar : G::Vars)
		{
			if (!(pVar->m_iFlags & VISUAL) || !bLoadNosave && pVar->m_iFlags & NOSAVE)
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

		SaveVisual(sConfigName, false);

		if (bNotify)
			SDK::Output("Amalgam", std::format("Visual config {} reset", sConfigName).c_str(), { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Reset visuals failed", { 175, 150, 255, 127 }, true, true);
	}
}