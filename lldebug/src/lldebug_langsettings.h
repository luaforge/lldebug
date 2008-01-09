//---------------------------------------------------------------------------
//
// Name:        MainFrame.h
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:33
// Description: MainFrame class declaration
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_LANGSETTINGS_H__
#define __LLDEBUG_LANGSETTINGS_H__

namespace lldebug {

/**
 * @brief Text style.
 */
enum FontStyle {
	FONTSTYLE_REGULAR = 0x00,
	FONTSTYLE_BOLD = 0x01,
	FONTSTYLE_ITALIC = 0x02,
	FONTSTYLE_UNDERL = 0x04,
	FONTSTYLE_HIDDEN = 0x08,
};

/**
 * @brief Editor styles infomation.
 */
struct StyleInfo {
	int style;
	wxChar *foreground;
	wxChar *background;
	FontStyle fontStyle;
	int letterCase;
	bool hotspot;
	const wxChar *words;
};

const int STYLE_END = ~0;

/// Styles info.
extern const StyleInfo s_stylePrefs[];

}

#endif
