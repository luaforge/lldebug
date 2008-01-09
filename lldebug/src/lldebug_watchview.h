//---------------------------------------------------------------------------
//
// Name:        MainFrame.h
// Author:      雅博
// Created:     2007/11/23 0:05:33
// Description: MainFrame class declaration
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_WATCHVIEW_H__
#define __LLDEBUG_WATCHVIEW_H__

#include "lldebug_controls.h"
#include "lldebug_luainfo.h"
#include <wx/treelistctrl.h>

namespace lldebug {

class WatchViewItemData;

/**
 * @brief ローカル変数とその値を表示するコントロールです。
 */
class WatchView : public wxTreeListCtrl {
public:
	enum Type {
		TYPE_LOCALWATCH,
		TYPE_GLOBALWATCH,
		TYPE_REGISTRYWATCH,
		TYPE_ENVIRONWATCH,
		TYPE_STACKWATCH,
		TYPE_WATCH,
	};

	typedef std::vector<wxTreeItemId> wxTreeItemIdList;

public:
	explicit WatchView(Context *ctx, wxWindow *parent, Type type);
	virtual ~WatchView();

	virtual wxTreeItemIdList GetItemChildren(const wxTreeItemId &item);
	virtual WatchViewItemData *GetItemData(const wxTreeItemId &item);

private:
	void CreateGUIControls();
	void UpdateVars(wxTreeItemId parent, const LuaVarList &vars);
	LuaVarList GetLuaVarList(lua_State *L, int level);
	void LayoutColumn(int selectedColumn);

private:
	void OnChangedState(wxChangedStateEvent &event);
	void OnUpdateSource(wxSourceLineEvent &event);
	void OnShow(wxShowEvent &event);
	void OnExpanded(wxTreeEvent &event);
	void OnEndLabelEdit(wxTreeEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnColEndDrag(wxListEvent &event);

	DECLARE_EVENT_TABLE();

private:
	mutex m_mutex;
	Context *m_ctx;
	Type m_type;
	lua_State *m_lua;
	int m_level;
};

}

#endif
