#pragma once
#include "../../SDK/SDK.h"
#include <filesystem>

Enum(EBStage, Normal = 0, NormalInverted, Random, RandomInverted);

class CMisc
{
private:
	void AutoJump(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoJumpbug(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoEdgebug(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoRevJump(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoStrafe(CTFPlayer* pLocal, CUserCmd* pCmd);
	void MovementLock(CTFPlayer* pLocal, CUserCmd* pCmd);
	void BreakJump(CTFPlayer* pLocal, CUserCmd* pCmd);
	void BreakShootSound(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd);
	void InstantRespawnMVM(CTFPlayer* pLocal);
	void ExecBuyBot(CTFPlayer* pLocal, CUserCmd* pCmd);
	void ResetBuyBot();
	void BuyBotJoinClass(int iClass);
	bool BuyBotWalkAwayFromStation(CTFPlayer* pLocal, CUserCmd* pCmd, const Vec3& vStation);
	void VoiceCommandSpam(CTFPlayer* pLocal);
	void ChatSpam(CTFPlayer* pLocal);
	void AutoDisguise(CTFPlayer* pLocal);

	void AchievementSpam(CTFPlayer* pLocal);
	void NoiseSpam(CTFPlayer* pLocal);
	void CallVoteSpam(CTFPlayer* pLocal);

	void AutoReport();
	void AutoRetry(CTFPlayer* pLocal);
	void CheatsBypass();
	void WeaponSway();

	void TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd);
	void ChargeBot(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoAirStickyPogo(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoCrouchNavbot(CTFPlayer* pLocal, CUserCmd* pCmd);
	void FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd);

	void AutoPeek(CTFPlayer* pLocal, CUserCmd* pCmd, bool bPost = false);
	void EdgeJump(CTFPlayer* pLocal, CUserCmd* pCmd, bool bPost = false);

	int m_iEdgeBugTicksLeft = 0;
	int m_iEdgeBugTicksTotal = 0;
	int m_iEdgeBugTicksUntilLand = 0;
	int m_iEdgeBugMoveStage = EBStageEnum::Normal;
	float m_flEdgeBugYawDelta = 0.f;
	float m_flEdgeBugStartYaw = 0.f;
	Vector2D m_vEdgeBugMove = {};
	bool m_bEdgeBugCrouch = false;
	bool m_bEdgeBugRepredict = false;
	bool m_bEdgeBug = false;
	bool m_bDuckSpeedActive = false;
	bool m_bChargeBotReady = false;
	Timer m_tChargeBotTimer;
	Timer m_tStickyPogoTimer;
	std::vector<Vec3> m_vEdgebugPath;

	bool m_bPeekPlaced = false;
	Vec3 m_vPeekReturnPos = {};

	//bool bSteamCleared = false;

	std::vector<std::string> m_vChatSpamLines;
	std::vector<std::string> m_vKillSayLines;
	void DoKillSay(int iVictim);
	void EnsureChatUtilsDoc();
	struct AutoReply_t { std::vector<std::string> vTriggers; std::vector<std::string> vReplies; };
	std::vector<AutoReply_t> m_vAutoReplies;
	std::vector<std::string> m_vF1Messages;
	std::vector<std::string> m_vF2Messages;
	Timer m_tChatSpamTimer;
	int m_iCurrentChatSpamIndex = 0;

	bool LoadLines(const char* szFileName, std::vector<std::string>& vLines, const char* szDefaultContent = nullptr);
	std::vector<std::string> ParseTokens(std::string str, char delimiter);

	enum class AchievementSpamState
	{
		IDLE,
		CLEARING,
		WAITING,
		AWARDING
	};

	AchievementSpamState m_eAchievementSpamState = AchievementSpamState::IDLE;
	Timer m_tAchievementSpamTimer;
	Timer m_tAchievementDelayTimer;
	int m_iAchievementSpamID = 0;
	std::string m_sAchievementSpamName = "";
	Timer m_tCallVoteSpamTimer;
	bool m_bAutoBalanceTeamChangePending = false;

	int m_iBuybotStep = 1;
	int m_iBuybotUpgradeSlotStep = 0;
	int m_iBuybotUpgradeIndex = 0;
	float m_flBuybotClock = 0.0f;
	float m_flBuybotStationPathStart = 0.0f;
	float m_flBuybotNavClock = 0.0f;
	float m_flBuybotClassClock = 0.0f;
	float m_flBuybotRetryClock = 0.0f;
	bool m_bBuybotUsingNav = false;
	bool m_bBuybotCashLimitReached = false;
	bool m_bBuybotFinishedUpgrades = false;
	Vec3 m_vBuybotStationTarget = {};

public:
	struct ProfileDumpResult_t
	{
		bool m_bResourceAvailable = false;
		bool m_bFileOpened = false;
		bool m_bSuccess = false;
		size_t m_uCandidateCount = 0;
		size_t m_uSkippedInvalid = 0;
		size_t m_uSkippedComma = 0;
		size_t m_uSkippedSessionDuplicate = 0;
		size_t m_uSkippedFileDuplicate = 0;
		size_t m_uAppendedCount = 0;
		size_t m_uAvatarsSaved = 0;
		size_t m_uAvatarMissed = 0;
		size_t m_uAvatarFailed = 0;
		std::filesystem::path m_outputPath;
		std::filesystem::path m_avatarFolder;
	};

	void RunPre(CTFPlayer* pLocal, CUserCmd* pCmd);
	void RunPost(CTFPlayer* pLocal, CUserCmd* pCmd);
	bool IsDuckSpeedActive() const { return m_bDuckSpeedActive; }

	void Event(IGameEvent* pEvent, uint32_t uNameHash);
	int AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoFaNJump(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void MicSpam();

	void PingReducer();
	void UnlockAchievements();
	void UnlockItemAchievements();
	void LockAchievements();
	void LockItemAchievements();
	void SetAutoBalanceTeamChangePending(bool bPending) { m_bAutoBalanceTeamChangePending = bPending; }
	void AutoMvmReadyUp();
	void OnBuyBotClassChangeBlocked();
	void OnVoteStart(int iCaller, int iTarget, const std::string& sTarget);
	void OnChatMessage(int iEntIndex, const std::string& sName, const std::string& sMsg);
	std::string ReplaceTags(std::string sMsg, std::string sTarget = "", std::string sInitiator = "", std::string sKiller = "", std::string sVictim = "");
	ProfileDumpResult_t DumpProfiles(bool bAnnounce = true);

	int m_iWishCmdrate = -1;
	//int m_iWishUpdaterate = -1;
	bool m_bAntiAFK = false;
	std::string m_sLastKilledName = "";
};

ADD_FEATURE(CMisc, Misc);
