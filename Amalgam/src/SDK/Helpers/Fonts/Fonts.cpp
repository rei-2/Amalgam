#include "Fonts.h"

void CFonts::Reload(float flDPI)
{
	m_mapFonts[FONT_ESP] = { Vars::Fonts::FONT_ESP::szName.Value.c_str(), int(Vars::Fonts::FONT_ESP::nTall.Value * flDPI), Vars::Fonts::FONT_ESP::nFlags.Value, Vars::Fonts::FONT_ESP::nWeight.Value };
	m_mapFonts[FONT_INDICATORS] = { Vars::Fonts::FONT_INDICATORS::szName.Value.c_str(), int(Vars::Fonts::FONT_INDICATORS::nTall.Value * flDPI), Vars::Fonts::FONT_INDICATORS::nFlags.Value, Vars::Fonts::FONT_INDICATORS::nWeight.Value };
	m_mapFonts[FONT_NAME] = { Vars::Fonts::FONT_NAME::szName.Value.c_str(), int(Vars::Fonts::FONT_NAME::nTall.Value * flDPI), Vars::Fonts::FONT_NAME::nFlags.Value, Vars::Fonts::FONT_NAME::nWeight.Value };
	m_mapFonts[FONT_CONDS] = { Vars::Fonts::FONT_CONDS::szName.Value.c_str(), int(Vars::Fonts::FONT_CONDS::nTall.Value * flDPI), Vars::Fonts::FONT_CONDS::nFlags.Value, Vars::Fonts::FONT_CONDS::nWeight.Value };

	for (auto& [_, fFont] : m_mapFonts)
	{
		I::MatSystemSurface->SetFontGlyphSet
		(
			fFont.m_dwFont = I::MatSystemSurface->CreateFont(),
			fFont.m_szName,		//name
			fFont.m_nTall,		//tall
			fFont.m_nWeight,	//weight
			0,					//blur
			0,					//scanlines
			fFont.m_nFlags		//flags
		);
	}
}

const Font_t& CFonts::GetFont(EFonts eFont)
{
	return m_mapFonts[eFont];
}