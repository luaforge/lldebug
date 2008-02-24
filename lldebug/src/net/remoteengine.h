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

#ifndef __LLDEBUG_REMOTEENGINE_H__
#define __LLDEBUG_REMOTEENGINE_H__

#include "net/command.h"

namespace lldebug {
namespace net {

class Connection;

typedef
	Command::CommandCallback
	CommandCallback;
typedef
	boost::function2<void, const Command &, const std::string &>
	StringCallback;
typedef
	boost::function2<void, const Command &, const Source &>
	SourceCallback;
typedef
	boost::function2<void, const Command &, const BreakpointList &>
	BreakpointListCallback;
typedef
	boost::function2<void, const Command &, const LuaVarList &>
	LuaVarListCallback;
typedef
	boost::function2<void, const Command &, const LuaVar &>
	LuaVarCallback;
typedef
	boost::function2<void, const Command &, const LuaBacktraceList &>
	BacktraceListCallback;

/**
 * @brief Remote engine for debugger.
 */
class RemoteEngine {
public:
	explicit RemoteEngine();
	virtual ~RemoteEngine();

	/// Get the asio::io_service object.
	boost::asio::io_service &GetService() {
		return m_service;
	}

	/// Is this connecting ?
	bool IsConnecting() {
		scoped_lock lock(m_mutex);
		return (m_connection != NULL);
	}

	/// Get the read command.
	Command GetCommand() {
		scoped_lock lock(m_mutex);
		return m_readCommandQueue.front();
	}

	/// Pop the read command.
	void PopCommand() {
		scoped_lock lock(m_mutex);
		m_readCommandQueue.pop();
	}

	/// Has this any read commands ?
	bool HasCommands() {
		scoped_lock lock(m_mutex);
		return !m_readCommandQueue.empty();
	}

	/// Start the debugger program (frame).
	int StartFrame(const std::string &serviceName);

	/// Start the debuggee program (context).
	int StartContext(const std::string &hostName, const std::string &serviceName, int waitSeconds);

	void ResponseSuccessed(const Command &command);
	void ResponseFailed(const Command &command);

	void ChangedState(bool isBreak);
	void UpdateSource(const std::string &key, int line, int updateCount, const CommandCallback &response);
	void ForceUpdateSource();
	void AddedSource(const Source &source);
	void SaveSource(const std::string &key, const string_array &sources);
	void SetUpdateCount(int updateCount);

	void SetBreakpoint(const Breakpoint &bp);
	void RemoveBreakpoint(const Breakpoint &bp);
	void ChangedBreakpointList(const BreakpointList &bps);

	void Break();
	void Resume();
	void StepInto();
	void StepOver();
	void StepReturn();

	void OutputLog(LogType type, const std::string &str, const std::string &key, int line);
	void EvalsToVarList(const string_array &eval, const LuaStackFrame &stackFrame,
						const LuaVarListCallback &callback);
	void EvalToMultiVar(const std::string &eval, const LuaStackFrame &stackFrame,
						const LuaVarListCallback &callback);
	void EvalToVar(const std::string &eval, const LuaStackFrame &stackFrame,
				   const LuaVarCallback &callback);
	
	void RequestFieldsVarList(const LuaVar &var, const LuaVarListCallback &callback);
	void RequestLocalVarList(const LuaStackFrame &stackFrame, const LuaVarListCallback &callback);
	void RequestEnvironVarList(const LuaStackFrame &stackFrame, const LuaVarListCallback &callback);
	void RequestGlobalVarList(const LuaVarListCallback &callback);
	void RequestRegistryVarList(const LuaVarListCallback &callback);
	void RequestStackList(const LuaVarListCallback &callback);
	void RequestSource(const std::string &key, const SourceCallback &callback);

	void ResponseString(const Command &command, const std::string &str);
	void ResponseSource(const Command &command, const Source &source);
	void ResponseBacktraceList(const Command &command, const LuaBacktraceList &backtraces);
	void ResponseVarList(const Command &command, const LuaVarList &vars);
	void ResponseVar(const Command &command, const LuaVar &var);

private:
	void ConnectionThread();

private:
	friend class Connection;
	bool OnConnectionConnected(shared_ptr<Connection> connection);
	void OnConnectionClosed(shared_ptr<Connection> connection,
							const boost::system::error_code &error);
	void OnRemoteCommand(const Command &command);

private:
	CommandHeader InitCommandHeader(RemoteCommandType type,
										  size_t dataSize,
										  int commandId = 0);
	void WriteCommand(RemoteCommandType type,
					  const CommandData &data);
	void WriteCommand(RemoteCommandType type,
					  const CommandData &data,
					  const CommandCallback &callback);
	void WriteResponse(const Command &readCommand,
					   RemoteCommandType type,
					   const CommandData &data);

private:
	boost::asio::io_service m_service;
	boost::shared_ptr<Connection> m_connection;
	boost::uint32_t m_commandIdCounter;

	shared_ptr<thread> m_thread;
	bool m_isExitThread;
	mutex m_mutex;

	struct WaitResponseCommand {
		CommandHeader header;
		CommandCallback response;
	};
	typedef std::list<WaitResponseCommand> WaitResponseCommandList;
	WaitResponseCommandList m_waitResponseCommandList;

	typedef std::queue<Command> ReadCommandQueue;
	ReadCommandQueue m_readCommandQueue; // commands that were read.
};

} // end of namespace net
} // end of namespace lldebug

#endif
