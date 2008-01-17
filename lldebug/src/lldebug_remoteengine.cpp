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
	
	struct ReadCommand {
		CommandHeader header;
		std::vector<char> data;
	};
	ReadCommand m_readCommand;

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

	/// Get a io_service object.
	boost::asio::io_service &GetService() {
		return m_ioService;
	}

	/// Get a socket object.
	boost::asio::ip::tcp::socket &GetSocket() {
		return m_socket;
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
		boost::asio::async_read(m_socket,
			boost::asio::buffer((void *)&m_readCommand.header, sizeof(CommandHeader)),
			boost::bind(
				&SocketBase::handleReadCommand, this,
				boost::asio::placeholders::error));
	}

	void handleReadCommand(const boost::system::error_code &error) {
		if (!error) {
			// Check that response is OK.
			if (m_readCommand.header.ctxId < 0) {
				doClose();
			}

			m_readCommand.data.resize(m_readCommand.header.dataSize + 1);

			if (m_readCommand.header.dataSize > 0) {
				boost::asio::async_read(m_socket,
					boost::asio::buffer(m_readCommand.data, m_readCommand.header.dataSize),
					boost::bind(
					&SocketBase::handleReadData, this,
					boost::asio::placeholders::error));
			}
			else {
				handleReadData(error);
			}
		}
		else {
			doClose();
		}
	}

	void handleReadData(const boost::system::error_code &error) {
		if (!error) {
			if (!m_readCommand.data.empty()) {
				m_readCommand.data.back() = '\0';
			}

			Command_ command;
			command.header = m_readCommand.header;
			command.data = std::string(&m_readCommand.data[0], m_readCommand.data.size());
			m_callback(command);
			asyncReadCommand();
		}
		else {
			doClose();
		}
	}

	void asyncWrite(const Command_ *command) {
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
		bool isProgress = !m_writeCommandQueue.empty();
		m_writeCommandQueue.push(command);

		if (!isProgress) {
			asyncWrite(&m_writeCommandQueue.front());
		}
	}

	void handleWrite(const boost::system::error_code& error) {
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
		m_socket.close();
	}
};


/*-----------------------------------------------------------------*/
class DebuggeeSocket : public SocketBase {
private:
	tcp::acceptor m_acceptor;

public:
	explicit DebuggeeSocket(boost::asio::io_service &ioService,
						  const CommandCallback &callback,
						  tcp::endpoint endpoint)
		: SocketBase(ioService, callback)
		, m_acceptor(ioService, endpoint) {

		m_acceptor.async_accept(GetSocket(),
			boost::bind(
				&DebuggeeSocket::handleAccept, this,
				boost::asio::placeholders::error));
	}

	virtual ~DebuggeeSocket() {
	}

protected:
	void handleAccept(const boost::system::error_code &error) {
		if (!error) {
			this->asyncReadCommand();
		}
		else {
			m_acceptor.async_accept(GetSocket(),
				boost::bind(
					&DebuggeeSocket::handleAccept, this,
					boost::asio::placeholders::error));
		}
	}
};


/*-----------------------------------------------------------------*/
class DebuggerSocket : public SocketBase {
private:
	tcp::resolver::iterator m_endpointIterator;
	bool m_isStartSuccessed;

public:
	explicit DebuggerSocket(boost::asio::io_service &ioService,
						  const CommandCallback &callback,
						  tcp::resolver::query query)
		: SocketBase(ioService, callback)
		, m_isStartSuccessed(false) {
		tcp::resolver resolver(ioService);
		m_endpointIterator = resolver.resolve(query);
	}

	virtual ~DebuggerSocket() {
	}

	/// Start connection.
	int Start(int waitSeconds) {
		tcp::endpoint endpoint = *m_endpointIterator;

		GetSocket().async_connect(endpoint,
			boost::bind(
				&DebuggerSocket::handleConnect, this,
				boost::asio::placeholders::error, ++m_endpointIterator));

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
		if (!error) {
			// The connection was successful.
			m_isStartSuccessed = true;
		}
		else if (endpointIterator != tcp::resolver::iterator()) {
			// The connection failed. Try the next endpoint in the list.
			tcp::endpoint endpoint = *endpointIterator;

			GetSocket().close();
			GetSocket().async_connect(endpoint,
				boost::bind(
					&DebuggerSocket::handleConnect, this,
					boost::asio::placeholders::error, ++endpointIterator));
		}
		else {
			std::cout << "Error: " << error << "\n";
		}
	}
};


/*-----------------------------------------------------------------*/
RemoteEngine::RemoteEngine()
	: m_isThreadActive(false), m_commandIdCounter(0) {
}

RemoteEngine::~RemoteEngine() {
	StopThread();
}

int RemoteEngine::StartDebuggee(int portNum) {
	scoped_lock lock(m_mutex);
	shared_ptr<DebuggeeSocket> socket;

	socket.reset(new DebuggeeSocket(m_ioService,
		boost::bind(
			&RemoteEngine::handleReadCommand, this, _1),
			tcp::endpoint(tcp::v4(), portNum)));

	StartThread();
	m_commandIdCounter = 1;
	m_socket = boost::shared_static_cast<SocketBase>(socket);
	return 0;
}

int RemoteEngine::StartDebugger(const std::string &hostName,
								int portNum, int waitSeconds) {
	scoped_lock lock(m_mutex);
	shared_ptr<DebuggerSocket> socket;
	char portStr[64];

	snprintf(portStr, sizeof(portStr), "%d", portNum);
	socket.reset(new DebuggerSocket(m_ioService,
		boost::bind(
			&RemoteEngine::handleReadCommand, this, _1),
			tcp::resolver::query(hostName, portStr)));

	if (socket->Start(waitSeconds) != 0) {
		return -1;
	}

	StartThread();
	m_commandIdCounter = 2;
	m_socket = boost::shared_static_cast<SocketBase>(socket);
	return 0;
}

bool RemoteEngine::HasReadCommand() {
	scoped_lock lock(m_mutex);

	return !m_readCommandQueue.empty();
}

Command_ RemoteEngine::GetReadCommand() {
	scoped_lock lock(m_mutex);

	return m_readCommandQueue.front();
}

void RemoteEngine::PopReadCommand() {
	scoped_lock lock(m_mutex);

	m_readCommandQueue.pop();
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
		boost::function0<void> fn = boost::bind(&RemoteEngine::serviceThread, this);
		m_thread.reset(new boost::thread(fn));
	}
}

void RemoteEngine::StopThread() {
	if (IsThreadActive()) {
		m_thread->join();
		m_thread.reset();
	}
}

void RemoteEngine::serviceThread() {
	// If IsThreadActive is true, the thread has started already.
	if (IsThreadActive()) {
		return;
	}

	SetThreadActive(true);
	try {
		while (!IsThreadActive()) {
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

	if (!isResponseCommand) {
		m_readCommandQueue.push(command);
	}
}

CommandHeader RemoteEngine::InitCommandHeader(RemoteCommandType type,
											  int ctxId,
											  size_t dataSize,
											  int commandId) {
	scoped_lock lock(m_mutex);
	CommandHeader header;

	header.type = type;
	header.ctxId = ctxId;
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
								int ctxId,
								const std::string &data) {
	scoped_lock lock(m_mutex);
	CommandHeader header;

	header = InitCommandHeader(type, ctxId, data.size());
	m_socket->Write(header, data);
}

void RemoteEngine::WriteCommand(RemoteCommandType type,
								int ctxId,
								const std::string &data,
								const CommandCallback &response) {
	scoped_lock lock(m_mutex);
	WaitResponseCommand wcommand;

	wcommand.header = InitCommandHeader(type, ctxId, data.size());
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
		readCommand.header.ctxId,
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

void RemoteEngine::StartConnection(int ctxId, const CommandCallback &callback) {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_START_CONNECTION,
		ctxId,
		"",
		callback);
}

void RemoteEngine::EndConnection(int ctxId) {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_END_CONNECTION,
		ctxId,
		"");
}

struct VarListResponseHandler {
	VarListCommandCallback m_callback;

	explicit VarListResponseHandler(const VarListCommandCallback &callback)
		: m_callback(callback) {
	}

	void operator()(const Command_ &command) {
		m_callback(command, SerializeToValue<LuaVarList>(command.data));
	}
};

void RemoteEngine::RequestGlobalVarList(int ctxId, const VarListCommandCallback &callback) {
	scoped_lock lock(m_mutex);

	WriteCommand(
		REMOTECOMMANDTYPE_REQUEST_GLOBALVARLIST,
		ctxId,
		"",
		VarListResponseHandler(callback));
}

void RemoteEngine::ResponseVarList(const Command_ &command, const LuaVarList &vars) {
	scoped_lock lock(m_mutex);

	WriteResponse(
		command,
		REMOTECOMMANDTYPE_VALUE_VARLIST,
		SerializeToData(vars));
}

}
