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
	, m_port(0), m_updateCount(0) {

	m_engine->SetOnRemoteCommand(
		boost::bind1st(
			boost::mem_fn(&Mediator::OnRemoteCommand),
			this));
}

Mediator::~Mediator() {
	m_engine.reset();
	ms_instance = NULL;
}

int Mediator::Initialize(unsigned short port) {
	if (m_engine->StartFrame(port) != 0) {
		return -1;
	}

	ms_instance = this;
	m_port = port;
	return 0;
}

void Mediator::SetMainFrame(MainFrame *frame) {
	m_frame = frame;
}

void Mediator::IncUpdateCount() {
	++m_updateCount;
	m_engine->SendSetUpdateCount(m_updateCount);
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

void Mediator::OutputLog(LogType type, const wxString &msg) {
	LogData logData(type, wxConvToUTF8(msg));
	
	OutputLogInternal(logData, true);
}

void Mediator::OutputLogInternal(const LogData &logData, bool sendRemote) {
	if (sendRemote) {
		m_engine->SendOutputLog(logData);
	}

	MainFrame *frame = GetFrame();
	if (frame != NULL) {
		wxDebugEvent event(wxEVT_DEBUG_OUTPUT_LOG, wxID_ANY, logData);
		frame->ProcessDebugEvent(event, frame, true);
	}

	if (logData.GetType() == LOGTYPE_ERROR) {
		wxMessageBox(wxConvFromUTF8(logData.GetLog()), _T("Error"), wxOK | wxICON_ERROR, GetFrame());
	}
}

void Mediator::OnRemoteCommand(const Command &command) {
	m_readCommands.push(command);

	MainFrame *frame = GetFrame();
	if (frame != NULL) {
		wxIdleEvent event;
		frame->AddPendingEvent(event);
	}
}

void Mediator::ProcessAllRemoteCommands() {
	while (!m_readCommands.empty()) {
		Command command = m_readCommands.front();
		m_readCommands.pop();

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
	case REMOTECOMMANDTYPE_START_CONNECTION:
		break;

	case REMOTECOMMANDTYPE_END_CONNECTION:
		m_breakpoints = BreakpointList(m_engine);
		m_sourceManager = SourceManager(m_engine);
		m_stackFrame = LuaStackFrame();
		m_updateCount = 0;
		if (frame != NULL) {
			wxDebugEvent event(wxEVT_DEBUG_END_DEBUG, wxID_ANY);
			frame->ProcessDebugEvent(event, frame, true);
		}

		// Try to start accepting, if possible.
		// Otherwise we shut down this program.
		if (m_port == 0 || m_engine->StartFrame(m_port) != 0) {
			if (frame != NULL) {
				frame->Close();
				wxWakeUpIdle();
			}
			else {
				wxExit();
			}
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
				m_engine->SendSetUpdateCount(m_updateCount);
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
			m_sourceManager.AddSource(source, false);

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
		{
			LogData logData;
			command.GetData().Get_OutputLog(logData);
			OutputLogInternal(logData, false);
		}
		break;

	case REMOTECOMMANDTYPE_FORCE_UPDATESOURCE:
	case REMOTECOMMANDTYPE_SAVE_SOURCE:
	case REMOTECOMMANDTYPE_SET_BREAKPOINT:
	case REMOTECOMMANDTYPE_REMOVE_BREAKPOINT:
	case REMOTECOMMANDTYPE_START:
	case REMOTECOMMANDTYPE_END:
	case REMOTECOMMANDTYPE_STEPINTO:
	case REMOTECOMMANDTYPE_STEPOVER:
	case REMOTECOMMANDTYPE_STEPRETURN:
	case REMOTECOMMANDTYPE_BREAK:
	case REMOTECOMMANDTYPE_RESUME:
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
	case REMOTECOMMANDTYPE_SUCCESSED:
	case REMOTECOMMANDTYPE_FAILED:
	case REMOTECOMMANDTYPE_VALUE_STRING:
	case REMOTECOMMANDTYPE_VALUE_VAR:
	case REMOTECOMMANDTYPE_VALUE_VARLIST:
	case REMOTECOMMANDTYPE_VALUE_SOURCE:
	case REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST:
	case REMOTECOMMANDTYPE_VALUE_BACKTRACELIST:
		BOOST_ASSERT(false && "Invalid remote command.");
		break;
	}
}

} // end of namespace visual
} // end of namespace lldebug

