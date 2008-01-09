//---------------------------------------------------------------------------
//
// Name:        MainFrame.h
// Author:      雅博
// Created:     2007/11/23 0:05:33
// Description: MainFrame class declaration
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_SOURCEVIEW_H__
#define __LLDEBUG_SOURCEVIEW_H__

#include "lldebug_controls.h"
#include <wx/aui/auibook.h>

namespace lldebug {

class SourceViewPage;

/**
 * @brief ソースコードを表示するコントロールです。
 */
class SourceView : public wxAuiNotebook {
public:
	explicit SourceView(Context *ctx, wxWindow *parent);
	virtual ~SourceView();

	void ToggleBreakPoint();

private:
	void CreateGUIControls();
	void OnChangedState(wxChangedStateEvent &event);
	void OnUpdateSource(wxSourceLineEvent &event);

	size_t FindPageFromKey(const std::string &key);
	SourceViewPage *GetPage(size_t i);
	SourceViewPage *GetSelected();

	DECLARE_EVENT_TABLE();

private:
	mutex m_mutex;
	Context *m_ctx;
};

}

#endif
