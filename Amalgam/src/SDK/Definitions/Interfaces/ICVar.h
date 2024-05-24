#pragma once
#include "../Misc/IAppSystem.h"
#include "../Misc/ConVar.h"
#include "../Types.h"

class ConCommandBase;
class ConCommand;

typedef int CVarDLLIdentifier_t;

class IConsoleDisplayFunc
{
public:
	virtual void ColorPrint(const Color_t& clr, const char* pMessage) = 0;
	virtual void Print(const char* pMessage) = 0;
	virtual void DPrint(const char* pMessage) = 0;
};

class ICvarQuery : public IAppSystem
{
public:
	virtual bool AreConVarsLinkable(const ConVar* child, const ConVar* parent) = 0;
};

class ICvar : public IAppSystem
{
public:
	virtual CVarDLLIdentifier_t AllocateDLLIdentifier() = 0;
	virtual void RegisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void UnregisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void UnregisterConCommands(CVarDLLIdentifier_t id) = 0;
	virtual const char* GetCommandLineValue(const char* pVariableName) = 0;
	virtual ConCommandBase* FindCommandBase(const char* name) = 0;
	virtual const ConCommandBase* FindCommandBase(const char* name) const = 0;
	virtual ConVar* FindVar(const char* var_name) = 0;
	virtual const ConVar* FindVar(const char* var_name) const = 0;
	virtual ConCommand* FindCommand(const char* name) = 0;
	virtual const ConCommand* FindCommand(const char* name) const = 0;
	virtual ConCommandBase* GetCommands(void) = 0;
	virtual const ConCommandBase* GetCommands(void) const = 0;
	virtual void InstallGlobalChangeCallback(FnChangeCallback_t callback) = 0;
	virtual void RemoveGlobalChangeCallback(FnChangeCallback_t callback) = 0;
	virtual void CallGlobalChangeCallbacks(ConVar* var, const char* pOldString, float flOldValue) = 0;
	virtual void InstallConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void RemoveConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
	virtual void ConsoleColorPrintf(const Color_t& clr, const char* pFormat, ...) const = 0;
	virtual void ConsolePrintf(const char* pFormat, ...) const = 0;
	virtual void ConsoleDPrintf(const char* pFormat, ...) const = 0;
	virtual void RevertFlaggedConVars(int nFlag) = 0;
	virtual void InstallCVarQuery(ICvarQuery* pQuery) = 0;
	virtual bool IsMaterialThreadSetAllowed() const = 0;
	virtual void QueueMaterialThreadSetValue(ConVar* pConVar, const char* pValue) = 0;
	virtual void QueueMaterialThreadSetValue(ConVar* pConVar, int nValue) = 0;
	virtual void QueueMaterialThreadSetValue(ConVar* pConVar, float flValue) = 0;
	virtual bool HasQueuedMaterialThreadConVarSets() const = 0;
	virtual int ProcessQueuedMaterialThreadConVarSets() = 0;

	class ICVarIteratorInternal
	{
	public:
		virtual ~ICVarIteratorInternal() {}
		virtual void SetFirst(void) = 0;
		virtual void Next(void) = 0;
		virtual	bool IsValid(void) = 0;
		virtual ConCommandBase* Get(void) = 0;
	};

	class Iterator
	{
	public:
		inline Iterator(ICvar* icvar);
		inline ~Iterator(void);
		inline void SetFirst(void);
		inline void Next(void);
		inline bool IsValid(void);
		inline ConCommandBase* Get(void);
	private:
		ICVarIteratorInternal* m_pIter;
	};

	virtual ICVarIteratorInternal* FactoryInternalIterator(void) = 0;
};

inline ICvar::Iterator::Iterator(ICvar* icvar)
{
	m_pIter = icvar->FactoryInternalIterator();
}

inline ICvar::Iterator::~Iterator(void)
{
	//delete m_pIter;
}

inline void ICvar::Iterator::SetFirst(void)
{
	m_pIter->SetFirst();
}

inline void ICvar::Iterator::Next(void)
{
	m_pIter->Next();
}

inline bool ICvar::Iterator::IsValid(void)
{
	return m_pIter->IsValid();
}

inline ConCommandBase* ICvar::Iterator::Get(void)
{
	return m_pIter->Get();
}

MAKE_INTERFACE_VERSION(ICvar, CVar, "vstdlib.dll", "VEngineCvar004");