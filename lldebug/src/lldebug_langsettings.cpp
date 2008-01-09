//---------------------------------------------------------------------------
//
// Name:        MainFrame.cpp
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:33
// Description: MainFrame class implementation
//
//---------------------------------------------------------------------------

#include "lldebug_prec.h"
#include "lldebug_langsettings.h"
#include "wx/wxscintilla.h"

namespace lldebug {

/// keywordlist
wxChar *s_Wordlist1 =
	wxT("and break do else elseif \
		end false for function if \
		in local nil not or \
		repeat return then true until while");
wxChar *s_Wordlist2 =
	wxT("require module"); //coroutine debug file io math os string");
wxChar *s_Wordlist3 =
	wxT("");

/// Styles info.
const StyleInfo s_stylePrefs [] = {
    {
		wxSCI_LUA_DEFAULT,
		wxT("BLACK"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_COMMENT,
		wxT("FOREST GREEN"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_COMMENTLINE,
		wxT("FOREST GREEN"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_COMMENTDOC,
		wxT("FOREST GREEN"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_NUMBER,
		wxT("BLACK"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_WORD,
		wxT("BLUE"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, s_Wordlist1
	}, {
		wxSCI_LUA_STRING,
		wxT("BROWN"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_CHARACTER,
		wxT("BROWN"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_LITERALSTRING,
		wxT("BROWN"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_PREPROCESSOR,
		wxT("GRAY"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_OPERATOR,
		wxT("VIOLET"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_IDENTIFIER,
		wxT("BLACK"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, true, NULL
	}, {
		wxSCI_LUA_STRINGEOL,
		wxT("BROWN"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}, {
		wxSCI_LUA_WORD2,
		wxT("BLACK"), wxT("WHITE"),
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, s_Wordlist2
	}, {
		STYLE_END,
		NULL, NULL,
		FONTSTYLE_REGULAR, wxSCI_CASE_MIXED, false, NULL
	}
};

}
