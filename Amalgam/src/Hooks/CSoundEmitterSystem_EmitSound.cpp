#include "../SDK/SDK.h"

#include <boost/algorithm/string.hpp>

MAKE_SIGNATURE(CSoundEmitterSystem_EmitSound, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 41 56 48 81 EC ? ? ? ? 49 8B D9", 0x0);
//MAKE_SIGNATURE(S_StartDynamicSound, "engine.dll", "4C 8B DC 57 48 81 EC", 0x0);
MAKE_SIGNATURE(S_StartSound, "engine.dll", "40 53 48 83 EC ? 48 83 79 ? ? 48 8B D9 75 ? 33 C0", 0x0);

class IRecipientFilter
{
public:
	virtual			~IRecipientFilter() {}

	virtual bool	IsReliable(void) const = 0;
	virtual bool	IsInitMessage(void) const = 0;

	virtual int		GetRecipientCount(void) const = 0;
	virtual int		GetRecipientIndex(int slot) const = 0;
};

struct CSoundParameters
{
	CSoundParameters()
	{
		channel = 0; // 0
		volume = 1.0f;  // 1.0f
		pitch = 100; // 100

		pitchlow = 100;
		pitchhigh = 100;

		soundlevel = SNDLVL_NORM; // 75dB
		soundname[0] = 0;
		play_to_owner_only = false;
		count = 0;

		delay_msec = 0;
	}

	int				channel;
	float			volume;
	int				pitch;
	int				pitchlow, pitchhigh;
	soundlevel_t	soundlevel;
	// For weapon sounds...
	bool			play_to_owner_only;
	int				count;
	char 			soundname[128];
	int				delay_msec;
};

struct EmitSound_t
{
	EmitSound_t() :
		m_nChannel(0),
		m_pSoundName(0),
		m_flVolume(1.0f),
		m_SoundLevel(0),
		m_nFlags(0),
		m_nPitch(100),
		m_nSpecialDSP(0),
		m_pOrigin(0),
		m_flSoundTime(0.0f),
		m_pflSoundDuration(0),
		m_bEmitCloseCaption(true),
		m_bWarnOnMissingCloseCaption(false),
		m_bWarnOnDirectWaveReference(false),
		m_nSpeakerEntity(-1),
		m_UtlVecSoundOrigin(),
		m_hSoundScriptHandle(-1)
	{
	}

	EmitSound_t(const CSoundParameters& src);

	int							m_nChannel;
	char const* m_pSoundName;
	float						m_flVolume;
	int				m_SoundLevel;
	int							m_nFlags;
	int							m_nPitch;
	int							m_nSpecialDSP;
	const Vector* m_pOrigin;
	float						m_flSoundTime; ///< NOT DURATION, but rather, some absolute time in the future until which this sound should be delayed
	float* m_pflSoundDuration;
	bool						m_bEmitCloseCaption;
	bool						m_bWarnOnMissingCloseCaption;
	bool						m_bWarnOnDirectWaveReference;
	int							m_nSpeakerEntity;
	mutable CUtlVector< Vector >	m_UtlVecSoundOrigin;  ///< Actual sound origin(s) (can be multiple if sound routed through speaker entity(ies) )
	mutable short		m_hSoundScriptHandle;
};

const static std::vector<const char*> vNoisemaker = { "items\\halloween", "items\\football_manager", "items\\japan_fundraiser", "items\\samurai\\tf_samurai_noisemaker", "items\\summer", "misc\\happy_birthday_tf", "misc\\jingle_bells" };

static bool ShouldBlockSound(const char* pSound)
{
	if (!Vars::Misc::Sound::Block.Value || !pSound)
		return false;

	std::string sSound = pSound;
	boost::algorithm::to_lower(sSound);

	if (Vars::Misc::Sound::Block.Value & Vars::Misc::Sound::BlockEnum::Footsteps && sSound.find("footsteps") != std::string::npos) // Footsteps
		return true;

	if (Vars::Misc::Sound::Block.Value & Vars::Misc::Sound::BlockEnum::Noisemaker) // Noisemaker
	{
		for (auto& sNoise : vNoisemaker)
		{
			if (sSound.find(sNoise) != std::string::npos)
				return true;
		}
	}

	if (Vars::Misc::Sound::Block.Value & Vars::Misc::Sound::BlockEnum::FryingPan && sSound.find("pan_") != std::string::npos) // Pan
		return true;

	if (Vars::Misc::Sound::Block.Value & Vars::Misc::Sound::BlockEnum::Water && // Water
		(sSound.find("ambient_mp3\\water\\water_splash") != std::string::npos || sSound.find("slosh") != std::string::npos || sSound.find("wade") != std::string::npos))
		return true;

	if (FNV1A::Hash32(pSound) == FNV1A::Hash32Const("Physics.WaterSplash")) // temporary fix for duplicate water sounds
		return true;

	return false;
}

MAKE_HOOK(CSoundEmitterSystem_EmitSound, S::CSoundEmitterSystem_EmitSound(), void,
	void* rcx, IRecipientFilter& filter, int entindex, const EmitSound_t& ep)
{
	if (ShouldBlockSound(ep.m_pSoundName))
		return;

	return CALL_ORIGINAL(rcx, filter, entindex, ep);
}

/*
MAKE_HOOK(S_StartDynamicSound, S::S_StartDynamicSound(), int,
	StartSoundParams_t& params)
{
	H::Entities.ManualNetwork(params);
	if (params.pSfx && ShouldBlockSound(params.pSfx->getname()))
		return 0;

	return CALL_ORIGINAL(params);
}
*/

MAKE_HOOK(S_StartSound, S::S_StartSound(), int,
	StartSoundParams_t& params)
{
	if (!params.staticsound)
		H::Entities.ManualNetwork(params);
	if (params.pSfx && ShouldBlockSound(params.pSfx->getname()))
		return 0;

	return CALL_ORIGINAL(params);
}
