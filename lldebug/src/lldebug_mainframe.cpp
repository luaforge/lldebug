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
#include "lldebug_remoteengine.h"
#include "lldebug_mainframe.h"
#include "lldebug_sourceview.h"
#include "lldebug_outputview.h"
//#include "lldebug_interactview.h"
#include "lldebug_watchview.h"
//#include "lldebug_backtraceview.h"

namespace lldebug {

enum {
	ID_MENU_BREAK,
	ID_MENU_RESTART,
	ID_MENU_STEPOVER,
	ID_MENU_STEPINTO,
	ID_MENU_STEPRETURN,
	ID_MENU_TOGGLE_BREAKPOINT,

	ID_MENU_SHOW_LOCALWATCH,
	ID_MENU_SHOW_GLOBALWATCH,
	ID_MENU_SHOW_REGISTRYWATCH,
	ID_MENU_SHOW_ENVIRONWATCH,
	ID_MENU_SHOW_WATCH,
	ID_MENU_SHOW_STACKWATCH,
	ID_MENU_SHOW_BACKTRACEVIEW,
	ID_MENU_SHOW_INTERACTVIEW,
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MainFrame::OnMenu)

	EVT_MENU(ID_MENU_BREAK, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_RESTART, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_STEPOVER, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_STEPINTO, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_STEPRETURN, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_TOGGLE_BREAKPOINT, MainFrame::OnMenu)

	EVT_MENU(ID_MENU_SHOW_LOCALWATCH, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_SHOW_GLOBALWATCH, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_SHOW_REGISTRYWATCH, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_SHOW_ENVIRONWATCH, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_SHOW_WATCH, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_SHOW_STACKWATCH, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_SHOW_BACKTRACEVIEW, MainFrame::OnMenu)
	EVT_MENU(ID_MENU_SHOW_INTERACTVIEW, MainFrame::OnMenu)
END_EVENT_TABLE()

MainFrame::MainFrame()
	: wxFrame(NULL, ID_MAINFRAME
		, wxT("Lua Debugger")
		, wxDefaultPosition, wxSize(800, 600)
		, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX)
	, m_auiManager(NULL), m_auiNotebook(NULL) {
	CreateGUIControls();
}

MainFrame::~MainFrame() {
	if (m_auiManager != NULL) {
		m_auiManager->UnInit();

		delete m_auiManager;
		m_auiManager = NULL;
	}

	// 子ウィンドウを削除した後、SetFrame(NULL)を行います。
	DeleteAllBars();
	DestroyChildren();
	Mediator::Get()->SetMainFrame(NULL);
}

void MainFrame::CreateGUIControls() {
	scoped_lock lock(m_mutex);
	SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("MS Gothic")));

	m_auiManager = new wxAuiManager(this);

	m_sourceView = new SourceView(this);
	m_auiManager->AddPane(m_sourceView, wxAuiPaneInfo()
		.Name(wxT("SourceView")).Caption(wxT("SourceView"))
		.CentrePane().Floatable(false).PaneBorder(false));

	ShowDebugWindow(ID_OUTPUTVIEW);
	ShowDebugWindow(ID_INTERACTVIEW);
	ShowDebugWindow(ID_GLOBALWATCHVIEW);
	ShowDebugWindow(ID_REGISTRYWATCHVIEW);
	//ShowDebugWindow(ID_ENVIRONWATCHVIEW);
	ShowDebugWindow(ID_STACKWATCHVIEW);
	//ShowDebugWindow(ID_WATCHVIEW);
	ShowDebugWindow(ID_BACKTRACEVIEW);
	ShowDebugWindow(ID_LOCALWATCHVIEW);

	wxMenu *fileMenu = new wxMenu;
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _("&Exit\tShift+F5"));

	wxMenu *viewMenu = new wxMenu;
	viewMenu->Append(ID_MENU_SHOW_LOCALWATCH, _("&LocalWatch"));
	viewMenu->Append(ID_MENU_SHOW_GLOBALWATCH, _("&GlobalWatch"));
	viewMenu->Append(ID_MENU_SHOW_REGISTRYWATCH, _("&RegistryWatch"));
	viewMenu->Append(ID_MENU_SHOW_ENVIRONWATCH, _("&EnvironWatch"));
	viewMenu->Append(ID_MENU_SHOW_WATCH, _("&Watch"));
	viewMenu->Append(ID_MENU_SHOW_STACKWATCH, _("&StackWatch"));
	viewMenu->Append(ID_MENU_SHOW_BACKTRACEVIEW, _("&BacktraceView"));
	viewMenu->Append(ID_MENU_SHOW_INTERACTVIEW, _("&InteractView"));
	
	wxMenu *debugMenu = new wxMenu;
	debugMenu->Append(ID_MENU_BREAK, _("&Break\tShift+Pause"));
	debugMenu->Append(ID_MENU_RESTART, _("&Restart\tF5"));
	debugMenu->AppendSeparator();
	debugMenu->Append(ID_MENU_STEPINTO, _("Step &Into\tF7"));
	debugMenu->Append(ID_MENU_STEPOVER, _("Step &Over\tF6"));
	debugMenu->Append(ID_MENU_STEPRETURN, _("Step Return\tF8"));
	debugMenu->AppendSeparator();
	debugMenu->Append(ID_MENU_TOGGLE_BREAKPOINT, _("&Toggle Breakpoint\tF9"));

	wxMenuBar *menuBar = new wxMenuBar(wxMB_DOCKABLE);
	menuBar->Append(fileMenu, _("&File"));
	menuBar->Append(viewMenu, _("&View"));
	menuBar->Append(debugMenu, _("&Debug"));
	SetMenuBar(menuBar);

	m_auiManager->Update();
	SetIcon(wxNullIcon);
	SetAutoLayout(true);
	Center();
}

void MainFrame::AddPendingDebugEvent(wxEvent &event, wxWindow *parent, bool sendAlways) {
	scoped_lock lock(m_mutex);

	if (!sendAlways && !parent->IsShown()) {
		return;
	}

	wxWindowList& children = parent->GetChildren();
	for (size_t i = 0; i < children.GetCount(); ++i) {
		AddPendingDebugEvent(event, children[i], sendAlways);
	}

	event.SetId(parent->GetId());
	parent->AddPendingEvent(event);
}

bool MainFrame::IsExistDebugWindow(int wintypeid) {
	scoped_lock lock(m_mutex);

	return (FindWindowById(wintypeid) != NULL);
}

void MainFrame::ShowDebugWindow(int wintypeid) {
	scoped_lock lock(m_mutex);

	// If a notebook is hidden, we have to close and recreate it.
	if (m_auiNotebook != NULL && !m_auiNotebook->IsShown()) {
		m_auiManager->DetachPane(m_auiNotebook);
		m_auiNotebook->Close();
		delete m_auiNotebook;
		m_auiNotebook = NULL;
	}

	// find and create a notebook window
	if (m_auiNotebook == NULL || !IsExistDebugWindow(ID_WINDOWHOLDER)) {
		m_auiNotebook = new wxAuiNotebook(
			this, ID_WINDOWHOLDER,
			wxDefaultPosition, wxSize(100, 300),
			wxAUI_NB_TOP | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_SPLIT
				| wxAUI_NB_CLOSE_ON_ACTIVE_TAB);

		m_auiManager->AddPane(m_auiNotebook, wxAuiPaneInfo()
			.Name(wxT("Controls")).Caption(wxT("Controls")).Bottom());
		m_auiManager->Update();
	}

	// If the view is already exist, do nothing.
	if (IsExistDebugWindow(wintypeid)) {
		return;
	}

	switch (wintypeid) {
	/*case ID_INTERACTVIEW:
		m_auiNotebook->AddPage(
			new InteractView(m_ctx, this),
			_("InteractView"));
		break;*/
	case ID_OUTPUTVIEW:
		m_auiNotebook->AddPage(
			new OutputView(this),
			_("OutputView"));
		break;
	case ID_LOCALWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(this, WatchView::TYPE_LOCALWATCH),
			_("LocalWatch"));
		break;
	case ID_GLOBALWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(this, WatchView::TYPE_GLOBALWATCH),
			_("GlobalWatch"));
		break;
	case ID_REGISTRYWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(this, WatchView::TYPE_REGISTRYWATCH),
			_("RegistryWatch"));
		break;
	case ID_STACKWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(this, WatchView::TYPE_STACKWATCH),
			_("StackWatch"));
		break;
	/*case ID_WATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(m_ctx, this, WatchView::TYPE_WATCH),
			_("Watch"));
		break;
	case ID_BACKTRACEVIEW:
		m_auiNotebook->AddPage(
			new BackTraceView(m_ctx, this),
			_("BackTrace"));
		break;*/
	default:
		return;
	}

	// Select and show new page.
	if (m_auiNotebook->GetPageCount() > 0) {
		m_auiNotebook->SetSelection(m_auiNotebook->GetPageCount() - 1);
	}
}

void MainFrame::OnMenu(wxCommandEvent &event) {
	scoped_lock lock(m_mutex);

	switch (event.GetId()) {
	case wxID_EXIT:
		Close(true);
		break;
	case ID_MENU_BREAK:
		Mediator::Get()->GetEngine()->Break();
		break;
	case ID_MENU_RESTART:
		Mediator::Get()->GetEngine()->Resume();
		break;
	case ID_MENU_STEPOVER:
		Mediator::Get()->GetEngine()->StepOver();
		break;
	case ID_MENU_STEPINTO:
		Mediator::Get()->GetEngine()->StepInto();
		break;
	case ID_MENU_STEPRETURN:
		Mediator::Get()->GetEngine()->StepReturn();
		break;
	case ID_MENU_TOGGLE_BREAKPOINT:
		m_sourceView->ToggleBreakpoint();
		break;

	case ID_MENU_SHOW_LOCALWATCH:
		ShowDebugWindow(ID_LOCALWATCHVIEW);
		break;
	case ID_MENU_SHOW_GLOBALWATCH:
		ShowDebugWindow(ID_GLOBALWATCHVIEW);
		break;
	case ID_MENU_SHOW_REGISTRYWATCH:
		ShowDebugWindow(ID_REGISTRYWATCHVIEW);
		break;
	case ID_MENU_SHOW_ENVIRONWATCH:
		ShowDebugWindow(ID_ENVIRONWATCHVIEW);
		break;
	case ID_MENU_SHOW_STACKWATCH:
		ShowDebugWindow(ID_STACKWATCHVIEW);
		break;
	case ID_MENU_SHOW_WATCH:
		ShowDebugWindow(ID_WATCHVIEW);
		break;
	case ID_MENU_SHOW_BACKTRACEVIEW:
		ShowDebugWindow(ID_BACKTRACEVIEW);
		break;
	case ID_MENU_SHOW_INTERACTVIEW:
		ShowDebugWindow(ID_INTERACTVIEW);
		break;
	}
}

}
