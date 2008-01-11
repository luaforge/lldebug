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
#include "lldebug_bootstrap.h"
#include "lldebug_mainframe.h"

IMPLEMENT_APP_NO_MAIN(lldebug::Application)

namespace lldebug {

BEGIN_EVENT_TABLE(ManagerFrame, wxFrame)
	EVT_CUSTOM(wxEVT_CREATE_FRAME, ID_MANAGER_FRAME, ManagerFrame::OnCreateFrame)
	EVT_CUSTOM(wxEVT_DESTROY_FRAME, ID_MANAGER_FRAME, ManagerFrame::OnDestroyFrame)
END_EVENT_TABLE()

ManagerFrame::ManagerFrame()
	: wxFrame(NULL, ID_MANAGER_FRAME, wxT(""), wxPoint(20000,0), wxSize(1,1))
	, m_frameCounter(0) {
}

ManagerFrame::~ManagerFrame() {
}

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


/* --------------------------------------------------------------- */
Application::Application()
	: m_frame(NULL) {
	SetAppName(wxT("LLDebug"));
}

Application::~Application() {
}

bool Application::OnInit() {
	class InitResult {
	public:
		explicit InitResult()
			: m_successed(false) {
		}
		~InitResult() {
			Bootstrap::Get()->Success(m_successed);
		}
		void Success() {
			m_successed = true;
		}
	private:
		bool m_successed;
	}
	resultObj;

    ManagerFrame* frame = new ManagerFrame();
	SetTopWindow(frame);
	//frame->Show();
	m_frame = frame;

	wxLogWindow *log = new wxLogWindow(frame, wxT("Logger"), true);
	wxLog::SetActiveTarget(log);

	resultObj.Success();
    return true;
}

int Application::OnExit() {
	return 0;
}

void SendCreateFrameEvent(Context *ctx) {
	assert(wxApp::GetInstance() != NULL);
	assert(wxGetApp().GetFrame() != NULL);
	ManagerFrame *frame = wxGetApp().GetFrame();

	wxContextEvent event(wxEVT_CREATE_FRAME, frame->GetId(), ctx);
	frame->AddPendingEvent(event);
}

void SendDestroyedFrameEvent(Context *ctx) {
	assert(wxApp::GetInstance() != NULL);
	assert(wxGetApp().GetFrame() != NULL);
	ManagerFrame *frame = wxGetApp().GetFrame();

	wxContextEvent event(wxEVT_DESTROY_FRAME, frame->GetId(), ctx);
	frame->AddPendingEvent(event);
}

}
