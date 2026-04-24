#pragma once
#include "../Interfaces/Interface.h"

enum InitReturnVal_t
{
	INIT_FAILED = 0,
	INIT_OK,
	INIT_LAST_VAL,
};

class IAppSystem
{
public:
	virtual bool Connect(CreateInterfaceFn factory) = 0;
	virtual void Disconnect() = 0;
	virtual void* QueryInterface(const char* pInterfaceName) = 0;
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;
};

template <class IInterface>
class CBaseAppSystem : public IInterface
{
public:
	virtual bool Connect(CreateInterfaceFn factory) = 0;
	virtual void Disconnect() = 0;
	virtual void* QueryInterface(const char* pInterfaceName) = 0;
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;
};

template <class IInterface>
class CTier0AppSystem : public CBaseAppSystem<IInterface>
{
public:
	bool m_bIsPrimaryAppSystem;
};

template <class IInterface, int ConVarFlag = 0>
class CTier1AppSystem : public CTier0AppSystem<IInterface>
{
	typedef CTier0AppSystem<IInterface> BaseClass;
};

template <class IInterface, int ConVarFlag = 0>
class CTier2AppSystem : public CTier1AppSystem<IInterface, ConVarFlag>
{
	typedef CTier1AppSystem<IInterface, ConVarFlag> BaseClass;
};

template<class IInterface, int ConVarFlag = 0>
class CTier3AppSystem : public CTier2AppSystem<IInterface, ConVarFlag>
{
	typedef CTier2AppSystem<IInterface, ConVarFlag> BaseClass;
};