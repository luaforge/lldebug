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

	REMOTECOMMANDTYPE_REQUEST_FIELDSVARLIST,
	REMOTECOMMANDTYPE_REQUEST_LOCALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
	REMOTECOMMANDTYPE_REQUEST_REGISTRYVARLIST,
	REMOTECOMMANDTYPE_REQUEST_ENVIRONVARLIST,
	REMOTECOMMANDTYPE_REQUEST_STACKLIST,
	REMOTECOMMANDTYPE_REQUEST_BACKTRACE,

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

/// Data type for command.
typedef std::vector<char> CommandData;

/**
 * @brief The command using TCP connection.
 */
class Command_ {
public:
	CommandHeader header;
	CommandData data;
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
	
	void RequestFieldsVarList(const LuaVar &var, const LuaVarListCallback &callback);
	void RequestLocalVarList(const LuaStackFrame &stackFrame, const LuaVarListCallback &callback);
	void RequestGlobalVarList(const LuaVarListCallback &callback);
	void RequestRegistryVarList(const LuaVarListCallback &callback);
	void RequestEnvironVarList(const LuaVarListCallback &callback);
	void RequestStackList(const LuaVarListCallback &callback);
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
					  const CommandData &data);
	void WriteCommand(RemoteCommandType type,
					  const CommandData &data,
					  const CommandCallback &callback);
	void WriteResponse(const Command_ &readCommand,
					   RemoteCommandType type,
					   const CommandData &data);
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


template <class Ch, class Tr=std::char_traits<Ch> >
class basic_vector_istreambuf : public std::basic_streambuf<Ch,Tr> {
public:
	explicit basic_vector_istreambuf(const std::vector<Ch> &data)
		: m_data(data), m_pos(0) {
		setbuf(0,0); // suppress buffering
		//setg(&*m_data.begin(), &*m_data.begin(), &*m_data.end());
	}

	virtual ~basic_vector_istreambuf() {
	}

protected:
	virtual std::streampos seekoff(
		std::streamoff off,
		std::ios::seek_dir dir,
		int nMode = std::ios::in | std::ios::out) {
		return Tr::eof();
	}

	virtual std::streampos seekpos( 
		std::streampos pos,
		int nMode = std::ios::in | std::ios::out) {
		return Tr::eof();
	}

	virtual int uflow() {
        if (m_pos >= m_data.size()) {
			return Tr::eof();
		}
		
		return Tr::to_int_type(m_data[m_pos++]);
	}

	virtual int underflow() {
		if (m_pos >= m_data.size()) {
			return Tr::eof();
		}
		
		return Tr::to_int_type(m_data[m_pos]);
	}

private:
	const std::vector<Ch> &m_data;
	typename std::vector<Ch>::size_type m_pos;
};

template <class Ch, class Tr=std::char_traits<Ch> >
class basic_vector_ostreambuf : public std::basic_streambuf<Ch,Tr> {
public:
	explicit basic_vector_ostreambuf() {
		setbuf(0,0); // suppress buffering
	}

	virtual ~basic_vector_ostreambuf() {
	}

	std::vector<Ch> container() {
		if (m_data.empty() || m_data.back() != 0) {
			m_data.push_back(0);
		}

		return m_data;
	}

protected:
	virtual std::streampos seekoff(
		std::streamoff off, 
		std::ios::seek_dir dir, 
		int nMode = std::ios::in | std::ios::out) {
		return Tr::eof();
	}

	virtual std::streampos seekpos( 
		std::streampos pos,
		int nMode = std::ios::in | std::ios::out) {
		return Tr::eof();
	}

	virtual int overflow(int c = EOF) {
		m_data.push_back(Tr::to_char_type(c));
		return 0;
	}

private:
	std::vector<Ch> m_data;
};

template <class Ch,class Tr=std::char_traits<Ch> >
class basic_vector_istream : public std::basic_istream<Ch,Tr> {
public:
	explicit basic_vector_istream(const CommandData &data) 
		: std::basic_istream<Ch,Tr>(m_buf = new basic_vector_istreambuf<Ch,Tr>(data)) {
	}

	~basic_vector_istream() {
		if (m_buf != NULL) {
			delete m_buf;
		}
	}

private:
	basic_vector_istreambuf<Ch> *m_buf;
};

template <class Ch,class Tr=std::char_traits<Ch> >
class basic_vector_ostream : public std::basic_ostream<Ch,Tr> {
public:
	explicit basic_vector_ostream() 
		: std::basic_ostream<Ch,Tr>(m_buf = new basic_vector_ostreambuf<Ch,Tr>()) {
	}

	~basic_vector_ostream() {
		if (m_buf != NULL) {
			delete m_buf;
		}
	}

	std::vector<Ch> container() {
		return m_buf->container();
	}

private:
	basic_vector_ostreambuf<Ch> *m_buf;
};


typedef basic_vector_istream<char> vector_istream;
typedef basic_vector_ostream<char> vector_ostream;


/**
 * @brief Serializer class
 */
struct Serializer {
	template<class T0>
	static CommandData ToData(const T0 &value0) {
		vector_ostream stream;
		serialize_oarchive ar(stream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		stream.flush();
		return stream.container();
	}

	template<class T0, class T1>
	static CommandData ToData(const T0 &value0, const T1 &value1) {
		vector_ostream stream;
		serialize_oarchive ar(stream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		ar << BOOST_SERIALIZATION_NVP(value1);
		stream.flush();
		return stream.container();
	}

	template<class T0, class T1, class T2>
	static CommandData ToData(const T0 &value0, const T1 &value1, const T2 &value2) {
		vector_ostream stream;
		serialize_oarchive ar(stream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		ar << BOOST_SERIALIZATION_NVP(value1);
		ar << BOOST_SERIALIZATION_NVP(value2);
		stream.flush();
		return stream.container();
	}

	template<class T0>
	static void ToValue(const CommandData &data, T0 &value0) {
		vector_istream stream(data);
		serialize_iarchive ar(stream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
	}

	template<class T0, class T1>
	static void ToValue(const CommandData &data, T0 &value0, T1 &value1) {
		vector_istream stream(data);
		serialize_iarchive ar(stream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
		ar >> BOOST_SERIALIZATION_NVP(value1);
	}

	template<class T0, class T1, class T2>
	static void ToValue(const CommandData &data, T0 &value0, T1 &value1, T2 &value2) {
		vector_istream stream(data);
		serialize_iarchive ar(stream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
		ar >> BOOST_SERIALIZATION_NVP(value1);
		ar >> BOOST_SERIALIZATION_NVP(value2);
	}
};

}

#endif
