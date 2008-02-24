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

#include "luainfo.h"
#include "visual/event.h"

namespace lldebug {
namespace visual {

class VariableWatch;

/**
 * @brief Control that shows contents of variables like global or etc.
 */
class WatchView : public wxPanel {
public:
	enum Type {
		TYPE_LOCALWATCH,
		TYPE_GLOBALWATCH,
		TYPE_REGISTRYWATCH,
		TYPE_ENVIRONWATCH,
		TYPE_STACKWATCH,
		TYPE_WATCH,
	};

public:
	explicit WatchView(wxWindow *parent, Type type);
	virtual ~WatchView();

private:
	void BeginUpdating();
	void OnChangedState(wxDebugEvent &event);
	void OnUpdateSource(wxDebugEvent &event);
	void OnFocusBacktraceLine(wxDebugEvent &event);
	void OnShow(wxShowEvent &event);

private:
	VariableWatch *m_watch;
	Type m_type;

	DECLARE_EVENT_TABLE();
};

/**
 * @brief Show contents of the one specific variable.
 */
class OneVariableWatchView : public wxFrame {
public:
	explicit OneVariableWatchView(wxWindow *parent, const wxString &valName,
								  const wxPoint &pos, const wxSize &size);
	virtual ~OneVariableWatchView();

	/// Was the mouse pointer in this control ?
	bool WasInMouse() const {
		return m_wasInMouse;
	}

private:
	struct VariableRequester;
	void SetHandler(wxWindow *target);
	void OnMotion(wxMouseEvent &event);

private:
	VariableWatch *m_watch;
	bool m_wasInMouse;

	DECLARE_EVENT_TABLE();
};


} // end of namespace visual
} // end of namespace lldebug

#endif
