#include "PlayerCore.h"

#include "PlayerUtils.h"
#include "../Configs/Configs.h"

void CPlayerlistCore::Run()
{
	static Timer saveTimer{ };
	if (saveTimer.Run(1000))
	{
		LoadTags();
		SaveTags();
		LoadPlayers();
		SavePlayers();
	}
}

void CPlayerlistCore::SavePlayers()
{
	if (!F::PlayerUtils.m_bSavePlayers || F::PlayerUtils.m_bLoadPlayers) // terrible if we end up saving while loading
		return;

	try
	{
		boost::property_tree::ptree writeTree;

		writeTree.put("NewTags", true);

		boost::property_tree::ptree tagTree;
		for (auto& [friendsID, vTags] : F::PlayerUtils.m_mPlayerTags)
		{
			if (vTags.empty())
				continue;

			boost::property_tree::ptree tagList;
			for (auto& iID : vTags)
			{
				boost::property_tree::ptree child; child.put("", F::PlayerUtils.IndexToTag(iID));
				tagList.push_back(std::make_pair("", child));
			}

			tagTree.put_child(std::to_string(friendsID), tagList);
		}
		writeTree.put_child("Tags", tagTree);

		boost::property_tree::ptree aliasTree;
		for (auto& [friendsID, sAlias] : F::PlayerUtils.m_mPlayerAliases)
		{
			if (sAlias.empty())
				continue;

			aliasTree.put(std::to_string(friendsID), sAlias);
		}
		writeTree.put_child("Aliases", aliasTree);

		// Save the file
		write_json(F::Configs.m_sConfigPath + "\\Core\\Players.json", writeTree);

		F::PlayerUtils.m_bSavePlayers = false;
	}
	catch (...)
	{
		SDK::Output("SavePlayers", "Failed", { 175, 150, 255, 255 });
	}
}

void CPlayerlistCore::LoadPlayers()
{
	if (!F::PlayerUtils.m_bLoadPlayers)
		return;

	try
	{
		if (std::filesystem::exists(F::Configs.m_sConfigPath + "\\Core\\Players.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(F::Configs.m_sConfigPath + "\\Core\\Players.json", readTree);
			F::PlayerUtils.m_mPlayerTags.clear();

			bool bNewTags = bool(readTree.get_child_optional("NewTags")); // newer system to support adding default tags better

			auto tagTree = readTree.get_child_optional("Tags");
			if (!tagTree)
				tagTree = readTree; // support format w/o tag tree
			
			for (auto& player : *tagTree)
			{
				uint32_t friendsID = std::stoi(player.first);

				for (auto& tag : player.second)
				{
					std::string sTag = std::string(tag.first.data()).empty() ? tag.second.data() : tag.first.data(); // account for dumb old format

					int iID = -1;
					try
					{	// new id based indexing
						iID = std::stoi(sTag);
						if (bNewTags)
							iID = F::PlayerUtils.TagToIndex(iID);
						else if (iID > 3)
							iID += TAG_COUNT - 3;
					}
					catch (...)
					{	// old string based indexing
						iID = F::PlayerUtils.GetTag(sTag);
					}

					if (iID == -1)
						continue;

					auto pTag = F::PlayerUtils.GetTag(iID);
					if (!pTag || !pTag->Assignable)
						continue;

					if (!F::PlayerUtils.HasTag(friendsID, iID))
						F::PlayerUtils.AddTag(friendsID, iID, false);
				}
			}

			if (auto aliasTree = readTree.get_child_optional("Aliases"))
			{
				for (auto& player : *aliasTree)
				{
					uint32_t friendsID = std::stoi(player.first);
					std::string sAlias = player.second.data();

					if (!sAlias.empty())
						F::PlayerUtils.m_mPlayerAliases[friendsID] = player.second.data();
				}
			}
		}
		// support legacy format & convert over
		if (std::filesystem::exists(F::Configs.m_sConfigPath + "\\Core\\Playerlist.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(F::Configs.m_sConfigPath + "\\Core\\Playerlist.json", readTree);

			for (auto& it : readTree)
			{
				uint32_t friendsID = std::stoi(it.first);

				int iPriority = 2;
				if (auto getValue = it.second.get_optional<int>("Mode")) { iPriority = std::max(*getValue, 0); }

				switch (iPriority)
				{
				case 1: F::PlayerUtils.AddTag(friendsID, F::PlayerUtils.TagToIndex(IGNORED_TAG), false); break;
				case 4: F::PlayerUtils.AddTag(friendsID, F::PlayerUtils.TagToIndex(CHEATER_TAG), false); break;
				}
			}
		}

		F::PlayerUtils.m_bLoadPlayers = false;
	}
	catch (...)
	{
		SDK::Output("LoadPlayers", "Failed", { 175, 150, 255, 255 });
	}
}

void CPlayerlistCore::SaveTags()
{
	if (!F::PlayerUtils.m_bSaveTags || F::PlayerUtils.m_bLoadTags) // terrible if we end up saving while loading
		return;

	try
	{
		boost::property_tree::ptree writeTree;

		writeTree.put("NewTags", true);

		boost::property_tree::ptree tagTree;
		{
			for (auto it = F::PlayerUtils.m_vTags.begin(); it != F::PlayerUtils.m_vTags.end(); it++)
			{
				int iID = std::distance(F::PlayerUtils.m_vTags.begin(), it);
				auto& tTag = *it;

				boost::property_tree::ptree tagEntry;
				tagEntry.put("Name", tTag.Name);
				tagEntry.put_child("Color", F::Configs.ColorToTree(tTag.Color));
				tagEntry.put("Priority", tTag.Priority);
				tagEntry.put("Label", tTag.Label);

				tagTree.put_child(std::to_string(F::PlayerUtils.IndexToTag(iID)), tagEntry);
			}
		}
		writeTree.put_child("Tags", tagTree);

		// Save the file
		write_json(F::Configs.m_sConfigPath + "\\Core\\Tags.json", writeTree);

		F::PlayerUtils.m_bSaveTags = false;
	}
	catch (...)
	{
		SDK::Output("SaveTags", "Failed", { 175, 150, 255, 255 });
	}
}

void CPlayerlistCore::LoadTags()
{
	if (!F::PlayerUtils.m_bLoadTags)
		return;

	try
	{
		if (std::filesystem::exists(F::Configs.m_sConfigPath + "\\Core\\Tags.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(F::Configs.m_sConfigPath + "\\Core\\Tags.json", readTree);
			F::PlayerUtils.m_vTags = {
				{ "Default", { 200, 200, 200, 255 }, 0, false, false, true },
				{ "Ignored", { 200, 200, 200, 255 }, -1, false, true, true },
				{ "Cheater", { 255, 100, 100, 255 }, 1, false, true, true },
				{ "Friend", { 100, 255, 100, 255 }, 0, true, false, true },
				{ "Party", { 100, 50, 255, 255 }, 0, true, false, true }
			};

			bool bNewTags = bool(readTree.get_child_optional("NewTags")); // newer system to support adding default tags better

			auto tagTree = readTree.get_child_optional("Tags");
			if (!tagTree)
				tagTree = readTree; // support format w/o tag tree

			for (auto& it : *tagTree)
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
					if (bNewTags)
						iID = F::PlayerUtils.TagToIndex(iID);
					else if (iID > 3)
						iID += TAG_COUNT - 3;
				}
				catch (...)
				{	// old string based indexing
					tTag.Name = it.first;
					iID = F::PlayerUtils.GetTag(it.first);
				}

				if (iID > -1 && iID < F::PlayerUtils.m_vTags.size())
				{
					F::PlayerUtils.m_vTags[iID].Name = tTag.Name;
					F::PlayerUtils.m_vTags[iID].Color = tTag.Color;
					F::PlayerUtils.m_vTags[iID].Priority = tTag.Priority;
					F::PlayerUtils.m_vTags[iID].Label = tTag.Label;
				}
				else
					F::PlayerUtils.m_vTags.push_back(tTag);
			}
		}

		F::PlayerUtils.m_bLoadTags = false;
	}
	catch (...)
	{
		SDK::Output("LoadTags", "Failed", { 175, 150, 255, 255 });
	}
}