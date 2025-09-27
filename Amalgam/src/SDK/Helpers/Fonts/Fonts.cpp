#include "Fonts.h"

void CFonts::Reload(float flDPI)
{
	m_mFonts[FONT_ESP] = { "Comic Sans MS Bold", int(13.f * flDPI), FONTFLAG_ANTIALIAS, 0 };
	m_mFonts[FONT_INDICATORS] = { "Comic Sans MS Bold", int(14.f * flDPI), FONTFLAG_ANTIALIAS, 0 };

	for (auto& [_, fFont] : m_mFonts)
	{
		if (fFont.m_dwFont = I::MatSystemSurface->CreateFont())
			I::MatSystemSurface->SetFontGlyphSet(fFont.m_dwFont, fFont.m_szName, fFont.m_nTall, fFont.m_nWeight, 0, 0, fFont.m_nFlags);
	}
}

const Font_t& CFonts::GetFont(EFonts eFont)
{
	return m_mFonts[eFont];
}