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
#include "net/netutils.h"

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
	: m_commandIdCounter(0), m_isFailed(false), m_isExitThread(false) {

	// To avoid duplicating the Id.
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
		m_onRemoteCommand.clear();
	}

	// We must join the thread.
	if (m_thread != NULL) {
		m_thread->join();
		m_thread.reset();
	}
}

int RemoteEngine::StartFrame(unsigned short port) {
	// Already connected.
	if (m_connection != NULL) {
		return 0;
	}

	// Start connection.
	shared_ptr<ServerConnector> connector(new ServerConnector(*this));
	return connector->Start(port);
}

int RemoteEngine::StartContext(const std::string &hostName,
							  unsigned short port) {
	// Already connected.
	if (m_connection != NULL) {
		return 0;
	}

	// Get a string of the port number.
	char portStr[128];
	snprintf(portStr, sizeof(portStr), "%d", port);

	// Start connection.
	shared_ptr<ClientConnector> connector(new ClientConnector(*this));
	return connector->Start(hostName, portStr);
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
			std::cout << ex.what() << std::endl;
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

void RemoteEngine::OnConnectionFailed() {
	scoped_lock lock(m_mutex);

	m_isFailed = true;
}

bool RemoteEngine::OnConnectionConnected(shared_ptr<Connection> connection) {
	scoped_lock lock(m_mutex);

	if (m_connection != NULL) {
		return false;
	}
	m_connection = connection;

	Command command(
		InitCommandHeader(REMOTECOMMANDTYPE_START_CONNECTION, 0),
		CommandData());
	OnRemoteCommand(command);
	return true;
}

void RemoteEngine::OnConnectionClosed(shared_ptr<Connection> connection,
									 const boost::system::error_code &error) {
	scoped_lock lock(m_mutex);

	if (m_connection == connection) {
		Command command(
			InitCommandHeader(REMOTECOMMANDTYPE_START_CONNECTION, 0),
			CommandData());
		OnRemoteCommand(command);

		m_connection.reset();

#ifdef LLDEBUG_VISUAL
		// Start connection.
		shared_ptr<ServerConnector> connector(new ServerConnector(*this));
		connector->Start(51123);
#endif
	}
}

void RemoteEngine::OnRemoteCommand(Command &command) {
	scoped_lock lock(m_mutex);
	EchoCommand(command);

	// First, find a response command.
	WaitResponseMap::iterator it =
		m_waitResponses.find(command.GetCommandId());
	if (it != m_waitResponses.end()) {
		command.SetResponse((*it).second);
		m_waitResponses.erase(it);
	}

	if (!m_onRemoteCommand.empty()) {
		OnRemoteCommandType callback = m_onRemoteCommand;
		lock.unlock();
		callback(command);
		lock.lock();
	}
}

void RemoteEngine::OutputLog(LogType type, const std::string &msg) {
	scoped_lock lock(m_mutex);
	LogData logData(type, msg);
	CommandData data;

	// Output log to the local.
	data.Set_OutputLog(logData);
	Command command(
		InitCommandHeader(REMOTECOMMANDTYPE_OUTPUT_LOG, data.GetSize()),
		data);
	OnRemoteCommand(command);

	// Output log through the network.
	SendOutputLog(logData);
}

CommandHeader RemoteEngine::InitCommandHeader(RemoteCommandType type,
											 size_t dataSize,
											 int commandId) {
	scoped_lock lock(m_mutex);
	CommandHeader header;
	header.u.type = type;
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

void RemoteEngine::SendCommand(RemoteCommandType type,
							   const CommandData &data) {
	scoped_lock lock(m_mutex);

	if (m_connection != NULL) {
		CommandHeader header;
		header = InitCommandHeader(type, data.GetSize());

		m_connection->WriteCommand(header, data);
	}
}

void RemoteEngine::SendCommand(RemoteCommandType type,
							   const CommandData &data,
							   const CommandCallback &response) {
	scoped_lock lock(m_mutex);

	if (m_connection != NULL) {
		CommandHeader header = InitCommandHeader(type, data.GetSize());

		m_connection->WriteCommand(header, data);
		m_waitResponses.insert(std::make_pair(header.commandId, response));
	}
}

void RemoteEngine::ResponseCommand(const Command &readCommand,
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

void RemoteEngine::SendChangedState(bool isBreak) {
	CommandData data;

	data.Set_ChangedState(isBreak);
	SendCommand(
		REMOTECOMMANDTYPE_CHANGED_STATE,
		data);
}

void RemoteEngine::SendUpdateSource(const std::string &key, int line,
									int updateSourceCount, bool isRefreshOnly,
									const CommandCallback &response) {
	CommandData data;

	data.Set_UpdateSource(key, line, updateSourceCount, isRefreshOnly);
	SendCommand(
		REMOTECOMMANDTYPE_UPDATE_SOURCE,
		data,
		response);
}

void RemoteEngine::SendForceUpdateSource() {
	SendCommand(
		REMOTECOMMANDTYPE_FORCE_UPDATESOURCE,
		CommandData());
}

void RemoteEngine::SendAddedSource(const Source &source) {
	CommandData data;

	data.Set_AddedSource(source);
	SendCommand(
		REMOTECOMMANDTYPE_ADDED_SOURCE,
		data);
}

void RemoteEngine::SendSaveSource(const std::string &key,
								  const string_array &sources) {
	CommandData data;

	data.Set_SaveSource(key, sources);
	SendCommand(
		REMOTECOMMANDTYPE_SAVE_SOURCE,
		data);
}

void RemoteEngine::SendSetUpdateCount(int updateCount) {
	CommandData data;

	data.Set_SetUpdateCount(updateCount);
	SendCommand(
		REMOTECOMMANDTYPE_SET_UPDATECOUNT,
		data);
}

/// Notify that the breakpoint was set.
void RemoteEngine::SendSetBreakpoint(const Breakpoint &bp) {
	CommandData data;

	data.Set_SetBreakpoint(bp);
	SendCommand(
		REMOTECOMMANDTYPE_SET_BREAKPOINT,
		data);
}

void RemoteEngine::SendRemoveBreakpoint(const Breakpoint &bp) {
	CommandData data;

	data.Set_RemoveBreakpoint(bp);
	SendCommand(
		REMOTECOMMANDTYPE_REMOVE_BREAKPOINT,
		data);
}

void RemoteEngine::SendChangedBreakpointList(const BreakpointList &bps) {
	CommandData data;

	data.Set_ChangedBreakpointList(bps);
	SendCommand(
		REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST,
		data);
}

void RemoteEngine::SendBreak() {
	SendCommand(
		REMOTECOMMANDTYPE_BREAK,
		CommandData());
}

void RemoteEngine::SendResume() {
	SendCommand(
		REMOTECOMMANDTYPE_RESUME,
		CommandData());
}

void RemoteEngine::SendStepInto() {
	SendCommand(
		REMOTECOMMANDTYPE_STEPINTO,
		CommandData());
}

void RemoteEngine::SendStepOver() {
	SendCommand(
		REMOTECOMMANDTYPE_STEPOVER,
		CommandData());
}

void RemoteEngine::SendStepReturn() {
	SendCommand(
		REMOTECOMMANDTYPE_STEPRETURN,
		CommandData());
}

void RemoteEngine::SendOutputLog(const LogData &logData) {
	LogData logData_ = logData;
	logData_.SetRemote();

	CommandData data;
	data.Set_OutputLog(logData_);
	SendCommand(
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

	int operator()(const Command &command) {
		LuaVarList vars;
		command.GetData().Get_ValueVarList(vars);
		return m_callback(command, vars);
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

	int operator()(const Command &command) {
		LuaVar var;
		command.GetData().Get_ValueVar(var);
		return m_callback(command, var);
	}
};

void RemoteEngine::SendEvalsToVarList(const string_array &evals,
									  const LuaStackFrame &stackFrame,
									  const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_EvalsToVarList(evals, stackFrame);
	SendCommand(
		REMOTECOMMANDTYPE_EVALS_TO_VARLIST,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::SendEvalToMultiVar(const std::string &eval,
									  const LuaStackFrame &stackFrame,
									  const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_EvalToMultiVar(eval, stackFrame);
	SendCommand(
		REMOTECOMMANDTYPE_EVAL_TO_MULTIVAR,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::SendEvalToVar(const std::string &eval,
								 const LuaStackFrame &stackFrame,
								 const LuaVarCallback &callback) {
	CommandData data;

	data.Set_EvalToVar(eval, stackFrame);
	SendCommand(
		REMOTECOMMANDTYPE_EVAL_TO_VAR,
		data,
		LuaVarResponseHandler(callback));
}

void RemoteEngine::SendRequestFieldsVarList(const LuaVar &var,
											const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_RequestFieldVarList(var);
	SendCommand(
		REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::SendRequestLocalVarList(const LuaStackFrame &stackFrame,
										   bool checkLocal, bool checkUpvalue,
										   bool checkEnviron,
										   const LuaVarListCallback &callback) {
	CommandData data;

	data.Set_RequestLocalVarList(stackFrame, checkLocal, checkUpvalue,
								 checkEnviron);
	SendCommand(
		REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
		data,
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::SendRequestGlobalVarList(const LuaVarListCallback &callback) {
	SendCommand(
		REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
		CommandData(),
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::SendRequestRegistryVarList(const LuaVarListCallback &callback) {
	SendCommand(
		REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
		CommandData(),
		LuaVarListResponseHandler(callback));
}

void RemoteEngine::SendRequestStackList(const LuaVarListCallback &callback) {
	SendCommand(
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

	int operator()(const Command &command) {
		Source source;
		command.GetData().Get_ValueSource(source);
		return m_callback(command, source);
	}
};

void RemoteEngine::SendRequestSource(const std::string &key,
									 const SourceCallback &callback) {
	CommandData data;

	data.Set_RequestSource(key);
	SendCommand(
		REMOTECOMMANDTYPE_REQUEST_SOURCE,
		data,
		SourceResponseHandler(callback));
}

/**
 * @brief Handle the response BacktraceList.
 */
struct BacktraceListHandler {
	LuaBacktraceListCallback m_callback;

	explicit BacktraceListHandler(const LuaBacktraceListCallback &callback)
		: m_callback(callback) {
	}

	int operator()(const Command &command) {
		LuaBacktraceList bts;
		command.GetData().Get_ValueBacktraceList(bts);
		return m_callback(command, bts);
	}
};

void RemoteEngine::SendRequestBacktraceList(const LuaBacktraceListCallback &callback) {
	SendCommand(
		REMOTECOMMANDTYPE_REQUEST_BACKTRACELIST,
		CommandData(),
		BacktraceListHandler(callback));
}


void RemoteEngine::ResponseSuccessed(const Command &command) {
	ResponseCommand(
		command,
		REMOTECOMMANDTYPE_SUCCESSED,
		CommandData());
}

void RemoteEngine::ResponseFailed(const Command &command) {
	ResponseCommand(
		command,
		REMOTECOMMANDTYPE_FAILED,
		CommandData());
}

void RemoteEngine::ResponseString(const Command &command, const std::string &str) {
	CommandData data;

	data.Set_ValueString(str);
	ResponseCommand(
		command,
		REMOTECOMMANDTYPE_VALUE_STRING,
		data);
}

void RemoteEngine::ResponseSource(const Command &command, const Source &source) {
	CommandData data;

	data.Set_ValueSource(source);
	ResponseCommand(
		command,
		REMOTECOMMANDTYPE_VALUE_SOURCE,
		data);
}

void RemoteEngine::ResponseVarList(const Command &command,
								   const LuaVarList &vars) {
	CommandData data;

	data.Set_ValueVarList(vars);
	ResponseCommand(
		command,
		REMOTECOMMANDTYPE_VALUE_VARLIST,
		data);
}

void RemoteEngine::ResponseVar(const Command &command, const LuaVar &var) {
	CommandData data;

	data.Set_ValueVar(var);
	ResponseCommand(
		command,
		REMOTECOMMANDTYPE_VALUE_VAR,
		data);
}

void RemoteEngine::ResponseBacktraceList(const Command &command,
										 const LuaBacktraceList &backtraces) {
	CommandData data;

	data.Set_ValueBacktraceList(backtraces);
	ResponseCommand(
		command,
		REMOTECOMMANDTYPE_VALUE_BACKTRACELIST,
		data);
}

} // end of namespace net
} // end of namespace lldebug
