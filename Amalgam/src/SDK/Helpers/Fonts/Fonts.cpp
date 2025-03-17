#include "Fonts.h"

void CFonts::Reload(float flDPI)
{
	m_mFonts[FONT_ESP] = { "Verdana", int(12.f * flDPI), FONTFLAG_ANTIALIAS, 0 };
	m_mFonts[FONT_INDICATORS] = { "Verdana", int(13.f * flDPI), FONTFLAG_ANTIALIAS, 0 };

	for (auto& [_, fFont] : m_mFonts)
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
	return m_mFonts[eFont];
}