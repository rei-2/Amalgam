#pragma once

#define FLOW_OUTGOING 0		
#define FLOW_INCOMING 1
#define MAX_FLOWS 2

class INetChannelInfo
{
public:
	virtual const char* GetName(void) const = 0;
	virtual const char* GetAddress(void) const = 0;
	virtual float GetTime(void) const = 0;
	virtual float GetTimeConnected(void) const = 0;
	virtual int GetBufferSize(void) const = 0;
	virtual int GetDataRate(void) const = 0;
	virtual bool IsLoopback(void) const = 0;
	virtual bool IsTimingOut(void) const = 0;
	virtual bool IsPlayback(void) const = 0;
	virtual float GetLatency(int flow) const = 0;
	virtual float GetAvgLatency(int flow) const = 0;
	virtual float GetAvgLoss(int flow) const = 0;
	virtual float GetAvgChoke(int flow) const = 0;
	virtual float GetAvgData(int flow) const = 0;
	virtual float GetAvgPackets(int flow) const = 0;
	virtual int GetTotalData(int flow) const = 0;
	virtual int GetSequenceNr(int flow) const = 0;
	virtual bool IsValidPacket(int flow, int frame_number) const = 0;
	virtual float GetPacketTime(int flow, int frame_number) const = 0;
	virtual int GetPacketBytes(int flow, int frame_number, int group) const = 0;
	virtual bool GetStreamProgress(int flow, int* received, int* total) const = 0;
	virtual float GetTimeSinceLastReceived(void) const = 0;
	virtual	float GetCommandInterpolationAmount(int flow, int frame_number) const = 0;
	virtual void GetPacketResponseLatency(int flow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const = 0;
	virtual void GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const = 0;
	virtual float GetTimeoutSeconds() const = 0;
};