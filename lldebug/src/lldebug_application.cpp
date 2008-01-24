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
#include "lldebug_application.h"
#include "lldebug_mainframe.h"

IMPLEMENT_APP(lldebug::Application)

int main(int argc, char *argv[]) {
	wxEntry(argc, argv);
	return 0;
}

namespace lldebug {

#if 0
void ManagerFrame::OnCreateFrame(wxEvent &event) {
	wxContextEvent &e = dynamic_cast<wxContextEvent &>(event);

	MainFrame* frame = new MainFrame(e.GetContext());
	wxTheApp->SetTopWindow(frame);
	frame->Show();
	++m_frameCounter;
}

void ManagerFrame::OnDestroyFrame(wxEvent &event) {
	//wxContextEvent &e = dynamic_cast<wxContextEvent &>(event);

	if (--m_frameCounter <= 0) {
		if (wxApp::GetInstance() != NULL) {
			wxTheApp->Exit();
			wxWakeUpIdle();
		}
	}
}
#endif


Application::Application()
	: m_frame(NULL) {
	SetAppName(wxT("lldebug debugger"));
}

Application::~Application() {
}

/*
static int wxToInt(const wxString &str) {
	int value = 0;

	for (size_t i = 0; i < str.Length(); ++i) {
		wxChar c = str[i];

		if (c < '0' || '9' < c) {
			return -1;
		}

		value = value * 10 + (c - '0');
	}

	return value;
}*/

bool Application::OnInit() {
	std::string hostName = wxConvToUTF8(this->argv[1]);
	std::string portName = wxConvToUTF8(this->argv[2]);

	if (m_mediator.Initialize(hostName, portName) != 0) {
		return false;
	}

	MainFrame* frame = new MainFrame();
	SetTopWindow(frame);
	frame->Show();

	wxLogWindow *log = new wxLogWindow(frame, wxT("Logger"), true);
	wxLog::SetActiveTarget(log);
	m_frame = frame;
    return true;
}

int Application::OnExit() {
	return 0;
}

}
