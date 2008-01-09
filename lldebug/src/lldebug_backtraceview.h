//---------------------------------------------------------------------------
//
// Name:        MainFrame.h
// Author:      雅博
// Created:     2007/11/23 0:05:33
// Description: MainFrame class declaration
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_BACKTRACEVIEW_H__
#define __LLDEBUG_BACKTRACEVIEW_H__

#include "lldebug_controls.h"
#include "lldebug_luainfo.h"
#include "wx/treelistctrl.h"

namespace lldebug {

class BackTraceViewItemData;

/**
 * @brief ローカル変数とその値を表示するコントロールです。
 */
class BackTraceView : public wxTreeListCtrl {
public:
	explicit BackTraceView(Context *ctx, wxWindow *parent);
	virtual ~BackTraceView();

	virtual BackTraceViewItemData *GetItemData(const wxTreeItemId &item);

private:
	void CreateGUIControls();
	void UpdateBackTrace();
	void LayoutColumn(int column);

private:
	void OnChangedState(wxChangedStateEvent &event);
	void OnUpdateSource(wxSourceLineEvent &event);
	void OnItemActivated(wxTreeEvent &event);
	void OnShow(wxShowEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnColEndDrag(wxListEvent &event);

	DECLARE_EVENT_TABLE();

private:
	mutex m_mutex;
	Context *m_ctx;
};

}

#endif
