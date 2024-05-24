#pragma once
#include "Interface.h"
#include "../Misc/bitbuf.h"

#define MAX_EVENT_NAME_LENGTH 32
#define MAX_EVENT_BITS 9
#define MAX_EVENT_NUMBER (1<<MAX_EVENT_BITS)
#define MAX_EVENT_BYTES 1024

class KeyValues;
class CGameEvent;

class IGameEvent
{
public:
	virtual ~IGameEvent() {};
	virtual const char* GetName() const = 0;
	virtual bool IsReliable() const = 0;
	virtual bool IsLocal() const = 0;
	virtual bool IsEmpty(const char* keyName = nullptr) = 0;
	virtual bool GetBool(const char* keyName = nullptr, bool defaultValue = false) = 0;
	virtual int GetInt(const char* keyName = nullptr, int defaultValue = 0) = 0;
	virtual float GetFloat(const char* keyName = nullptr, float defaultValue = 0.0f) = 0;
	virtual const char* GetString(const char* keyName = nullptr, const char* defaultValue = "") = 0;
	virtual void SetBool(const char* keyName, bool value) = 0;
	virtual void SetInt(const char* keyName, int value) = 0;
	virtual void SetFloat(const char* keyName, float value) = 0;
	virtual void SetString(const char* keyName, const char* value) = 0;
};

class IGameEventListener2
{
public:
	virtual	~IGameEventListener2(void) {};
	virtual void FireGameEvent(IGameEvent* event) = 0;
};

class IGameEventManager2 : public IBaseInterface
{
public:
	virtual	~IGameEventManager2(void) {};
	virtual int LoadEventsFromFile(const char* filename) = 0;
	virtual void Reset() = 0;
	virtual bool AddListener(IGameEventListener2* listener, const char* name, bool bServerSide) = 0;
	virtual bool FindListener(IGameEventListener2* listener, const char* name) = 0;
	virtual void RemoveListener(IGameEventListener2* listener) = 0;
	virtual IGameEvent* CreateEvent(const char* name, bool bForce = false) = 0;
	virtual bool FireEvent(IGameEvent* event, bool bDontBroadcast = false) = 0;
	virtual bool FireEventClientSide(IGameEvent* event) = 0;
	virtual IGameEvent* DuplicateEvent(IGameEvent* event) = 0;
	virtual void FreeEvent(IGameEvent* event) = 0;
	virtual bool SerializeEvent(IGameEvent* event, bf_write* buf) = 0;
	virtual IGameEvent* UnserializeEvent(bf_read* buf) = 0;
};

MAKE_INTERFACE_VERSION(IGameEventManager2, GameEventManager, "engine.dll", "GAMEEVENTSMANAGER002");

class IGameEventListener
{
public:
	virtual	~IGameEventListener(void) {};
	virtual void FireGameEvent(KeyValues* event) = 0;
};

class IGameEventManager : public IBaseInterface
{
public:
	virtual	~IGameEventManager(void) {};
	virtual int LoadEventsFromFile(const char* filename) = 0;
	virtual void  Reset() = 0;
	virtual KeyValues* GetEvent(const char* name) = 0;
	virtual bool AddListener(IGameEventListener* listener, const char* event, bool bIsServerSide) = 0;
	virtual bool AddListener(IGameEventListener* listener, bool bIsServerSide) = 0;
	virtual void RemoveListener(IGameEventListener* listener) = 0;
	virtual bool FireEvent(KeyValues* event) = 0;
	virtual bool FireEventServerOnly(KeyValues* event) = 0;
	virtual bool FireEventClientOnly(KeyValues* event) = 0;
	virtual bool SerializeKeyValues(KeyValues* event, bf_write* buf, CGameEvent* eventtype = nullptr) = 0;
	virtual KeyValues* UnserializeKeyValue(bf_read* msg) = 0;
};