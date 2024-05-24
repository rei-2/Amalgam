#pragma once
#include "../Feature/Feature.h"
#include <unordered_map>

struct KeyStorage
{
	bool bIsDown = false;
	bool bIsPressed = false;
	bool bIsDouble = false;
	bool bIsReleased = false;
	long long iPressTime = 0;
};

class CKeyHandler
{
	std::unordered_map<int, KeyStorage> StorageMap;

public:
	void StoreKey(int iKey, KeyStorage* pStorage = nullptr);

	// Is the button currently down?
	bool Down(int iKey, const bool bStore = true, KeyStorage* pStorage = nullptr);

	// Was the button just pressed? This will only be true once.
	bool Pressed(int iKey, const bool bStore = true, KeyStorage* pStorage = nullptr);

	// Was the button double clicked? This will only be true once.
	bool Double(int iKey, const bool bStore = true, KeyStorage* pStorage = nullptr);

	// Was the button just released? This will only be true once.
	bool Released(int iKey, const bool bStore = true, KeyStorage* pStorage = nullptr);
};

ADD_FEATURE_CUSTOM(CKeyHandler, KeyHandler, U);