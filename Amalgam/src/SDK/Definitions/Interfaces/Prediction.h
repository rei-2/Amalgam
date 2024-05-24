#pragma once
#include "IMoveHelper.h"
#include "../Misc/IPrediction.h"

class CMoveData;
class CUserCmd;
class CBasePlayer;

class CPrediction : public IPrediction
{
public:
	virtual ~CPrediction() {};
	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Update(int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) = 0;
	virtual void OnReceivedUncompressedPacket() = 0;
	virtual void PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet) = 0;
	virtual void PostEntityPacketReceived() = 0;
	virtual void PostNetworkDataReceived(int commands_acknowledged) = 0;
	virtual bool InPrediction() = 0;
	virtual bool IsFirstTimePredicted() = 0;
	virtual int	GetIncomingPacketNumber() = 0;
	virtual void GetViewOrigin(Vec3& org) = 0;
	virtual void SetViewOrigin(Vec3& org) = 0;
	virtual void GetViewAngles(QAngle& ang) = 0;
	virtual void SetViewAngles(QAngle& ang) = 0;
	virtual void GetLocalViewAngles(QAngle& ang) = 0;
	virtual void SetLocalViewAngles(QAngle& ang) = 0;
	virtual void RunCommand(CBasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper) = 0;
	virtual void SetupMove(CBasePlayer* player, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move) = 0;
	virtual void FinishMove(CBasePlayer* player, CUserCmd* ucmd, CMoveData* move) = 0;
	virtual void SetIdealPitch(CBasePlayer* player, const Vec3& origin, const Vec3& angles, const Vec3& viewheight) = 0;
	virtual void _Update(bool received_new_world_update, bool validframe, int incoming_acknowledged, int outgoing_command) = 0;

	CHandle<CBaseEntity> m_hLastGround;
	bool m_bInPrediction;
	bool m_bFirstTimePredicted;
	bool m_bOldCLPredictValue;
	bool m_bEnginePaused;
	int m_nPreviousStartFrame;
	int m_nCommandsPredicted;
	int m_nServerCommandsAcknowledged;
	int m_bPreviousAckHadErrors;
	int m_nIncomingPacketNumber;
	float m_flIdealPitch;
};


MAKE_INTERFACE_VERSION(CPrediction, Prediction, "client.dll", "VClientPrediction001");