/*
 */

#include "lldebug_prec.h"
#include "lldebug_remoteengine.h"

#include <boost/bind.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <sstream>

namespace lldebug {

using boost::asio::ip::tcp;

class SocketBase {
private:
	boost::asio::io_service &m_ioService;
	boost::asio::ip::tcp::socket m_socket;
	CommandCallback m_callback;
	mutex m_mutex;
	
	typedef std::queue<Command_> WriteCommandQueue;
	WriteCommandQueue m_writeCommandQueue;
	
public:
	/// Write command data.
	void Write(const CommandHeader &header, const std::string &data) {
		Command_ writeCommand;
		writeCommand.header = header;
		writeCommand.data = data;

		m_ioService.post(boost::bind(&SocketBase::doWrite, this, writeCommand));
	}

	/// Close this socket.
	void Close() {
		m_ioService.post(boost::bind(&SocketBase::doClose, this));
	}

	/// Get the io_service object.
	boost::asio::io_service &GetService() {
		scoped_lock lock(m_mutex);
		return m_ioService;
	}

	/// Get the socket object.
	boost::asio::ip::tcp::socket &GetSocket() {
		scoped_lock lock(m_mutex);
		return m_socket;
	}

	/// Get the callback.
	CommandCallback GetCallback() {
		scoped_lock lock(m_mutex);
		return m_callback;
	}

	/// Get the mutex object.
	mutex &GetMutex() {
		return m_mutex;
	}

protected:
	SocketBase(boost::asio::io_service &ioService,
			   const CommandCallback &callback)
		: m_ioService(ioService)
		, m_socket(ioService), m_callback(callback) {
	}

	virtual ~SocketBase() {
	}

	void asyncReadCommand() {
		scoped_lock lock(m_mutex);
		shared_ptr<Command_> command(new Command_);

		boost::asio::async_read(m_socket,
			boost::asio::buffer(&command->header, sizeof(CommandHeader)),
			boost::bind(
				&SocketBase::handleReadCommand, this, command,
				boost::asio::placeholders::error));
	}

	void handleReadCommand(shared_ptr<Command_> command,
						   const boost::system::error_code &error) {
		scoped_lock lock(m_mutex);

		if (!error) {
			// Check that response is OK.
			if (command->header.ctxId < 0) {
				doClose();
			}

			command->data.resize(command->header.dataSize + 1);

			if (command->header.dataSize > 0) {
				boost::asio::async_read(m_socket,
					boost::asio::buffer(command->data, command->header.dataSize),
					boost::bind(
						&SocketBase::handleReadData, this, command,
						boost::asio::placeholders::error));
			}
			else {
				handleReadData(command, error);
			}
		}
		else {
			doClose();
		}
	}

	void handleReadData(shared_ptr<Command_> command,
						const boost::system::error_code &error) {
		scoped_lock lock(m_mutex);

		if (!error) {
			m_callback(*command);
			asyncReadCommand();
		}
		else {
			doClose();
		}
	}

	void asyncWrite(const Command_ *command) {
		scoped_lock lock(m_mutex);

		boost::asio::async_write(m_socket,
			boost::asio::buffer(&command->header, sizeof(CommandHeader)),
			boost::bind(
			&SocketBase::handleWrite, this,
			boost::asio::placeholders::error));

		if (command->header.dataSize > 0) {
			boost::asio::async_write(m_socket,
				boost::asio::buffer(command->data, command->header.dataSize),
				boost::bind(
				&SocketBase::handleWrite, this,
				boost::asio::placeholders::error));
		}
	}

	void doWrite(const Command_ &command) {
		scoped_lock lock(m_mutex);
		bool isProgress = !m_writeCommandQueue.empty();
		m_writeCommandQueue.push(command);

		if (!isProgress) {
			asyncWrite(&m_writeCommandQueue.front());
		}
	}

	void handleWrite(const boost::system::error_code& error) {
		scoped_lock lock(m_mutex);

		if (!error) {
			m_writeCommandQueue.pop();
			if (!m_writeCommandQueue.empty()) {
				asyncWrite(&m_writeCommandQueue.front());
			}
		}
		else {
			doClose();
		}
	}

	void doClose() {
		scoped_lock lock(m_mutex);

		m_socket.close();
	}
};


/*-----------------------------------------------------------------*/
class FrameSocket : public SocketBase {
private:
	tcp::acceptor m_acceptor;

public:
	explicit FrameSocket(boost::asio::io_service &ioService,
						 const CommandCallback &callback,
						 tcp::endpoint endpoint)
		: SocketBase(ioService, callback)
		, m_acceptor(ioService, endpoint) {

		m_acceptor.async_accept(GetSocket(),
			boost::bind(
				&FrameSocket::handleAccept, this,
				boost::asio::placeholders::error));
	}

	virtual ~FrameSocket() {
	}

protected:
	void handleAccept(const boost::system::error_code &error) {
		scoped_lock lock(GetMutex());

		if (!error) {
			this->asyncReadCommand();

			Command_ command;
			command.header.ctxId = -1;
			command.header.dataSize = 0;
			command.header.commandId = -1;
			//command.header.type = REMOTECOMMANDTYPE_SUCCESS_CONNECTION;
			GetCallback()(command);
		}
		else {
			m_acceptor.async_accept(GetSocket(),
				boost::bind(
					&FrameSocket::handleAccept, this,
					boost::asio::placeholders::error));
		}
	}
};


/*-----------------------------------------------------------------*/
class ContextSocket : public SocketBase {
private:
	tcp::resolver::iterator m_endpointIterator;
	bool m_isStartSuccessed;

public:
	explicit ContextSocket(boost::asio::io_service &ioService,
						   const CommandCallback &callback,
						   tcp::resolver::query query)
		: SocketBase(ioService, callback)
		, m_isStartSuccessed(false) {
		tcp::resolver resolver(ioService);
		m_endpointIterator = resolver.resolve(query);
	}

	virtual ~ContextSocket() {
	}

	/// Start connection.
	int Start(int waitSeconds) {
		scoped_lock lock(GetMutex());
		tcp::resolver::iterator endpoint_iterator = m_endpointIterator;
		tcp::endpoint endpoint = *endpoint_iterator;

		GetSocket().async_connect(endpoint,
			boost::bind(
				&ContextSocket::handleConnect, this,
				boost::asio::placeholders::error, endpoint_iterator));

		// Wait for connection if need.
		if (waitSeconds >= 0) {
			boost::xtime current, end;
			boost::xtime_get(&end, boost::TIME_UTC);
			end.sec += waitSeconds;

			// m_isStartSuccessed become true in handleConnect.
			while (!m_isStartSuccessed) {
				boost::xtime_get(&current, boost::TIME_UTC);
				if (boost::xtime_cmp(current, end) >= 0) {
					return -1;
				}

				// Do async operation.
				GetService().poll_one();
				GetService().reset();
				boost::thread::yield();
			}
		}

		this->asyncReadCommand();
		return 0;
	}

protected:
	virtual void handleConnect(const boost::system::error_code &error,
							   tcp::resolver::iterator endpointIterator) {
		scoped_lock lock(GetMutex());

		if (!error) {
			// The connection was successful.
			m_isStartSuccessed = true;
		}
		else {
			//if (endpointIterator != tcp::resolver::iterator()) {
			// The connection failed. Try the next endpoint in the list.
			tcp::endpoint endpoint = *endpointIterator;

			GetSocket().close();
			GetSocket().async_connect(endpoint,
				boost::bind(
					&ContextSocket::handleConnect, this,
					boost::asio::placeholders::error, endpointIterator));
		}
		/*else {
			std::cout << "Error: " << error << "\n";
		}*/
	}
};


/*-----------------------------------------------------------------*/
RemoteEngine::RemoteEngine()
	: m_isThreadActive(false), m_commandIdCounter(0)
	, m_ctxId(-1) {
}

RemoteEngine::~RemoteEngine() {
	StopThread();
}

int RemoteEngine::StartContext(int portNum, int ctxId) {
	scoped_lock lock(m_mutex);
	shared_ptr<ContextSocket> socket;

	// limit time
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.sec += waitSeconds;

	// Create socket object.
	socket.reset(new ContextSocket(m_ioService,
		boost::bind(
			&RemoteEngine::handleReadCommand, this, _1),
			tcp::resolver::query(hostName, portName)));

	// Start connection.
	if (socket->Start(waitSeconds) != 0) {
		return -1;
	}

	StartThread();
	m_commandIdCounter = 2;
	m_socket = boost::shared_static_cast<SocketBase>(socket);

	// Wait for START_CONNECTION and ctxId value.
	if (m_ctxId < 0) {
		m_ctxCond.timed_wait(lock, xt);

		if (m_ctxId < 0) {
			return -1;
		}
	}

	return 0;
}

int RemoteEngine::StartFrame(const std::string &hostName,
							 const std::string &portName,
							 int waitSeconds) {
	scoped_lock lock(m_mutex);
	shared_ptr<FrameSocket> socket;

	socket.reset(new FrameSocket(m_ioService,
		boost::bind(
			&RemoteEngine::handleReadCommand, this, _1),
			tcp::endpoint(tcp::v4(), portNum)));

	StartThread();
	m_commandIdCounter = 1;
	m_ctxId = ctxId;
	m_socket = boost::shared_static_cast<SocketBase>(socket);
	return 0;
}

void RemoteEngine::SetCtxId(int ctxId) {
	scoped_lock lock(m_mutex);

	if (ctxId != ctxId) {
		m_ctxId = ctxId;
		m_ctxCond.notify_all();
	}
}

void RemoteEngine::SetReadCommandHandler(shared_ptr<ICommandHandler> handler) {
	scoped_lock lock(m_mutex);

	m_readCommandHandler = handler;
}

void RemoteEngine::SetReadCommandCallback(const CommandCallback &callback) {
	scoped_lock lock(m_mutex);

	m_readCommandCallback = callback;
}

bool RemoteEngine::IsThreadActive() {
	scoped_lock lock(m_mutex);

	return m_isThreadActive;
}

void RemoteEngine::SetThreadActive(bool is) {
	scoped_lock lock(m_mutex);

	m_isThreadActive = is;
}

void RemoteEngine::StartThread() {
	scoped_lock lock(m_mutex);

	if (!IsThreadActive()) {
		boost::function0<void> fn
			= boost::bind(&RemoteEngine::serviceThread, this);
		m_thread.reset(new boost::thread(fn));
	}
}

void RemoteEngine::StopThread() {
	if (IsThreadActive()) {
		SetThreadActive(false);

		m_thread->join();
		m_thread.reset();
	}
}

void RemoteEngine::serviceThread() {
	// If IsThreadActive is true, the thread has already started.
	if (IsThreadActive()) {
		return;
	}

	SetThreadActive(true);
	try {
		while (IsThreadActive()) {
			// 10 works are set.
			for (int i = 0; i < 10; ++i) {
				m_ioService.poll_one();
			}

			m_ioService.reset();
			boost::thread::yield();
		}
	}
	catch (std::exception &) {
	}

	SetThreadActive(false);
}

template<class T0>
static std::string SerializeToStr(const T0 &value0) {
	std::stringstream sstream;
	boost::archive::xml_oarchive ar(sstream);

	ar << BOOST_SERIALIZATION_NVP(value0);
	sstream.flush();
	return sstream.str();
}

template<class T0, class T1>
static std::string SerializeToStr(const T0 &value0, const T1 &value1) {
	std::stringstream sstream;
	boost::archive::xml_oarchive ar(sstream);

	ar << BOOST_SERIALIZATION_NVP(value0);
	ar << BOOST_SERIALIZATION_NVP(value1);
	sstream.flush();
	return sstream.str();
}

template<class T0>
static void DeserializeToValue(const std::string &data, T0 &value0) {
	std::stringstream sstream(data);
	boost::archive::xml_iarchive ar(sstream);

	ar >> BOOST_SERIALIZATION_NVP(value0);
}

template<class T0, class T1>
static void DeserializeToValue(const std::string &data, T0 &value0, T1 &value1) {
	std::stringstream sstream(data);
	boost::archive::xml_iarchive ar(sstream);

	ar >> BOOST_SERIALIZATION_NVP(value0);
	ar >> BOOST_SERIALIZATION_NVP(value1);
}

void RemoteEngine::handleReadCommand(const Command_ &command) {
	scoped_lock lock(m_mutex);
	bool isResponseCommand = false;

	// First, find a response command.
	WaitResponseCommandList::iterator it = m_waitResponseCommandList.begin();
	while (it != m_waitResponseCommandList.end()) {
		const CommandHeader &header_ = (*it).header;

		if (command.header.ctxId == header_.ctxId
			&& command.header.commandId == header_.commandId) {
			isResponseCommand = true;
			(*it).response(command);
			it = m_waitResponseCommandList.erase(it);
		}
		else {
			++it;
		}
	}

	switch (command.header.type) {
	case REMOTECOMMANDTYPE_START_CONNECTION:
		SetCtxId(command.header.ctxId);
		//ResponseSuccessed(command);
		break;
	case REMOTECOMMANDTYPE_END_CONNECTION:
		SetCtxId(-1);
		break;
	}

	if (!isResponseCommand) {
		if (m_readCommandHandler != NULL) {
			// m_readCommandHandler may be changed.
			shared_ptr<ICommandHandler> handler = m_readCommandHandler;

			lock.unlock();
			switch (command.header.type) {
			case REMOTECOMMANDTYPE_CHANGED_STATE: {
				bool isBreak;
				DeserializeToValue(command.data, isBreak);
				handler->OnChangedState(command, isBreak);
				}
				break;
			case REMOTECOMMANDTYPE_UPDATE_SOURCE: {
				std::string key;
				int line;
				DeserializeToValue(command.data, key, line);
				handler->OnUpdateSource(command, key, line);
				}
				break;
			case REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST: {
				BreakpointList bps(this);
				DeserializeToValue(command.data, bps);
				handler->OnChangedBreakpointList(command, bps);
				}
				break;
			}
			lock.lock();
		}

		if (m_readCommandCallback) {
			CommandCallback callback = m_readCommandCallback;

			lock.unlock();
			callback(command);
			lock.lock();
		}
	}
}

CommandHeader RemoteEngine::InitCommandHeader(RemoteCommandType type,
											  size_t dataSize,
											  int commandId) {
	scoped_lock lock(m_mutex);
	CommandHeader header;

	header.type = type;
	header.ctxId = m_ctxId;
	header.dataSize = (boost::uint32_t)dataSize;

	// Set a new value to header.commandId if commandId == 0
	if (commandId == 0) {
		header.commandId = m_commandIdCounter;
		m_commandIdCounter += 2;
	}
	else {
		header.commandId = commandId;
	}

	return header;
}

void RemoteEngine::WriteCommand(RemoteCommandType type,
								const std::string &data) {
	scoped_lock lock(m_mutex);
	CommandHeader header;

	header = InitCommandHeader(type, data.size());
	m_socket->Write(header, data);
}

void RemoteEngine::WriteCommand(RemoteCommandType type,
								const std::string &data,
								const CommandCallback &response) {
	scoped_lock lock(m_mutex);
	WaitResponseCommand wcommand;

	wcommand.header = InitCommandHeader(type, data.size());
	wcommand.response = response;
	m_socket->Write(wcommand.header, data);
	m_waitResponseCommandList.push_back(wcommand);
}

void RemoteEngine::WriteResponse(const Command_ &readCommand,
								 RemoteCommandType type,
								 const std::string &data) {
	scoped_lock lock(m_mutex);
	CommandHeader header;

	header = InitCommandHeader(
		type,
		data.size(),
		readCommand.header.commandId);
	m_socket->Write(header, data);
}

void RemoteEngine::ResponseSuccessed(const Command_ &command) {
	scoped_lock lock(m_mutex);

	WriteResponse(command, REMOTECOMMANDTYPE_SUCCESSED, "");
}

void RemoteEngine::ResponseFailed(const Command_ &command) {
	scoped_lock lock(m_mutex);

	WriteResponse(command, REMOTECOMMANDTYPE_FAILED, "");
}

void RemoteEngine::StartConnection() {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_START_CONNECTION,
		"");
}

void RemoteEngine::EndConnection() {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_END_CONNECTION,
		"");
}

void RemoteEngine::ChangedState(bool isBreak) {
}

void RemoteEngine::UpdateSource(const std::string &key, int line) {
}

void RemoteEngine::AddSource(const Source &source) {
}

/// Notify that the breakpoint was set.
void RemoteEngine::SetBreakpoint(const Breakpoint &bp) {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_SET_BREAKPOINT,
		SerializeToStr(bp));
}

void RemoteEngine::RemoveBreakpoint(const Breakpoint &bp) {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_REMOVE_BREAKPOINT,
		SerializeToStr(bp));
}

void RemoteEngine::ChangedBreakpointList(const BreakpointList &bps) {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_CHANGED_BREAKPOINTLIST,
		SerializeToStr(bps));
}

struct VarListResponseHandler {
	LuaVarListCallback m_callback;

	explicit VarListResponseHandler(const LuaVarListCallback &callback)
		: m_callback(callback) {
	}

	void operator()(const Command_ &command) {
		LuaVarList vars;
		DeserializeToValue(command.data, vars);
		m_callback(command, vars);
	}
};

void RemoteEngine::RequestGlobalVarList(const LuaVarListCallback &callback) {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
		"",
		VarListResponseHandler(callback));
}

void RemoteEngine::ResponseVarList(const Command_ &command, const LuaVarList &vars) {
	scoped_lock lock(m_mutex);

	WriteResponse(
		command,
		REMOTECOMMANDTYPE_VALUE_VARLIST,
		SerializeToStr(vars));
}

}
