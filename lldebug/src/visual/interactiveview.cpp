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

#include "precomp.h"
#include "configfile.h"
#include "visual/interactiveview.h"
#include "visual/mediator.h"
#include "visual/strutils.h"

namespace lldebug {
namespace visual {

/**
 * @brief Run button
 */
class InteractiveView::RunButton : public wxButton {
public:
	explicit RunButton(InteractiveView *parent, wxPoint pos, wxSize size)
		: wxButton(parent, wxID_ANY, _("Run"), pos, size)
		, m_parent(parent) {
	}

	virtual ~RunButton() {
	}

protected:
	virtual void OnButton(wxCommandEvent &) {
		m_parent->Run();
	}

private:
	InteractiveView *m_parent;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(InteractiveView::RunButton, wxButton)
	EVT_BUTTON(wxID_ANY, InteractiveView::RunButton::OnButton)
END_EVENT_TABLE()


/*-----------------------------------------------------------------*/
/**
 * @brief 
 */
class InteractiveView::TextInput : public wxTextCtrl {
private:
	typedef std::list<wxString> HistoryTexts;

public:
	explicit TextInput(InteractiveView *parent, wxPoint pos, wxSize size)
		: wxTextCtrl(parent, wxID_ANY, wxT(""), pos, size
			, wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB)
		, m_parent(parent), m_historyPos(m_historyTexts.end()) {
	}

	virtual ~TextInput() {
	}

	/// Add the evaled string.
	void AddHistory(const wxString &str) {
		// Delete the old history, if any.
		while (m_historyTexts.size() > 300) {
			m_historyTexts.pop_front();
		}

		m_historyTexts.push_back(str);
		m_historyPos = m_historyTexts.end();
		m_inputText.Clear();
	}

private:
	/// Set the text of the history.
	void SetHistory(HistoryTexts::iterator it) {
		if (it == m_historyTexts.end()) {
			ChangeValue(m_inputText);
		}
		else {
			ChangeValue(*it);
		}

		long pos = GetLastPosition();
		SetSelection(pos, pos);
	}

	/// Iterate the history.
	void OnChar(wxKeyEvent &event) {
		if (!event.ShiftDown()) {
			switch (event.GetKeyCode()) {
			case WXK_RETURN:
				m_parent->Run();
				return;
			case WXK_TAB:
				return;
			case WXK_UP:
				if (m_historyPos != m_historyTexts.begin()) {
					if (m_historyPos == m_historyTexts.end()) {
						m_inputText = GetValue();
					}

					SetHistory(--m_historyPos);
				}
				return;
			case WXK_DOWN:
				if (m_historyPos != m_historyTexts.end()) {
					SetHistory(++m_historyPos);
				}
				return;
			}
		}

		event.Skip();
	}

private:
	InteractiveView *m_parent;
	HistoryTexts m_historyTexts;
	HistoryTexts::iterator m_historyPos;
	wxString m_inputText;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(InteractiveView::TextInput, wxTextCtrl)
	EVT_CHAR(InteractiveView::TextInput::OnChar)
END_EVENT_TABLE()


/*-----------------------------------------------------------------*/
BEGIN_EVENT_TABLE(InteractiveView, wxPanel)
	EVT_DEBUG_CHANGED_STATE(wxID_ANY, InteractiveView::OnChangedState)
END_EVENT_TABLE()

InteractiveView::InteractiveView(wxWindow *parent)
	: wxPanel(parent, ID_INTERACTIVEVIEW) {
	CreateGUIControls();

	lldebug::GetConfigFileName("interactive.dat.tmp");
}

InteractiveView::~InteractiveView() {
}

void InteractiveView::CreateGUIControls() {
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	m_text = new wxTextCtrl(this, wxID_ANY, wxT("output")
		, wxPoint(0,0), wxSize(400,300)
		, wxTE_READONLY | wxTE_MULTILINE | wxVSCROLL);
	sizer->Add(m_text, 1, wxALIGN_TOP | wxEXPAND);

	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_input = new TextInput(this, wxPoint(0,200), wxSize(300,GetCharHeight() + 8));
	sizer2->Add(m_input, 1, wxALIGN_LEFT | wxEXPAND | wxALL, 0);

	RunButton *m_run = new RunButton(this, wxPoint(300,300), wxSize(64, 1));
	sizer2->Add(m_run, 0, wxALIGN_RIGHT | wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 1);

	sizer->Add(sizer2, 0, wxALIGN_BOTTOM | wxEXPAND);

	SetAutoLayout(true);
	SetSizer(sizer);
	sizer->SetSizeHints(this);
}

void InteractiveView::OnChangedState(wxDebugEvent &event) {
	Enable(event.IsBreak());
}

/// Output log str.
void InteractiveView::OutputLog(const wxString &str) {
	m_text->AppendText(_T("\n"));
	m_text->AppendText(str);
}

/**
 * @brief 
 */
struct EvalResponseHandler {
	InteractiveView *m_view;
	bool m_isVar;

	explicit EvalResponseHandler(InteractiveView *view, bool isVar)
		: m_view(view), m_isVar(isVar) {
	}

	int operator()(const net::Command &command, const LuaVarList &vars) {
		bool successed = false;

		if (vars.empty()) {
			if (m_isVar) {
				m_view->OutputLog(_T("error"));
			}
			else {
				m_view->OutputLog(_T("success"));
				successed = true;
			}
		}
		else {
			if (m_isVar) {
				successed = true;
			}

			// Set the variable's texts.
			for (LuaVarList::size_type i = 0; i < vars.size(); ++i) {
				m_view->OutputLog(wxConvFromUTF8(vars[i].GetValue()));
			}
		}

		if (!successed) {
			return -1;
		}

		// Increment update count for WatchView and other, if need.
		Mediator::Get()->GetEngine()->ForceUpdateSource();
		return 0;
	}
	};

void InteractiveView::Run() {
	wxString str = m_input->GetValue().Strip(wxString::both);
	std::string evalstr;
	bool isVar = false;

	// There is nothing to do.
	if (str.IsEmpty()) {
		return;
	}

	// '$' is the symbol that indicates to print the variable.
	if (str[0] == wxT('$')) {
		wxString stripped = str;
		stripped = stripped.Remove(0, 1).Strip(wxString::both);

		evalstr = "return lldebug.tostring_detail(";
		evalstr += wxConvToUTF8(stripped);
		evalstr += ")";
		isVar = true;
	}
	else {
		evalstr = wxConvToUTF8(str);
		isVar = false;
	}

	// Eval the string.
	Mediator::Get()->GetEngine()->EvalToMultiVar(
		evalstr,
		Mediator::Get()->GetStackFrame(),
		EvalResponseHandler(this, isVar));

	// Show evaled text.
	m_text->AppendText(_T("\n"));
	m_text->AppendText(_T("> "));
	m_text->AppendText(str);

	m_input->AddHistory(str);
	m_input->SetFocus();
	m_input->Clear();
}

} // end of namespace visual
} // end of namespace lldebug
