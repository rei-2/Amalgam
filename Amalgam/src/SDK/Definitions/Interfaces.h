#pragma once

#include "../../Utils/Macros/Macros.h"

#include "Interfaces/CClientModeShared.h"
#include "Interfaces/CClientState.h"
#include "Interfaces/CGlobalVarsBase.h"
#include "Interfaces/CHLClient.h"
#include "Interfaces/CTFGameRules.h"
#include "Interfaces/CTFGCClientSystem.h"
#include "Interfaces/CTFPartyClient.h"
#include "Interfaces/IClientEntityList.h"
#include "Interfaces/ICVar.h"
#include "Interfaces/IEngineTrace.h"
#include "Interfaces/IEngineVGui.h"
#include "Interfaces/IGameEvents.h"
#include "Interfaces/IGameMovement.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IInputSystem.h"
#include "Interfaces/IKeyValuesSystem.h"
#include "Interfaces/IMaterialSystem.h"
#include "Interfaces/IMatSystemSurface.h"
#include "Interfaces/IMoveHelper.h"
#include "Interfaces/IPanel.h"
#include "Interfaces/IStudioRender.h"
#include "Interfaces/IUniformRandomStream.h"
#include "Interfaces/IVEngineClient.h"
#include "Interfaces/IViewRender.h"
#include "Interfaces/IVModelInfo.h"
#include "Interfaces/IVModelRender.h"
#include "Interfaces/IVRenderView.h"
#include "Interfaces/Prediction.h"
#include "Interfaces/SteamInterfaces.h"
#include "Interfaces/ViewRenderBeams.h"
#include "Interfaces/VPhysics.h"

#include <d3d9.h>
MAKE_INTERFACE_SIGNATURE(IDirect3DDevice9, DirectXDevice, "shaderapidx9.dll, shaderapivk.dll", "48 8B 0D ? ? ? ? 48 8B 01 FF 50 ? 8B F8", 0x0, 1)

class CNullInterfaces
{
private:
	bool m_bFailed = false;

public:
	bool Initialize();
};

ADD_FEATURE_CUSTOM(CNullInterfaces, Interfaces, H);