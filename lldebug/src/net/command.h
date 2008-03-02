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

#ifndef __LLDEBUG_COMMAND_H__
#define __LLDEBUG_COMMAND_H__

#include "sysinfo.h"
#include "luainfo.h"

namespace lldebug {
namespace net {

class RemoteEngine;
class Connection;

/// Internal type of the command data impl.
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

	REMOTECOMMANDTYPE_EVALS_TO_VARLIST,
	REMOTECOMMANDTYPE_EVAL_TO_MULTIVAR,
	REMOTECOMMANDTYPE_EVAL_TO_VAR,

	REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST,
	REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
	REMOTECOMMANDTYPE_REQUEST_STACKLIST,
	REMOTECOMMANDTYPE_REQUEST_SOURCE,
	REMOTECOMMANDTYPE_REQUEST_BACKTRACELIST,

	REMOTECOMMANDTYPE_VALUE_STRING,
	REMOTECOMMANDTYPE_VALUE_SOURCE,
	REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST,
	REMOTECOMMANDTYPE_VALUE_VARLIST,
	REMOTECOMMANDTYPE_VALUE_VAR,
	REMOTECOMMANDTYPE_VALUE_BACKTRACELIST,
};

/**
 * @brief The header of the command using TCP connection.
 */
struct CommandHeader {
	RemoteCommandType type;
	boost::uint32_t commandId;
	boost::uint32_t dataSize;
};

/**
 * @brief Data type for command contents.
 */
class CommandData {
public:
	explicit CommandData();
	explicit CommandData(const container_type &data);
	~CommandData();

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

	void Get_UpdateSource(std::string &key, int &line, int &updateCount,
						  bool &isRefreshOnly) const;
	void Set_UpdateSource(const std::string &key, int line, int updateCount,
						  bool isRefreshOnly);

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

	void Get_EvalsToVarList(string_array &evals, LuaStackFrame &stackFrame) const;
	void Set_EvalsToVarList(const string_array &evals, const LuaStackFrame &stackFrame);

	void Get_EvalToMultiVar(std::string &eval, LuaStackFrame &stackFrame) const;
	void Set_EvalToMultiVar(const std::string &eval, const LuaStackFrame &stackFrame);

	void Get_EvalToVar(std::string &eval, LuaStackFrame &stackFrame) const;
	void Set_EvalToVar(const std::string &eval, const LuaStackFrame &stackFrame);

	void Get_RequestFieldVarList(LuaVar &var) const;
	void Set_RequestFieldVarList(const LuaVar &var);

	void Get_RequestLocalVarList(LuaStackFrame &stackFrame, bool &checkLocal,
								 bool &checkUpvalue, bool &checkEnviron) const;
	void Set_RequestLocalVarList(const LuaStackFrame &stackFrame, bool checkLocal,
								 bool checkUpvalue, bool checkEnviron);

	void Get_RequestSource(std::string &key);
	void Set_RequestSource(const std::string &key);

	void Get_ValueString(std::string &str) const;
	void Set_ValueString(const std::string &str);

	void Get_ValueSource(Source &source) const;
	void Set_ValueSource(const Source &source);

	void Get_ValueVarList(LuaVarList &vars) const;
	void Set_ValueVarList(const LuaVarList &vars);

	void Get_ValueVar(LuaVar &var) const;
	void Set_ValueVar(const LuaVar &var);

	void Get_ValueBacktraceList(LuaBacktraceList &backtraces) const;
	void Set_ValueBacktraceList(const LuaBacktraceList &backtraces);

private:
	container_type m_data;
};

/**
 * @brief The command using TCP connection.
 */
class Command {
public:
	typedef
		boost::function1<int, const Command &>
		CommandCallback;

public:
	explicit Command(const CommandHeader &header,
					 const CommandData &data)
		: m_header(header), m_data(data) {
	}

	explicit Command() {
	}

	~Command() {
	}

	/// Get the type of this command.
	RemoteCommandType GetType() const {
		return m_header.type;
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
	CommandHeader &GetHeader() {
		return m_header;
	}

	/// Get the const header of this command.
	const CommandHeader &GetHeader() const {
		return m_header;
	}

	/// Get the const data of the command.
	CommandData &GetData() {
		return m_data;
	}

	/// Get the data of the command.
	const CommandData &GetData() const {
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
	friend class Connection;

	/// Resize the impl data.
	void ResizeData() {
		m_data.GetImplData().resize(m_header.dataSize);
	}

	/// Set the response callback.
	void SetResponse(const CommandCallback &response) {
		m_response = response;
	}

private:
	CommandHeader m_header;
	CommandData m_data;
	CommandCallback m_response;
};

} // end of namespace net
} // end of namespace lldebug

#endif
