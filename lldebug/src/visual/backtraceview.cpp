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

#include "precomp.h"
#include "visual/mediator.h"
#include "visual/backtraceview.h"
#include "visual/strutils.h"

namespace lldebug {
namespace visual {

/**
 * @brief 
 */
class BacktraceViewItemData : public wxTreeItemData {
public:
	explicit BacktraceViewItemData(const LuaBacktrace &bt)
		: m_backtrace(bt) {
	}

	virtual ~BacktraceViewItemData() {
	}

	/// Get the backtrace info.
	const LuaBacktrace &GetBacktrace() const {
		return m_backtrace;
	}

private:
	LuaBacktrace m_backtrace;
};

BEGIN_EVENT_TABLE(BacktraceView, wxTreeListCtrl)
	EVT_SIZE(BacktraceView::OnSize)
	EVT_SHOW(BacktraceView::OnShow)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, BacktraceView::OnItemActivated)
	EVT_LIST_COL_END_DRAG(wxID_ANY, BacktraceView::OnColEndDrag)
	EVT_LLDEBUG_CHANGED_STATE(ID_BACKTRACEVIEW, BacktraceView::OnChangedState)
	EVT_LLDEBUG_UPDATE_SOURCE(ID_BACKTRACEVIEW, BacktraceView::OnUpdateSource)
END_EVENT_TABLE()

BacktraceView::BacktraceView(wxWindow *parent)
	: wxTreeListCtrl(parent, ID_BACKTRACEVIEW
		, wxDefaultPosition, wxDefaultSize
		, wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT
		| wxTR_ROW_LINES | wxTR_COL_LINES
		| wxTR_FULL_ROW_HIGHLIGHT | wxALWAYS_SHOW_SB) {
	CreateGUIControls();
}

BacktraceView::~BacktraceView() {
}

void BacktraceView::CreateGUIControls() {
	AddColumn(_("File"), 80, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Line"), 40, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Function"), 120, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Type"), 60, wxALIGN_LEFT, -1, true, true);
	SetLineSpacing(2);

	AddRoot(wxT(""));
}

BacktraceView::wxTreeItemIdList
BacktraceView::GetItemChildren(const wxTreeItemId &item) {
	wxTreeItemIdList result;
	wxTreeItemIdValue cookie;

	for (wxTreeItemId child = GetFirstChild(item, cookie);
		child.IsOk();
		child = GetNextChild(item, cookie)) {
		result.push_back(child);
	}

	return result;
}

BacktraceViewItemData *BacktraceView::GetItemData(const wxTreeItemId &item) {
	return static_cast<BacktraceViewItemData *>(wxTreeListCtrl::GetItemData(item));
}

struct BacktraceView::UpdateHandler {
	BacktraceView *m_view;
	explicit UpdateHandler(BacktraceView *view)
		: m_view(view) {
	}
	void operator()(const lldebug::net::Command &command,
					const LuaBacktraceList &bts) {
		m_view->DoUpdate(bts);
	}
	};

void BacktraceView::BeginUpdating() {
	Mediator::Get()->GetEngine()->RequestBacktraceList(
		UpdateHandler(this));
}

bool BacktraceView::IsSameContents(const LuaBacktraceList &backtraces) {
	wxTreeItemIdList children = GetItemChildren(GetRootItem());

	if (children.size() != backtraces.size()) {
		return false;
	}

	for (LuaBacktraceList::size_type i = 0; i < backtraces.size(); ++i) {
		const LuaBacktrace &bt1 = backtraces[i];
		const LuaBacktrace &bt2 = GetItemData(children[i])->GetBacktrace();

		if (bt1.GetFuncName() != bt2.GetFuncName()
			|| bt1.GetKey() != bt2.GetKey() || bt1.GetLine() != bt1.GetLine()
			|| bt1.GetLua() != bt2.GetLua() || bt1.GetLevel() != bt2.GetLevel()) {
			return false;
		}
	}

	return true;
}

/// Update child variables of vars actually.
void BacktraceView::DoUpdate(const LuaBacktraceList &backtraces) {
	if (IsSameContents(backtraces)) {
		return;
	}

	wxTreeItemId root = GetRootItem();

	DeleteChildren(root);
	for (LuaBacktraceList::size_type i = 0; i < backtraces.size(); ++i) {
		const LuaBacktrace &backtrace = backtraces[i];

		wxTreeItemId item = AppendItem(
			root, wxEmptyString, -1, -1,
			new BacktraceViewItemData(backtrace));

		// Set texts of columns.
		if (backtrace.GetTitle().empty()) {
			SetItemText(item, 0, wxT("unknown"));
		}
		else {
			SetItemText(item, 0, wxConvFromUTF8(backtrace.GetTitle()));
		}
		SetItemText(item, 1,
			wxString::Format(wxT("%d"), backtrace.GetLine()));
		SetItemText(item, 2,
			wxConvFromUTF8(backtrace.GetFuncName()));
		SetItemText(item, 3,
			(backtrace.GetLine() >= 0 ? wxT("lua") : wxT("native")));
	}
}

void BacktraceView::OnChangedState(wxDebugEvent &event) {
	event.Skip();

	Enable(event.IsBreak());
	if (event.IsBreak() && IsEnabled() && IsShown()) {
		BeginUpdating();
	}
}

void BacktraceView::OnUpdateSource(wxDebugEvent &event) {
	event.Skip();

	if (IsEnabled() && IsShown()) {
		BeginUpdating();
	}
}

void BacktraceView::OnItemActivated(wxTreeEvent &event) {
	event.Skip();

	if (IsEnabled() && IsShown()) {
		BacktraceViewItemData *data = GetItemData(event.GetItem());
		Mediator::Get()->FocusBacktraceLine(data->GetBacktrace());
	}
}

void BacktraceView::OnShow(wxShowEvent &event) {
	event.Skip();

	if (event.GetShow() && IsEnabled() && IsShown()) {
		BeginUpdating();
	}
}

void BacktraceView::LayoutColumn(int selectedColumn) {
	// Calc the amount of the columns.
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

void BacktraceView::OnSize(wxSizeEvent &event) {
	event.Skip();
	LayoutColumn(-1);
}

void BacktraceView::OnColEndDrag(wxListEvent &event) {
	event.Skip();
	LayoutColumn(event.GetColumn());
}

} // end of namespace visual
} // end of namespace lldebug
