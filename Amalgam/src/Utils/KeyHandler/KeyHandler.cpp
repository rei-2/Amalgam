#include "KeyHandler.h"

#include "../../SDK/SDK.h"
#include <Windows.h>

void CKeyHandler::StoreKey(int iKey, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_mKeyStorage[iKey];

	// down
	const bool bDown = iKey && GetAsyncKeyState(iKey) & 0x8000 && SDK::IsGameWindowInFocus();

	// pressed
	const bool bPressed = bDown && !pStorage->m_bIsDown;

	// double click
	const double dFloatTime = SDK::PlatFloatTime();
	const bool bDouble = bPressed && dFloatTime < pStorage->m_dPressTime + 0.25;

	// released
	const bool bReleased = !bDown && pStorage->m_bIsDown;

	pStorage->m_bIsDown = bDown;
	pStorage->m_bIsPressed = bPressed;
	pStorage->m_bIsDouble = bDouble;
	pStorage->m_bIsReleased = bReleased;
	if (bPressed)
		pStorage->m_dPressTime = dFloatTime;
}

bool CKeyHandler::Down(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_mKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsDown;
}

bool CKeyHandler::Pressed(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_mKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsPressed;
}

bool CKeyHandler::Double(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_mKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsDouble;
}

bool CKeyHandler::Released(int iKey, const bool bStore, KeyStorage* pStorage)
{
	if (!pStorage)
		pStorage = &m_mKeyStorage[iKey];

	if (bStore)
		StoreKey(iKey, pStorage);

	return pStorage->m_bIsReleased;
}