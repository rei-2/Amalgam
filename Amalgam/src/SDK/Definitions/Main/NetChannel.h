#pragma once
#include "INetworkSystem.h"
#include "UtlVector.h"
#include "../Misc/bitbuf.h"
#include "../Misc/BaseTypes.h"

enum
{
	FRAG_NORMAL_STREAM = 0,
	FRAG_FILE_STREAM,
	MAX_STREAMS
};

#define NET_MAX_DATAGRAM_PAYLOAD 1400
#define NET_MAX_PAYLOAD_BITS 11
#define DEFAULT_RATE 10000
#define SIGNON_TIME_OUT	120.0f
#define CONNECTION_PROBLEM_TIME	15.0f

#define MAX_RATE 50000
#define MIN_RATE 100

#define FRAGMENT_BITS 8
#define FRAGMENT_SIZE (1<<FRAGMENT_BITS)
#define MAX_FILE_SIZE_BITS	26
#define MAX_FILE_SIZE ((1<<MAX_FILE_SIZE_BITS)-1)

#define NET_MAX_PAYLOAD 4000
#define NET_MAX_MESSAGE 4096
#define MIN_ROUTEABLE_PACKET 16
#define MAX_ROUTEABLE_PACKET 1400
#define UDP_HEADER_SIZE 28

#define PACKET_FLAG_RELIABLE (1<<0)
#define PACKET_FLAG_CHOKED (1<<1)

class CUDPSocket;
class CUtlBuffer;
class CNetPacket;

class INetChannel;

class INetworkMessageHandler
{
public:
	virtual void OnConnectionClosing(INetChannel* channel, char const* reason) = 0;
	virtual void OnConnectionStarted(INetChannel* channel) = 0;
	virtual void OnPacketStarted(int inseq, int outseq) = 0;
	virtual void OnPacketFinished() = 0;
	virtual ~INetworkMessageHandler() {}
};

class INetChannelHandler
{
public:
	virtual	~INetChannelHandler(void) {};
	virtual void ConnectionStart(INetChannel* chan) = 0;
	virtual void ConnectionClosing(const char* reason) = 0;
	virtual void ConnectionCrashed(const char* reason) = 0;
	virtual void PacketStart(int incoming_sequence, int outgoing_acknowledged) = 0;
	virtual void PacketEnd(void) = 0;
	virtual void FileRequested(const char* fileName, unsigned int transferID) = 0;
	virtual void FileReceived(const char* fileName, unsigned int transferID) = 0;
	virtual void FileDenied(const char* fileName, unsigned int transferID) = 0;
};

class CNetPacket;
//{
//	DECLARE_FIXEDSIZE_ALLOCATOR(CNetPacket);
//
//public:
//	netadr_t m_From;
//	CUDPSocket *m_pSource;
//	float m_flReceivedTime;
//	unsigned char *m_pData;
//	bf_read m_Message;
//	int m_nSizeInBytes;
//
//private:
//	int m_nRefCount;
//};

class IConnectionlessPacketHandler
{
public:
	virtual bool ProcessConnectionlessPacket(CNetPacket* packet) = 0;
	virtual	~IConnectionlessPacketHandler(void) {};
};

class ILookupChannel
{
public:
	virtual INetChannel* FindNetChannel(const netadr_t& from) = 0;
};