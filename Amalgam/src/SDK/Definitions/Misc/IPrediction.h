#pragma once
#include "../Types.h"

class IPrediction
{
public:
	virtual ~IPrediction(void) {};
	virtual void Init(void) = 0;
	virtual void Shutdown(void) = 0;
	virtual void Update(int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) = 0;
	virtual void PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet) = 0;
	virtual void PostEntityPacketReceived(void) = 0;
	virtual void PostNetworkDataReceived(int commands_acknowledged) = 0;
	virtual void OnReceivedUncompressedPacket(void) = 0;
	virtual void GetViewOrigin(Vector& org) = 0;
	virtual void SetViewOrigin(Vector& org) = 0;
	virtual void GetViewAngles(QAngle& ang) = 0;
	virtual void SetViewAngles(QAngle& ang) = 0;
	virtual void GetLocalViewAngles(QAngle& ang) = 0;
	virtual void SetLocalViewAngles(QAngle& ang) = 0;
};