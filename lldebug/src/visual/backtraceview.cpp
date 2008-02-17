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
#include "visual/backtraceview.h"
#include "visual/mediator.h"

namespace lldebug {
namespace visual {

class BacktraceViewItemData : public wxTreeItemData {
public:
	explicit BacktraceViewItemData(const LuaBacktrace &bt)
		: m_backtrace(bt) {
	}

	virtual ~BacktraceViewItemData() {
	}

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
	scoped_lock lock(m_mutex);

	AddColumn(_("File"), 80, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Line"), 40, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Function"), 120, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Type"), 60, wxALIGN_LEFT, -1, true, true);
	SetLineSpacing(2);

	AddRoot(wxT(""));
}

BacktraceViewItemData *BacktraceView::GetItemData(const wxTreeItemId &item) {
	scoped_lock lock(m_mutex);
	return static_cast<BacktraceViewItemData *>(wxTreeListCtrl::GetItemData(item));
}

void BacktraceView::UpdateBackTrace() {
	LuaBacktraceList backtraces;
	wxTreeItemId root = GetRootItem();
	DeleteChildren(root);

	for (LuaBacktraceList::size_type i = 0; i < backtraces.size(); ++i) {
		const LuaBacktrace &info = backtraces[i];

		wxTreeItemId item = AppendItem(
			root, wxEmptyString, -1, -1,
			new BacktraceViewItemData(info));

		// カラムを設定します。
		wxString value;
		if (info.GetTitle().empty()) {
			SetItemText(item, 0, wxT("unknown"));
		}
		else {
			value = wxConvFromUTF8(info.GetTitle());
			SetItemText(item, 0, value);
		}

		value = wxString::Format(wxT("%d"), info.GetLine());
		SetItemText(item, 1, value);
		value = wxConvFromUTF8(info.GetFuncName());
		SetItemText(item, 2, value);
		value = (info.GetLine() >= 0 ? wxT("lua") : wxT("native"));
		SetItemText(item, 3, value);
	}
}

void BacktraceView::OnChangedState(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	Enable(event.IsBreak());
	if (event.IsBreak() && IsEnabled() && IsShown()) {
		//UpdateBacktrace();
	}
}

void BacktraceView::OnUpdateSource(wxDebugEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (IsEnabled() && IsShown()) {
//		UpdateBacktrace();
	}
}

void BacktraceView::OnItemActivated(wxTreeEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (IsEnabled() && IsShown()) {
		BacktraceViewItemData *data = GetItemData(event.GetItem());
		Mediator::Get()->FocusBacktraceLine(data->GetBacktrace());
	}
}

void BacktraceView::OnShow(wxShowEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (event.GetShow() && IsEnabled() && IsShown()) {
		UpdateBackTrace();
	}
}

void BacktraceView::LayoutColumn(int selectedColumn) {
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
