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
#include "lldebug_mediator.h"
#include "lldebug_remoteengine.h"
#include "lldebug_watchview.h"

namespace lldebug {

class WatchViewItemData : public wxTreeItemData {
public:
	explicit WatchViewItemData(const LuaVar &var)
		: m_var(var), m_updateSourceCount(-1) {
	}

	virtual ~WatchViewItemData() {
	}

	const LuaVar &GetVar() const {
		return m_var;
	}

	int GetUpdateSourceCount() const {
		return m_updateSourceCount;
	}

	void SetUpdateSourceCount(int updateSourceCount) {
		m_updateSourceCount = updateSourceCount;
	}

private:
	LuaVar m_var;
	int m_updateSourceCount;
};

BEGIN_EVENT_TABLE(WatchView, wxTreeListCtrl)
	EVT_SIZE(WatchView::OnSize)
	EVT_SHOW(WatchView::OnShow)
	EVT_LLDEBUG_CHANGED_STATE(wxID_ANY, WatchView::OnChangedState)
	EVT_LLDEBUG_UPDATE_SOURCE(wxID_ANY, WatchView::OnUpdateSource)

	EVT_TREE_ITEM_EXPANDED(wxID_ANY, WatchView::OnExpanded)
	EVT_TREE_END_LABEL_EDIT(wxID_ANY, WatchView::OnEndLabelEdit)
	EVT_LIST_COL_END_DRAG(wxID_ANY, WatchView::OnColEndDrag)
END_EVENT_TABLE()

static int GetWatchViewId(WatchView::Type type) {
	switch (type) {
	case WatchView::TYPE_LOCALWATCH:
		return ID_LOCALWATCHVIEW;
	case WatchView::TYPE_GLOBALWATCH:
		return ID_GLOBALWATCHVIEW;
	case WatchView::TYPE_REGISTRYWATCH:
		return ID_REGISTRYWATCHVIEW;
	case WatchView::TYPE_ENVIRONWATCH:
		return ID_ENVIRONWATCHVIEW;
	case WatchView::TYPE_STACKWATCH:
		return ID_STACKWATCHVIEW;
	case WatchView::TYPE_WATCH:
		return ID_WATCHVIEW;
	}

	return -1;
}

WatchView::WatchView(wxWindow *parent, Type type)
	: wxTreeListCtrl(parent, GetWatchViewId(type)
		, wxDefaultPosition, wxDefaultSize
		, wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT
		| wxTR_EDIT_LABELS | wxTR_ROW_LINES | wxTR_COL_LINES
		| wxTR_FULL_ROW_HIGHLIGHT | wxALWAYS_SHOW_SB)
	, m_type(type) {
	CreateGUIControls();
}

WatchView::~WatchView() {
}

void WatchView::CreateGUIControls() {
	scoped_lock lock(m_mutex);

	if (m_type == TYPE_STACKWATCH) {
		AddColumn(_("Index"), 80, wxALIGN_LEFT, -1, true, true);
	}
	else {
		AddColumn(_("Name"), 80, wxALIGN_LEFT, -1, true, true);
	}

	AddColumn(_("Value"), 120, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Type"), 60, wxALIGN_LEFT, -1, true, true);
	SetLineSpacing(2);

	AddRoot(wxT(""), -1, -1, new WatchViewItemData(LuaVar()));
}

WatchView::wxTreeItemIdList WatchView::GetItemChildren(const wxTreeItemId &item) {
	scoped_lock lock(m_mutex);
	wxTreeItemIdList result;
	wxTreeItemIdValue cookie;

	for (wxTreeItemId child = GetFirstChild(item, cookie);
		child.IsOk();
		child = GetNextChild(item, cookie)) {
		result.push_back(child);
	}

	return result;
}

WatchViewItemData *WatchView::GetItemData(const wxTreeItemId &item) {
	scoped_lock lock(m_mutex);
	return static_cast<WatchViewItemData *>(wxTreeListCtrl::GetItemData(item));
}

class WatchView::UpdateVars {
public:
	explicit UpdateVars(WatchView *view, wxTreeItemId item, bool isExpand)
		: m_view(view), m_item(item), m_isExpand(isExpand) {
	}

	bool IsNeed() {
		WatchViewItemData *data = m_view->GetItemData(m_item);
		return (data->GetUpdateSourceCount() < Mediator::Get()->GetUpdateSourceCount());
	}

	void operator()(const Command_ &command, const LuaVarList &vars) {
		m_view->DoUpdateVars(m_item, vars, m_isExpand);
	}

private:
	WatchView *m_view;
	wxTreeItemId m_item;
	bool m_isExpand;
};

void WatchView::BeginUpdateVars(bool isExpand) {
	scoped_lock lock(m_mutex);
	UpdateVars callback = UpdateVars(this, GetRootItem(), isExpand);

	if (!callback.IsNeed()) {
		if (isExpand) {
			wxTreeItemIdList children = GetItemChildren(GetRootItem());

			wxTreeItemIdList::iterator it;
			for (it = children.begin(); it != children.end(); ++it) {
				WatchViewItemData *data = GetItemData(*it);
				BeginUpdateVars(*it, data->GetVar(), false);
			}
		}
		return;
	}

	switch (m_type) {
	case TYPE_LOCALWATCH:
		Mediator::Get()->GetEngine()->RequestLocalVarList(
			LuaStackFrame(LuaHandle(), 0), callback);
		break;
	case TYPE_GLOBALWATCH:
		Mediator::Get()->GetEngine()->RequestGlobalVarList(callback);
		break;
	case TYPE_REGISTRYWATCH:
		Mediator::Get()->GetEngine()->RequestRegistryVarList(callback);
		break;
	case TYPE_ENVIRONWATCH:
		Mediator::Get()->GetEngine()->RequestEnvironVarList(callback);
		break;
	case TYPE_STACKWATCH:
		Mediator::Get()->GetEngine()->RequestStackList(callback);
		break;
	case TYPE_WATCH:
		break;
	}
}

void WatchView::BeginUpdateVars(wxTreeItemId item, const LuaVar &var, bool isExpand) {
	scoped_lock lock(m_mutex);
	UpdateVars callback = UpdateVars(this, item, isExpand);

	if (callback.IsNeed()) {
		Mediator::Get()->GetEngine()->RequestFieldsVarList(var, callback);
	}
	else {
		if (isExpand) {
			wxTreeItemIdList children = GetItemChildren(item);

			wxTreeItemIdList::iterator it;
			for (it = children.begin(); it != children.end(); ++it) {
				WatchViewItemData *data = GetItemData(*it);
				BeginUpdateVars(*it, data->GetVar(), false);
			}
		}
	}
}

struct CompareItem {
	WatchView *watch;
	const LuaVar &var;

	explicit CompareItem(WatchView *watch_, const LuaVar &var_)
		: watch(watch_), var(var_) {
	}

	bool operator()(const wxTreeItemId &item) {
		WatchViewItemData *data = watch->GetItemData(item);
		return (data != NULL && data->GetVar().IsOk() ? data->GetVar() == var : false);
	}
};

void WatchView::DoUpdateVars(wxTreeItemId parent, const LuaVarList &vars, bool isExpand) {
	scoped_lock lock(m_mutex);
	// 既存の子アイテムリストで、アイテムの追加・削除時に使います。
	wxTreeItemIdList children = GetItemChildren(parent);

	for (LuaVarList::size_type i = 0; i < vars.size(); ++i) {
		const LuaVar &var = vars[i];
		wxTreeItemIdList::iterator it;
		wxTreeItemId item;

		// 子アイテムがすでに登録されているか探します。
		it = std::find_if(children.begin(), children.end(), CompareItem(this, var));
		if (it == children.end()) {
			item = AppendItem(parent, wxEmptyString, -1, -1, new WatchViewItemData(var));

			wxString name = wxConvFromUTF8(var.GetName());
			SetItemText(item, 0, name);
		}
		else {
			// 既存のアイテムがあったらそれを使います。
			item = *it;
			children.erase(it);

			// 古いデータは削除しないとリークします。
			WatchViewItemData *oldData = GetItemData(item);
			if (oldData != NULL) {
				delete oldData;
			}
			SetItemData(item, new WatchViewItemData(var));
		}

		// カラムを設定します。
		wxString value = wxConvFromUTF8(var.GetValue());
		wxString type = wxConvFromUTF8(var.GetValueTypeName());
		SetItemText(item, 1, value);
		SetItemText(item, 2, type);

		// 子アイテムがあればそれも更新します。
		if (var.HasFields()) {
			if (IsExpanded(item)) {
				BeginUpdateVars(item, var, true);
			}
			else if (isExpand) {
				BeginUpdateVars(item, var, false);
			}
			else if (!HasChildren(item)) {
				AppendItem(item, _T("$<item for lazy evalution>"));
			}
		}
		else {
			// アイテムが開かれているかの状態は子供がなくなっても保存されます。
			// なので一応閉じておきます。
			Collapse(item);

			// アイテムが無いなら子アイテムを消します。
			DeleteChildren(item);
		}
	}

	// 子アイテムに残っているアイテムは削除対象になります。
	for (wxTreeItemIdList::iterator it = children.begin();
		it != children.end();
		++it) {
		Delete(*it);
	}

	// Set updated mark.
	WatchViewItemData *data = GetItemData(parent);
	data->SetUpdateSourceCount(Mediator::Get()->GetUpdateSourceCount());
}

void WatchView::OnChangedState(wxChangedStateEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	Enable(event.IsBreak());
}

void WatchView::OnUpdateSource(wxSourceLineEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (IsEnabled() && IsShown()) {
		BeginUpdateVars(true);
	}
}

void WatchView::OnShow(wxShowEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (event.GetShow() && IsEnabled() && IsShown()) {
		BeginUpdateVars(true);
	}
}

void WatchView::OnExpanded(wxTreeEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	WatchViewItemData *data = GetItemData(event.GetItem());
	BeginUpdateVars(event.GetItem(), data->GetVar(), true);
}

void WatchView::OnEndLabelEdit(wxTreeEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (m_type == TYPE_WATCH) {
		//wxTreeItemId item = event.GetItem();
		//WatchViewItemData *data = GetItemData(item);
	}
}

void WatchView::LayoutColumn(int selectedColumn) {
	scoped_lock lock(m_mutex);

	// 合計カラムサイズを取得します。
	int col_w = 0;
	int sel_w = 0;
	for (int i = 0; i < GetColumnCount(); ++i) {
		if (i <= selectedColumn && i != GetColumnCount() - 1) {
			sel_w += GetColumnWidth(i); 
		}
		else {
			col_w += GetColumnWidth(i);
		}
	}
	
	// 小さすぎる場合はエラーとして扱います。
	int width = GetClientSize().GetWidth();
				//- wxSystemSettings::GetMetric(wxSYS_VSCROLL_X) - 2;
	double rate = (double)(width - sel_w) / col_w;
	if (rate < 0.0001) {
		return;
	}

	// カラムサイズを同じ比率だけ変化させます。
	for (int i = 0; i < GetColumnCount(); ++i) {
		if (i <= selectedColumn && i != GetColumnCount() - 1) {
			continue;
		}

		int w = GetColumnWidth(i);
		SetColumnWidth(i, (int)(w * rate));
	}
}

void WatchView::OnSize(wxSizeEvent &event) {
	event.Skip();
	LayoutColumn(-1);
}

void WatchView::OnColEndDrag(wxListEvent &event) {
	event.Skip();
	LayoutColumn(event.GetColumn());
}

}
