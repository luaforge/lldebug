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
#include "net/netutils.h"
#include "visual/mediator.h"
#include "visual/mainframe.h"
#include "visual/strutils.h"

namespace lldebug {
namespace visual {

Mediator *Mediator::ms_instance = NULL;

Mediator::Mediator()
	: m_engine(new RemoteEngine), m_frame(NULL)
	, m_breakpoints(m_engine), m_sourceManager(m_engine)
	, m_updateCount(0) {
}

Mediator::~Mediator() {
	m_engine.reset();
	ms_instance = NULL;
}

int Mediator::Initialize(const std::string &hostName, const std::string &portName) {
	if (m_engine->StartFrame(portName) != 0) {
		return -1;
	}

	/*boost::xtime end;
	boost::xtime_get(&end, boost::TIME_UTC);
	end.sec += 30;

	// IsOpen become true in handleConnect.
	while (!m_engine->IsConnecting()) {
		boost::xtime current;
		boost::xtime_get(&current, boost::TIME_UTC);
		if (boost::xtime_cmp(current, end) >= 0) {
			return -1;
		}

		current.nsec += 100 * 1000 * 1000;
		boost::thread::sleep(current);
	}*/

	ms_instance = this;
	return 0;
}

void Mediator::SetMainFrame(MainFrame *frame) {
	m_frame = frame;
}

void Mediator::IncUpdateCount() {
	++m_updateCount;
	m_engine->SetUpdateCount(m_updateCount);
}

void Mediator::FocusErrorLine(const std::string &key, int line) {
	MainFrame *frame = GetFrame();

	wxDebugEvent event(wxEVT_DEBUG_FOCUS_ERRORLINE, wxID_ANY, key, line);
	frame->ProcessDebugEvent(event, frame, true);
}

void Mediator::FocusBacktraceLine(const LuaBacktrace &bt) {
	MainFrame *frame = GetFrame();

	IncUpdateCount();
	m_stackFrame = LuaStackFrame(bt.GetLua(), bt.GetLevel());

	wxDebugEvent event(wxEVT_DEBUG_FOCUS_BACKTRACELINE, wxID_ANY, bt);
	frame->ProcessDebugEvent(event, frame, true);
}

void Mediator::ProcessAllRemoteCommands() {
	while (m_engine->HasCommands()) {
		Command command = m_engine->GetCommand();
		m_engine->PopCommand();

		try {
			if (command.IsResponse()) {
				command.CallResponse();
			}
			else {
				ProcessRemoteCommand(command);
			}
		}
		catch (std::exception &ex) {
			wxLogMessage(_T("%s"), ex.what());
		}
	}
}

void Mediator::ProcessRemoteCommand(const Command &command) {
	MainFrame *frame = GetFrame();

	// Process remote commands.
	switch (command.GetType()) {
	case REMOTECOMMANDTYPE_END_CONNECTION:
		m_breakpoints = BreakpointList(m_engine);
//		m_sourceManager = SourceManager(&*m_engine);
		m_stackFrame = LuaStackFrame();
		m_updateCount = 0;
		if (frame != NULL) {
			wxDebugEvent event(wxEVT_DEBUG_END_DEBUG, wxID_ANY);
			frame->ProcessDebugEvent(event, frame, true);
		}
		break;

	case REMOTECOMMANDTYPE_CHANGED_STATE:
		if (frame != NULL) {
			bool isBreak;
			command.GetData().Get_ChangedState(isBreak);

			wxDebugEvent event(wxEVT_DEBUG_CHANGED_STATE, wxID_ANY, isBreak);
			frame->ProcessDebugEvent(event, frame, true);
		}
		break;

	case REMOTECOMMANDTYPE_SET_UPDATECOUNT:
		{
			int updateCount;
			command.GetData().Get_SetUpdateCount(updateCount);

			if (updateCount > m_updateCount) {
				m_updateCount = updateCount;
			}
		}
		break;

	case REMOTECOMMANDTYPE_UPDATE_SOURCE:
		{
			std::string key;
			int line, updateCount;
			bool isRefreshOnly;
			command.GetData().Get_UpdateSource(
				key, line, updateCount, isRefreshOnly);

			// Update info.
			if (updateCount > m_updateCount) {
				m_updateCount = updateCount;
			}
			else {
				++m_updateCount;
				m_engine->SetUpdateCount(m_updateCount);
			}

			// If isRefreshOnly is true, don't change the stack frame.
			if (!isRefreshOnly) {
				m_stackFrame = LuaStackFrame(LuaHandle(), 0);
			}

			if (frame != NULL) {
				wxDebugEvent event(
					wxEVT_DEBUG_UPDATE_SOURCE, wxID_ANY,
					key, line, updateCount, isRefreshOnly);
				frame->ProcessDebugEvent(event, frame, true);
				m_engine->ResponseSuccessed(command);
			}
		}
		break;

	case REMOTECOMMANDTYPE_ADDED_SOURCE:
		{
			Source source;
			command.GetData().Get_AddedSource(source);
			m_sourceManager.AddSource(source);

			if (frame != NULL) {
				wxDebugEvent event(wxEVT_DEBUG_ADDED_SOURCE, wxID_ANY, source);
				frame->ProcessDebugEvent(event, frame, true);
			}
		}
		break;

	case REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST:
		{
			BreakpointList bps(m_engine);
			command.GetData().Get_ChangedBreakpointList(bps);
			m_breakpoints = bps;

			if (frame != NULL) {
				wxDebugEvent event(wxEVT_DEBUG_CHANGED_BREAKPOINTS, wxID_ANY);
				frame->ProcessDebugEvent(event, frame, true);
			}
		}
		break;

	case REMOTECOMMANDTYPE_OUTPUT_LOG:
		if (frame != NULL) {
			LogType logType;
			std::string str, key;
			int line;
			command.GetData().Get_OutputLog(logType, str, key, line);

			wxDebugEvent event(wxEVT_DEBUG_OUTPUT_LOG, wxID_ANY,
				logType, wxConvFromUTF8(str), key, line);
			frame->ProcessDebugEvent(event, frame, true);
		}
		break;

	case REMOTECOMMANDTYPE_SUCCESSED:
	case REMOTECOMMANDTYPE_FAILED:
	case REMOTECOMMANDTYPE_START_CONNECTION:
	case REMOTECOMMANDTYPE_FORCE_UPDATESOURCE:
	case REMOTECOMMANDTYPE_SAVE_SOURCE:
	case REMOTECOMMANDTYPE_SET_BREAKPOINT:
	case REMOTECOMMANDTYPE_REMOVE_BREAKPOINT:
	case REMOTECOMMANDTYPE_BREAK:
	case REMOTECOMMANDTYPE_RESUME:
	case REMOTECOMMANDTYPE_STEPINTO:
	case REMOTECOMMANDTYPE_STEPOVER:
	case REMOTECOMMANDTYPE_STEPRETURN:
	case REMOTECOMMANDTYPE_EVALS_TO_VARLIST:
	case REMOTECOMMANDTYPE_EVAL_TO_MULTIVAR:
	case REMOTECOMMANDTYPE_EVAL_TO_VAR:
	case REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_STACKLIST:
	case REMOTECOMMANDTYPE_REQUEST_BACKTRACELIST:
	case REMOTECOMMANDTYPE_REQUEST_SOURCE:
	case REMOTECOMMANDTYPE_VALUE_STRING:
	case REMOTECOMMANDTYPE_VALUE_VAR:
	case REMOTECOMMANDTYPE_VALUE_VARLIST:
	case REMOTECOMMANDTYPE_VALUE_SOURCE:
	case REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST:
	case REMOTECOMMANDTYPE_VALUE_BACKTRACELIST:
		//BOOST_ASSERT(false && "Invalid remote command.");
		break;
	}
}

} // end of namespace visual
} // end of namespace lldebug

