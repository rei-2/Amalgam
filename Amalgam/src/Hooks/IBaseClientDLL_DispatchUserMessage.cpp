#include "../SDK/SDK.h"

#include "../Features/Visuals/ChatBubbles/ChatBubbles.h"
#include "../Features/Misc/Misc.h"
#include "../Features/Output/Output.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/Misc/AutoVote/AutoVote.h"
#include "../Features/Aimbot/AutoHeal/AutoHeal.h"

//#define DEBUG_VISUALS
#ifdef DEBUG_VISUALS
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#endif

MAKE_HOOK(IBaseClientDLL_DispatchUserMessage, U::Memory.GetVirtual(I::BaseClientDLL, 36), bool,
	void* rcx, UserMessageType type, bf_read& msgData)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IBaseClientDLL_DispatchUserMessage[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, type, msgData);
#endif

	auto bufData = reinterpret_cast<const char*>(msgData.m_pData);
	msgData.SetAssertOnOverflow(false);
	msgData.Seek(0);

	switch (type)
	{
	case VoteStart:
		F::Output.UserMessage(msgData);
		F::AutoVote.UserMessage(msgData);

		break;
	case VoiceSubtitle:
	{
		int iEntityID = msgData.ReadByte();
		int iVoiceMenu = msgData.ReadByte();
		int iCommandID = msgData.ReadByte();
		
#ifdef _DEBUG
		// Debug: Show voice commands being captured (only when chatbubbles is enabled)
		if (Vars::Competitive::Features::ChatBubbles.Value)
		{
			I::CVar->ConsolePrintf("VoiceSubtitle: Entity %d, Menu %d, Command %d\n", 
			                      iEntityID, iVoiceMenu, iCommandID);
		}
#endif
		
		if (iVoiceMenu == 1 && iCommandID == 6)
			F::AutoHeal.m_mMedicCallers[iEntityID] = true;
		
		// Add ChatBubbles voice command handling
		F::ChatBubbles.OnVoiceSubtitle(iEntityID, iVoiceMenu, iCommandID);

		break;
	}
	case SayText:
	{
		// SayText is rarely used in TF2 - skip for now
		break;
	}
	case SayText2:
	{
		if (Vars::Competitive::Features::ChatBubblesTextChat.Value && Vars::Competitive::Features::ChatBubbles.Value)
		{
			try
			{
				int entityIndex = msgData.ReadByte();
				if (entityIndex <= 0 || entityIndex > 64)
					break;
				
				msgData.ReadByte(); // Skip chat type
				
				// Read all the message parts like the Lua script
				char content[256] = {0};
				char name[256] = {0};  
				char message[256] = {0};
				char param3[256] = {0};
				char param4[256] = {0};
				
				msgData.ReadString(content, sizeof(content), true);
				msgData.ReadString(name, sizeof(name), true);
				msgData.ReadString(message, sizeof(message), true);
				msgData.ReadString(param3, sizeof(param3), true);
				msgData.ReadString(param4, sizeof(param4), true);
				
				std::string playerName;
				std::string chatText;
				
				// Check if it's a TF_Chat format message
				std::string contentStr = content;
				if (contentStr.find("TF_Chat") != std::string::npos) {
					if (strlen(name) > 0 && strlen(message) > 0) {
						playerName = name;
						chatText = message;
					}
				} else {
					// Try to parse "PlayerName: Message" format from any of the parts
					const char* parts[] = {content, name, message, param3, param4};
					
					for (int i = 0; i < 5; i++) {
						if (strlen(parts[i]) > 0) {
							std::string part = parts[i];
							size_t colonPos = part.find(": ");
							if (colonPos != std::string::npos) {
								playerName = part.substr(0, colonPos);
								chatText = part.substr(colonPos + 2);
								
								// Remove *DEAD* and *TEAM* prefixes like the Lua script
								if (playerName.find("*DEAD*") == 0)
									playerName = playerName.substr(6);
								if (playerName.find("*TEAM*") == 0)
									playerName = playerName.substr(6);
								
								// Trim whitespace
								size_t start = playerName.find_first_not_of(" \t");
								size_t end = playerName.find_last_not_of(" \t");
								if (start != std::string::npos && end != std::string::npos)
									playerName = playerName.substr(start, end - start + 1);
								
								break;
							}
						}
					}
				}
				
				if (!playerName.empty() && !chatText.empty()) {
					// Optional team filtering
					if (Vars::Competitive::Features::ChatBubblesEnemyOnly.Value) {
						auto pPlayer = I::ClientEntityList->GetClientEntity(entityIndex);
						if (pPlayer) {
							auto pTFPlayer = pPlayer->As<CTFPlayer>();
							auto pLocal = H::Entities.GetLocal();
							if (pTFPlayer && pLocal && pTFPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
								break;
						}
					}
					
#ifdef _DEBUG
					I::CVar->ConsolePrintf("SayText2: %s: %s\n", playerName.c_str(), chatText.c_str());
#endif
					
					// Remove leading character if it exists (like the Lua script does)
					if (chatText.length() > 1 && (chatText[0] == '\x01' || chatText[0] == '\x03')) {
						chatText = chatText.substr(1);
					}
					
					std::string fullMessage = "[Chat] " + chatText;
					F::ChatBubbles.AddChatMessage(fullMessage, playerName, entityIndex, false);
				}
			}
			catch (...)
			{
				// Silently handle exceptions
			}
		}
		break;
	}
	case TextMsg:
	{
		char rawMsg[256]; msgData.ReadString(rawMsg, sizeof(rawMsg), true);
		msgData.Seek(0);
		std::string sMsg = rawMsg;
		if (!sMsg.empty())
		{
			sMsg.erase(sMsg.begin());

			if (F::NoSpreadHitscan.ParsePlayerPerf(sMsg))
				return true;

#ifdef DEBUG_VISUALS
			if (sMsg.find("[Box] ") == 0)
			{
				try
				{
					sMsg.replace(0, strlen("[Box] "), "");
					std::vector<std::string> vValues = {};
					boost::split(vValues, sMsg, boost::is_any_of(" "));
					if (vValues.size() != 17)
						return true;

					Vec3 vOrigin = { std::stof(vValues[0]), std::stof(vValues[1]), std::stof(vValues[2]) };
					Vec3 vMins = { std::stof(vValues[3]), std::stof(vValues[4]), std::stof(vValues[5]) };
					Vec3 vMaxs = { std::stof(vValues[6]), std::stof(vValues[7]), std::stof(vValues[8]) };
					Vec3 vAngles = { std::stof(vValues[9]), std::stof(vValues[10]), std::stof(vValues[11]) };
					Color_t tColor = { byte(std::stoi(vValues[12])), byte(std::stoi(vValues[13])), byte(std::stoi(vValues[14])), byte(255 - std::stoi(vValues[15])) };
					float flDuration = std::stof(vValues[16]);

					G::BoxStorage.emplace_back(vOrigin, vMins, vMaxs, vAngles, I::GlobalVars->curtime + flDuration, tColor, Color_t(0, 0, 0, 0), true);
				}
				catch (...) {}

				return true;
			}
			if (sMsg.find("[Line] ") == 0)
			{
				try
				{
					sMsg.replace(0, strlen("[Line] "), "");
					std::vector<std::string> vValues = {};
					boost::split(vValues, sMsg, boost::is_any_of(" "));
					if (vValues.size() != 11)
						return true;

					Vec3 vStart = { std::stof(vValues[0]), std::stof(vValues[1]), std::stof(vValues[2]) };
					Vec3 vEnd = { std::stof(vValues[3]), std::stof(vValues[4]), std::stof(vValues[5]) };
					Color_t tColor = { byte(std::stoi(vValues[6])), byte(std::stoi(vValues[7])), byte(std::stoi(vValues[8])), byte(255 - std::stoi(vValues[9])) };
					float flDuration = std::stof(vValues[10]);

					G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vStart, vEnd), I::GlobalVars->curtime + flDuration, tColor, true);
				}
				catch (...) {}

				return true;
			}
#endif

			if (Vars::Misc::Automation::AntiAutobalance.Value && FNV1A::Hash32(sMsg.c_str()) == FNV1A::Hash32Const("#TF_Autobalance_TeamChangePending"))
				I::EngineClient->ClientCmd_Unrestricted("retry");
		}
		break;
	}
	case VGUIMenu:
		if (Vars::Visuals::Removals::MOTD.Value && bufData
			&& FNV1A::Hash32(bufData) == FNV1A::Hash32Const("info"))
		{
			I::EngineClient->ClientCmd_Unrestricted("closedwelcomemenu");
			return true;
		}
		break;
	case ForcePlayerViewAngles:
		return Vars::Visuals::Removals::AngleForcing.Value ? true : CALL_ORIGINAL(rcx, type, msgData);
	case SpawnFlyingBird:
	case PlayerGodRayEffect:
	case PlayerTauntSoundLoopStart:
	case PlayerTauntSoundLoopEnd:
		return Vars::Visuals::Removals::Taunts.Value ? true : CALL_ORIGINAL(rcx, type, msgData);
	case Shake:
	case Fade:
	case Rumble:
		return Vars::Visuals::Removals::ScreenEffects.Value ? true : CALL_ORIGINAL(rcx, type, msgData);
	}

	msgData.Seek(0);
	return CALL_ORIGINAL(rcx, type, msgData);
}