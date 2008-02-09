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
#include "lldebug_outputview.h"

namespace lldebug {

class OutputView::InnerTextCtrl : public wxScintilla {
private:
	enum {
		MARGIN_INFO = 1,
		MARKNUM_ERROR = 1,
	};

	struct ViewData {
		std::string key;
		int line;
	};
	typedef std::map<int, ViewData> DataMap;
	DataMap m_dataMap;

	mutex m_mutex;

	DECLARE_EVENT_TABLE();

public:
	explicit InnerTextCtrl(wxWindow *parent)
		: wxScintilla(parent, wxID_ANY) {
		CreateGUIControls();
	}

	virtual ~InnerTextCtrl() {
	}

	void CreateGUIControls() {
		scoped_lock lock(m_mutex);

		SetReadOnly(true);
		SetViewEOL(false);
		SetWrapMode(wxSCI_WRAP_NONE);
		SetEdgeMode(wxSCI_EDGE_NONE);
		SetViewWhiteSpace(wxSCI_WS_INVISIBLE);
		SetLayoutCache(wxSCI_CACHE_PAGE);
		SetLexer(wxSCI_LEX_NULL);

		/// Setup the selectable error marker.
		MarkerDefine(MARKNUM_ERROR, wxSCI_MARK_ARROWS);
		MarkerSetForeground(MARKNUM_ERROR, wxColour(_T("RED")));

		// Setup the infomation margin.
		StyleSetForeground(wxSCI_STYLE_DEFAULT,
			wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		SetMarginType(MARGIN_INFO, wxSCI_MARGIN_FORE);
		SetMarginWidth(MARGIN_INFO, 16);
		SetMarginSensitive(MARGIN_INFO, false);
		SetMarginMask(MARGIN_INFO, (1 << MARKNUM_ERROR));
	}

	/// Add raw text that is std::string.
	void AddTextRawStd(const std::string &str) {
		scoped_lock lock(m_mutex);

		if (str.empty()) {
			return;
		}

		AddTextRaw(str.c_str());
	}

	/// Output log.
	void OutputLog(LogType type, const wxString &str, const std::string &key, int line) {
		scoped_lock lock(m_mutex);

		if (type == LOGTYPE_INTERACTIVE) {
			return;
		}

		if (!key.empty()) {
			ViewData data;
			data.key = key;
			data.line = line;
			m_dataMap[GetLineCount() - 1] = data;
		}

		if (type == LOGTYPE_ERROR && !key.empty()) {
			MarkerAdd(GetLineCount() - 1, MARKNUM_ERROR);
		}

		SetReadOnly(false);
		if (!key.empty()) {
			const Source *source = Mediator::Get()->GetSource(key);
			AddTextRawStd(source->GetTitle());
			AddTextRaw("(");
			AddTextRawStd(boost::lexical_cast<std::string>(line));
			AddTextRaw("): ");
		}
		AddText(str);
		AddTextRaw("\n");
		SetReadOnly(true);
	}

	void OnDClick(wxScintillaEvent &event) {
		scoped_lock lock(m_mutex);
		//event.Skip();

		// Out of selectable range.
		if (event.GetPosition() < 0) {
			return;
		}

		// Goto the specify source line.
		int line = LineFromPosition(event.GetPosition());
		DataMap::iterator it = m_dataMap.find(line);
		if (it != m_dataMap.end()) {
			const ViewData &data = it->second;
			Mediator::Get()->FocusErrorLine(data.key, data.line);

			SetSelection(
				PositionFromLine(line),
				GetLineEndPosition(line));
		}
	}
};

BEGIN_EVENT_TABLE(OutputView::InnerTextCtrl, wxScintilla)
	EVT_SCI_DOUBLECLICK(wxID_ANY, OutputView::InnerTextCtrl::OnDClick)
END_EVENT_TABLE()


/*-----------------------------------------------------------------*/
BEGIN_EVENT_TABLE(OutputView, wxListBox)
	EVT_SIZE(OutputView::OnSize)
	EVT_LLDEBUG_OUTPUT_LOG(wxID_ANY, OutputView::OnOutputLog)
END_EVENT_TABLE()

OutputView::OutputView(wxWindow *parent)
	: wxPanel(parent, ID_OUTPUTVIEW, wxPoint(0, 0), wxSize(100, 200)) {
	CreateGUIControls();
}

OutputView::~OutputView() {
}

void OutputView::CreateGUIControls() {
	scoped_lock lock(m_mutex);

	m_text = new InnerTextCtrl(this);
}

void OutputView::OnSize(wxSizeEvent &event) {
	scoped_lock lock(m_mutex);

	m_text->SetSize(GetClientSize());
}

void OutputView::OutputLog(LogType logType, const wxString &str, const std::string &key, int line) {
	scoped_lock lock(m_mutex);

	wxDebugEvent event(wxEVT_OUTPUT_LOG, GetId(), logType, str, key, line);
	ProcessEvent(event);
}

void OutputView::OnOutputLog(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);

	m_text->OutputLog(event.GetLogType(), event.GetStr(), event.GetKey(), event.GetLine());
}

}
