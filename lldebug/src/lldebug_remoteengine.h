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
	REMOTECOMMANDTYPE_UPDATE_SOURCE,
	REMOTECOMMANDTYPE_ADDED_SOURCE,

	REMOTECOMMANDTYPE_BREAK,
	REMOTECOMMANDTYPE_RESUME,
	REMOTECOMMANDTYPE_STEPINTO,
	REMOTECOMMANDTYPE_STEPOVER,
	REMOTECOMMANDTYPE_STEPRETURN,

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
class Command_ {
public:
	CommandHeader header;
	std::string data;
};

typedef
	boost::function1<void, const Command_ &>
	CommandCallback;
typedef
	boost::function2<void, const Command_ &, const LuaVarList &>
	LuaVarListCallback;
typedef
	boost::function2<void, const Command_ &, const BreakpointList &>
	BreakpointListCallback;
typedef
	boost::function2<void, const Command_ &, const LuaBacktraceList &>
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

	void ResponseSuccessed(const Command_ &command);
	void ResponseFailed(const Command_ &command);
	void StartConnection(int ctxId);
	void EndConnection();

	void ChangedState(bool isBreak);
	void UpdateSource(const std::string &key, int line);
	void AddedSource(const Source &source);

	void SetBreakpoint(const Breakpoint &bp);
	void RemoveBreakpoint(const Breakpoint &bp);
	void ChangedBreakpointList(const BreakpointList &bps);

	void Break();
	void Resume();
	void StepInto();
	void StepOver();
	void StepReturn();
	
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
	int m_ctxId;
	boost::uint32_t m_commandIdCounter;
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


/**
 * @brief Serializer class
 */
struct Serializer {
	template<class T0>
	static std::string ToStr(const T0 &value0) {
		std::stringstream sstream;
		boost::archive::xml_oarchive ar(sstream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		sstream.flush();
		return sstream.str();
	}

	template<class T0, class T1>
	static std::string ToStr(const T0 &value0, const T1 &value1) {
		std::stringstream sstream;
		boost::archive::xml_oarchive ar(sstream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		ar << BOOST_SERIALIZATION_NVP(value1);
		sstream.flush();
		return sstream.str();
	}

	template<class T0>
	static void ToValue(const std::string &data, T0 &value0) {
		std::stringstream sstream(data);
		boost::archive::xml_iarchive ar(sstream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
	}

	template<class T0, class T1>
	static void ToValue(const std::string &data, T0 &value0, T1 &value1) {
		std::stringstream sstream(data);
		boost::archive::xml_iarchive ar(sstream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
		ar >> BOOST_SERIALIZATION_NVP(value1);
	}
};

}

#endif
