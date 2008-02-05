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

#ifndef __LLDEBUG_MEDIATOR_H__
#define __LLDEBUG_MEDIATOR_H__

#include "lldebug_sysinfo.h"
#include "lldebug_luainfo.h"

namespace lldebug {

class Application;
class MainFrame;
class RemoteEngine;
class Command;

/**
 * @brief Ç∑Ç◊ÇƒÇÃÉNÉâÉXÇ…ã§í ÇÃèÓïÒÇï€éùÇµÇ‹Ç∑ÅB
 */
class Mediator {
public:
	/// Get the class instance.
	/// It is valid after the calling of 'Initialize' function.
	static Mediator *Get() {
		return ms_instance;
	}

	/// Get the ID of the 'lldebug::Context' class.
	int GetCtxId();

	/// Focus the specified file and line.
	void ShowSourceLine(const std::string &key, int line);

	/// Process the remote command.
	void OnRemoteCommand(const Command &command);

	/// Get the RemoteEngine object.
	RemoteEngine *GetEngine() {
		scoped_lock lock(m_mutex);
		return m_engine.get();
	}

	/// Get the BreakpointList object.
	BreakpointList &GetBreakpoints() {
		scoped_lock lock(m_mutex);
		return m_breakpoints;
	}

	/// Get the SourceManager object.
	SourceManager &GetSourceManager() {
		scoped_lock lock(m_mutex);
		return m_sourceManager;
	}

	/// Get the source contents.
	const Source *GetSource(const std::string &key) {
		scoped_lock lock(m_mutex);
		return m_sourceManager.Get(key);
	}

	/// Save the source.
	int SaveSource(const std::string &key, const string_array &source) {
		scoped_lock lock(m_mutex);
		return m_sourceManager.Save(key, source);
	}

	/// Find the breakpoint.
	Breakpoint FindBreakpoint(const std::string &key, int line) {
		scoped_lock lock(m_mutex);
		return m_breakpoints.Find(key, line);
	}

	/// Find the next breakpoint.
	Breakpoint NextBreakpoint(const Breakpoint &bp) {
		scoped_lock lock(m_mutex);
		return m_breakpoints.Next(bp);
	}

	/// Set the breakpoint.
	void SetBreakpoint(const Breakpoint &bp) {
		scoped_lock lock(m_mutex);
		m_breakpoints.Set(bp);
	}

	/// Toggle on/off of the breakpoint.
	void ToggleBreakpoint(const std::string &key, int line) {
		scoped_lock lock(m_mutex);
		m_breakpoints.Toggle(key, line);
	}

	/// Get the stack frame for the local vars.
	const LuaStackFrame &GetStackFrame() {
		scoped_lock lock(m_mutex);
		return m_stackFrame;
	}

	/// Set the stack frame for the local vars.
	void SetStackFrame(const LuaStackFrame &stackFrame) {
		scoped_lock lock(m_mutex);
		m_stackFrame = stackFrame;
	}

	/// Get the count of 'UpdateSource'.
	int GetUpdateSourceCount() {
		scoped_lock lock(m_mutex);
		return m_updateSourceCount;
	}

private:
	friend class Application;
	explicit Mediator();
	virtual ~Mediator();

	/// This function can be called from only 'lldebug::MainFrame' class.
	int Initialize(const std::string &hostName, const std::string &portName);

private:
	friend MainFrame;
	void SetMainFrame(MainFrame *frame);

private:
	static Mediator *ms_instance;

	mutex m_mutex;
	shared_ptr<RemoteEngine> m_engine;
	MainFrame *m_frame;
	BreakpointList m_breakpoints;
	SourceManager m_sourceManager;

	LuaStackFrame m_stackFrame;
	int m_updateSourceCount;
};

}

#endif
