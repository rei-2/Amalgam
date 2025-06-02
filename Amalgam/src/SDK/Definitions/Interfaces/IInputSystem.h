#pragma once
#include "../Misc/IAppSystem.h"
#include "../Misc/AnalogCode.h"
#include "../Misc/ButtonCode.h"
#include "../Misc/InputEnums.h"
#include "../Steam/ISteamController.h"

class IInputSystem : public IAppSystem
{
public:
	virtual void AttachToWindow(void* hWnd) = 0;
	virtual void DetachFromWindow() = 0;
	virtual void EnableInput(bool bEnable) = 0;
	virtual void EnableMessagePump(bool bEnable) = 0;
	virtual void PollInputState() = 0;
	virtual int GetPollTick() const = 0;
	virtual bool IsButtonDown(ButtonCode_t code) const = 0;
	virtual int GetButtonPressedTick(ButtonCode_t code) const = 0;
	virtual int GetButtonReleasedTick(ButtonCode_t code) const = 0;
	virtual int GetAnalogValue(AnalogCode_t code) const = 0;
	virtual int GetAnalogDelta(AnalogCode_t code) const = 0;
	virtual int GetEventCount() const = 0;
	virtual const InputEvent_t* GetEventData() const = 0;
	virtual void PostUserEvent(const InputEvent_t& event) = 0;
	virtual int GetJoystickCount() const = 0;
	virtual void EnableJoystickInput(int nJoystick, bool bEnable) = 0;
	virtual void EnableJoystickDiagonalPOV(int nJoystick, bool bEnable) = 0;
	virtual void SampleDevices(void) = 0;
	virtual void SetRumble(float fLeftMotor, float fRightMotor, int userId) = 0;
	virtual void StopRumble(void) = 0;
	virtual void ResetInputState() = 0;
	virtual void SetPrimaryUserId(int userId) = 0;
	virtual const char* ButtonCodeToString(ButtonCode_t code) const = 0;
	virtual const char* AnalogCodeToString(AnalogCode_t code) const = 0;
	virtual ButtonCode_t StringToButtonCode(const char* pString) const = 0;
	virtual AnalogCode_t StringToAnalogCode(const char* pString) const = 0;
	virtual void SleepUntilInput(int nMaxSleepTimeMS = -1) = 0;
	virtual ButtonCode_t VirtualKeyToButtonCode(int nVirtualKey) const = 0;
	virtual int ButtonCodeToVirtualKey(ButtonCode_t code) const = 0;
	virtual ButtonCode_t ScanCodeToButtonCode(int lParam) const = 0;
	virtual int GetPollCount() const = 0;
	virtual void SetCursorPosition(int x, int y) = 0;
	virtual void* GetHapticsInterfaceAddress() const = 0;
	virtual void SetNovintPure(bool bPure) = 0;
	virtual bool GetRawMouseAccumulators(int& accumX, int& accumY) = 0;
	virtual void SetConsoleTextMode(bool bConsoleTextMode) = 0;
	virtual ISteamController* SteamControllerInterface() = 0;
	virtual uint32 GetNumSteamControllersConnected() = 0;
	virtual bool IsSteamControllerActive() = 0;
	virtual bool IsSteamControllerConnected() = 0;
	virtual int GetSteamControllerIndexForSlot(int nSlot) = 0;
	virtual bool GetRadialMenuStickValues(int nSlot, float& fX, float& fY) = 0;
	virtual void ActivateSteamControllerActionSetForSlot(uint64 nSlot, GameActionSet_t eActionSet) = 0;
	virtual ControllerActionSetHandle_t GetActionSetHandle(GameActionSet_t eActionSet) = 0;
	virtual ControllerActionSetHandle_t GetActionSetHandle(const char* szActionSet) = 0;
	virtual EControllerActionOrigin GetSteamControllerActionOrigin(const char* action, GameActionSet_t action_set) = 0;
	virtual EControllerActionOrigin GetSteamControllerActionOrigin(const char* action, ControllerActionSetHandle_t action_set_handle) = 0;
	virtual const wchar_t* GetSteamControllerFontCharacterForActionOrigin(EControllerActionOrigin origin) = 0;
	virtual const wchar_t* GetSteamControllerDescriptionForActionOrigin(EControllerActionOrigin origin) = 0;
	virtual void SetSkipControllerInitialization(bool bSkip) = 0;
};

MAKE_INTERFACE_VERSION(IInputSystem, InputSystem, "inputsystem.dll", "InputSystemVersion001");