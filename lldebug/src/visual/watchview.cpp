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
#include "visual/watchview.h"
#include "visual/mainframe.h"

#include "wx/treelistctrl.h"

namespace lldebug {
namespace visual {

typedef std::vector<wxTreeItemId> wxTreeItemIdList;

/**
 * @brief The data of the VariableWatch.
 */
class VariableWatchItemData : public wxTreeItemData {
public:
	explicit VariableWatchItemData(const LuaVar &var)
		: m_var(var), m_updateCount(-1) {
	}

	virtual ~VariableWatchItemData() {
	}

	/// Get the LuaVar object.
	const LuaVar &GetVar() const {
		return m_var;
	}

	/// Get the update count.
	int GetUpdateCount() const {
		return m_updateCount;
	}

	/// Update the update count.
	void Updated() {
		m_updateCount = Mediator::Get()->GetUpdateCount();
	}

private:
	LuaVar m_var;
	int m_updateCount;
};


/// The type of a function that requests the LuaVarList from RemoteEngine.
typedef
	boost::function1<void, const LuaVarListCallback &>
	VarListRequester;

/**
 * @brief The common implementation of 'VariableWatch'.
 */
class VariableWatch : public wxTreeListCtrl {
public:
	explicit VariableWatch(wxWindow *parent, int id,
						   bool isShowColumn, bool isShowType,
						   bool isLabelEditable, bool isEvalLabels,
						   const VarListRequester &requester)
		: wxTreeListCtrl(parent, id
			, wxDefaultPosition, wxDefaultSize
			, wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT
				| wxTR_EDIT_LABELS | wxTR_ROW_LINES | wxTR_COL_LINES
				| wxTR_FULL_ROW_HIGHLIGHT | wxALWAYS_SHOW_SB
				| (isShowColumn ? 0 : wxTR_HIDE_COLUMNS))
		, m_isLabelEditable(isLabelEditable), m_isEvalLabels(isEvalLabels)
		, m_requester(requester) {

		if (true) {
			// Set the header font.
			wxFont font(GetFont());
			font.SetPointSize(9);
			SetFont(font);

			AddColumn(_("Name"), 80, wxALIGN_LEFT, -1, true, true);
			AddColumn(_("Value"), 120, wxALIGN_LEFT, -1, true, true);
			if (isShowType) {
				AddColumn(_("Type"), 60, wxALIGN_LEFT, -1, true, true);
			}
			SetLineSpacing(2);
		}

		AddRoot(wxT(""), -1, -1, new VariableWatchItemData(LuaVar()));
		if (isLabelEditable) {
			AppendItem(
				GetRootItem(), wxT("                                "),
				-1, -1, new VariableWatchItemData(LuaVar()));
		}
	}

	virtual ~VariableWatch() {
	}

	/// Get children of the item.
	virtual wxTreeItemIdList GetItemChildren(const wxTreeItemId &item) {
		wxTreeItemIdList result;
		wxTreeItemIdValue cookie;

		for (wxTreeItemId child = GetFirstChild(item, cookie);
			child.IsOk();
			child = GetNextChild(item, cookie)) {
			result.push_back(child);
		}

		return result;
	}

	/// Get the data of the item.
	virtual VariableWatchItemData *GetItemData(const wxTreeItemId &item) {
		return static_cast<VariableWatchItemData *>(wxTreeListCtrl::GetItemData(item));
	}

private:
	/// This object is called when the request var list
	/// is done and results are returned.
	struct RequestVarListCallback {
		explicit RequestVarListCallback(VariableWatch *watch, wxTreeItemId item, bool isExpanded)
			: m_watch(watch), m_item(item), m_isExpanded(isExpanded)
			, m_updateCount(Mediator::Get()->GetUpdateCount()) {
		}

		/// This method may be called from the other thread.
		/// @param vars    result of the request
		void operator()(const lldebug::Command &command, const LuaVarList &vars) {
			static int count = 0;
			Mediator::Get()->GetFrame()->SetTitle(wxString::Format(_T("%d"), ++count));
			if (m_updateCount < Mediator::Get()->GetUpdateCount()) {
				return;
			}

			m_watch->DoUpdateVars(m_item, vars, m_isExpanded);
		}

	private:
		VariableWatch *m_watch;
		wxTreeItemId m_item;
		bool m_isExpanded;
		int m_updateCount;
	};

	friend struct RequestVarsCallback;

public:
	/// Begin updating contents.
	void BeginUpdating(wxTreeItemId item, bool isExpanded,
						 const VarListRequester &request) {
		VariableWatchItemData *data = GetItemData(item);
		//bool isRoot = (item == GetRootItem());
		bool isNeedUpdate =
			(data->GetUpdateCount() < Mediator::Get()->GetUpdateCount());

		if (isNeedUpdate) {
			RequestVarListCallback callback(this, item, isExpanded);
			request(callback);
		}
		else if (isExpanded) {
			wxTreeItemIdList children = GetItemChildren(item);

			wxTreeItemIdList::iterator it;
			for (it = children.begin(); it != children.end(); ++it) {
				VariableWatchItemData *data = GetItemData(*it);
				BeginUpdating(*it, false, data->GetVar());
			}
		}
	}

	/// Request for the fields of the var.
	struct FieldsRequester {
		explicit FieldsRequester(const LuaVar &var)
			: m_var(var) {
		}
		void operator()(const LuaVarListCallback &callback) {
			Mediator::Get()->GetEngine()->RequestFieldsVarList(m_var, callback);
		}
	private:
		LuaVar m_var;
		};

	/// Begin the updating the fields of the var.
	void BeginUpdating(wxTreeItemId item, bool isExpanded, const LuaVar &var) {
		BeginUpdating(item, isExpanded, FieldsRequester(var));
	}

	/// Request for the results of the label evaluations.
	struct EvalLabelRequester {
		explicit EvalLabelRequester(VariableWatch *watch)
			: m_watch(watch) {
		}
		void operator()(const LuaVarListCallback &callback) {
			wxTreeItemId item = m_watch->GetRootItem();
			wxTreeItemIdValue cookie;
			string_array labels;

			for (wxTreeItemId child = m_watch->GetFirstChild(item, cookie);
				child.IsOk();
				child = m_watch->GetNextChild(item, cookie)) {
				wxString label = m_watch->GetItemText(child);

				if (label.Strip(wxString::both).IsEmpty()) {
					labels.push_back("");
				}
				else {
					std::string str = "return ";
					str += wxConvToUTF8(label);
					labels.push_back(str);
				}
			}

			Mediator::Get()->GetEngine()->EvalsToVarList(
				labels,
				Mediator::Get()->GetStackFrame(),
				callback);
		}
	private:
		VariableWatch *m_watch;
		};

	/// Begin updating vars.
	void BeginUpdating() {
		if (m_isLabelEditable) {
			BeginUpdating(GetRootItem(), true, EvalLabelRequester(this));
		}
		else {
			BeginUpdating(GetRootItem(), true, m_requester);
		}
	}

private:
	/// Compare the item vars.
	struct ItemComparator {
		explicit ItemComparator(VariableWatch *watch, const LuaVar &var)
			: m_watch(watch), m_var(var) {
		}

		bool operator()(const wxTreeItemId &item) {
			VariableWatchItemData *data = m_watch->GetItemData(item);
			return
				( data != NULL && data->GetVar().IsOk()
				? data->GetVar().GetName() == m_var.GetName()
				: false);
		}

	private:
		VariableWatch *m_watch;
		const LuaVar &m_var;
	};

	/// Update child variables of vars actually.
	void DoUpdateVars(wxTreeItemId parent, const LuaVarList &vars,
					  bool isExpand) {
		// The current chilren list.
		// If the item is updated, it is removed.
		wxTreeItemIdList children = GetItemChildren(parent);
		bool isEvalLabels = (m_isEvalLabels && parent == GetRootItem());

		// If each tree's label were evaluted...
		if (isEvalLabels) {
			wxASSERT(children.size() == vars.size());
		}

		for (LuaVarList::size_type i = 0; i < vars.size(); ++i) {
			const LuaVar &var = vars[i];
			wxTreeItemIdList::iterator it;
			wxTreeItemId item;

			// Set item to 'it'.
			if (isEvalLabels) {
				// Because the item of 'children' is eliminated.
				it = children.begin();
			} else {
				// Find the item that has the same LuaVar object.
				it = std::find_if(children.begin(), children.end(), ItemComparator(this, var));
			}

			// Does the item exist ?
			if (it == children.end()) { // No!
				item = AppendItem(parent, wxEmptyString, -1, -1, new VariableWatchItemData(var));

				wxString name = wxConvFromUTF8(var.GetName());
				SetItemText(item, 0, name);
			}
			else {
				// Use the exist item.
				item = *it;
				children.erase(it);

				// Replace the item data.
				VariableWatchItemData *oldData = GetItemData(item);
				if (oldData != NULL) {
					delete oldData;
				}
				SetItemData(item, new VariableWatchItemData(var));
			}

			// To avoid the useless refresh, check change of the title.
			wxString value = wxConvFromUTF8(var.GetValue());
			if (GetItemText(item, 1) != value) {
				SetItemText(item, 1, value);
			}

			// Check whether it has the type column.
			if (GetColumnCount() >= 3) {
				wxString type = wxConvFromUTF8(var.GetValueTypeName());
				if (GetItemText(item, 2) != type) {
					SetItemText(item, 2, type);
				}
			}

			// Refresh the chilren, too.
			if (var.HasFields()) {
				if (IsExpanded(item)) {
					BeginUpdating(item, true, var);
				}
				else if (isExpand) {
					BeginUpdating(item, false, var);
				}
				else if (!HasChildren(item)) {
					AppendItem(item, _T("$<item for lazy evalution>"));
				}
			}
			else {
				if (HasChildren(item)) {
					// The state whether the item is expanded or collapsed
					// has been saved, so collapse it carefully.
					if (IsExpanded(item)) {
						Collapse(item);
					}

					// Delete all child items.
					DeleteChildren(item);
				}

				// Update was done.
				VariableWatchItemData *data = GetItemData(item);
				data->Updated();
			}
		}

		// Remove all items that were not refreshed or appended.
		wxTreeItemIdList::iterator it;
		for (it = children.begin(); it != children.end(); ++it) {
			Delete(*it);
		}

		// Update was done.
		VariableWatchItemData *data = GetItemData(parent);
		data->Updated();
	}

private:
	void OnExpanded(wxTreeEvent &event) {
		event.Skip();

		VariableWatchItemData *data = GetItemData(event.GetItem());
		BeginUpdating(event.GetItem(), true, data->GetVar());
	}

	void OnEndLabelEdit(wxTreeEvent &event) {
		event.Skip();

		if (m_isLabelEditable) {
			// The last label isn't only the white space,
			// add a new label.
			wxTreeItemIdValue cookie;
			wxTreeItemId item = GetLastChild(GetRootItem(), cookie);
			if (item == event.GetItem()) {
				wxString label = event.GetLabel();
				if (!label.Strip(wxString::both).IsEmpty()) {
					AppendItem(
						GetRootItem(), wxT("                     "),
						-1, -1, new VariableWatchItemData(LuaVar()));
				}
			}

			// If it is changed, we update the value of the var.
			if (GetItemText(event.GetItem()) != event.GetLabel()) {
				SetItemText(event.GetItem(), event.GetLabel());
				Mediator::Get()->IncUpdateCount();
				BeginUpdating();
			}
		}
		else {
			event.Veto();
		}
	}

private:
	/// Resize the target and next columns to fit the window width.
	void LayoutColumn(int targetColumn) {
		if (GetColumnCount() <= 1) {
			if (GetColumnCount() == 1) {
				SetColumnWidth(0, GetClientSize().GetWidth());
			}
			return;
		}

		int resizeItem = targetColumn + 1;
		if (resizeItem == GetColumnCount()) {
			resizeItem = GetColumnCount() - 1;
		}
		
		int columnsWidth = 0; // column's width
		for (int i = 0; i < GetColumnCount(); ++i) {
			if (i != resizeItem) {
				columnsWidth += GetColumnWidth(i);
			}
		}

		// Do resize the column.
		SetColumnWidth(
			resizeItem,
			GetClientSize().GetWidth() - columnsWidth);
	}

	/// Fit the the columns to the window width.
	void FitColumns() {
		int curWidth = 0; // column's size
		for (int i = 0; i < GetColumnCount(); ++i) {
			curWidth += GetColumnWidth(i); 
		}
	
		// prevWidth is the currently resizable width.
		int width = GetClientSize().GetWidth();
		double ratio = (double)width / curWidth;
		if (ratio < 0.001) {
			return;
		}

		// Resize the columns with the same ratio.
		for (int i = 0; i < GetColumnCount(); ++i) {
			int w = GetColumnWidth(i);
			SetColumnWidth(i, (int)(w * ratio));
		}
	}

	void OnSize(wxSizeEvent &event) {
		event.Skip();
		FitColumns();
	}

	void OnColEndDrag(wxListEvent &event) {
		event.Skip();
		LayoutColumn(event.GetColumn());
	}

private:
	struct UpdateData {
		int updateCount;
		wxTreeItemId item;
		LuaVarList vars;
		bool isExpanded;
	};
	std::queue<UpdateData> m_queue;

	bool m_isLabelEditable;
	bool m_isEvalLabels;
	VarListRequester m_requester;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(VariableWatch, wxTreeListCtrl)
	EVT_SIZE(VariableWatch::OnSize)
	EVT_TREE_ITEM_EXPANDED(wxID_ANY, VariableWatch::OnExpanded)
	EVT_TREE_END_LABEL_EDIT(wxID_ANY, VariableWatch::OnEndLabelEdit)
	EVT_LIST_COL_END_DRAG(wxID_ANY, VariableWatch::OnColEndDrag)
END_EVENT_TABLE()


/*-----------------------------------------------------------------*/
/**
 * @brief Request for the val value evalution.
 */
struct OneVariableWatchView::VariableRequester {
	explicit VariableRequester(const wxString &valName) {
		m_valNameUTF8 = wxConvToUTF8(valName);
	}

	void operator()(const LuaVarListCallback &callback) {
		std::string eval = "return ";
		eval += m_valNameUTF8;

		Mediator::Get()->GetEngine()->EvalToMultiVar(
			eval,
			LuaStackFrame(LuaHandle(), 0),
			callback);
	}

private:
	std::string m_valNameUTF8;
};

OneVariableWatchView::OneVariableWatchView(wxWindow *parent,
										   const wxString &valName,
										   const wxPoint &pos,
										   const wxSize &size)
	: wxFrame(parent, wxID_ANY, _T(""), pos, size
		, wxFRAME_TOOL_WINDOW | wxRESIZE_BORDER
		| wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT)
	, m_wasInMouse(false) {

	m_watch = new VariableWatch(
		this, wxID_ANY,
		true, true, false, true,
		VariableRequester(valName));
	m_watch->AppendItem(
		m_watch->GetRootItem(), valName, -1, -1,
		new VariableWatchItemData(LuaVar()));
	m_watch->BeginUpdating();
	SetHandler(m_watch);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_watch, 1, wxEXPAND);
	SetSizer(sizer);
	sizer->SetSizeHints(this);
}

OneVariableWatchView::~OneVariableWatchView() {
}

void OneVariableWatchView::SetHandler(wxWindow *target) {
	if (target == NULL) {
		return;
	}

	// Set the handler.
	target->SetNextHandler(this);

	// Set the handler to the children.
	wxWindowList children = target->GetChildren();
	for (size_t i = 0; i < children.GetCount(); ++i) {
		SetHandler(children[i]);
	}
}

void OneVariableWatchView::OnMotion(wxMouseEvent &event) {
	event.Skip();
	m_wasInMouse = true;
}

BEGIN_EVENT_TABLE(OneVariableWatchView, wxFrame)
	EVT_MOTION(OneVariableWatchView::OnMotion)
END_EVENT_TABLE()


/*-----------------------------------------------------------------*/
BEGIN_EVENT_TABLE(WatchView, wxPanel)
	EVT_SHOW(WatchView::OnShow)
	EVT_LLDEBUG_CHANGED_STATE(wxID_ANY, WatchView::OnChangedState)
	EVT_LLDEBUG_UPDATE_SOURCE(wxID_ANY, WatchView::OnUpdateSource)
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

struct VarUpdateRequester {
	explicit VarUpdateRequester(WatchView::Type type)
		: m_type(type) {
	}
	void operator()(const LuaVarListCallback &callback) {
		switch (m_type) {
		case WatchView::TYPE_LOCALWATCH:
			Mediator::Get()->GetEngine()->RequestLocalVarList(
				Mediator::Get()->GetStackFrame(), callback);
			break;
		case WatchView::TYPE_ENVIRONWATCH:
			Mediator::Get()->GetEngine()->RequestEnvironVarList(
				Mediator::Get()->GetStackFrame(), callback);
			break;
		case WatchView::TYPE_GLOBALWATCH:
			Mediator::Get()->GetEngine()->RequestGlobalVarList(callback);
			break;
		case WatchView::TYPE_REGISTRYWATCH:
			Mediator::Get()->GetEngine()->RequestRegistryVarList(callback);
			break;
		case WatchView::TYPE_STACKWATCH:
			Mediator::Get()->GetEngine()->RequestStackList(callback);
			break;
		case WatchView::TYPE_WATCH:
			wxASSERT(false);
			break;
		}
	}
private:
	WatchView::Type m_type;
};

WatchView::WatchView(wxWindow *parent, Type type)
	: wxPanel(parent, GetWatchViewId(type))
	, m_type(type) {

	if (type == TYPE_WATCH) {
		m_watch = new VariableWatch(
			this, wxID_ANY, true, true,
			true, true, VarListRequester());
	}
	else {
		m_watch = new VariableWatch(
			this, wxID_ANY, true, true,
			false, false, VarUpdateRequester(type));
	}

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_watch, 1, wxEXPAND);
	SetSizer(sizer);
	sizer->SetSizeHints(this);
}

WatchView::~WatchView() {
}

void WatchView::BeginUpdating() {
	m_watch->BeginUpdating();
}

void WatchView::OnChangedState(wxDebugEvent &event) {
	event.Skip();

	Enable(event.IsBreak());
}

void WatchView::OnUpdateSource(wxDebugEvent &event) {
	event.Skip();

	if (IsEnabled() && IsShown()) {
		BeginUpdating();
	}
}

void WatchView::OnShow(wxShowEvent &event) {
	event.Skip();

	if (event.GetShow() && IsEnabled() && IsShown()) {
		BeginUpdating();
	}
}

} // end of namespace visual
} // end of namespace lldebug
