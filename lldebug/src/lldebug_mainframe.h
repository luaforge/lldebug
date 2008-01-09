//---------------------------------------------------------------------------
//
// Name:        MainFrame.h
// Author:      雅博
// Created:     2007/11/23 0:05:33
// Description: MainFrame class declaration
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_DEBUGFRAME_H__
#define __LLDEBUG_DEBUGFRAME_H__

#include "lldebug_controls.h"
#include <wx/aui/aui.h>
#include <wx/aui/auibook.h>

namespace lldebug {

/**
 * @brief メインウィンドウです。
 */
class MainFrame : public wxFrame {
public:
	explicit MainFrame(Context *ctx);
	virtual ~MainFrame();

	void ChangedState(bool isBreak);
	void UpdateSource(const std::string &key, int line);
	void SetViewSource(const std::string &key, int line, lua_State *L, int level);

private:
	void CreateGUIControls();
	void AddPendingLLDebugEvent(wxEvent &event, wxWindow *parent, bool sendAlways);
	bool IsExistWindow(int wintypeid);
	void ShowView(int wintypeid);
	void OnClose(wxCloseEvent &event);
	void OnMenu(wxCommandEvent &event);

	DECLARE_EVENT_TABLE();
		
private:
	Context *m_ctx;
	mutex m_mutex;

	wxAuiManager *m_auiManager;
	wxAuiNotebook *m_auiNotebook;
	SourceView *m_sourceView;
};

}

#endif
