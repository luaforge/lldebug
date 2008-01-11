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
#include "lldebug_backtraceview.h"
#include "lldebug_context.h"
#include "lldebug_mainframe.h"

namespace lldebug {

class BackTraceViewItemData : public wxTreeItemData {
public:
	explicit BackTraceViewItemData(const LuaBackTraceInfo &info)
		: m_info(info) {
	}

	virtual ~BackTraceViewItemData() {
	}

	const LuaBackTraceInfo &GetInfo() const {
		return m_info;
	}

private:
	LuaBackTraceInfo m_info;
};

BEGIN_EVENT_TABLE(BackTraceView, wxTreeListCtrl)
	EVT_SIZE(BackTraceView::OnSize)
	EVT_SHOW(BackTraceView::OnShow)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, BackTraceView::OnItemActivated)
	EVT_LIST_COL_END_DRAG(wxID_ANY, BackTraceView::OnColEndDrag)
	EVT_LLDEBUG_CHANGED_STATE(ID_BACKTRACEVIEW, BackTraceView::OnChangedState)
	EVT_LLDEBUG_UPDATE_SOURCE(ID_BACKTRACEVIEW, BackTraceView::OnUpdateSource)
END_EVENT_TABLE()

BackTraceView::BackTraceView(Context *ctx, wxWindow *parent)
	: wxTreeListCtrl(parent, ID_BACKTRACEVIEW + ctx->GetId()
		, wxDefaultPosition, wxDefaultSize
		, wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT
		| wxTR_ROW_LINES | wxTR_COL_LINES
		| wxTR_FULL_ROW_HIGHLIGHT | wxALWAYS_SHOW_SB)
	, m_ctx(ctx) {
	CreateGUIControls();
}

BackTraceView::~BackTraceView() {
}

void BackTraceView::CreateGUIControls() {
	scoped_lock lock(m_mutex);

	AddColumn(_("File"), 80, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Line"), 40, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Function"), 120, wxALIGN_LEFT, -1, true, true);
	AddColumn(_("Type"), 60, wxALIGN_LEFT, -1, true, true);
	SetLineSpacing(2);

	AddRoot(wxT(""));
}

BackTraceViewItemData *BackTraceView::GetItemData(const wxTreeItemId &item) {
	scoped_lock lock(m_mutex);
	return static_cast<BackTraceViewItemData *>(wxTreeListCtrl::GetItemData(item));
}

void BackTraceView::UpdateBackTrace() {
	LuaBackTrace backTrace = m_ctx->LuaGetBackTrace();
	wxTreeItemId root = GetRootItem();
	DeleteChildren(root);

	for (LuaBackTrace::size_type i = 0; i < backTrace.size(); ++i) {
		const LuaBackTraceInfo &info = backTrace[i];

		wxTreeItemId item = AppendItem(
			root, wxEmptyString, -1, -1,
			new BackTraceViewItemData(info));

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

void BackTraceView::OnChangedState(wxChangedStateEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	Enable(event.GetValue());
	if (event.GetValue() && IsEnabled() && IsShown()) {
		UpdateBackTrace();
	}
}

void BackTraceView::OnUpdateSource(wxSourceLineEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (IsEnabled() && IsShown()) {
		UpdateBackTrace();
	}
}

void BackTraceView::OnItemActivated(wxTreeEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (IsEnabled() && IsShown()) {
		BackTraceViewItemData *data = GetItemData(event.GetItem());
		const LuaBackTraceInfo &info = data->GetInfo();

		m_ctx->GetFrame()->SetViewSource(
			info.GetKey(), info.GetLine(),
			info.GetLua(), info.GetLevel());
	}
}

void BackTraceView::OnShow(wxShowEvent &event) {
	scoped_lock lock(m_mutex);
	event.Skip();

	if (event.GetShow() && IsEnabled() && IsShown()) {
		UpdateBackTrace();
	}
}

void BackTraceView::LayoutColumn(int selectedColumn) {
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

void BackTraceView::OnSize(wxSizeEvent &event) {
	event.Skip();
	LayoutColumn(-1);
}

void BackTraceView::OnColEndDrag(wxListEvent &event) {
	event.Skip();
	LayoutColumn(event.GetColumn());
}

}
