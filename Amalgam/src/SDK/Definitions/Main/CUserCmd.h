#pragma once
#include "../Types.h"

class CUserCmd
{
public:
	virtual ~CUserCmd() {};
	int	command_number = 0;
	int	tick_count = 0;
	QAngle viewangles = {};
	float forwardmove = 0.0f;
	float sidemove = 0.0f;
	float upmove = 0.0f;
	int buttons = 0;
	unsigned char impulse = 0;
	int weaponselect = 0;
	int weaponsubtype = 0;
	int random_seed = 0;
	short mousedx = 0;
	short mousedy = 0;
	bool hasbeenpredicted = false;

	CUserCmd() {};

	CUserCmd(const CUserCmd& other)
	{
		command_number = other.command_number;
		tick_count = other.tick_count;
		viewangles = other.viewangles;
		forwardmove = other.forwardmove;
		sidemove = other.sidemove;
		upmove = other.upmove;
		buttons = other.buttons;
		impulse = other.impulse;
		weaponselect = other.weaponselect;
		weaponsubtype = other.weaponsubtype;
		random_seed = other.random_seed;
		mousedx = other.mousedx;
		mousedy = other.mousedy;
		hasbeenpredicted = other.hasbeenpredicted;
	}
};

#define IN_ATTACK (1 << 0)
#define IN_JUMP (1 << 1)
#define IN_DUCK (1 << 2)
#define IN_FORWARD (1 << 3)
#define IN_BACK (1 << 4)
#define IN_USE (1 << 5)
#define IN_CANCEL (1 << 6)
#define IN_LEFT (1 << 7)
#define IN_RIGHT (1 << 8)
#define IN_MOVELEFT (1 << 9)
#define IN_MOVERIGHT (1 << 10)
#define IN_ATTACK2 (1 << 11)
#define IN_RUN (1 << 12)
#define IN_RELOAD (1 << 13)
#define IN_ALT1 (1 << 14)
#define IN_ALT2 (1 << 15)
#define IN_SCORE (1 << 16)
#define IN_SPEED (1 << 17)
#define IN_WALK (1 << 18)
#define IN_ZOOM (1 << 19)
#define IN_WEAPON1 (1 << 20)
#define IN_WEAPON2 (1 << 21)
#define IN_BULLRUSH (1 << 22)
#define IN_GRENADE1 (1 << 23)
#define IN_GRENADE2 (1 << 24)
#define	IN_ATTACK3 (1 << 25)