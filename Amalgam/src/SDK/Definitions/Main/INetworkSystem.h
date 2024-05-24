#pragma once
#include "INetworkMessage.h"
#include "../Misc/IAppSystem.h"
#include "../Misc/NetAdr.h"

enum
{
	NETWORKSYSTEM_DEFAULT_SERVER_PORT = 27001,
	NETWORKSYSTEM_DEFAULT_CLIENT_PORT = 27002
};

typedef int ConnectionHandle_t;

enum ConnectionStatus_t
{
	CONNECTION_STATE_DISCONNECTED = 0,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTION_FAILED,
	CONNECTION_STATE_CONNECTED
};

enum NetworkEventType_t
{
	NETWORK_EVENT_CONNECTED = 0,
	NETWORK_EVENT_DISCONNECTED,
	NETWORK_EVENT_MESSAGE_RECEIVED,
};

struct NetworkEvent_t
{
	NetworkEventType_t m_nType;
};

struct NetworkConnectionEvent_t : public NetworkEvent_t
{
	void* m_pChannel;
};

struct NetworkDisconnectionEvent_t : public NetworkEvent_t
{
	void* m_pChannel;
};

struct NetworkMessageReceivedEvent_t : public NetworkEvent_t
{
	void* m_pChannel;
	INetworkMessage* m_pNetworkMessage;
};

class INetworkSystem : public IAppSystem
{
public:
	virtual bool RegisterMessage(INetworkMessage* msg) = 0;
	virtual bool StartServer(unsigned short nServerListenPort = NETWORKSYSTEM_DEFAULT_SERVER_PORT) = 0;
	virtual void ShutdownServer() = 0;
	virtual void ServerReceiveMessages() = 0;
	virtual void ServerSendMessages() = 0;
	virtual bool StartClient(unsigned short nClientListenPort = NETWORKSYSTEM_DEFAULT_CLIENT_PORT) = 0;
	virtual void ShutdownClient() = 0;
	virtual void ClientSendMessages() = 0;
	virtual void ClientReceiveMessages() = 0;
	virtual void* ConnectClientToServer(const char* pServer, int nServerListenPort = NETWORKSYSTEM_DEFAULT_SERVER_PORT) = 0;
	virtual void DisconnectClientFromServer(void* pChan) = 0;
	virtual NetworkEvent_t* FirstNetworkEvent() = 0;
	virtual NetworkEvent_t* NextNetworkEvent() = 0;
	virtual const char* GetLocalHostName(void) const = 0;
	virtual const char* GetLocalAddress(void) const = 0;
};