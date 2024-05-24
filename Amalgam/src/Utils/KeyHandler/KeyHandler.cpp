#include "KeyHandler.h"

#include "../../SDK/SDK.h"
#include <Windows.h>
#include <chrono>

void CKeyHandler::StoreKey(int iKey, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &StorageMap[iKey];

	// down
	const bool bDown = iKey && GetAsyncKeyState(iKey) & 0x8000 && SDK::IsGameWindowInFocus();

	// pressed
	const bool bPressed = bDown && !pStorage->bIsDown;

	// double click
	const auto iEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	const bool bDouble = bPressed && iEpoch < pStorage->iPressTime + 250;

	// released
	const bool bReleased = !bDown && pStorage->bIsDown;

	pStorage->bIsDown = bDown;
	pStorage->bIsPressed = bPressed;
	pStorage->bIsDouble = bDouble;
	pStorage->bIsReleased = bReleased;
	if (bPressed)
		pStorage->iPressTime = iEpoch;
}

bool CKeyHandler::Down(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &StorageMap[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->bIsDown;
}

bool CKeyHandler::Pressed(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &StorageMap[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->bIsPressed;
}

bool CKeyHandler::Double(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &StorageMap[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->bIsDouble;
}

bool CKeyHandler::Released(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &StorageMap[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->bIsReleased;
}