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
#include "lldebug_mediator.h"
#include "lldebug_sourceview.h"
#include "lldebug_langsettings.h"
#include "wx/wxscintilla.h"

namespace lldebug {

#if 0
     wxT("FOREST GREEN"), wxT("WHITE"),
     wxT("KHAKI"), wxT("WHITE"),
     wxT("BROWN"), wxT("WHITE"),
     wxT("ORANGE"), wxT("WHITE"),
     wxT("VIOLET"), wxT("WHITE"),
     wxT("BLUE"), wxT("WHITE"),
     wxT("SIENNA"), wxT("WHITE"),
     wxT("ORCHID"), wxT("WHITE"),
     wxT("GREY"), wxT("WHITE"),
     wxT("DARK GREY"), wxT("WHITE"),
#endif

/**
 * @brief ソースコードを表示するコントロールです。
 */
class SourceViewPage : public wxScintilla {
	enum {
		MARGIN_LINENUM = 0,
		MARGIN_DEBUG = 1,
		MARGIN_FOLDING = 2,
		MARGIN_DIVIDER = 3,

		MARKNUM_BREAKPOINT = 1,
		MARKNUM_RUNNING = 2,
		MARKNUM_BACKTRACE = 3,
	};

public:
	explicit SourceViewPage(SourceView *parent)
		: wxScintilla(parent, wxID_ANY)
		, m_parent(parent), m_initialized(false)
		, m_isModified(false)
		, m_currentLine(-1), m_markedLine(-1) {
		CreateGUIControls();
	}

	virtual ~SourceViewPage() {
	}

private:
	void CreateGUIControls() {
		scoped_lock lock(m_mutex);

		// default font for all styles
		SetViewEOL(false);
		SetEdgeMode(false ? wxSCI_EDGE_LINE : wxSCI_EDGE_NONE);
		SetViewWhiteSpace(wxSCI_WS_INVISIBLE);
		SetOvertype(false);
		SetReadOnly(false);
		SetWrapMode(wxSCI_WRAP_NONE);
		//SetHotspotActiveUnderline(true);
		SetLexer(wxSCI_LEX_LUA);
		SetLayoutCache(wxSCI_CACHE_PAGE);

		// set spaces and indention
		SetTabWidth(4);
		SetUseTabs(true);
		SetTabIndents(true);
		SetBackSpaceUnIndents(true);
		SetIndent(true ? 2 : 0);
		SetIndentationGuides(false);

		SetVisiblePolicy(wxSCI_VISIBLE_STRICT | wxSCI_VISIBLE_SLOP, 1);
		SetXCaretPolicy(wxSCI_CARET_EVEN | wxSCI_VISIBLE_STRICT | wxSCI_CARET_SLOP, 1);
		SetYCaretPolicy(wxSCI_CARET_EVEN | wxSCI_VISIBLE_STRICT | wxSCI_CARET_SLOP, 1);

		// Initialize language settings.
		wxFont font = GetFont();
//		(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
//		false, wxT("MS Gothic"));
        int keywordNum = 0;
		for (size_t i = 0; s_stylePrefs[i].style != STYLE_END; ++i) {
			const StyleInfo &curType = s_stylePrefs[i];
			int style = curType.style;

            if (curType.foreground != NULL) {
                StyleSetForeground(style, wxColour(curType.foreground));
            }
            if (curType.background != NULL) {
                StyleSetBackground(style, wxColour(curType.background));
            }
			StyleSetFont(style, font);
            StyleSetBold(style, (curType.fontStyle & FONTSTYLE_BOLD) > 0);
            StyleSetItalic(style, (curType.fontStyle & FONTSTYLE_ITALIC) > 0);
            StyleSetUnderline(style, (curType.fontStyle & FONTSTYLE_UNDERL) > 0);
            StyleSetVisible(style, (curType.fontStyle & FONTSTYLE_HIDDEN) == 0);
            StyleSetCase(style, curType.letterCase);
			//StyleSetHotSpot(style, curType.hotspot);
            if (curType.words != NULL) {
                SetKeyWords(keywordNum, curType.words);
                ++keywordNum;
            }
        }

		// Set the line number margin.
		int lineNumMargin = TextWidth(wxSCI_STYLE_LINENUMBER, wxT("_9999"));
		SetMarginType(MARGIN_LINENUM, wxSCI_MARGIN_NUMBER);
		SetMarginWidth(MARGIN_LINENUM, lineNumMargin);
		StyleSetForeground(wxSCI_STYLE_LINENUMBER, wxColour(wxT("BLACK")));
		StyleSetBackground(wxSCI_STYLE_LINENUMBER, wxColour(wxT("WHITE")));

		// Set the debug info margin.
		StyleSetForeground(wxSCI_STYLE_DEFAULT,
			wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		SetMarginType(MARGIN_DEBUG, wxSCI_MARGIN_FORE);
		SetMarginWidth(MARGIN_DEBUG, 16);
		SetMarginSensitive(MARGIN_DEBUG, true);
		SetMarginMask(MARGIN_DEBUG,
			(1 << MARKNUM_BREAKPOINT) | (1 << MARKNUM_RUNNING) | (1 << MARKNUM_BACKTRACE));

		// Set the space margin (only the visual).
		StyleSetBackground(wxSCI_STYLE_DEFAULT, wxColour(wxT("WHITE")));
		SetMarginType(MARGIN_DIVIDER, wxSCI_MARGIN_BACK);
		SetMarginWidth(MARGIN_DIVIDER, 4);
		SetMarginSensitive(MARGIN_DIVIDER, false);
		SetMarginMask(MARGIN_DIVIDER, 0);

		// Set the folding margins.
		SetMarginType(MARGIN_FOLDING, wxSCI_MARGIN_SYMBOL);
		SetMarginMask(MARGIN_FOLDING, wxSCI_MASK_FOLDERS);
		SetFoldMarginColour(true, wxColour(_T("WHITE")));
		SetFoldMarginHiColour(true, wxColour(_T("WHITE")));
		SetMarginWidth(MARGIN_FOLDING, 12);
		SetMarginSensitive(MARGIN_FOLDING, true);
		SetProperty(wxT("fold"), wxT("1"));
		SetProperty(wxT("fold.comment"), wxT("1"));
		SetProperty(wxT("fold.compact"), wxT("1"));
		SetFoldFlags(
			wxSCI_FOLDFLAG_LINEBEFORE_CONTRACTED |
			wxSCI_FOLDFLAG_LINEAFTER_CONTRACTED);

		// Set the folding markers.
		wxColour foldColour = wxColour(wxT("DARK GREY"));
		MarkerDefine(wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS);
		MarkerSetBackground(wxSCI_MARKNUM_FOLDER, foldColour);
		MarkerSetForeground(wxSCI_MARKNUM_FOLDER, wxColour(wxT("WHITE")));
		MarkerDefine(wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS);
		MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPEN, foldColour);
		MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPEN, wxColour(wxT("WHITE")));

		wxColour lineColour = wxColour(wxT("GREEN YELLOW"));
		MarkerDefine(wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_VLINE);
		MarkerSetBackground(wxSCI_MARKNUM_FOLDERSUB, lineColour);
		MarkerDefine(wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_TCORNERCURVE);
		MarkerSetBackground(wxSCI_MARKNUM_FOLDERMIDTAIL, lineColour);
		MarkerDefine(wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_LCORNERCURVE);
		MarkerSetBackground(wxSCI_MARKNUM_FOLDERTAIL, lineColour);

		MarkerDefine(wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_ARROWDOWN);
		MarkerDefine(wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_ARROW);
		MarkerSetForeground(wxSCI_MARKNUM_FOLDEROPENMID, foldColour);
		MarkerSetForeground(wxSCI_MARKNUM_FOLDEREND, foldColour);
		MarkerSetBackground(wxSCI_MARKNUM_FOLDEROPENMID, wxColour(wxT("WHITE")));
		MarkerSetBackground(wxSCI_MARKNUM_FOLDEREND, wxColour(wxT("WHITE")));

		// Set the breakpoint marker.
		MarkerDefine(MARKNUM_BREAKPOINT, wxSCI_MARK_CIRCLE);
		MarkerSetForeground(MARKNUM_BREAKPOINT, wxColour(_T("ORANGE")));
		MarkerSetBackground(MARKNUM_BREAKPOINT, wxColour(_T("RED")));

		/// Set the marker indicates current running source and line.
		MarkerDefine(MARKNUM_RUNNING, wxSCI_MARK_SHORTARROW);
		MarkerSetForeground(MARKNUM_RUNNING, wxColour(_T("RED")));
		MarkerSetBackground(MARKNUM_RUNNING, wxColour(_T("YELLOW")));

		/// Set the marker indicates backtrace source and line.
		MarkerDefine(MARKNUM_BACKTRACE, wxSCI_MARK_BACKGROUND);
		MarkerSetForeground(MARKNUM_BACKTRACE, wxColour(_T("YELLOW")));
		MarkerSetBackground(MARKNUM_BACKTRACE, wxColour(_T("GREEN")));
	}

	/// Fold the source, if any.
	void OnMarginClick(wxScintillaEvent &event) {
		scoped_lock lock(m_mutex);

		if (event.GetMargin() == MARGIN_FOLDING) {
			int lineClick = LineFromPosition(event.GetPosition());
			int levelClick = GetFoldLevel(lineClick);
			if ((levelClick & wxSCI_FOLDLEVELHEADERFLAG) > 0) {
				ToggleFold(lineClick);
			}
		}
		else if (event.GetMargin() == MARGIN_DEBUG) {
			int lineClick = LineFromPosition(event.GetPosition());
			ToggleBreakpointFromLine(lineClick);
		}
	}

	/// Save source if 'Ctrl+S' key was pressed.
	void OnKeyDown(wxKeyEvent &event) {
		scoped_lock lock(m_mutex);
		event.Skip();

		if (event.ControlDown() && event.GetKeyCode() == 'S') {
			SaveSource();
		}
	}

	/// Enable or disable the modified mark.
	void ChangeModified(bool modified) {
		if (!m_initialized || modified == m_isModified) {
			return;
		}

		size_t sel = m_parent->GetPageIndex(this);
		if (sel != wxNOT_FOUND) {
			if (modified) {
				// Add '*' to the source title.
				m_parent->SetPageText(sel, m_title + wxT("*"));
			}
			else {
				// Set the source title.
				m_parent->SetPageText(sel, m_title);
			}

			m_isModified = modified;
		}
	}

	/// Modified mark may be set, if any.
	void OnModified(wxScintillaEvent &event) {
		scoped_lock lock(m_mutex);
		event.Skip();

		if (event.GetModificationType()
			& (wxSCI_MOD_INSERTTEXT | wxSCI_MOD_DELETETEXT)) {
			ChangeModified(true);
		}
	}

	/// Indent if newline was added.
	void OnCharAdded(wxScintillaEvent &event) {
		scoped_lock lock(m_mutex);

		// Change this if support for mac files with \r is needed
		if (event.GetKey() == '\n' || event.GetKey() == '\r') {
			int currentLine = GetCurrentLine();
			if (currentLine <= 0) {
				return;
			}

			// width of one indent character
			int indentWidth = (GetUseTabs() ? GetTabWidth() : 1);
			if (indentWidth == 0) {
				return;
			}

			// indent as prev line level
			int indentSize = GetLineIndentation(currentLine - 1);
			SetLineIndentation(currentLine, indentSize);

			// position = (line start pos) + (tabs count) + (space count)
			GotoPos(PositionFromLine(currentLine)
				+ (indentSize / indentWidth)
				+ (indentSize % indentWidth));

			// notify that the text was changed
			ChangeModified(true);
		}
	}

	void OnHotSpotClick(wxScintillaEvent &event) {
		scoped_lock lock(m_mutex);
		//int pos = event.GetPosition();
		//CallTipShow(pos, event.GetText());
	}

	/// Refresh the breakpoint marks.
	void OnChangedBreakpoints(wxDebugEvent &event) {
		scoped_lock lock(m_mutex);
		MarkerDeleteAll(MARKNUM_BREAKPOINT);

		BreakpointList &bps = Mediator::Get()->GetBreakpoints();
		Breakpoint bp;
		for (bp = bps.First(GetKey()); bp.IsOk(); bp = bps.Next(bp)) {
			MarkerAdd(bp.GetLine(), MARKNUM_BREAKPOINT);
		}
	}

public:
	/// Get the source key.
	const std::string &GetKey() const {
		return m_key;
	}

	/// Get the source title.
	const wxString &GetTitle() const {
		return m_title;
	}

	/// Initialize this object.
	void Initialize(const Source &source) {
		scoped_lock lock(m_mutex);

		std::string str;
		for (string_array::size_type i = 0; i < source.GetLineCount(); ++i) {
			// The encoding is UTF8.
			str += source.GetSourceLine(i);

			// Don't insert line breaks at the last line.
			if (i < source.GetLineCount()) {
				str += "\n";
			}
		}

		// AddTextRaw accepts only the UTF8 string.
		AddTextRaw(str.c_str());

		SetReadOnly(source.GetPath().empty());

		// The title is converted to UTF8.
		m_key = source.GetKey();
		m_title = wxConvFromUTF8(source.GetTitle());
		m_path = wxConvFromUTF8(source.GetPath());
		m_currentLine = -1;
		m_initialized = true;

		OnChangedBreakpoints(wxDebugEvent(wxEVT_CHANGED_BREAKPOINTS, GetId()));
	}

	/// Focus the current running line.
	int FocusCurrentLine(int line, bool isCurrentRunning=true) {
		scoped_lock lock(m_mutex);
		wxASSERT((line < 0) || (0 < line && line <= GetLineCount()));

		// Line base is different.
		if (line > 0) {
			--line;
		}

		if (isCurrentRunning && m_currentLine >= 0) {
			MarkerDelete(m_currentLine, MARKNUM_RUNNING);
			m_currentLine = -1;
		}
		
		// Hide backtrace mark always.
		if (m_markedLine >= 0) {
			MarkerDeleteAll(MARKNUM_BACKTRACE);
			m_markedLine = -1;
		}

		// Set current line.
		if (line > 0) {
			EnsureVisible(line);
			int pos = PositionFromLine(line);
			SetSelection(pos, pos);

			if (isCurrentRunning) {
				MarkerAdd(line, MARKNUM_RUNNING);
			}
			else {
				MarkerAdd(line, MARKNUM_BACKTRACE);
			}
		}

		if (isCurrentRunning) {
			m_currentLine = line;
		}
		else {
			m_markedLine = line;
		}

		return 0;
	}

	void ToggleBreakpointFromLine(int line) {
		scoped_lock lock(m_mutex);
		line = median(line, 0, GetLineCount());
		Mediator::Get()->ToggleBreakpoint(m_key, line);
	}

	void ToggleBreakpoint() {
		scoped_lock lock(m_mutex);
		int from, to;
		GetSelection(&from, &to);
		ToggleBreakpointFromLine(LineFromPosition(to));
	}

	/// Focus the error line.
	void FocusErrorLine(int line) {
		scoped_lock lock(m_mutex);

		if (line <= 0) {
			return;
		}

		EnsureVisible(line - 1);
		SetFocus();
		SetSelection(
			PositionFromLine(line - 1),
			GetLineEndPosition(line - 1));
	}

	/// Change weather this object is enable.
	void ChangeEnable(bool enable) {
		scoped_lock lock(m_mutex);

		if (!enable) {
			FocusCurrentLine(-1, true);
		}
	}

	/// Save source text.
	void SaveSource() {
		scoped_lock lock(m_mutex);
		if (m_path.IsEmpty()) {
			return;
		}

		string_array array;
		for (int i = 0; i < GetLineCount(); ++i) {
			wxCharBuffer cbuffer = GetLineRaw(i);
			std::string buffer = (cbuffer != NULL ? cbuffer.data() : "");

			// trim newlines
			while (!buffer.empty()) {
				char c = *(--buffer.end());
				if (c == '\n' || c == '\r') {
					buffer.erase(buffer.length() - 1);
				}
				else {
					break;
				}
			}

			array.push_back(buffer);
		}

		// Erase the last line if it is newline only.
		if (!array.empty() && array.back().empty()) {
			array.pop_back();
		}

		//SaveFile(m_path);
		ChangeModified(false);
	}

private:
	SourceView *m_parent;
	mutex m_mutex;
	bool m_initialized;
	bool m_isModified;

	std::string m_key;
	wxString m_title;
	wxString m_path;
	int m_currentLine;
	int m_markedLine;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(SourceViewPage, wxScintilla)
	EVT_KEY_DOWN(SourceViewPage::OnKeyDown)
	EVT_SCI_MODIFIED(wxID_ANY, SourceViewPage::OnModified)
	EVT_SCI_MARGINCLICK(wxID_ANY, SourceViewPage::OnMarginClick)
	EVT_SCI_CHARADDED(wxID_ANY, SourceViewPage::OnCharAdded)
	EVT_SCI_HOTSPOT_CLICK(wxID_ANY, SourceViewPage::OnHotSpotClick)
	EVT_LLDEBUG_CHANGED_BREAKPOINTS(wxID_ANY, SourceViewPage::OnChangedBreakpoints)
END_EVENT_TABLE()


/*-----------------------------------------------------------------*/
BEGIN_EVENT_TABLE(SourceView, wxAuiNotebook)
	EVT_LLDEBUG_CHANGED_STATE(wxID_ANY, SourceView::OnChangedState)
	EVT_LLDEBUG_UPDATE_SOURCE(wxID_ANY, SourceView::OnUpdateSource)
	EVT_LLDEBUG_ADDED_SOURCE(wxID_ANY, SourceView::OnAddedSource)
	EVT_LLDEBUG_FOCUS_ERRORLINE(wxID_ANY, SourceView::OnFocusErrorLine)
	EVT_LLDEBUG_FOCUS_BACKTRACELINE(wxID_ANY, SourceView::OnFocusBacktraceLine)
END_EVENT_TABLE()

SourceView::SourceView(wxWindow *parent)
	: wxAuiNotebook(parent, ID_SOURCEVIEW
		, wxDefaultPosition, wxDefaultSize
		, wxAUI_NB_TOP | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS) {
	CreateGUIControls();
}

SourceView::~SourceView() {
}

void SourceView::CreateGUIControls() {
	scoped_lock lock(m_mutex);

	std::list<Source> sources = Mediator::Get()->GetSourceManager().GetList();
	std::list<Source>::iterator it;
	for (it = sources.begin(); it != sources.end(); ++it) {
		CreatePage(*it);
	}
}

size_t SourceView::FindPageFromKey(const std::string &key) {
	scoped_lock lock(m_mutex);

	for (size_t i = 0; i < GetPageCount(); ++i) {
		SourceViewPage *page = GetPage(i);

		if (page->GetKey() == key) {
			return i;
		}
	}

	return (size_t)wxNOT_FOUND;
}

SourceViewPage *SourceView::GetPage(size_t i) {
	scoped_lock lock(m_mutex);

	wxWindow *page = wxAuiNotebook::GetPage(i);
	return dynamic_cast<SourceViewPage *>(page);
}

SourceViewPage *SourceView::GetSelected() {
	scoped_lock lock(m_mutex);
	size_t sel = GetSelection();

	if (sel == wxNOT_FOUND) {
		return NULL;
	}

	return GetPage(sel);
}

void SourceView::CreatePage(const Source &source) {
	scoped_lock lock(m_mutex);

	SourceViewPage *page = new SourceViewPage(this);
	page->Initialize(source);
	AddPage(page, page->GetTitle(), true);
}

void SourceView::OnChangedState(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);

	SourceViewPage *page = GetSelected();
	if (page != NULL) {
		page->ChangeEnable(event.IsBreak());
	}
}

void SourceView::ToggleBreakpoint() {
	scoped_lock lock(m_mutex);
	SourceViewPage *page = GetSelected();

	if (page != NULL) {
		page->ToggleBreakpoint();
	}
}

void SourceView::OnUpdateSource(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);
	
	for (size_t i = 0; i < GetPageCount(); ++i) {
		SourceViewPage *page = GetPage(i);

		if (page->GetKey() == event.GetKey()) {
			page->FocusCurrentLine(event.GetLine());
			SetSelection(i);
		}
		else {
			page->FocusCurrentLine(-1);
		}
	}
}

void SourceView::OnAddedSource(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);

	CreatePage(event.GetSource());
}

void SourceView::OnFocusErrorLine(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);

	for (size_t i = 0; i < GetPageCount(); ++i) {
		SourceViewPage *page = GetPage(i);

		if (page->GetKey() == event.GetKey()) {
			page->FocusErrorLine(event.GetLine());
			SetSelection(i);
			break;
		}
	}
}

void SourceView::OnFocusBacktraceLine(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);

	for (size_t i = 0; i < GetPageCount(); ++i) {
		SourceViewPage *page = GetPage(i);

		if (page->GetKey() == event.GetKey()) {
			page->FocusCurrentLine(event.GetLine(), false);
			SetSelection(i);
		}
		else {
			page->FocusCurrentLine(-1, false);
		}
	}
}

}
