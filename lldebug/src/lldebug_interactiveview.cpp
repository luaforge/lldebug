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
#include "lldebug_interactiveview.h"
#include "lldebug_mediator.h"
#include "lldebug_remoteengine.h"

namespace lldebug {

class InteractiveView::RunButton : public wxButton {
public:
	explicit RunButton(InteractiveView *parent, wxPoint pos, wxSize size)
		: wxButton(parent, wxID_ANY, wxT("Run"), pos, size)
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

class InteractiveView::TextInput : public wxTextCtrl {
public:
	explicit TextInput(InteractiveView *parent, wxPoint pos, wxSize size)
		: wxTextCtrl(parent, wxID_ANY, wxT(""), pos, size
			, wxTE_MULTILINE)
		, m_parent(parent) {
		Clear();
	}

	virtual ~TextInput() {
	}

private:
	void OnChar(wxKeyEvent &event) {
		if (!event.ShiftDown() && event.GetKeyCode() == WXK_RETURN) {
			m_parent->Run();
		}
		else {
			event.Skip();
		}
	}

private:
	InteractiveView *m_parent;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(InteractiveView::RunButton, wxButton)
	EVT_BUTTON(wxID_ANY, InteractiveView::RunButton::OnButton)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(InteractiveView::TextInput, wxTextCtrl)
	EVT_CHAR(InteractiveView::TextInput::OnChar)
END_EVENT_TABLE()


/*-----------------------------------------------------------------*/
BEGIN_EVENT_TABLE(InteractiveView, wxPanel)
	EVT_LLDEBUG_CHANGED_STATE(wxID_ANY, InteractiveView::OnChangedState)
	EVT_LLDEBUG_OUTPUT_INTERACTIVEVIEW(wxID_ANY, InteractiveView::OnOutputInteractiveView)
END_EVENT_TABLE()

InteractiveView::InteractiveView(wxWindow *parent)
	: wxPanel(parent, ID_INTERACTIVEVIEW) {
	CreateGUIControls();
}

InteractiveView::~InteractiveView() {
}

void InteractiveView::CreateGUIControls() {
	scoped_lock lock(m_mutex);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	m_text = new wxTextCtrl(this, wxID_ANY, wxT("output")
		, wxPoint(0,0), wxSize(400,300)
		, wxTE_READONLY | wxTE_MULTILINE | wxVSCROLL);
	sizer->Add(m_text, 1, wxALIGN_TOP | wxEXPAND);

	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_input = new TextInput(this, wxPoint(0,200), wxSize(300,GetCharHeight() + 8));
	sizer2->Add(m_input, 1, wxALIGN_LEFT | wxEXPAND | wxALL, 4);

	RunButton *m_run = new RunButton(this, wxPoint(300,300), wxSize(64, 1));
	sizer2->Add(m_run, 0, wxALIGN_RIGHT | wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 4);

	sizer->Add(sizer2, 0, wxALIGN_BOTTOM | wxEXPAND);

	SetAutoLayout(true);
	SetSizer(sizer);
	sizer->Layout();
	sizer->Fit(this);
	sizer->SetSizeHints(this);
}

void InteractiveView::OnChangedState(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);
	Enable(event.IsBreak());
}

void InteractiveView::OutputLog(const wxString &str) {
	scoped_lock lock(m_mutex);

	wxDebugEvent event(
		wxEVT_OUTPUT_INTERACTIVEVIEW,
		GetId(), str);
	AddPendingEvent(event);
}

void InteractiveView::OnOutputInteractiveView(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);

	m_text->AppendText(_T("\n"));
	m_text->AppendText(event.GetStr());
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

	void operator()(const Command &command, const std::string &str) {
		if (m_isVar) {
			if (!str.empty()) {
				m_view->OutputLog(wxConvFromUTF8(str));
			}
		}
		else {
			if (str.empty()) {
				m_view->OutputLog(_T("success"));
			}
			else {
				m_view->OutputLog(wxConvFromUTF8(str));
			}
		}
	}
	};

void InteractiveView::Run() {
	scoped_lock lock(m_mutex);
	wxString str = m_input->GetValue().Strip(wxString::both);
	std::string evalstr;
	bool isVar = false;

	// There is nothing to do.
	if (str.IsEmpty()) {
		return;
	}

	// '$' is the symbol indicates the variable.
	if (str[0] == wxT('$')) {
		wxString stripped = str;
		stripped = stripped.Remove(0, 1).Strip(wxString::both);

		evalstr = "lldebug_output_interactive(";
		evalstr += wxConvToUTF8(stripped);
		evalstr += ")";
		isVar = true;
	}
	else {
		evalstr = wxConvToUTF8(str);
		isVar = false;
	}

	Mediator::Get()->GetEngine()->Eval(
		evalstr,
		EvalResponseHandler(this, isVar));

	m_text->AppendText(_T("\n"));
	m_text->AppendText(_T("> "));
	m_text->AppendText(str);
	m_input->SetFocus();
	m_input->Clear();
}

}
