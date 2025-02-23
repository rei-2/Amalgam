#pragma once
#include "../Feature/Feature.h"
#include <unordered_map>

struct KeyStorage
{
	bool m_bIsDown = false;
	bool m_bIsPressed = false;
	bool m_bIsDouble = false;
	bool m_bIsReleased = false;
	double m_dPressTime = 0;
};

class CKeyHandler
{
public:
	void StoreKey(int iKey, KeyStorage* pStorage = nullptr);

	// Is the button currently down?
	bool Down(int iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);

	// Was the button just pressed? This will only be true once.
	bool Pressed(int iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);

	// Was the button double clicked? This will only be true once.
	bool Double(int iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);

	// Was the button just released? This will only be true once.
	bool Released(int iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);

	std::unordered_map<int, KeyStorage> m_mKeyStorage;
};

ADD_FEATURE_CUSTOM(CKeyHandler, KeyHandler, U);