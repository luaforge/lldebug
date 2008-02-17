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

#ifndef __LLDEBUG_REMOTECOMMAND_H__
#define __LLDEBUG_REMOTECOMMAND_H__

#include "sysinfo.h"
#include "luainfo.h"

namespace lldebug {
namespace net {

class RemoteEngine;
class SocketBase;

typedef std::vector<char> container_type;

/**
 * @brief Type of the command using TCP connection.
 */
enum RemoteCommandType {
	REMOTECOMMANDTYPE_SUCCESSED,
	REMOTECOMMANDTYPE_FAILED,

	REMOTECOMMANDTYPE_START_CONNECTION,
	REMOTECOMMANDTYPE_END_CONNECTION,

	REMOTECOMMANDTYPE_CHANGED_STATE,
	REMOTECOMMANDTYPE_UPDATE_SOURCE,
	REMOTECOMMANDTYPE_FORCE_UPDATESOURCE,
	REMOTECOMMANDTYPE_ADDED_SOURCE,
	REMOTECOMMANDTYPE_SAVE_SOURCE,
	REMOTECOMMANDTYPE_SET_UPDATECOUNT,

	REMOTECOMMANDTYPE_SET_BREAKPOINT,
	REMOTECOMMANDTYPE_REMOVE_BREAKPOINT,
	REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST,

	REMOTECOMMANDTYPE_BREAK,
	REMOTECOMMANDTYPE_RESUME,
	REMOTECOMMANDTYPE_STEPINTO,
	REMOTECOMMANDTYPE_STEPOVER,
	REMOTECOMMANDTYPE_STEPRETURN,
	REMOTECOMMANDTYPE_OUTPUT_LOG,
	REMOTECOMMANDTYPE_EVAL,

	REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST,
	REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
	REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST,
	REMOTECOMMANDTYPE_REQUEST_EVALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_STACKLIST,
	REMOTECOMMANDTYPE_REQUEST_BACKTRACE,

	REMOTECOMMANDTYPE_VALUE_STRING,
	REMOTECOMMANDTYPE_VALUE_VARLIST,
	REMOTECOMMANDTYPE_VALUE_BACKTRACELIST,
	REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST,
};

/**
 * @brief The header of the command using TCP connection.
 */
struct RemoteCommandHeader {
	RemoteCommandType type;
	boost::int32_t ctxId;
	boost::uint32_t commandId;
	boost::uint32_t dataSize;
};

/**
 * @brief Data type for command contents.
 */
class RemoteCommandData {
public:
	explicit RemoteCommandData();
	explicit RemoteCommandData(const container_type &data);
	~RemoteCommandData();

	/// Get the size of this data.
	container_type::size_type GetSize() const {
		return m_data.size();
	}

	/// Get the impl data of command.
	container_type &GetImplData() {
		return m_data;
	}

	/// Get the impl data of command.
	const container_type &GetImplData() const {
		return m_data;
	}

	/// Get string for debug.
	std::string ToString() const {
		if (m_data.empty()) {
			return std::string("");
		}
		else {
			return std::string(&*m_data.begin(), m_data.size());
		}
	}

public:
	void Get_ChangedState(bool &isBreak) const;
	void Set_ChangedState(bool isBreak);

	void Get_UpdateSource(std::string &key, int &line, int &updateCount) const;
	void Set_UpdateSource(const std::string &key, int line, int updateCount);

	void Get_AddedSource(Source &source) const;
	void Set_AddedSource(const Source &source);

	void Get_SaveSource(std::string &key, string_array &sources) const;
	void Set_SaveSource(const std::string &key, const string_array &sources);

	void Get_SetUpdateCount(int &updateCount) const;
	void Set_SetUpdateCount(int updateCount);

	void Get_SetBreakpoint(Breakpoint &bp) const;
	void Set_SetBreakpoint(const Breakpoint &bp);

	void Get_RemoveBreakpoint(Breakpoint &bp) const;
	void Set_RemoveBreakpoint(const Breakpoint &bp);

	void Get_ChangedBreakpointList(BreakpointList &bps) const;
	void Set_ChangedBreakpointList(const BreakpointList &bps);

	void Get_OutputLog(LogType &type, std::string &str, std::string &key, int &line) const;
	void Set_OutputLog(LogType type, const std::string &str, const std::string &key, int line);

	void Get_Eval(std::string &str, LuaStackFrame &stackFrame) const;
	void Set_Eval(const std::string &str, const LuaStackFrame &stackFrame);

	void Get_RequestFieldVarList(LuaVar &var) const;
	void Set_RequestFieldVarList(const LuaVar &var);

	void Get_RequestLocalVarList(LuaStackFrame &stackFrame) const;
	void Set_RequestLocalVarList(const LuaStackFrame &stackFrame);

	void Get_RequestEnvironVarList(LuaStackFrame &stackFrame) const;
	void Set_RequestEnvironVarList(const LuaStackFrame &stackFrame);

	void Get_RequestEvalVarList(string_array &array, LuaStackFrame &stackFrame) const;
	void Set_RequestEvalVarList(const string_array &array, const LuaStackFrame &stackFrame);

	void Get_ValueString(std::string &str) const;
	void Set_ValueString(const std::string &str);

	void Get_ValueVarList(LuaVarList &vars) const;
	void Set_ValueVarList(const LuaVarList &vars);

	void Get_ValueBacktraceList(LuaBacktraceList &backtraces) const;
	void Set_ValueBacktraceList(const LuaBacktraceList &backtraces);

private:
	container_type m_data;
};

/**
 * @brief The command using TCP connection.
 */
class RemoteCommand {
public:
	typedef
		boost::function1<void, const RemoteCommand &>
		RemoteCommandCallback;

public:
	RemoteCommand(const RemoteCommandHeader &header, const RemoteCommandData &data)
		: m_header(header), m_data(data) {
	}

	explicit RemoteCommand() {
	}

	~RemoteCommand() {
	}

	/// Get the type of this command.
	RemoteCommandType GetType() const {
		return m_header.type;
	}

	/// Get the context id.
	int GetCtxId() const {
		return m_header.ctxId;
	}

	/// Get the command id.
	boost::uint32_t GetCommandId() const {
		return m_header.commandId;
	}

	/// Get the size of this data.
	boost::uint32_t GetDataSize() const {
		return m_header.dataSize;
	}

	/// Get the header of this command.
	RemoteCommandHeader &GetHeader() {
		return m_header;
	}

	/// Get the const header of this command.
	const RemoteCommandHeader &GetHeader() const {
		return m_header;
	}

	/// Get the const data of the command.
	RemoteCommandData &GetData() {
		return m_data;
	}

	/// Get the data of the command.
	const RemoteCommandData &GetData() const {
		return m_data;
	}

	/// Get the const impl data of the command data.
	std::vector<char> &GetImplData() {
		return m_data.GetImplData();
	}

	/// Get the const impl data of the command data.
	const std::vector<char> &GetImplData() const {
		return m_data.GetImplData();
	}

	/// Is this a response command ?
	bool IsResponse() const {
		return (m_response != NULL);
	}

	/// Call response function.
	void CallResponse() {
		m_response(*this);
		m_response.clear();
	}

	/// Get string for debug.
	std::string ToString() const {
		return m_data.ToString();
	}

private:
	friend class RemoteEngine;
	friend class SocketBase;

	/// Resize the impl data.
	void ResizeData() {
		m_data.GetImplData().resize(m_header.dataSize);
	}

	/// Set the response callback.
	void SetResponse(const RemoteCommandCallback &response) {
		m_response = response;
	}

private:
	RemoteCommandHeader m_header;
	RemoteCommandData m_data;
	RemoteCommandCallback m_response;
};

} // end of namespace net
} // end of namespace lldebug

#endif
