#include "../SDK/SDK.h"

#include "../Features/TickHandler/TickHandler.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/Misc/Misc.h"

MAKE_SIGNATURE(CNetChan_SendNetMsg, "engine.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 8B F1 45 0F B6 F1", 0x0);

MAKE_HOOK(CNetChan_SendNetMsg, S::CNetChan_SendNetMsg(), bool,
	CNetChannel* pNetChan, INetMessage& msg, bool bForceReliable, bool bVoice)
{
	switch (msg.GetType())
	{
	case net_SetConVar:
	{
		auto pMsg = reinterpret_cast<NET_SetConVar*>(&msg);
		for (int i = 0; i < pMsg->m_ConVars.Count(); i++)
		{
			NET_SetConVar::CVar_t* localCvar = &pMsg->m_ConVars[i];

			// intercept and change any vars we want to control
			switch (FNV1A::Hash32(localCvar->Name))
			{
			case FNV1A::Hash32Const("cl_interp"): strncpy_s(localCvar->Value, std::to_string(F::Backtrack.m_flWishInterp).c_str(), MAX_OSPATH); break;
			case FNV1A::Hash32Const("cl_interp_ratio"): strncpy_s(localCvar->Value, "1.0", MAX_OSPATH); break;
			case FNV1A::Hash32Const("cl_interpolate"): strncpy_s(localCvar->Value, "1", MAX_OSPATH); break;
			case FNV1A::Hash32Const("cl_cmdrate"): strncpy_s(localCvar->Value, std::to_string(F::Misc.iWishCmdrate).c_str(), MAX_OSPATH); break;
			case FNV1A::Hash32Const("cl_updaterate"): strncpy_s(localCvar->Value, std::to_string(F::Misc.iWishUpdaterate).c_str(), MAX_OSPATH); break;
			}

			if (Vars::Debug::Logging.Value)
			{
				switch (FNV1A::Hash32(localCvar->Name))
				{
				case FNV1A::Hash32Const("cl_interp"):
				case FNV1A::Hash32Const("cl_interp_ratio"):
				case FNV1A::Hash32Const("cl_interpolate"):
				case FNV1A::Hash32Const("cl_cmdrate"):
				case FNV1A::Hash32Const("cl_updaterate"):
					SDK::Output("SendNetMsg", std::format("{}: {}", localCvar->Name, localCvar->Value).c_str(), { 100, 0, 255, 255 });
				}
			}
		}
		break;
	}
	case clc_VoiceData:
		// stop lag with voice chat
		bVoice = true;
		break;
	case clc_FileCRCCheck:
		// whitelist
		if (Vars::Misc::Exploits::BypassPure.Value)
			return false;
		break;
	case clc_RespondCvarValue:
		// causes b1g crash
		if (Vars::Visuals::Removals::ConvarQueries.Value)
		{
			auto pMsg = reinterpret_cast<uintptr_t*>(&msg);
			if (!pMsg) break;

			auto cvarName = reinterpret_cast<const char*>(pMsg[6]);
			if (!cvarName) break;
			switch (FNV1A::Hash32(cvarName))
			{
			case FNV1A::Hash32Const("mat_dxlevel"):
				cvarName = nullptr;
			}
			if (!cvarName) break;

			auto pConVar = U::ConVars.FindVar(cvarName);
			if (!pConVar) break;

			auto defaultValue = pConVar->m_pParent->m_pszDefaultValue;
			if (!defaultValue) break;

			pMsg[7] = uintptr_t(defaultValue);
			SDK::Output("Removals::ConvarQueries", msg.ToString());
		}
		break;
	case clc_Move:
	{
		const auto pMsg = reinterpret_cast<CLC_Move*>(&msg);

		{
			const int nLastOutGoingCommand = I::ClientState->lastoutgoingcommand;
			const int nChokedCommands = I::ClientState->chokedcommands;
			const int nNextCommandNr = nLastOutGoingCommand + nChokedCommands + 1;

			byte data[4000] = {};
			pMsg->m_DataOut.StartWriting(data, sizeof(data));
			pMsg->m_nNewCommands = std::clamp(1 + nChokedCommands, 0, MAX_NEW_COMMANDS);
			const int nExtraCommands = nChokedCommands + 1 - pMsg->m_nNewCommands;
			const int nCmdBackup = std::max(2, nExtraCommands);
			pMsg->m_nBackupCommands = std::clamp(nCmdBackup, 0, MAX_BACKUP_COMMANDS);

			const int nNumCmds = pMsg->m_nNewCommands + pMsg->m_nBackupCommands;
			int nFrom = -1;
			bool bOk = true;
			for (int nTo = nNextCommandNr - nNumCmds + 1; nTo <= nNextCommandNr; nTo++)
			{
				const bool bIsNewCmd = nTo >= nNextCommandNr - pMsg->m_nNewCommands + 1;
				bOk = bOk && I::BaseClientDLL->WriteUsercmdDeltaToBuffer(&pMsg->m_DataOut, nFrom, nTo, bIsNewCmd);
				nFrom = nTo;
			}

			if (bOk)
			{
				if (nExtraCommands > 0)
					pNetChan->m_nChokedPackets -= nExtraCommands;

				CALL_ORIGINAL(pNetChan, reinterpret_cast<INetMessage&>(*pMsg), bForceReliable, bVoice);
			}
		}

		if (!F::Ticks.m_bSpeedhack)
		{
			static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
			const int iAllowedNewCommands = std::max((sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24) - F::Ticks.m_iShiftedTicks, 0);
			const int iCmdCount = pMsg->m_nNewCommands + pMsg->m_nBackupCommands - 3;
			if (iCmdCount > iAllowedNewCommands)
			{
				SDK::Output("clc_Move", std::format("{:d} sent <{:d} | {:d}>, max was {:d}.", iCmdCount + 3, pMsg->m_nNewCommands, pMsg->m_nBackupCommands, iAllowedNewCommands).c_str(), { 255, 0, 0, 255 });
				F::Ticks.m_iDeficit = iCmdCount - iAllowedNewCommands;
			}
		}

		return true;
	}
	}

	return CALL_ORIGINAL(pNetChan, msg, bForceReliable, bVoice);
}