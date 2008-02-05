/*
 */

#ifndef __LLDEBUG_REMOTEENGINE_H__
#define __LLDEBUG_REMOTEENGINE_H__

#include "lldebug_sysinfo.h"
#include "lldebug_luainfo.h"

namespace lldebug {

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
	REMOTECOMMANDTYPE_ADDED_SOURCE,

	REMOTECOMMANDTYPE_SET_BREAKPOINT,
	REMOTECOMMANDTYPE_REMOVE_BREAKPOINT,
	REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST,

	REMOTECOMMANDTYPE_BREAK,
	REMOTECOMMANDTYPE_RESUME,
	REMOTECOMMANDTYPE_STEPINTO,
	REMOTECOMMANDTYPE_STEPOVER,
	REMOTECOMMANDTYPE_STEPRETURN,
	REMOTECOMMANDTYPE_OUTPUT_LOG,

	REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST,
	REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
	REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST,
	REMOTECOMMANDTYPE_REQUEST_STACKLIST,
	REMOTECOMMANDTYPE_REQUEST_BACKTRACE,

	REMOTECOMMANDTYPE_VALUE_VARLIST,
	REMOTECOMMANDTYPE_VALUE_BACKTRACELIST,
	REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST,
};

/**
 * @brief The header of the command using TCP connection.
 */
struct CommandHeader {
	RemoteCommandType type;
	boost::int32_t ctxId;
	boost::uint32_t commandId;
	boost::uint32_t dataSize;
};

/**
 * @brief Data type for command contents.
 */
class CommandData {
public:
	explicit CommandData();
	explicit CommandData(const std::vector<char> &data);
	~CommandData();

	/// Get the size of this data.
	std::vector<char>::size_type GetSize() const {
		return m_data.size();
	}

	/// Get the impl data of command.
	std::vector<char> &GetImplData() {
		return m_data;
	}

	/// Get the impl data of command.
	const std::vector<char> &GetImplData() const {
		return m_data;
	}

public:
	void Get_ChangedState(bool &isBreak) const;
	void Set_ChangedState(bool isBreak);

	void Get_UpdateSource(std::string &key, int &line, int &updateSourceCount) const;
	void Set_UpdateSource(const std::string &key, int line, int updateSourceCount);

	void Get_AddedSource(Source &source) const;
	void Set_AddedSource(const Source &source);

	void Get_SetBreakpoint(Breakpoint &bp) const;
	void Set_SetBreakpoint(const Breakpoint &bp);

	void Get_RemoveBreakpoint(Breakpoint &bp) const;
	void Set_RemoveBreakpoint(const Breakpoint &bp);

	void Get_ChangedBreakpointList(BreakpointList &bps) const;
	void Set_ChangedBreakpointList(const BreakpointList &bps);

	void Get_OutputLog(LogType &type, std::string &str, std::string &key, int &line) const;
	void Set_OutputLog(LogType type, const std::string &str, const std::string &key, int line);

	void Get_RequestFieldVarList(LuaVar &var) const;
	void Set_RequestFieldVarList(const LuaVar &var);

	void Get_RequestLocalVarList(LuaStackFrame &stackFrame) const;
	void Set_RequestLocalVarList(const LuaStackFrame &stackFrame);

	void Get_ValueVarList(LuaVarList &vars) const;
	void Set_ValueVarList(const LuaVarList &vars);

	void Get_ValueBacktraceList(LuaBacktraceList &backtraces) const;
	void Set_ValueBacktraceList(const LuaBacktraceList &backtraces);

private:
	std::vector<char> m_data;
};

/**
 * @brief The command using TCP connection.
 */
class Command {
public:
	explicit Command();
	explicit Command(const CommandHeader &header, const CommandData &data);
	~Command();

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

	/// Resize the impl data.
	void ResizeData() {
		m_data.GetImplData().resize(m_header.dataSize);
	}

private:
	CommandHeader m_header;
	CommandData m_data;
};

typedef
	boost::function1<void, const Command &>
	CommandCallback;
typedef
	boost::function2<void, const Command &, const LuaVarList &>
	LuaVarListCallback;
typedef
	boost::function2<void, const Command &, const BreakpointList &>
	BreakpointListCallback;
typedef
	boost::function2<void, const Command &, const LuaBacktraceList &>
	BacktraceListCallback;

class SocketBase;

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

	void SetReadCommandCallback(const CommandCallback &callback);

	void ResponseSuccessed(const Command &command);
	void ResponseFailed(const Command &command);
	void StartConnection(int ctxId);
	void EndConnection();

	void ChangedState(bool isBreak);
	void UpdateSource(const std::string &key, int line, int updateSourceCount, const CommandCallback &response);
	void AddedSource(const Source &source);

	void SetBreakpoint(const Breakpoint &bp);
	void RemoveBreakpoint(const Breakpoint &bp);
	void ChangedBreakpointList(const BreakpointList &bps);

	void Break();
	void Resume();
	void StepInto();
	void StepOver();
	void StepReturn();

	void OutputLog(LogType type, const std::string &str, const std::string &key, int line);
	
	void RequestFieldsVarList(const LuaVar &var, const LuaVarListCallback &callback);
	void RequestLocalVarList(const LuaStackFrame &stackFrame, const LuaVarListCallback &callback);
	void RequestGlobalVarList(const LuaVarListCallback &callback);
	void RequestRegistryVarList(const LuaVarListCallback &callback);
	void RequestEnvironVarList(const LuaVarListCallback &callback);
	void RequestStackList(const LuaVarListCallback &callback);
	void ResponseVarList(const Command &command, const LuaVarList &vars);

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
	void SetCtxId(int ctxId);
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
	void HandleReadCommand(const Command &command);
	void ServiceThread();

private:
	shared_ptr<thread> m_thread;
	bool m_isThreadActive;
	mutex m_mutex;
	condition m_ctxCond;

	boost::asio::io_service m_ioService;
	boost::shared_ptr<SocketBase> m_socket;
	int m_ctxId;
	boost::uint32_t m_commandIdCounter;
	CommandCallback m_readCommandCallback;

	struct WaitResponseCommand {
		CommandHeader header;
		CommandCallback response;
	};
	typedef std::list<WaitResponseCommand> WaitResponseCommandList;
	WaitResponseCommandList m_waitResponseCommandList;

	//typedef std::queue<Command> ReadCommandQueue;
	//ReadCommandQueue m_readCommandQueue; // commands that were read.
};

/**
 * @brief This class must share all fields.
 */
struct BooleanCallbackWaiter {
	explicit BooleanCallbackWaiter();
	~BooleanCallbackWaiter();

	/// This method is called from RemoteEngine.
	void operator()(const Command &command);

	/// Wait reponse.
	bool Wait();

private:
	struct Impl;
	shared_ptr<Impl> impl; ///< shared object
};

}

#endif
