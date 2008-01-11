/*
 * Copyright (c) 2005-2008  cielacanth <cielacanth AT s60.xrea.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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
