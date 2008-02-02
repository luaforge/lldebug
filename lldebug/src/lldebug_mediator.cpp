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
#include "lldebug_mainframe.h"

namespace lldebug {

Mediator *Mediator::ms_instance = NULL;

Mediator::Mediator()
	: m_engine(new RemoteEngine), m_frame(NULL)
	, m_breakpoints(m_engine.get()), m_sourceManager(m_engine.get())
	, m_updateSourceCount(0) {

	// Set the callback function called when the end of the reading commands.
	CommandCallback callback = boost::bind1st(
		boost::mem_fn(&Mediator::OnRemoteCommand), this);
	m_engine->SetReadCommandCallback(callback);
}

Mediator::~Mediator() {
	if (m_engine != NULL && m_engine->IsConnected()) {
		m_engine->EndConnection();
	}

	ms_instance = NULL;
}

int Mediator::GetCtxId() {
	scoped_lock lock(m_mutex);
	return m_engine->GetCtxId();
}

int Mediator::Initialize(const std::string &hostName, const std::string &portName) {
	scoped_lock lock(m_mutex);

	if (m_engine->StartFrame(hostName, portName, 20) != 0) {
		return -1;
	}

	ms_instance = this;
	return 0;
}

/**
 * @brief Calling the frame->RemoteCommandCallback function.
 * 
 * This class is same as 'boost::bind(&MainFrame::XXX, frame, _1)',
 * but boost.bind consumes too much compile time.
 */
/*struct RemoteCommandHandler {
	explicit RemoteCommandHandler(Mediator *mediator)
		: m_mediator(mediator) {
	}

	void operator()(const Command &command) {
		m_mediator->RemoteCommandCallback(command);
	}

private:
	Mediator *m_mediator;
};*/

void Mediator::SetMainFrame(MainFrame *frame) {
	scoped_lock lock(m_mutex);

	if (m_frame == frame) {
		return;
	}

	// Start or end TCP connection.
	if (frame != NULL) {
		m_engine->StartConnection(GetCtxId());
	}
	else {
		m_engine->EndConnection();
	}

	m_frame = frame;
}

void Mediator::OnRemoteCommand(const Command &command) {
	//scoped_lock lock(m_mutex);

	// Process remote commands.
	switch (command.GetType()) {
	case REMOTECOMMANDTYPE_END_CONNECTION:
		wxExit();
		break;

	case REMOTECOMMANDTYPE_CHANGED_STATE:
		if (m_frame != NULL) {
			bool isBreak;
			command.GetData().Get_ChangedState(isBreak);

			wxChangedStateEvent event(wxEVT_CHANGED_STATE, wxID_ANY, isBreak);
			m_frame->AddPendingDebugEvent(event, m_frame, true);
		}
		break;

	case REMOTECOMMANDTYPE_UPDATE_SOURCE:
		if (m_frame != NULL) {
			std::string key;
			int line, updateSourceCount;
			command.GetData().Get_UpdateSource(key, line, updateSourceCount);

			wxSourceLineEvent event(
				wxEVT_UPDATE_SOURCE, wxID_ANY,
				key, line, updateSourceCount);
			m_updateSourceCount = updateSourceCount;
			m_frame->AddPendingDebugEvent(event, m_frame, true);
			m_engine->ResponseSuccessed(command);
		}
		break;

	case REMOTECOMMANDTYPE_ADDED_SOURCE:
		{
			Source source;
			command.GetData().Get_AddedSource(source);
			m_sourceManager.Add(source);

			if (m_frame != NULL) {
				wxSourceEvent event(wxEVT_ADDED_SOURCE, wxID_ANY, source);
				m_frame->AddPendingDebugEvent(event, m_frame, true);
			}
		}
		break;

	case REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST:
		{
			BreakpointList bps(m_engine.get());
			command.GetData().Get_ChangedBreakpointList(bps);
			{
				scoped_lock lock(m_mutex);
				m_breakpoints = bps;
			}

			if (m_frame != NULL) {
				wxBreakpointEvent event(wxEVT_CHANGED_BREAKPOINTS, wxID_ANY, bps);
				m_frame->AddPendingDebugEvent(event, m_frame, true);
			}
		}
		break;

	case REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_STACKLIST:
	case REMOTECOMMANDTYPE_REQUEST_BACKTRACE:
	case REMOTECOMMANDTYPE_VALUE_VARLIST:
	case REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST:
	case REMOTECOMMANDTYPE_VALUE_BACKTRACELIST:
		BOOST_ASSERT(false && "Invalid remote command.");
		break;
	}
}

}
