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

#include <boost/functional.hpp>

namespace lldebug {

Mediator *Mediator::ms_instance = NULL;

Mediator::Mediator()
	: m_engine(new RemoteEngine), m_frame(NULL)
	, m_breakpoints(m_engine.get()), m_sourceManager(m_engine.get()) {

	// Set the callback called when the end of the reading commands.
	CommandCallback callback = boost::bind1st(
		boost::mem_fn(&Mediator::RemoteCommandCallback), this);
	m_engine->SetReadCommandCallback(callback);
}

Mediator::~Mediator() {
	ms_instance = NULL;
}

int Mediator::GetCtxId() {
	scoped_lock lock(m_mutex);
	return m_engine->GetCtxId();
}

int Mediator::Initialize(const std::string &hostName, const std::string &portName) {
	scoped_lock lock(m_mutex);

	if (m_engine->StartFrame(hostName, portName, 300) != 0) {
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

	void operator()(const Command_ &command) {
		m_mediator->RemoteCommandCallback(command);
	}

private:
	Mediator *m_mediator;
};*/

void Mediator::SetMainFrame(MainFrame *frame) {
	scoped_lock lock(m_mutex);

	m_frame = frame;
}

void Mediator::RemoteCommandCallback(const Command_ &command) {
	scoped_lock lock(m_mutex);
	lock.unlock();

	// Process remote commands.
	switch (command.header.type) {
	case REMOTECOMMANDTYPE_END_CONNECTION:
		wxExit();
		break;

	case REMOTECOMMANDTYPE_CHANGED_STATE:
		if (GetFrame() != NULL) {
			bool isBreak;
			Serializer::ToValue(command.data, isBreak);

			wxChangedStateEvent event(wxEVT_CHANGED_STATE, wxID_ANY, isBreak);
			GetFrame()->AddPendingDebugEvent(event, m_frame, true);
		}
		break;

	case REMOTECOMMANDTYPE_UPDATE_SOURCE:
		if (GetFrame() != NULL) {
			std::string key;
			int line;
			Serializer::ToValue(command.data, key, line);

			wxSourceLineEvent event(wxEVT_UPDATE_SOURCE, wxID_ANY, key, line);
			GetFrame()->AddPendingDebugEvent(event, m_frame, false);
		}
		break;

	case REMOTECOMMANDTYPE_ADDED_SOURCE:
		{
			Source source;
			Serializer::ToValue(command.data, source);
			m_sourceManager.Add(source);

			if (GetFrame() != NULL) {
				wxSourceEvent event(wxEVT_ADDED_SOURCE, wxID_ANY, source);
				GetFrame()->AddPendingDebugEvent(event, m_frame, true);
			}
		}
		break;

	case REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST:
		{
			BreakpointList bps(m_engine.get());
			Serializer::ToValue(command.data, bps);
		
			m_breakpoints = bps;
		}
		break;

	case REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_STACKVARLIST:
	case REMOTECOMMANDTYPE_REQUEST_BREAKPOINTLIST:
	case REMOTECOMMANDTYPE_REQUEST_BACKTRACE:
	case REMOTECOMMANDTYPE_VALUE_VARLIST:
	case REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST:
	case REMOTECOMMANDTYPE_VALUE_BACKTRACE:
		BOOST_ASSERT(false && "Invalid remote command.");
		break;
	}
}

}
