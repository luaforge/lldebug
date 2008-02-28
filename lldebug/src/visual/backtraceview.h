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

#ifndef __LLDEBUG_BACKTRACEVIEW_H__
#define __LLDEBUG_BACKTRACEVIEW_H__

#include "luainfo.h"
#include "visual/event.h"

#include "wx/treelistctrl.h"

namespace lldebug {
namespace visual {

class BacktraceViewItemData;

/**
 * @brief ローカル変数とその値を表示するコントロールです。
 */
class BacktraceView : public wxTreeListCtrl {
public:
	typedef std::vector<wxTreeItemId> wxTreeItemIdList;

public:
	explicit BacktraceView(wxWindow *parent);
	virtual ~BacktraceView();

	virtual wxTreeItemIdList GetItemChildren(const wxTreeItemId &item);
	virtual BacktraceViewItemData *GetItemData(const wxTreeItemId &item);

private:
	void CreateGUIControls();
	void BeginUpdating();
	bool IsSameContents(const LuaBacktraceList &backtraces);
	void DoUpdate(const LuaBacktraceList &backtraces);
	void LayoutColumn(int column);

	struct UpdateHandler;
	friend struct UpdateHandler;

private:
	void OnChangedState(wxDebugEvent &event);
	void OnUpdateSource(wxDebugEvent &event);
	void OnItemActivated(wxTreeEvent &event);
	void OnShow(wxShowEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnColEndDrag(wxListEvent &event);

private:
	DECLARE_EVENT_TABLE();
};

} // end of namespace visual
} // end of namespace lldebug

#endif
