#pragma once

class IConVar;
class CCommand;

#define FCVAR_NONE 0 
#define FCVAR_UNREGISTERED (1<<0)
#define FCVAR_DEVELOPMENTONLY (1<<1)
#define FCVAR_GAMEDLL (1<<2)
#define FCVAR_CLIENTDLL (1<<3)
#define FCVAR_HIDDEN (1<<4)
#define FCVAR_PROTECTED (1<<5)
#define FCVAR_SPONLY (1<<6)
#define	FCVAR_ARCHIVE (1<<7)
#define	FCVAR_NOTIFY (1<<8)
#define	FCVAR_USERINFO (1<<9)
#define FCVAR_CHEAT (1<<14)
#define FCVAR_PRINTABLEONLY (1<<10)
#define FCVAR_UNLOGGED (1<<11)
#define FCVAR_NEVER_AS_STRING (1<<12)
#define FCVAR_REPLICATED (1<<13)
#define FCVAR_DEMO (1<<16)
#define FCVAR_DONTRECORD (1<<17) 
#define FCVAR_RELOAD_MATERIALS (1<<20)
#define FCVAR_RELOAD_TEXTURES (1<<21)
#define FCVAR_NOT_CONNECTED (1<<22)
#define FCVAR_MATERIAL_SYSTEM_THREAD (1<<23)
#define FCVAR_ARCHIVE_XBOX (1<<24)
#define FCVAR_ACCESSIBLE_FROM_THREADS (1<<25)
#define FCVAR_SERVER_CAN_EXECUTE (1<<28)
#define FCVAR_SERVER_CANNOT_QUERY (1<<29)
#define FCVAR_CLIENTCMD_CAN_EXECUTE (1<<30)
#define FCVAR_EXEC_DESPITE_DEFAULT (1<<31)
#define FCVAR_INTERNAL_USE (1<<15)
#define FCVAR_ALLOWED_IN_COMPETITIVE (1<<18)
#define FCVAR_MATERIAL_THREAD_MASK ( FCVAR_RELOAD_MATERIALS | FCVAR_RELOAD_TEXTURES | FCVAR_MATERIAL_SYSTEM_THREAD )	

typedef void (*FnChangeCallback_t)(IConVar* var, const char* pOldValue, float flOldValue);

class IConVar
{
public:
	virtual void SetValue(const char* pValue) = 0;
	virtual void SetValue(float flValue) = 0;
	virtual void SetValue(int nValue) = 0;
	virtual const char* GetName(void) const = 0;
	virtual bool IsFlagSet(int nFlag) const = 0;
};