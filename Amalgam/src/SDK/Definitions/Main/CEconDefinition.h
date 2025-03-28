#pragma once
#include "KeyValues.h"
#include "../Steam/SteamClientPublic.h"

class CEconNotification
{
public:
	virtual ~CEconNotification() = 0;
	virtual void SetLifetime(float flSeconds) = 0;
	virtual float GetExpireTime() const = 0;
	virtual float GetInGameLifeTime() const = 0;
	virtual bool BShowInGameElements() const = 0;
	virtual void MarkForDeletion() = 0;

	enum EType
	{
		// Can only be deleted
		eType_Basic,
		// Can be accept or declined
		eType_AcceptDecline,
		// Can be triggered or deleted
		eType_Trigger,
		// Can only be triggered
		eType_MustTrigger,
	};

	virtual EType NotificationType() = 0;
	virtual bool BHighPriority() = 0;
	virtual void Trigger() = 0;
	virtual void Accept() = 0;
	virtual void Decline() = 0;
	virtual void Deleted() = 0;
	virtual void Expired() = 0;
	virtual void UpdateTick() = 0;
	virtual const char* GetUnlocalizedHelpText() = 0;
	virtual void* CreateUIElement(bool bMainMenu) const = 0;

	const char* m_pText;
	const char* m_pSoundFilename;
	float m_flExpireTime;
	KeyValues* m_pKeyValues;
	wchar_t m_wszBuffer[1024];
	CSteamID m_steamID;

private:
	friend class CEconNotificationQueue;
	int m_iID;
	bool m_bInUse;
};