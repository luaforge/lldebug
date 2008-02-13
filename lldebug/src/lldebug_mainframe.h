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

#ifndef __LLDEBUG_MAINFRAME_H__
#define __LLDEBUG_MAINFRAME_H__

#include "lldebug_controls.h"
#include <wx/aui/aui.h>
#include <wx/aui/auibook.h>

namespace lldebug {

/**
 * @brief メインウィンドウです。
 */
class MainFrame : public wxFrame {
public:
	explicit MainFrame();
	virtual ~MainFrame();

	void AddPendingDebugEvent(wxEvent &event, wxWindow *parent, bool sendAlways);
	void ProcessDebugEvent(wxEvent &event, wxWindow *parent, bool sendAlways);
	bool IsExistDebugWindow(int wintypeid);
	void ShowDebugWindow(int wintypeid);

private:
	void CreateGUIControls();
	void OnIdle(wxIdleEvent &event);
	void OnMenu(wxCommandEvent &event);
	wxAuiNotebook *GetAuiNotebook();

private:
	DECLARE_EVENT_TABLE();
	//DECLARE_DYNAMIC_CLASS(wxFrame);
		
private:
	mutex m_mutex;

	wxAuiManager *m_auiManager;
	SourceView *m_sourceView;
};

}

#endif
