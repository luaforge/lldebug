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
#include "visual/strutils.h"

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
		: m_var(var), m_requestCount(-1), m_updateCount(-1) {
	}

	virtual ~VariableWatchItemData() {
	}

	/// Get the LuaVar object.
	const LuaVar &GetVar() const {
		return m_var;
	}

	/// Get the request count.
	int GetRequestCount() const {
		return m_requestCount;
	}

	/// Requested this var.
	void Requested() {
		int count = Mediator::Get()->GetUpdateCount();

		if (m_requestCount < count) {
			m_requestCount = count;
		}
	}

	/// Get the update count.
	int GetUpdateCount() const {
		return m_updateCount;
	}

	/// Update the update count.
	void Updated() {
		int count = Mediator::Get()->GetUpdateCount();

		if (m_requestCount < count) {
			m_requestCount = count;
		}
		if (m_updateCount < count) {
			m_updateCount = count;
		}
	}

private:
	LuaVar m_var;
	int m_requestCount;
	int m_updateCount;
};

/// The type of a function that requests LuaVarList from RemoteEngine.
typedef
	boost::function1<void, const LuaVarListCallback &>
	VarListRequester;
		
typedef
	boost::function1<void, const LuaMultiVarListCallback &>
	MultiVarListRequester;

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

		ms_aliveInstanceSet.insert(this);
	}
	
	virtual ~VariableWatch() {
		ms_aliveInstanceSet.erase(this);
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
		int operator()(const lldebug::Command &command, const LuaVarList &vars) {
			// If update count was changed, new request might be sent.
			if (m_updateCount != Mediator::Get()->GetUpdateCount()) {
				return -1;
			}

			// Is m_watch still alive ?
			if (ms_aliveInstanceSet.find(m_watch) == ms_aliveInstanceSet.end()) {
				return -1;
			}

			m_watch->DoUpdateVars(m_item, vars, m_isExpanded);
			return 0;
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
		if (data == NULL) {
			return;
		}

		// Does this need to request newbies ?
		if (data->GetRequestCount() < Mediator::Get()->GetUpdateCount()) {
			RequestVarListCallback callback(this, item, isExpanded);
			request(callback);
			data->Requested();
		}
		else if (isExpanded) {
			wxTreeItemIdList children = GetItemChildren(item);

			wxTreeItemIdList::iterator it;
			for (it = children.begin(); it != children.end(); ++it) {
				VariableWatchItemData *data = GetItemData(*it);
				if (data != NULL) {
					BeginUpdating(*it, false, data->GetVar());
				}
			}
		}
	}

	/// Request for the fields of the var.
	struct FieldsRequester {
		explicit FieldsRequester(const LuaVar &var)
			: m_var(var) {
		}
		void operator()(const LuaVarListCallback &callback) {
			Mediator::Get()->GetEngine()->SendRequestFieldsVarList(
				m_var, callback);
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
					str += wxConvToCtxEnc(label);
					labels.push_back(str);
				}
			}

			Mediator::Get()->GetEngine()->SendEvalsToVarList(
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

	/// Clear this view.
	void Clear() {
		DeleteRoot();
		AddRoot(wxT(""), -1, -1, new VariableWatchItemData(LuaVar()));
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
		VariableWatchItemData *parentData = GetItemData(parent);
		if (parentData->GetUpdateCount() == Mediator::Get()->GetUpdateCount()) {
			return;
		}

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

				wxString name = wxConvFromCtxEnc(var.GetName());
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
			wxString value = wxConvFromCtxEnc(var.GetValue());
			if (GetItemText(item, 1) != value) {
				SetItemText(item, 1, value);
			}

			// Check whether it has the type column.
			if (GetColumnCount() >= 3) {
				wxString type = wxConvFromCtxEnc(var.GetValueTypeName());
				if (GetItemText(item, 2) != type) {
					SetItemText(item, 2, type);
				}
			}

			// Refresh the chilren, too.
			if (var.HasFields()) {
				if (!HasChildren(item)) {
					// Item for lazy evalution
					AppendItem(item, _T(""));
				}

				if (IsExpanded(item)) {
					BeginUpdating(item, true, var);
				}
				else if (isExpand) {
					BeginUpdating(item, false, var);
				}
			}
			else {
				if (HasChildren(item)) {
					// The state whether the item is expanded or collapsed
					// has been saved, so collapse it carefully.
					if (IsExpanded(item)) {
						//Collapse(item);
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
		parentData->Updated();
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
	/// Get the width that can show vars.
	int GetContentWidth() {
		return 
			( GetClientSize().GetWidth()
			- wxSystemSettings::GetMetric(wxSYS_VSCROLL_X));
	}

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
			GetContentWidth() - columnsWidth);
	}

	/// Fit the the columns to the window width.
	void FitColumns() {
		int curWidth = 0; // column's size
		for (int i = 0; i < GetColumnCount(); ++i) {
			curWidth += GetColumnWidth(i); 
		}
	
		// prevWidth is the currently resizable width.
		int width = GetContentWidth();
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

	void OnEndDebug(wxDebugEvent &event) {
		event.Skip();
		Clear();
	}

private:
	typedef std::set<VariableWatch *> InstanceSet;
	static InstanceSet ms_aliveInstanceSet;

	bool m_isLabelEditable;
	bool m_isEvalLabels;
	VarListRequester m_requester;

	DECLARE_EVENT_TABLE();
};

/// Alive instance set.
VariableWatch::InstanceSet VariableWatch::ms_aliveInstanceSet;

BEGIN_EVENT_TABLE(VariableWatch, wxTreeListCtrl)
	EVT_SIZE(VariableWatch::OnSize)
	EVT_TREE_ITEM_EXPANDED(wxID_ANY, VariableWatch::OnExpanded)
	EVT_TREE_END_LABEL_EDIT(wxID_ANY, VariableWatch::OnEndLabelEdit)
	EVT_LIST_COL_END_DRAG(wxID_ANY, VariableWatch::OnColEndDrag)
	EVT_DEBUG_END_DEBUG(wxID_ANY, VariableWatch::OnEndDebug)
END_EVENT_TABLE()


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

		ms_aliveInstanceSet.insert(this);
	}

	virtual ~VariableWatch() {
		ms_aliveInstanceSet.erase(this);
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
		int operator()(const lldebug::Command &command, const LuaVarList &vars) {
			// If update count was changed, new request might be sent.
			if (m_updateCount != Mediator::Get()->GetUpdateCount()) {
				return -1;
			}

			// Is m_watch still alive ?
			if (ms_aliveInstanceSet.find(m_watch) == ms_aliveInstanceSet.end()) {
				return -1;
			}

			m_watch->DoUpdateVars(m_item, vars, m_isExpanded);
			return 0;
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
		if (data == NULL) {
			return;
		}

		// Does this need to request newbies ?
		if (data->GetRequestCount() < Mediator::Get()->GetUpdateCount()) {
			RequestVarListCallback callback(this, item, isExpanded);
			request(callback);
			data->Requested();
		}
		else if (isExpanded) {
			wxTreeItemIdList children = GetItemChildren(item);

			wxTreeItemIdList::iterator it;
			for (it = children.begin(); it != children.end(); ++it) {
				VariableWatchItemData *data = GetItemData(*it);
				if (data != NULL) {
					BeginUpdating(*it, false, data->GetVar());
				}
			}
		}
	}

	/// Request for the fields of the var.
	struct FieldsRequester {
		explicit FieldsRequester(const LuaVar &var)
			: m_var(var) {
		}
		void operator()(const LuaVarListCallback &callback) {
			Mediator::Get()->GetEngine()->SendRequestFieldsVarList(
				m_var, callback);
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
					str += wxConvToCtxEnc(label);
					labels.push_back(str);
				}
			}

			Mediator::Get()->GetEngine()->SendEvalsToVarList(
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

	/// Clear this view.
	void Clear() {
		DeleteRoot();
		AddRoot(wxT(""), -1, -1, new VariableWatchItemData(LuaVar()));
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
		VariableWatchItemData *parentData = GetItemData(parent);
		if (parentData->GetUpdateCount() == Mediator::Get()->GetUpdateCount()) {
			return;
		}

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

				wxString name = wxConvFromCtxEnc(var.GetName());
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
			wxString value = wxConvFromCtxEnc(var.GetValue());
			if (GetItemText(item, 1) != value) {
				SetItemText(item, 1, value);
			}

			// Check whether it has the type column.
			if (GetColumnCount() >= 3) {
				wxString type = wxConvFromCtxEnc(var.GetValueTypeName());
				if (GetItemText(item, 2) != type) {
					SetItemText(item, 2, type);
				}
			}

			// Refresh the chilren, too.
			if (var.HasFields()) {
				if (!HasChildren(item)) {
					// Item for lazy evalution
					AppendItem(item, _T(""));
				}

				if (IsExpanded(item)) {
					BeginUpdating(item, true, var);
				}
				else if (isExpand) {
					BeginUpdating(item, false, var);
				}
			}
			else {
				if (HasChildren(item)) {
					// The state whether the item is expanded or collapsed
					// has been saved, so collapse it carefully.
					if (IsExpanded(item)) {
						//Collapse(item);
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
		parentData->Updated();
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
	/// Get the width that can show vars.
	int GetContentWidth() {
		return 
			( GetClientSize().GetWidth()
			- wxSystemSettings::GetMetric(wxSYS_VSCROLL_X));
	}

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
			GetContentWidth() - columnsWidth);
	}

	/// Fit the the columns to the window width.
	void FitColumns() {
		int curWidth = 0; // column's size
		for (int i = 0; i < GetColumnCount(); ++i) {
			curWidth += GetColumnWidth(i); 
		}
	
		// prevWidth is the currently resizable width.
		int width = GetContentWidth();
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

	void OnEndDebug(wxDebugEvent &event) {
		event.Skip();
		Clear();
	}

private:
	typedef std::set<VariableWatch *> InstanceSet;
	static InstanceSet ms_aliveInstanceSet;

	bool m_isLabelEditable;
	bool m_isEvalLabels;
	VarListRequester m_requester;

	DECLARE_EVENT_TABLE();
};


} // end of namespace visual
} // end of namespace lldebug
