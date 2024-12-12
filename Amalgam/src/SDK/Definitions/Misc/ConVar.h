#pragma once
#include "IConVar.h"
#include "../Main/UtlVector.h"

class ConVar;
class CCommand;
class ConCommand;
class ConCommandBase;
struct characterset_t;

class IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(ConCommandBase* pVar) = 0;
};

typedef void (*FnCommandCallbackVoid_t)(void);
typedef void (*FnCommandCallback_t)(const CCommand& command);

#define COMMAND_COMPLETION_MAXITEMS 64
#define COMMAND_COMPLETION_ITEM_LENGTH 64

typedef int  (*FnCommandCompletionCallback)(const char* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

class ICommandCallback
{
public:
	virtual void CommandCallback(const CCommand& command) = 0;
};

class ICommandCompletionCallback
{
public:
	virtual int CommandCompletionCallback(const char* pPartial, CUtlVector< /*CUtlString*/ void* >& commands) = 0;
};

class ConCommandBase
{
public:
	virtual ~ConCommandBase(void) = 0;
	virtual	bool IsCommand(void) const = 0;
	virtual bool IsFlagSet(int flag) const = 0;
	virtual void AddFlags(int flags) = 0;
	virtual const char* GetName(void) const = 0;
	virtual const char* GetHelpText(void) const = 0;
	virtual bool IsRegistered(void) const = 0;
	virtual void* GetDLLIdentifier() const = 0;
	virtual void CreateBase(const char* pName, const char* pHelpString = 0, int flags = 0) = 0;
	virtual void Init() = 0;

public:
	ConCommandBase* m_pNext;
	bool m_bRegistered;
	const char* m_pszName;
	const char* m_pszHelpString;
	int m_nFlags;

protected:
	static ConCommandBase* s_pConCommandBases;
	static IConCommandBaseAccessor* s_pAccessor;
};

enum
{
	COMMAND_MAX_ARGC = 64,
	COMMAND_MAX_LENGTH = 512
};

class CCommand
{
public:
	int ArgC() const;
	const char** ArgV() const;
	const char* ArgS() const;
	const char* GetCommandString() const;
	const char* operator[](int nIndex) const;
	const char* Arg(int nIndex) const;

	int m_nArgc;
	int m_nArgv0Size;
	char m_pArgSBuffer[COMMAND_MAX_LENGTH];
	char m_pArgvBuffer[COMMAND_MAX_LENGTH];
	const char* m_ppArgv[COMMAND_MAX_ARGC];
};
inline int CCommand::ArgC() const
{
	return m_nArgc;
}
inline const char** CCommand::ArgV() const
{
	return m_nArgc ? (const char**)m_ppArgv : NULL;
}
inline const char* CCommand::ArgS() const
{
	return m_nArgv0Size ? &m_pArgSBuffer[m_nArgv0Size] : "";
}
inline const char* CCommand::GetCommandString() const
{
	return m_nArgc ? m_pArgSBuffer : "";
}
inline const char* CCommand::operator[](int nIndex) const
{
	return Arg(nIndex);
}
inline const char* CCommand::Arg(int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_nArgc)
		return "";
	return m_ppArgv[nIndex];
}

class ConCommand : public ConCommandBase
{
public:
	virtual ~ConCommand(void) = 0;
	virtual	bool IsCommand(void) const = 0;
	virtual int AutoCompleteSuggest(const char* partial, CUtlVector< /*CUtlString*/ void* >& commands) = 0;
	virtual bool CanAutoComplete(void) = 0;
	virtual void Dispatch(const CCommand& command) = 0;

private:
	union
	{
		FnCommandCallbackVoid_t m_fnCommandCallbackV1;
		FnCommandCallback_t m_fnCommandCallback;
		ICommandCallback* m_pCommandCallback;
	};

	union
	{
		FnCommandCompletionCallback	m_fnCompletionCallback;
		ICommandCompletionCallback* m_pCommandCompletionCallback;
	};

	bool m_bHasCompletionCallback : 1;
	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;
};

class ConVar : public ConCommandBase, public IConVar
{
public:
	virtual ~ConVar(void) {};
	virtual bool IsFlagSet(int flag) const = 0;
	virtual const char* GetHelpText(void) const = 0;
	virtual bool IsRegistered(void) const = 0;
	virtual const char* GetName(void) const = 0;
	virtual void AddFlags(int flags) = 0;
	virtual	bool IsCommand(void) const = 0;

	inline float GetFloat(void) const;
	inline int GetInt(void) const;
	inline bool GetBool() const { return !!GetInt(); }
	inline char const* GetString(void) const;

	virtual void SetValue(const char* value) = 0;
	virtual void SetValue(float value) = 0;
	virtual void SetValue(int value) = 0;

private:
	virtual void InternalSetValue(const char* value) = 0;
	virtual void InternalSetFloatValue(float fNewValue, bool bForce = false) = 0;
	virtual void InternalSetIntValue(int nValue) = 0;
	virtual bool ClampValue(float& value) = 0;
	virtual void ChangeStringValue(const char* tempVal, float flOldValue) = 0;
	virtual void Init() = 0;

	int GetFlags() { return m_pParent->m_nFlags; }

public:
	ConVar* m_pParent;
	const char* m_pszDefaultValue;
	char* m_pszString;
	int m_StringLength;
	float m_fValue;
	int m_nValue;
	bool m_bHasMin;
	float m_fMinVal;
	bool m_bHasMax;
	float m_fMaxVal;
	bool m_bHasCompMin;
	float m_fCompMinVal;
	bool m_bHasCompMax;
	float m_fCompMaxVal;
	bool m_bCompetitiveRestrictions;
	FnChangeCallback_t m_fnChangeCallback;
};

inline float ConVar::GetFloat(void) const
{
	return m_pParent->m_fValue;
}

inline int ConVar::GetInt(void) const
{
	return m_pParent->m_nValue;
}

inline const char* ConVar::GetString(void) const
{
	if (m_nFlags & FCVAR_NEVER_AS_STRING)
		return "FCVAR_NEVER_AS_STRING";

	return (m_pParent->m_pszString) ? m_pParent->m_pszString : "";
}