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
	if (!F::PlayerUtils.bSavePlayers)
		return;

	try
	{
		boost::property_tree::ptree writeTree;

		// Put map entries into ptree
		for (const auto& [friendsID, vTags] : F::PlayerUtils.mPlayerTags)
		{
			// don't fill with pointless info
			if (!vTags.size())
				continue;

			boost::property_tree::ptree tagTree;
			for (const auto& sTag : vTags)
			{
				boost::property_tree::ptree child; child.put("", sTag);
				tagTree.push_back(std::make_pair("", child));
			}

			writeTree.put_child(std::to_string(friendsID), tagTree);
		}

		// Save the file
		write_json(F::Configs.sConfigPath + "\\Core\\Players.json", writeTree);

		F::PlayerUtils.bSavePlayers = false;
	}
	catch (...) {}
}

void CPlayerlistCore::LoadPlayers()
{
	if (!F::PlayerUtils.bLoadPlayers)
		return;

	try
	{
		if (std::filesystem::exists(F::Configs.sConfigPath + "\\Core\\Players.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(F::Configs.sConfigPath + "\\Core\\Players.json", readTree);
			F::PlayerUtils.mPlayerTags.clear();

			for (auto& player : readTree)
			{
				uint32_t friendsID = std::stoi(player.first);

				for (auto& tag : player.second)
				{
					std::string sTag = std::string(tag.first.data()).empty() ? tag.second.data() : tag.first.data(); // account for dumb old format

					PriorityLabel_t plTag;
					if (F::PlayerUtils.GetTag(sTag, &plTag) && !plTag.Assignable)
						continue;

					if (!F::PlayerUtils.HasTag(friendsID, sTag))
						F::PlayerUtils.AddTag(friendsID, sTag, false);
				}
			}
		}
		// support legacy format & convert over
		if (std::filesystem::exists(F::Configs.sConfigPath + "\\Core\\Playerlist.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(F::Configs.sConfigPath + "\\Core\\Playerlist.json", readTree);

			for (auto& it : readTree)
			{
				uint32_t friendsID = std::stoi(it.first);

				int iPriority = 2;
				if (auto getValue = it.second.get_optional<int>("Mode")) { iPriority = std::max(*getValue, 0); }

				if (iPriority == 4)
					F::PlayerUtils.AddTag(friendsID, "Cheater", false);
				if (iPriority == 1)
					F::PlayerUtils.AddTag(friendsID, "Ignored", false);
			}
		}

		F::PlayerUtils.bLoadPlayers = false;
	}
	catch (...) {}
}

void CPlayerlistCore::SaveTags()
{
	if (!F::PlayerUtils.bSaveTags)
		return;

	try
	{
		boost::property_tree::ptree writeTree;

		// Put map entries into ptree
		for (const auto& [sTag, plTag] : F::PlayerUtils.mTags)
		{
			boost::property_tree::ptree tagTree;
			tagTree.put_child("Color", F::Configs.ColorToTree(plTag.Color));
			tagTree.put("Priority", plTag.Priority);
			tagTree.put("Label", plTag.Label);

			writeTree.put_child(sTag, tagTree);
		}

		// Save the file
		write_json(F::Configs.sConfigPath + "\\Core\\Tags.json", writeTree);

		F::PlayerUtils.bSaveTags = false;
	}
	catch (...) {}
}

void CPlayerlistCore::LoadTags()
{
	if (!F::PlayerUtils.bLoadTags)
		return;

	try
	{
		if (std::filesystem::exists(F::Configs.sConfigPath + "\\Core\\Tags.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(F::Configs.sConfigPath + "\\Core\\Tags.json", readTree);
			F::PlayerUtils.mTags = {
				{ "Default", { { 200, 200, 200, 255 }, 0, false, false, true } },
				{ "Ignored", { { 200, 200, 200, 255 }, -1, false, true, true } },
				{ "Cheater", { { 255, 100, 100, 255 }, 1, false, true, true } },
				{ "Friend", { { 100, 255, 100, 255 }, 0, true, false, true } }
			};

			for (auto& it : readTree)
			{
				PriorityLabel_t plTag = {};
				if (const auto getChild = it.second.get_child_optional("Color")) { F::Configs.TreeToColor(*getChild, plTag.Color); }
				if (auto getValue = it.second.get_optional<int>("Priority")) { plTag.Priority = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Label")) { plTag.Label = *getValue; }

				std::string sTag = it.first;
				F::PlayerUtils.mTags[sTag].Color = plTag.Color;
				F::PlayerUtils.mTags[sTag].Priority = plTag.Priority;
				F::PlayerUtils.mTags[sTag].Label = plTag.Label;
			}
		}

		F::PlayerUtils.bLoadTags = false;
	}
	catch (...) {}
}