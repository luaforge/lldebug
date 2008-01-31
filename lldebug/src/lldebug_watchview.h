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
	explicit WatchView(wxWindow *parent, Type type);
	virtual ~WatchView();

	virtual wxTreeItemIdList GetItemChildren(const wxTreeItemId &item);
	virtual WatchViewItemData *GetItemData(const wxTreeItemId &item);

private:
	void CreateGUIControls();
	void LayoutColumn(int selectedColumn);

	class UpdateVars;
	friend class UpdateVars;
	void BeginUpdateVars(bool isExpand);
	void BeginUpdateVars(wxTreeItemId item, const LuaVar &var, bool isExpand);
	void DoUpdateVars(wxTreeItemId parent, const LuaVarList &vars, bool isExpand);

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
	Type m_type;
};

}

#endif
