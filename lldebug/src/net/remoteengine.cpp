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
#include "net/connection.h"
#include "net/remoteengine.h"
#include "net/echostream.h"

namespace lldebug {
namespace net {

using namespace boost::asio::ip;

/**
 * @brief The connection thread starter.
 */
class ThreadObj {
public:
	typedef void (RemoteEngine::* Method)();

	explicit ThreadObj(RemoteEngine *engine, const Method &method)
		: m_engine(engine), m_method(method) {
	}

	void operator()() const {
		(m_engine->*m_method)();
	}

private:
	RemoteEngine *m_engine;
	Method m_method;
};


RemoteEngine::RemoteEngine()
	: m_isExitThread(false), m_commandIdCounter(0) {

	// To avoid duplicating the number.
#ifdef LLDEBUG_CONTEXT
	m_commandIdCounter = 1;
#else
	m_commandIdCounter = 2;
#endif

	ThreadObj fn(this, &RemoteEngine::ConnectionThread);
	m_thread.reset(new boost::thread(fn));
}

RemoteEngine::~RemoteEngine() {
	if (m_connection != NULL) {
		OnConnectionClosed(m_connection, boost::system::error_code());
	}

	{
		scoped_lock lock(m_mutex);
		m_isExitThread = true;
	}

	// We must join the thread.
	if (m_thread != NULL) {
		m_thread->join();
		m_thread.reset();
	}
}

int RemoteEngine::StartFrame(const std::string &serviceName) {
	// Already connected.
	if (m_connection != NULL) {
		return 0;
	}

	// Start connection.
	shared_ptr<ServerConnector> connector(new ServerConnector(*this));
	connector->Start(serviceName);
	return 0;
}

int RemoteEngine::StartContext(const std::string &hostName,
							   const std::string &serviceName,
							   int waitSeconds) {
	// Already connected.
	if (m_connection != NULL) {
		return 0;
	}

	// Start connection.
	shared_ptr<ClientConnector> connector(new ClientConnector(*this));
	connector->Start(hostName, serviceName);

	// Wait for connection if need.
	boost::xtime current, end;
	boost::xtime_get(&end, boost::TIME_UTC);
	end.sec += waitSeconds;

	// IsOpen become true in handleConnect.
	while (!IsConnecting()) {
		boost::xtime_get(&current, boost::TIME_UTC);
		if (boost::xtime_cmp(current, end) >= 0) {
			return -1;
		}

		current.nsec += 100 * 1000 * 1000;
		boost::thread::sleep(current);
	}

	return 0;
}

/// Connection thread.
void RemoteEngine::ConnectionThread() {
	for (;;) {
		{ scoped_lock lock(m_mutex);
			if (m_isExitThread) {
				break;
			}
		}

		// Do works.
		size_t doneWorks = 0;
		try {
			// 10 works are set.
			for (int i = 0; i < 100; ++i) {
				doneWorks += m_service.poll_one();
			}

			m_service.reset();
		}
		catch (std::exception &ex) {
			echo_ostream echo;
			echo << ex.what() << std::endl;
		}

		// Wait, if any ...
		if (doneWorks == 0) {
			boost::xtime xt;
			boost::xtime_get(&xt, boost::TIME_UTC);
			xt.nsec += 10 * 1000 * 1000; // 1ms = 1000 * 1000nsec
			boost::thread::sleep(xt);
		}
	}
}

bool RemoteEngine::OnConnectionConnected(shared_ptr<Connection> connection) {
	scoped_lock lock(m_mutex);

	if (m_connection != NULL) {
		return false;
	}

	m_connection = connection;
	WriteCommand(
		REMOTECOMMANDTYPE_START_CONNECTION,
		CommandData());
	return true;
}

void RemoteEngine::OnConnectionClosed(shared_ptr<Connection> connection,
									  const boost::system::error_code &error) {
	scoped_lock lock(m_mutex);

	if (m_connection == connection) {
		Command command(
			InitCommandHeader(REMOTECOMMANDTYPE_END_CONNECTION, 0),
			CommandData());
		m_readCommandQueue.push(command);

		m_connection.reset();
	}
}

void RemoteEngine::OnRemoteCommand(const Command &command_) {
	scoped_lock lock(m_mutex);
	Command command = command_;

	// First, find a response command.
	WaitResponseCommandList::iterator it = m_waitResponseCommandList.begin();
	while (it != m_waitResponseCommandList.end()) {
		const CommandHeader &header_ = (*it).header;

		if (command.GetCommandId() == header_.commandId) {
			CommandCallback response = (*it).response;
			it = m_waitResponseCommandList.erase(it);
			command.SetResponse(response);
			break;
		}
		else {
			++it;
		}
	}

	m_readCommandQueue.push(command);
}

CommandHeader RemoteEngine::InitCommandHeader(RemoteCommandType type,
											  size_t dataSize,
											  int commandId) {
	scoped_lock lock(m_mutex);
	CommandHeader header;
	header.type = type;
	header.dataSize = (boost::uint32_t)dataSize;

	// Set a new commandId. if commandId == 0
	if (commandId == 0) {
		header.commandId = m_commandIdCounter;
		m_commandIdCounter += 2;
	}
	else {
		header.commandId = commandId;
	}

	return header;
}

void RemoteEngine::WriteCommand(RemoteCommandType type,
								const CommandData &data) {
	scoped_lock lock(m_mutex);

	if (m_connection != NULL) {
		CommandHeader header;
		header = InitCommandHeader(type, data.GetSize());

		m_connection->WriteCommand(header, data);
	}
}

void RemoteEngine::WriteCommand(RemoteCommandType type,
								const CommandData &data,
								const CommandCallback &response) {
	scoped_lock lock(m_mutex);

	if (m_connection != NULL) {
		WaitResponseCommand wcommand;
		wcommand.header = InitCommandHeader(type, data.GetSize());
		wcommand.response = response;

		m_connection->WriteCommand(wcommand.header, data);
		m_waitResponseCommandList.push_back(wcommand);
	}
}

void RemoteEngine::WriteResponse(const Command &readCommand,
								 RemoteCommandType type,
								 const CommandData &data) {
	scoped_lock lock(m_mutex);

	if (m_connection != NULL) {
		CommandHeader header = InitCommandHeader(
			type,
			data.GetSize(),
			readCommand.GetCommandId());

		m_connection->WriteCommand(header, data);
	}
}

void RemoteEngine::ResponseSuccessed(const Command &command) {
	WriteResponse(
		command,
		REMOTECOMMANDTYPE_SUCCESSED,
		CommandData());
}

void RemoteEngine::ResponseFailed(const Command &command) {
	WriteResponse(
		command,
		REMOTECOMMANDTYPE_FAILED,
		CommandData());
}

void RemoteEngine::ChangedState(bool isBreak) {
	CommandData data;

	data.Set_ChangedState(isBreak);
	WriteCommand(
		REMOTECOMMANDTYPE_CHANGED_STATE,
		data);
}

void RemoteEngine::UpdateSource(const std::string &key, int line, int updateSourceCount, const CommandCallback &response) {
	CommandData data;

	data.Set_UpdateSource(key, line, updateSourceCount);
	WriteCommand(
		REMOTECOMMANDTYPE_UPDATE_SOURCE,
		data,
		response);
}

void RemoteEngine::ForceUpdateSource() {
	WriteCommand(
		REMOTECOMMANDTYPE_FORCE_UPDATESOURCE,
		CommandData());
}

void RemoteEngine::AddedSource(const Source &source) {
	CommandData data;

	data.Set_AddedSource(source);
	WriteCommand(
		REMOTECOMMANDTYPE_ADDED_SOURCE,
		data);
}

void RemoteEngine::SaveSource(const std::string &key,
							  const string_array &sources) {
	CommandData data;

	data.Set_SaveSource(key, sources);
	WriteCommand(
		REMOTECOMMANDTYPE_SAVE_SOURCE,
		data);
}

void RemoteEngine::SetUpdateCount(int updateCount) {
	CommandData data;

	data.Set_SetUpdateCount(updateCount);
	WriteCommand(
		REMOTECOMMANDTYPE_SET_UPDATECOUNT,
		data);
}

/// Notify that the breakpoint was set.
void RemoteEngine::SetBreakpoint(const Breakpoint &bp) {
	CommandData data;

	data.Set_SetBreakpoint(bp);
	WriteCommand(
		REMOTECOMMANDTYPE_SET_BREAKPOINT,
		data);
}

void RemoteEngine::RemoveBreakpoint(const Breakpoint &bp) {
	CommandData data;

	data.Set_RemoveBreakpoint(bp);
	WriteCommand(
		REMOTECOMMANDTYPE_REMOVE_BREAKPOINT,
		data);
}

void RemoteEngine::ChangedBreakpointList(const BreakpointList &bps) {
	CommandData data;

	data.Set_ChangedBreakpointList(bps);
	WriteCommand(
		REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST,
		data);
}

void RemoteEngine::Break() {
	WriteCommand(
		REMOTECOMMANDTYPE_BREAK,
		CommandData());
}

void RemoteEngine::Resume() {
	WriteCommand(
		REMOTECOMMANDTYPE_RESUME,
		CommandData());
}

void RemoteEngine::StepInto() {
	WriteCommand(
		REMOTECOMMANDTYPE_STEPINTO,
		CommandData());
}

void RemoteEngine::StepOver() {
	WriteCommand(
		REMOTECOMMANDTYPE_STEPOVER,
		CommandData());
}

void RemoteEngine::StepReturn() {
	WriteCommand(
		REMOTECOMMANDTYPE_STEPRETURN,
		CommandData());
}

void RemoteEngine::OutputLog(LogType type, const std::string &str, const std::string &key, int line) {
	CommandData data;

	data.Set_OutputLog(type, str, key, line);
	WriteCommand(
		REMOTECOMMANDTYPE_OUTPUT_LOG,
		data);
}

/**
 * @brief Handle the response VarList.
 */
struct LuaVarListResponseHandler {
	LuaVarListCallback m_callback;

	explicit LuaVarListResponseHandler(const LuaVarListCallback &callback)
		: m_callback(callback) {
	}

	void operator()(const Command &command) {
		LuaVarList vars;
		command.GetData().Get_ValueVarList(vars);
		m_callback(command, vars);
	}
};

/**
 * @brief Handle the response VarList.
 */
struct LuaVarResponseHandler {
	LuaVarCallback m_callback;

	explicit LuaVarResponseHandler(const LuaVarCallback &callback)
		: m_callback(callback) {
	}

	void operator()(const Command &command) {
		LuaVar var;
		command.GetData().Get_ValueVar(var);
		m_callback(command, var);
	}
};

void RemoteEngine::EvalsToVarList(const string_array &evals,
								  const LuaStackFrame &stackFrame,
								  const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_EvalsToVarList(evals, stackFrame);
	WriteCommand(
		REMOTECOMMANDTYPE_EVALS_TO_VARLIST,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::EvalToMultiVar(const std::string &eval,
								  const LuaStackFrame &stackFrame,
								  const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_EvalToMultiVar(eval, stackFrame);
	WriteCommand(
		REMOTECOMMANDTYPE_EVAL_TO_MULTIVAR,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::EvalToVar(const std::string &eval,
							 const LuaStackFrame &stackFrame,
							 const LuaVarCallback &callback) {
	CommandData data;

	data.Set_EvalToVar(eval, stackFrame);
	WriteCommand(
		REMOTECOMMANDTYPE_EVAL_TO_VAR,
		data,
		LuaVarResponseHandler(callback));
}

void RemoteEngine::RequestFieldsVarList(const LuaVar &var,
										const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_RequestFieldVarList(var);
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::RequestLocalVarList(const LuaStackFrame &stackFrame,
									   const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_RequestLocalVarList(stackFrame);
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::RequestEnvironVarList(const LuaStackFrame &stackFrame,
										 const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_RequestLocalVarList(stackFrame);
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::RequestGlobalVarList(const LuaVarListCallback &callback) {
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
		CommandData(),
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::RequestRegistryVarList(const LuaVarListCallback &callback) {
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
		CommandData(),
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::RequestStackList(const LuaVarListCallback &callback) {
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_STACKLIST,
		CommandData(),
		LuaVarListResponseHandler(callback));
}

/**
 * @brief Handle the response Source.
 */
struct SourceResponseHandler {
	SourceCallback m_callback;

	explicit SourceResponseHandler(const SourceCallback &callback)
		: m_callback(callback) {
	}

	void operator()(const Command &command) {
		Source source;
		command.GetData().Get_ValueSource(source);
		m_callback(command, source);
	}
};

void RemoteEngine::RequestSource(const std::string &key,
								 const SourceCallback &callback) {
	CommandData data;

	data.Set_RequestSource(key);
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_SOURCE,
		data);
}

/**
 * @brief Handle the response BacktraceList.
 */
struct BacktraceListHandler {
	LuaBacktraceListCallback m_callback;

	explicit BacktraceListHandler(const LuaBacktraceListCallback &callback)
		: m_callback(callback) {
	}

	void operator()(const Command &command) {
		LuaBacktraceList bts;
		command.GetData().Get_ValueBacktraceList(bts);
		m_callback(command, bts);
	}
};

void RemoteEngine::RequestBacktraceList(const LuaBacktraceListCallback &callback) {
	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_BACKTRACELIST,
		CommandData(),
		BacktraceListHandler(callback));
}

void RemoteEngine::ResponseString(const Command &command, const std::string &str) {
	CommandData data;

	data.Set_ValueString(str);
	WriteResponse(
		command,
		REMOTECOMMANDTYPE_VALUE_STRING,
		data);
}

void RemoteEngine::ResponseSource(const Command &command, const Source &source) {
	CommandData data;

	data.Set_ValueSource(source);
	WriteResponse(
		command,
		REMOTECOMMANDTYPE_VALUE_SOURCE,
		data);
}

void RemoteEngine::ResponseVarList(const Command &command, const LuaVarList &vars) {
	CommandData data;

	data.Set_ValueVarList(vars);
	WriteResponse(
		command,
		REMOTECOMMANDTYPE_VALUE_VARLIST,
		data);
}

void RemoteEngine::ResponseVar(const Command &command, const LuaVar &var) {
	CommandData data;

	data.Set_ValueVar(var);
	WriteResponse(
		command,
		REMOTECOMMANDTYPE_VALUE_VAR,
		data);
}

void RemoteEngine::ResponseBacktraceList(const Command &command, const LuaBacktraceList &backtraces) {
	CommandData data;

	data.Set_ValueBacktraceList(backtraces);
	WriteResponse(
		command,
		REMOTECOMMANDTYPE_VALUE_BACKTRACELIST,
		data);
}

} // end of namespace net
} // end of namespace lldebug
