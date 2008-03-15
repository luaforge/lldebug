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
#include "visual/mediator.h"
#include "visual/mainframe.h"
#include "visual/application.h"
#include "visual/strutils.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>

IMPLEMENT_APP(lldebug::visual::Application)

namespace lldebug {
namespace visual {

Application::Application()
	: m_mediator(new Mediator), m_locale(wxLANGUAGE_DEFAULT) {
	SetAppName(wxT("lldebug frame"));
}

Application::~Application() {
}

bool Application::OnInit() {
#ifdef __WXMSW__
	wxString path = wxFileName(argv[0]).GetPath(wxPATH_GET_VOLUME);
	path += _T("\\..\\..\\po");
	wxLocale::AddCatalogLookupPathPrefix(path);
#endif

	// Initialize the catalogs we'll be using
	//m_locale.AddCatalog(wxT("lldebug"));

	// This catalog is installed in standard location on Linux systems and
	// shows that you may make use of the standard message catalogs as well
	// 
	// if it's not installed on your system, it is just silently ignored
#ifdef __LINUX__
	{
		wxLogNull noLog;
		m_locale.AddCatalog(_T("fileutils"));
	}
#endif

	// Get port number, default value was decided randomly.
	unsigned short port = 24752;
	if (this->argc > 1) {
		int tmpNum = ToPortNumber(this->argv[1]);
		if (tmpNum == -1) {
			return false;
		}

		port = (unsigned short)tmpNum;
	}

	// Start to accept as a debug server.
	if (m_mediator->Initialize(port) != 0) {
		return false;
	}

	MainFrame *frame = new MainFrame();
	SetTopWindow(frame);
	frame->Show();
	m_mediator->SetMainFrame(frame);

	wxLogWindow *log = new wxLogWindow(frame, _("lldebug logger"), true);
	wxLog::SetActiveTarget(log);

	/*wxString dir = wxStandardPaths().GetLocalizedResourcesDir(wxT("ja"), wxStandardPaths::ResourceCat_Messages);
	wxLogMessage(_T("%s"), dir.c_str());
	wxLogMessage(_T("%s"), path.c_str());*/
    return true;
}

int Application::OnExit() {
	return 0;
}

} // end of namespace visual
} // end of namespace lldebug
