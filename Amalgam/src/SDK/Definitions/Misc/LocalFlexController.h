#pragma once

#pragma warning (disable : 26812)

enum LocalFlexController_t
{
	DUMMY_FLEX_CONTROLLER = 0x7fffffff
};

inline LocalFlexController_t& operator++(LocalFlexController_t& a) { return a = LocalFlexController_t(int(a) + 1); }
inline LocalFlexController_t& operator--(LocalFlexController_t& a) { return a = LocalFlexController_t(int(a) - 1); }
inline LocalFlexController_t  operator++(LocalFlexController_t& a, int) { LocalFlexController_t t = a; a = LocalFlexController_t(int(a) + 1); return t; }
inline LocalFlexController_t  operator--(LocalFlexController_t& a, int) { LocalFlexController_t t = a; a = LocalFlexController_t(int(a) - 1); return t; }