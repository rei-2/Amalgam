#pragma once
#include "INetMessage.h"

class INetChannel;

enum
{
	NETWORKSYSTEM_FIRST_GROUP = 1
};

class INetworkMessage
{
public:
	virtual void SetNetChannel(INetChannel* netchan) = 0;
	virtual void SetReliable(bool state) = 0;
	virtual	bool ReadFromBuffer(bf_read& buffer) = 0;
	virtual	bool WriteToBuffer(bf_write& buffer) = 0;
	virtual bool IsReliable(void) const = 0;
	virtual int GetGroup(void) const = 0;
	virtual int GetType(void) const = 0;
	virtual const char* GetGroupName(void) const = 0;
	virtual const char* GetName(void) const = 0;
	virtual INetChannel* GetNetChannel(void) const = 0;
	virtual const char* ToString(void) const = 0;
	virtual void Release() = 0;
	virtual	~INetworkMessage() {};
};

class CNetworkMessage : public INetworkMessage
{
public:
	CNetworkMessage()
	{
		m_bReliable = true;
		m_pNetChannel = nullptr;
	}

	virtual void Release()
	{
		delete this;
	}

	virtual ~CNetworkMessage() {};

	virtual void SetReliable(bool state)
	{
		m_bReliable = state;
	}

	virtual bool IsReliable() const
	{
		return m_bReliable;
	}

	virtual void SetNetChannel(INetChannel* netchan)
	{
		m_pNetChannel = netchan;
	}

	virtual bool Process()
	{
		return false;
	}

	INetChannel* GetNetChannel() const
	{
		return m_pNetChannel;
	}

protected:
	bool m_bReliable;
	INetChannel* m_pNetChannel;
};