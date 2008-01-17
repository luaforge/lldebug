/*
 */

#ifndef __LLDEBUG_REMOTEENGINE_H__
#define __LLDEBUG_REMOTEENGINE_H__

#include <boost/function.hpp>

#include "lldebug_sysinfo.h"
#include "lldebug_luainfo.h"

namespace lldebug {

enum RemoteCommandType {
	REMOTECOMMANDTYPE_SUCCESSED,
	REMOTECOMMANDTYPE_FAILED,

	REMOTECOMMANDTYPE_START_CONNECTION,
	REMOTECOMMANDTYPE_END_CONNECTION,

	REMOTECOMMANDTYPE_CHANGED_STATE,
	REMOTECOMMANDTYPE_UPDATE_SOURCE,

	REMOTECOMMANDTYPE_BREAK,
	REMOTECOMMANDTYPE_ADD_BREAKPOINT,
	REMOTECOMMANDTYPE_REMOVE_BREAKPOINT,
	REMOTECOMMANDTYPE_TOGGLE_BREAKPOINT,
	REMOTECOMMANDTYPE_CHANGED_BREAKPOINT,

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

struct CommandHeader {
	RemoteCommandType type;
	boost::int32_t ctxId;
	boost::uint32_t commandId;
	boost::uint32_t dataSize;
};

struct Command_ {
	CommandHeader header;
	std::string data;
};

typedef boost::function1<void, const Command_ &> CommandCallback;
typedef boost::function2<void, const Command_ &, bool> FlagCommandCallback;
typedef boost::function2<void, const Command_ &, const LuaVarList &> VarListCommandCallback;

class SocketBase;

class RemoteEngine {
public:
	explicit RemoteEngine();
	virtual ~RemoteEngine();

	int StartDebuggee(int portNum);
	int StartDebugger(const std::string &hostName, int portNum, int waitSeconds);

	bool HasReadCommand();
	Command_ GetReadCommand();
	void PopReadCommand();

	void ResponseSuccessed(const Command_ &command);
	void ResponseFailed(const Command_ &command);
	void StartConnection(int ctxId, const CommandCallback &callback);
	void EndConnection(int ctxId);
	
	void RequestGlobalVarList(int ctxId, const VarListCommandCallback &callback);
	void ResponseVarList(const Command_ &command, const LuaVarList &vars);

private:
	bool IsThreadActive();
	void SetThreadActive(bool is);
	void StartThread();
	void StopThread();
	CommandHeader InitCommandHeader(RemoteCommandType type,
									int ctxId,
									size_t dataSize,
									int commandId = 0);
	void WriteCommand(RemoteCommandType type, int ctxId,
					  const std::string &data);
	void WriteCommand(RemoteCommandType type, int ctxId,
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

	boost::asio::io_service m_ioService;
	boost::shared_ptr<SocketBase> m_socket;
	boost::uint32_t m_commandIdCounter;

	struct WaitResponseCommand {
		CommandHeader header;
		CommandCallback response;
	};
	typedef std::list<WaitResponseCommand> WaitResponseCommandList;
	WaitResponseCommandList m_waitResponseCommandList;

	typedef std::queue<Command_> ReadCommandQueue;
	ReadCommandQueue m_readCommandQueue; // commands that were read.
};

template<class Ty>
std::string SerializeToData(const Ty &value) {
	std::stringstream sstream;
	boost::archive::xml_oarchive ar(sstream);

	ar & BOOST_SERIALIZATION_NVP(value);
	sstream.flush();
	return sstream.str();
}

template<class Ty>
Ty SerializeToValue(const std::string &data) {
	std::stringstream sstream(data);
	boost::archive::xml_iarchive ar(sstream);

	Ty value;
	ar & BOOST_SERIALIZATION_NVP(value);
	return value;
}

}

#endif
