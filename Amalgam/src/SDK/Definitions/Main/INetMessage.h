#pragma once
#include "../Misc/bitbuf.h"
#include "../Types.h"

#define	MAX_OSPATH		260			// max length of a filesystem pathname
#define net_NOP 0        // nop command used for padding
#define net_Disconnect 1 // disconnect, last message in connection
#define net_File 2       // file transmission message request/deny
#define net_Tick 3        // send last world tick
#define net_StringCmd 4   // a string command
#define net_SetConVar 5   // sends one/multiple convar settings
#define net_SignonState 6 // signals current signon state
#define clc_ClientInfo 8       
#define clc_Move 9             
#define clc_VoiceData 10       
#define clc_BaselineAck 11     
#define clc_ListenEvents 12    
#define clc_RespondCvarValue 13
#define clc_FileCRCCheck 14    
#define clc_CmdKeyValues 16  

class INetChannel;

enum
{
	GENERIC = 0,	// must be first and is default group
	LOCALPLAYER,	// bytes for local player entity update
	OTHERPLAYERS,	// bytes for other players update
	ENTITIES,		// all other entity bytes
	SOUNDS,			// game sounds
	EVENTS,			// event messages
	USERMESSAGES,	// user messages
	ENTMESSAGES,	// entity messages
	VOICE,			// voice data
	STRINGTABLE,	// a stringtable update
	MOVE,			// client move cmds
	STRINGCMD,		// string command
	SIGNON,			// various signondata
	TOTAL,			// must be last and is not a real group
};

class INetMessage
{
public:
	virtual ~INetMessage() {};
	virtual void SetNetChannel(INetChannel* netchan) = 0; // netchannel this message is from/for
	virtual void SetReliable(bool state) = 0;             // set to true if it's a reliable message
	virtual bool Process(void) = 0; // calles the recently set handler to process this message
	virtual bool ReadFromBuffer(bf_read& buffer) = 0; // returns true if parsing was OK
	virtual bool WriteToBuffer(bf_write& buffer) = 0; // returns true if writing was OK
	virtual bool IsReliable(void) const = 0; // true, if message needs reliable handling
	virtual int GetType(void) const = 0;         // returns module specific header tag eg svc_serverinfo
	virtual int GetGroup(void) const = 0;        // returns net message group of this message
	virtual const char* GetName(void) const = 0; // returns network message name, eg "svc_serverinfo"
	virtual INetChannel* GetNetChannel(void) const = 0;
	virtual const char* ToString(void) const = 0; // returns a human readable string about message content
};

class CNetMessage : public INetMessage
{
public:
	CNetMessage()
	{
		m_bReliable = true;
		m_NetChannel = nullptr;
	}

	virtual ~CNetMessage() {};

	virtual int GetGroup() const { return GENERIC; }
	INetChannel* GetNetChannel() const { return m_NetChannel; }

	virtual void SetReliable(bool state) { m_bReliable = state; };
	virtual bool IsReliable() const { return m_bReliable; };
	virtual void SetNetChannel(INetChannel* netchan) { m_NetChannel = netchan; }
	virtual bool Process() { return false; }; // no handler set

protected:
	bool m_bReliable;          // true if message should be send reliable
	INetChannel* m_NetChannel; // netchannel this message is from/for
	byte pad0[8];
};

class CLC_Move : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read& buffer);
	bool WriteToBuffer(bf_write& buffer);
	const char* ToString() const;
	int GetType() const { return clc_Move; }
	const char* GetName() const { return "clc_Move"; }
	void* m_pMessageHandler;
	int GetGroup() const { return MOVE; }

	CLC_Move() { m_bReliable = false; }

public:
	int m_nBackupCommands;
	int m_nNewCommands;
	int m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class NET_SetConVar : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read& buffer);
	bool WriteToBuffer(bf_write& buffer);
	const char* ToString() const;
	int GetType() const { return net_SetConVar; }
	const char* GetName() const { return "net_SetConVar"; }
	void* m_pMessageHandler;
	int GetGroup() const { return STRINGCMD; }

	NET_SetConVar() {}
	NET_SetConVar(const char* name, const char* value)
	{
		CVar_t cvar;
		strncpy_s(cvar.Name, name, MAX_OSPATH);
		strncpy_s(cvar.Value, value, MAX_OSPATH);
		ConVar = cvar;
	}

public:
	typedef struct CVar_s
	{
		char Name[MAX_OSPATH];
		char Value[MAX_OSPATH];
	} CVar_t;
	CVar_t ConVar;
};

class NET_SignonState : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read& buffer);
	bool WriteToBuffer(bf_write& buffer);
	const char* ToString() const;
	int GetType() const { return net_SignonState; }
	const char* GetName() const { return "net_SignonState"; }
	void* m_pMessageHandler;

	int	GetGroup() const { return SIGNON; }

	NET_SignonState() {};
	NET_SignonState(int state, int spawncount) { m_nSignonState = state; m_nSpawnCount = spawncount; };

public:
	int m_nSignonState;			// See SIGNONSTATE_ defines
	int m_nSpawnCount;			// server spawn count (session number)
};

class NET_Tick : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read& buffer);
	bool WriteToBuffer(bf_write& buffer);
	const char* ToString() const;
	int GetType() const { return net_Tick; }
	const char* GetName() const { return "net_Tick"; }
	void* m_pMessageHandler;

	NET_Tick()
	{
		m_bReliable = false;
		m_flHostFrameTime = 0;
		m_flHostFrameTimeStdDeviation = 0;
	};
	NET_Tick(int tick, float hostFrametime, float hostFrametime_stddeviation)
	{
		m_bReliable = false;
		m_nTick = tick;
		m_flHostFrameTime = hostFrametime;
		m_flHostFrameTimeStdDeviation = hostFrametime_stddeviation;
	};

public:
	int			m_nTick;
	float		m_flHostFrameTime;
	float		m_flHostFrameTimeStdDeviation;
};

class CLC_VoiceData : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read& buffer);
	bool WriteToBuffer(bf_write& buffer);
	const char* ToString() const;
	int GetType() const { return clc_VoiceData; }
	const char* GetName() const { return "clc_VoiceData"; }
	void* m_pMessageHandler;

	int GetGroup() const { return VOICE; }

	CLC_VoiceData() { m_bReliable = false; }

public:
	int m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
	uint64_t m_xuid;
};

class CLC_RespondCvarValue : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read& buffer);
	bool WriteToBuffer(bf_write& buffer);
	const char* ToString() const;
	int GetType() const { return clc_RespondCvarValue; }
	const char* GetName() const { return "clc_RespondCvarValue"; }
	void* m_pMessageHandler;

	int m_iCookie;

	const char* m_szCvarName;
	const char* m_szCvarValue;	// The sender sets this, and it automatically points it at m_szCvarNameBuffer when receiving.

	int	m_eStatusCode;

private:
	char m_szCvarNameBuffer[256];
	char m_szCvarValueBuffer[256];
};

class SVC_FixAngle : public CNetMessage
{
	SVC_FixAngle() { m_bReliable = false; };
	SVC_FixAngle(bool bRelative, QAngle angle) { m_bReliable = false; m_bRelative = bRelative; m_Angle = angle; }

public:
	bool			m_bRelative;
	QAngle			m_Angle;
};

class CLC_BaselineAck : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read& buffer);
	bool WriteToBuffer(bf_write& buffer);
	const char* ToString() const;
	int GetType() const { return clc_BaselineAck; }
	const char* GetName() const { return "clc_BaselineAck"; }
	void* m_pMessageHandler;

	CLC_BaselineAck() {};
	CLC_BaselineAck(int tick, int baseline) { m_nBaselineTick = tick; m_nBaselineNr = baseline; }

	int	GetGroup() const { return ENTITIES; }

public:
	int		m_nBaselineTick;	// sequence number of baseline
	int		m_nBaselineNr;		// 0 or 1 		
};