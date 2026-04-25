#pragma once
#include "../Macros/Macros.h"
#include "../../SDK/Definitions/Types.h"

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
	void StoreKey(byte iKey, KeyStorage* pStorage = nullptr);

	bool Down(byte iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);
	bool Pressed(byte iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);
	bool Double(byte iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);
	bool Released(byte iKey, const bool bStore = false, KeyStorage* pStorage = nullptr);

	std::array<KeyStorage, 256> m_aKeyStorage = {};

public:
	std::string& String(byte iKey, const bool bStore = false);

	std::array<std::string, 256> m_aStringStorage = {};
};

ADD_FEATURE_CUSTOM(CKeyHandler, KeyHandler, U);