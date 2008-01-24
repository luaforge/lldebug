/*
 */

#ifndef __LLDEBUG_REMOTEENGINE_H__
#define __LLDEBUG_REMOTEENGINE_H__

#include <boost/function.hpp>

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
	REMOTECOMMANDTYPE_ADD_SOURCE,
	REMOTECOMMANDTYPE_UPDATE_SOURCE,

	REMOTECOMMANDTYPE_BREAK,

	REMOTECOMMANDTYPE_SET_BREAKPOINT,
	REMOTECOMMANDTYPE_REMOVE_BREAKPOINT,
	REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST,

	REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
	REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST,
	REMOTECOMMANDTYPE_REQUEST_STACKVARLIST,
	REMOTECOMMANDTYPE_REQUEST_BACKTRACE,
	REMOTECOMMANDTYPE_REQUEST_BREAKPOINTLIST,

	REMOTECOMMANDTYPE_VALUE_VARLIST,
	REMOTECOMMANDTYPE_VALUE_BACKTRACE,
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
 * @brief The command using TCP connection.
 */
struct Command_ {
	CommandHeader header;
	std::string data;
};

/**
 * @brief
 */
class ICommandHandler {
public:
	explicit ICommandHandler() {}
	virtual ~ICommandHandler() {}

	virtual void OnSuccessed(const Command_ &command) {}
	virtual void OnFailed(const Command_ &command) {}

	/// Callback for REMOTECOMMANDTYPE_START_CONNECTION.
	virtual void OnStartConnection(const Command_ &command) {}
	virtual void OnEndConnection(const Command_ &command) {}

	virtual void OnChangedState(const Command_ &command, bool isBreak) {}
	virtual void OnUpdateSource(const Command_ &command, const std::string &key, int line) {}
	virtual void OnAddSource(const Command_ &command, const Source &source) {}

	virtual void OnSetBreakpoint(const Command_ &command, const Breakpoint &bp) {}
	virtual void OnRemoveBreakpoint(const Command_ &command, const Breakpoint &bp) {}
	virtual void OnChangedBreakpointList(const Command_ &command, const BreakpointList &bps) {}

	virtual void OnRequestGlobalVarList(const Command_ &command) {}

/*
	REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
	REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST,
	REMOTECOMMANDTYPE_REQUEST_STACKVARLIST,
	REMOTECOMMANDTYPE_REQUEST_BACKTRACE,
	REMOTECOMMANDTYPE_REQUEST_BREAKPOINTLIST,

	REMOTECOMMANDTYPE_VALUE_VARLIST,
	REMOTECOMMANDTYPE_VALUE_BACKTRACE,
	REMOTECOMMANDTYPE_VALUE_BREAKPOINTLIST,*/
};

typedef boost::function1<void, const Command_ &> CommandCallback;
typedef boost::function2<void, const Command_ &, const LuaVarList &> LuaVarListCallback;
typedef boost::function2<void, const Command_ &, const BreakpointList &> BreakpointListCallback;

class SocketBase;

/**
 * @brief Remote engine for debugger.
 */
class RemoteEngine {
public:
	explicit RemoteEngine();
	virtual ~RemoteEngine();

	/// Start the debuggee program (context).
	int StartContext(int portNum, int ctxId);

	/// Start the debugger program (frame).
	int StartFrame(const std::string &hostName, const std::string &portName, int waitSeconds);

	void SetReadCommandHandler(shared_ptr<ICommandHandler> handler);
	void SetReadCommandCallback(const CommandCallback &callback);

	void ResponseSuccessed(const Command_ &command);
	void ResponseFailed(const Command_ &command);
	void StartConnection();
	void EndConnection();

	void ChangedState(bool isBreak);
	void UpdateSource(const std::string &key, int line);
	void AddSource(const Source &source);

	void SetBreakpoint(const Breakpoint &bp);
	void RemoveBreakpoint(const Breakpoint &bp);
	void ChangedBreakpointList(const BreakpointList &bps);
	
	void RequestGlobalVarList(const LuaVarListCallback &callback);
	void ResponseVarList(const Command_ &command, const LuaVarList &vars);

	/// Get the id of the Context object.
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
					  const std::string &data);
	void WriteCommand(RemoteCommandType type,
					  const std::string &data,
					  const CommandCallback &callback);
	void WriteResponse(const Command_ &readCommand,
					   RemoteCommandType type,
					   const std::string &data);
	void handleReadCommand(const Command_ &command);
	void serviceThread();

private:
	shared_ptr<thread> m_thread;
	bool m_isThreadActive;
	mutex m_mutex;
	condition m_ctxCond;

	boost::asio::io_service m_ioService;
	boost::shared_ptr<SocketBase> m_socket;
	boost::uint32_t m_commandIdCounter;
	int m_ctxId;
	shared_ptr<ICommandHandler> m_readCommandHandler;
	CommandCallback m_readCommandCallback;

	struct WaitResponseCommand {
		CommandHeader header;
		CommandCallback response;
	};
	typedef std::list<WaitResponseCommand> WaitResponseCommandList;
	WaitResponseCommandList m_waitResponseCommandList;

	//typedef std::queue<Command_> ReadCommandQueue;
	//ReadCommandQueue m_readCommandQueue; // commands that were read.
};

}

#endif
