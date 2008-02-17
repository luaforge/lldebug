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

#include "remotecommand.h"

namespace lldebug {
namespace net {

typedef
	RemoteCommand::RemoteCommandCallback
	RemoteCommandCallback;
typedef
	boost::function2<void, const RemoteCommand &, const std::string &>
	StringCallback;
typedef
	boost::function2<void, const RemoteCommand &, const LuaVarList &>
	LuaVarListCallback;
typedef
	boost::function2<void, const RemoteCommand &, const BreakpointList &>
	BreakpointListCallback;
typedef
	boost::function2<void, const RemoteCommand &, const LuaBacktraceList &>
	BacktraceListCallback;

/**
 * @brief Remote engine for debugger.
 */
class RemoteEngine {
public:
	explicit RemoteEngine();
	virtual ~RemoteEngine();

	/// Start the debuggee program (context).
	int StartContext(int portNum, int ctxId, int waitSeconds);

	/// Start the debugger program (frame).
	int StartFrame(const std::string &hostName, const std::string &portName,
				   int waitSeconds);

	/// Is the socket connected ?
	bool IsConnected();

	RemoteCommand GetCommand();
	void PopCommand();
	bool HasCommand();

	void ResponseSuccessed(const RemoteCommand &command);
	void ResponseFailed(const RemoteCommand &command);

	void ChangedState(bool isBreak);
	void UpdateSource(const std::string &key, int line, int updateCount, const RemoteCommandCallback &response);
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
	void Eval(const std::string &str, const LuaStackFrame &stackFrame,
			  const StringCallback &callback);
	
	void RequestFieldsVarList(const LuaVar &var, const LuaVarListCallback &callback);
	void RequestLocalVarList(const LuaStackFrame &stackFrame, const LuaVarListCallback &callback);
	void RequestEnvironVarList(const LuaStackFrame &stackFrame, const LuaVarListCallback &callback);
	void RequestEvalVarList(const string_array &array, const LuaStackFrame &stackFrame, const LuaVarListCallback &callback);
	void RequestGlobalVarList(const LuaVarListCallback &callback);
	void RequestRegistryVarList(const LuaVarListCallback &callback);
	void RequestStackList(const LuaVarListCallback &callback);

	void ResponseString(const RemoteCommand &command, const std::string &str);
	void ResponseVarList(const RemoteCommand &command, const LuaVarList &vars);
	void ResponseBacktraceList(const RemoteCommand &command, const LuaBacktraceList &backtraces);

	/// Get id of the Context object.
	int GetCtxId() {
		scoped_lock lock(m_mutex);
		return m_ctxId;
	}

private:
	bool IsThreadActive();
	void SetThreadActive(bool is);
	void StartThread();
	void StopThread();
	void DoStartConnection(int ctxId);
	void DoEndConnection();
	void SetCtxId(int ctxId);
	RemoteCommandHeader InitCommandHeader(RemoteCommandType type,
									size_t dataSize,
									int commandId = 0);
	void WriteCommand(RemoteCommandType type,
					  const RemoteCommandData &data);
	void WriteCommand(RemoteCommandType type,
					  const RemoteCommandData &data,
					  const RemoteCommandCallback &callback);
	void WriteResponse(const RemoteCommand &readCommand,
					   RemoteCommandType type,
					   const RemoteCommandData &data);
	void HandleReadCommand(const RemoteCommand &command);
	void ServiceThread();

	friend class SocketBase;

private:
	shared_ptr<thread> m_thread;
	bool m_isThreadActive;
	mutex m_mutex;
	condition m_ctxCond;

	boost::asio::io_service m_ioService;
	boost::shared_ptr<SocketBase> m_socket;
	int m_ctxId;
	boost::uint32_t m_commandIdCounter;

	struct WaitResponseCommand {
		RemoteCommandHeader header;
		RemoteCommandCallback response;
	};
	typedef std::list<WaitResponseCommand> WaitResponseCommandList;
	WaitResponseCommandList m_waitResponseCommandList;

	typedef std::queue<RemoteCommand> ReadCommandQueue;
	ReadCommandQueue m_readCommandQueue; // commands that were read.
};

} // end of namespace net
} // end of namespace lldebug

#endif
