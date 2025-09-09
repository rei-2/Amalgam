#include "PlayerCore.h"

#include "PlayerUtils.h"
#include "../Configs/Configs.h"

void CPlayerlistCore::Run()
{
	static Timer tTimer = {};
	if (!tTimer.Run(1.f))
		return;

	LoadPlayerlist();
	SavePlayerlist();
}

void CPlayerlistCore::SavePlayerlist()
{
	if (!F::PlayerUtils.m_bSave || F::PlayerUtils.m_bLoad) // terrible if we end up saving while loading
		return;

	try
	{
		boost::property_tree::ptree tWrite;

		{
			boost::property_tree::ptree tSub;
			for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
			{
				int iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);
				auto& tTag = *it;

				boost::property_tree::ptree tChild;
				F::Configs.SaveJson(tChild, "Name", tTag.m_sName);
				F::Configs.SaveJson(tChild, "Color", tTag.m_tColor);
				F::Configs.SaveJson(tChild, "Priority", tTag.m_iPriority);
				F::Configs.SaveJson(tChild, "Label", tTag.m_bLabel);

				tSub.put_child(std::to_string(F::PlayerUtils.IndexToTag(iID)), tChild);
			}
			tWrite.put_child("Config", tSub);
		}

		{
			boost::property_tree::ptree tSub;
			for (auto& [uAccountID, vTags] : F::PlayerUtils.m_mPlayerTags)
			{
				if (vTags.empty())
					continue;

				boost::property_tree::ptree tChild;
				for (auto& iID : vTags)
				{
					boost::property_tree::ptree t;
					t.put("", F::PlayerUtils.IndexToTag(iID));
					tChild.push_back({ "", t });
				}

				tSub.put_child(std::to_string(uAccountID), tChild);
			}
			tWrite.put_child("Tags", tSub);
		}

		{
			boost::property_tree::ptree tSub;
			for (auto& [uAccountID, sAlias] : F::PlayerUtils.m_mPlayerAliases)
			{
				if (!sAlias.empty())
					tSub.put(std::to_string(uAccountID), sAlias);
			}
			tWrite.put_child("Aliases", tSub);
		}

		write_json(F::Configs.m_sCorePath + "Players.json", tWrite);

		F::PlayerUtils.m_bSave = false;
		SDK::Output("Amalgam", "Saved playerlist", { 175, 150, 255 }, OUTPUT_CONSOLE | OUTPUT_DEBUG | OUTPUT_TOAST | OUTPUT_MENU);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Save playerlist failed", { 175, 150, 255, 127 }, OUTPUT_CONSOLE | OUTPUT_DEBUG);
	}
}

void CPlayerlistCore::LoadPlayerlist()
{
	if (!F::PlayerUtils.m_bLoad)
		return;

	try
	{
		if (!std::filesystem::exists(F::Configs.m_sCorePath + "Players.json"))
		{
			F::PlayerUtils.m_bLoad = false;
			return;
		}

		boost::property_tree::ptree tRead;
		read_json(F::Configs.m_sCorePath + "Players.json", tRead);

		F::PlayerUtils.m_mPlayerTags.clear();
		F::PlayerUtils.m_mPlayerAliases.clear();
		F::PlayerUtils.m_vTags = {
			{ "Default", { 200, 200, 200, 255 }, 0, false, false, true },
			{ "Ignored", { 200, 200, 200, 255 }, -1, false, true, true },
			{ "Cheater", { 255, 100, 100, 255 }, 1, false, true, true },
			{ "Friend", { 100, 255, 100, 255 }, 0, true, false, true },
			{ "Party", { 100, 50, 255, 255 }, 0, true, false, true },
			{ "F2P", { 255, 255, 255, 255 }, 0, true, false, true }
		};

		if (auto tSub = tRead.get_child_optional("Config"))
		{
			for (auto& [sName, tChild] : *tSub)
			{
				PriorityLabel_t tTag = {};
				F::Configs.LoadJson(tChild, "Name", tTag.m_sName);
				F::Configs.LoadJson(tChild, "Color", tTag.m_tColor);
				F::Configs.LoadJson(tChild, "Priority", tTag.m_iPriority);
				F::Configs.LoadJson(tChild, "Label", tTag.m_bLabel);

				int iID = F::PlayerUtils.TagToIndex(std::stoi(sName));
				if (iID > -1 && iID < F::PlayerUtils.m_vTags.size())
				{
					F::PlayerUtils.m_vTags[iID].m_sName = tTag.m_sName;
					F::PlayerUtils.m_vTags[iID].m_tColor = tTag.m_tColor;
					F::PlayerUtils.m_vTags[iID].m_iPriority = tTag.m_iPriority;
					F::PlayerUtils.m_vTags[iID].m_bLabel = tTag.m_bLabel;
				}
				else
					F::PlayerUtils.m_vTags.push_back(tTag);
			}
		}

		if (auto tSub = tRead.get_child_optional("Tags"))
		{
			for (auto& [sName, tChild] : *tSub)
			{
				uint32_t uAccountID = std::stoul(sName);
				for (auto& [_, tTag] : tChild)
				{
					const std::string& sTag = tTag.data();

					int iID = F::PlayerUtils.TagToIndex(std::stoi(sTag));
					auto pTag = F::PlayerUtils.GetTag(iID);
					if (!pTag || !pTag->m_bAssignable)
						continue;

					if (!F::PlayerUtils.HasTag(uAccountID, iID))
						F::PlayerUtils.AddTag(uAccountID, iID, false);
				}
			}
		}

		if (auto tSub = tRead.get_child_optional("Aliases"))
		{
			for (auto& [sName, jAlias] : *tSub)
			{
				uint32_t uAccountID = std::stoul(sName);
				const std::string& sAlias = jAlias.data();

				if (!sAlias.empty())
					F::PlayerUtils.m_mPlayerAliases[uAccountID] = sAlias;
			}
		}

		F::PlayerUtils.m_bLoad = false;
		SDK::Output("Amalgam", "Loaded playerlist", { 175, 150, 255 }, OUTPUT_CONSOLE | OUTPUT_DEBUG | OUTPUT_TOAST | OUTPUT_MENU);
	}
	catch (...)
	{
		SDK::Output("Amalgam", "Load playerlist failed", { 175, 150, 255, 127 }, OUTPUT_CONSOLE | OUTPUT_DEBUG);
	}
}