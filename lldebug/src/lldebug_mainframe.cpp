//---------------------------------------------------------------------------
//
// Name:        MainFrame.cpp
// Author:      雅博
// Created:     2007/11/23 0:05:33
// Description: MainFrame class implementation
//
//---------------------------------------------------------------------------

#include "lldebug_prec.h"
#include "lldebug_context.h"
#include "lldebug_mainframe.h"
#include "lldebug_sourceview.h"
#include "lldebug_interactview.h"
#include "lldebug_watchview.h"
#include "lldebug_backtraceview.h"

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
	EVT_CLOSE(MainFrame::OnClose)

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

MainFrame::MainFrame(Context *ctx)
	: wxFrame(NULL, ID_MAINFRAME + ctx->GetId(), wxT("Lua Debugger")
	, wxDefaultPosition, wxSize(800, 600)
	, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX)
	, m_ctx(ctx), m_auiManager(NULL), m_auiNotebook(NULL) {
	scoped_lock lock(m_mutex);

	CreateGUIControls();
	m_ctx->SetFrame(this); // フレームの初期化後に設定します。
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
	m_ctx->SetFrame(NULL);
}

void MainFrame::CreateGUIControls() {
	scoped_lock lock(m_mutex);
	SetFont(wxFont(11, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("MS Gothic")));

	m_auiManager = new wxAuiManager(this);

	m_sourceView = new SourceView(m_ctx, this);
	m_auiManager->AddPane(m_sourceView, wxAuiPaneInfo()
		.Name(wxT("SourceView")).Caption(wxT("SourceView"))
		.CentrePane().Floatable(false).PaneBorder(false));

	ShowView(ID_INTERACTVIEW);
	ShowView(ID_GLOBALWATCHVIEW);
	ShowView(ID_REGISTRYWATCHVIEW);
	//ShowView(ID_ENVIRONWATCHVIEW);
	ShowView(ID_STACKWATCHVIEW);
	//ShowView(ID_WATCHVIEW);
	ShowView(ID_BACKTRACEVIEW);
	ShowView(ID_LOCALWATCHVIEW);

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
	debugMenu->Append(ID_MENU_TOGGLE_BREAKPOINT, _("&Toggle BreakPoint\tF9"));

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

void MainFrame::AddPendingLLDebugEvent(wxEvent &event, wxWindow *parent, bool sendAlways) {
	scoped_lock lock(m_mutex);

	if (!sendAlways && !parent->IsShown()) {
		return;
	}

	wxWindowList& children = parent->GetChildren();
	for (size_t i = 0; i < children.GetCount(); ++i) {
		AddPendingLLDebugEvent(event, children[i], sendAlways);
	}

	event.SetId(parent->GetId());
	parent->AddPendingEvent(event);
}

void MainFrame::ChangedState(bool isBreak) {
	scoped_lock lock(m_mutex);

	wxChangedStateEvent event(wxEVT_CHANGED_STATE, wxID_ANY, isBreak);
	AddPendingLLDebugEvent(event, this, true);
}

void MainFrame::UpdateSource(const std::string &key, int line) {
	scoped_lock lock(m_mutex);

	// Get source title from key.
	std::string title;
	const Source *source = m_ctx->GetSource(key);
	if (source != NULL) {
		title = source->GetTitle();
	}

	wxSourceLineEvent event(
		wxEVT_UPDATE_SOURCE, wxID_ANY,
		m_ctx, NULL, true, key, title, line, 0);
	AddPendingLLDebugEvent(event, this, false);
}

void MainFrame::SetViewSource(const std::string &key, int line, lua_State *L, int level) {
	scoped_lock lock(m_mutex);

	// Get source title from key.
	std::string title;
	const Source *source = m_ctx->GetSource(key);
	if (source != NULL) {
		title = source->GetTitle();
	}

	wxSourceLineEvent event(
		wxEVT_UPDATE_SOURCE, wxID_ANY,
		m_ctx, L, false, key, title, line, level);
	AddPendingLLDebugEvent(event, this, false);
}

void MainFrame::OnClose(wxCloseEvent &event) {
	scoped_lock lock(m_mutex);

	SendDestroyedFrameEvent(m_ctx);
	event.Skip();
}

bool MainFrame::IsExistWindow(int wintypeid) {
	scoped_lock lock(m_mutex);

	return (FindWindowById(wintypeid + m_ctx->GetId()) != NULL);
}

void MainFrame::ShowView(int wintypeid) {
	scoped_lock lock(m_mutex);

	// If a notebook is hidden, we have to close and recreate it.
	if (m_auiNotebook != NULL && !m_auiNotebook->IsShown()) {
		m_auiManager->DetachPane(m_auiNotebook);
		m_auiNotebook->Close();
		delete m_auiNotebook;
		m_auiNotebook = NULL;
	}

	// find and create a notebook window
	if (m_auiNotebook == NULL || !IsExistWindow(ID_WINDOWHOLDER)) {
		m_auiNotebook = new wxAuiNotebook(
			this, ID_WINDOWHOLDER + m_ctx->GetId(),
			wxDefaultPosition, wxSize(100, 300),
			wxAUI_NB_TOP | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_SPLIT
				| wxAUI_NB_CLOSE_ON_ACTIVE_TAB);

		m_auiManager->AddPane(m_auiNotebook, wxAuiPaneInfo()
			.Name(wxT("Controls")).Caption(wxT("Controls")).Bottom());
		m_auiManager->Update();
	}

	// If the view is already exist, do nothing.
	if (IsExistWindow(wintypeid)) {
		return;
	}

	switch (wintypeid) {
	case ID_INTERACTVIEW:
		m_auiNotebook->AddPage(
			new InteractView(m_ctx, this),
			_("InteractView"));
		break;
	case ID_LOCALWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(m_ctx, this, WatchView::TYPE_LOCALWATCH),
			_("LocalWatch"));
		break;
	case ID_GLOBALWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(m_ctx, this, WatchView::TYPE_GLOBALWATCH),
			_("GlobalWatch"));
		break;
	case ID_REGISTRYWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(m_ctx, this, WatchView::TYPE_REGISTRYWATCH),
			_("RegistryWatch"));
		break;
	/*case ID_ENVIRONWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(m_ctx, this, WatchView::TYPE_ENVIRONWATCH),
			_("EnvironWatch"));
		break;*/
	case ID_STACKWATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(m_ctx, this, WatchView::TYPE_STACKWATCH),
			_("StackWatch"));
		break;
	case ID_WATCHVIEW:
		m_auiNotebook->AddPage(
			new WatchView(m_ctx, this, WatchView::TYPE_WATCH),
			_("Watch"));
		break;
	case ID_BACKTRACEVIEW:
		m_auiNotebook->AddPage(
			new BackTraceView(m_ctx, this),
			_("BackTrace"));
		break;
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
		m_ctx->PushCommand(Command::TYPE_PAUSE);
		break;
	case ID_MENU_RESTART:
		m_ctx->PushCommand(Command::TYPE_RESTART);
		break;
	case ID_MENU_STEPOVER:
		m_ctx->PushCommand(Command::TYPE_STEPOVER);
		break;
	case ID_MENU_STEPINTO:
		m_ctx->PushCommand(Command::TYPE_STEPINTO);
		break;
	case ID_MENU_STEPRETURN:
		m_ctx->PushCommand(Command::TYPE_STEPRETURN);
		break;
	case ID_MENU_TOGGLE_BREAKPOINT:
		m_sourceView->ToggleBreakPoint();
		break;

	case ID_MENU_SHOW_LOCALWATCH:
		ShowView(ID_LOCALWATCHVIEW);
		break;
	case ID_MENU_SHOW_GLOBALWATCH:
		ShowView(ID_GLOBALWATCHVIEW);
		break;
	case ID_MENU_SHOW_REGISTRYWATCH:
		ShowView(ID_REGISTRYWATCHVIEW);
		break;
	case ID_MENU_SHOW_ENVIRONWATCH:
		ShowView(ID_ENVIRONWATCHVIEW);
		break;
	case ID_MENU_SHOW_STACKWATCH:
		ShowView(ID_STACKWATCHVIEW);
		break;
	case ID_MENU_SHOW_WATCH:
		ShowView(ID_WATCHVIEW);
		break;
	case ID_MENU_SHOW_BACKTRACEVIEW:
		ShowView(ID_BACKTRACEVIEW);
		break;
	case ID_MENU_SHOW_INTERACTVIEW:
		ShowView(ID_INTERACTVIEW);
		break;
	}
}

}
