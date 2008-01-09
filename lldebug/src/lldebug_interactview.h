//---------------------------------------------------------------------------
//
// Name:        MainFrame.h
// Author:      雅博
// Created:     2007/11/23 0:05:33
// Description: MainFrame class declaration
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_INTERACTVIEW_H__
#define __LLDEBUG_INTERACTVIEW_H__

#include "lldebug_controls.h"

namespace lldebug {

/**
 * @brief ソースコードを表示するコントロールです。
 */
class InteractView : public wxPanel {
public:
	explicit InteractView(Context *ctx, wxWindow *parent);
	virtual ~InteractView();

	void Run();

private:
	void CreateGUIControls();
	void OnChangedState(wxChangedStateEvent &event);

	DECLARE_EVENT_TABLE();

private:
	mutex m_mutex;
	Context *m_ctx;

	wxTextCtrl *m_text;

	class TextInput;
	TextInput *m_input;

	class RunButton;
	RunButton *m_run;
};

}

#endif
