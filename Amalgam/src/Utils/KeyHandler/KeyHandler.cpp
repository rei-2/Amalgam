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
	const double dFloatTime = SDK::PlatFloatTime();
	const bool bDouble = bPressed && dFloatTime < pStorage->dPressTime + 0.25;

	// released
	const bool bReleased = !bDown && pStorage->bIsDown;

	pStorage->bIsDown = bDown;
	pStorage->bIsPressed = bPressed;
	pStorage->bIsDouble = bDouble;
	pStorage->bIsReleased = bReleased;
	if (bPressed)
		pStorage->dPressTime = dFloatTime;
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