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

#include "precomp.h"
#include "net/connection.h"
#include "net/remoteengine.h"
#include "net/netutils.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>

namespace lldebug {
namespace net {

using namespace boost::asio::ip;

#define CONNECTION_TRACE(msg) \
	m_engine.OutputLog(LOGTYPE_TRACE, (msg));

ServerConnector::ServerConnector(RemoteEngine &engine)
	: m_engine(engine), m_acceptor(m_engine.GetService())
	, m_handleCommandCount(0) {
}

ServerConnector::~ServerConnector() {
}

void ServerConnector::Start(unsigned short port) {
	if (m_connection != NULL) {
		return;
	}
	m_connection.reset(new Connection(m_engine));

	// Bind server address.
	tcp::endpoint endpoint(tcp::v4(), port);

	// Try to accept.
	m_acceptor.open(endpoint.protocol());
	m_acceptor.bind(endpoint);
	m_acceptor.listen();
	m_acceptor.async_accept(m_connection->GetSocket(),
		boost::bind(
			&ServerConnector::HandleAccept, shared_from_this(),
			boost::asio::placeholders::error));

	CONNECTION_TRACE("Waiting to accept...");
}

/// Called after the accept.
void ServerConnector::HandleAccept(const boost::system::error_code &error) {
	if (!error) {
		CONNECTION_TRACE("Succeeded in acceptance.");
		CONNECTION_TRACE("Confirming whether the connection is correct...");

		// Try to write command.
		shared_ptr<CommandHeader> writeHeader(new CommandHeader);
		writeHeader->type = REMOTECOMMANDTYPE_START_CONNECTION;
		writeHeader->commandId = 0;
		writeHeader->dataSize = 0;
		m_connection->GetSocket().async_write_some(
			boost::asio::buffer(&*writeHeader, sizeof(CommandHeader)),
			boost::bind(
				&ServerConnector::HandleCommand, shared_from_this(),
				writeHeader, boost::asio::placeholders::error));

		// Try to read command.
		shared_ptr<CommandHeader> readHeader(new CommandHeader);
		boost::asio::async_read(m_connection->GetSocket(),
			boost::asio::buffer(&*readHeader, sizeof(CommandHeader)),
			boost::asio::transfer_all(),
			boost::bind(
				&ServerConnector::HandleCommand, shared_from_this(),
				readHeader, boost::asio::placeholders::error));
	}
	else {
		CONNECTION_TRACE("Failed to accept.");
		m_connection->Failed();
	}
}

/// Called after the reading or writing command.
void ServerConnector::HandleCommand(shared_ptr<CommandHeader> header,
									const boost::system::error_code &error) {
	if (!error && header->type == REMOTECOMMANDTYPE_START_CONNECTION) {
		++m_handleCommandCount;

		// If the reading and writing commands were done.
		if (m_handleCommandCount >= 2) {
			CONNECTION_TRACE("Succeeded in confirming.");

			// The connection was done successfully.
			m_connection->Connected();
		}
	}
	else {
		CONNECTION_TRACE("Failed to confirm.");
		m_connection->Failed();
	}
}


/*-----------------------------------------------------------------*/
ClientConnector::ClientConnector(RemoteEngine &engine)
	: m_engine(engine), m_resolver(m_engine.GetService())
	, m_handleCommandCount(0) {
}

ClientConnector::~ClientConnector() {
}

void ClientConnector::Start(const std::string &hostName,
							const std::string &serviceName) {
	if (m_connection != NULL) {
		return;
	}
	m_connection.reset(new Connection(m_engine));

	// Resolve server address (service name).
	tcp::resolver_query query(tcp::v4(), hostName, serviceName);
	m_resolver.async_resolve(query,
		boost::bind(
			&ClientConnector::HandleResolve, shared_from_this(),
			boost::asio::placeholders::iterator,
			boost::asio::placeholders::error));
	CONNECTION_TRACE("Trying to resolve the ip address...");
}

void ClientConnector::HandleResolve(tcp::resolver_iterator nextEndpoint,
									const boost::system::error_code &error) {
	if (!error) {
		CONNECTION_TRACE("Succeeded in resolving.");
		CONNECTION_TRACE("Trying to connect with the server...");

		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 1;
		boost::thread::sleep(xt);

		tcp::endpoint endpoint = *nextEndpoint;
		m_connection->GetSocket().async_connect(endpoint,
			boost::bind(
				&ClientConnector::HandleConnect, shared_from_this(),
				++nextEndpoint, boost::asio::placeholders::error));
	}
	else {
		CONNECTION_TRACE("Failed to resolve.");
		m_connection->Failed();
	}
}

/// Called after the connect.
void ClientConnector::HandleConnect(tcp::resolver::iterator nextEndpoint,
									const boost::system::error_code &error) {
	if (!error) {
		CONNECTION_TRACE("Succeeded in connecting.");
		CONNECTION_TRACE("Confirming whether the connection is correct...");

		// Try to write command.
		shared_ptr<CommandHeader> writeHeader(new CommandHeader);
		writeHeader->type = REMOTECOMMANDTYPE_START_CONNECTION;
		writeHeader->commandId = 0;
		writeHeader->dataSize = 0;
		m_connection->GetSocket().async_write_some(
			boost::asio::buffer(&*writeHeader, sizeof(CommandHeader)),
			boost::bind(
				&ClientConnector::HandleCommand, shared_from_this(),
				writeHeader, boost::asio::placeholders::error));

		// Try to read command.
		shared_ptr<CommandHeader> readHeader(new CommandHeader);
		m_connection->GetSocket().async_read_some(
			boost::asio::buffer(&*readHeader, sizeof(CommandHeader)),
			boost::bind(
				&ClientConnector::HandleCommand, shared_from_this(),
				readHeader, boost::asio::placeholders::error));
	}
	else {
		CONNECTION_TRACE("Failed to connect.");

		if (nextEndpoint != tcp::resolver::iterator()) {
			CONNECTION_TRACE("Trying to connect to the other address...");

			tcp::endpoint endpoint = *nextEndpoint;
			// Try the next endpoint in the list.
			m_connection->GetSocket().close();
			m_connection->GetSocket().async_connect(endpoint,
				boost::bind(
					&ClientConnector::HandleConnect, shared_from_this(),
					++nextEndpoint, boost::asio::placeholders::error));
		}
		else {
			m_connection->Failed();
		}
	}
}

/// Called after the reading or writing command.
void ClientConnector::HandleCommand(shared_ptr<CommandHeader> header,
									const boost::system::error_code &error) {
	if (!error && header->type == REMOTECOMMANDTYPE_START_CONNECTION) {
		++m_handleCommandCount;

		// If the reading and writing commands were done.
		if (m_handleCommandCount >= 2) {
			CONNECTION_TRACE("Succeeded in confirming.");

			// The connection was done successfully.
			m_connection->Connected();
		}
	}
	else {
		CONNECTION_TRACE("Failed to confirm.");
		m_connection->Failed();
	}
}


/*-----------------------------------------------------------------*/
Connection::Connection(RemoteEngine &engine)
	: m_engine(engine), m_service(engine.GetService())
	, m_socket(engine.GetService()), m_isConnected(false) {
}

Connection::~Connection() {
}

void Connection::Close() {
	m_service.post(
		boost::bind(
			&Connection::DoClose, shared_from_this(),
			boost::system::error_code()));
}

void Connection::WriteCommand(const CommandHeader &header,
							  const CommandData &data) {
	Command command(header, data);

	m_service.post(
		boost::bind(
			&Connection::DoWriteCommand, shared_from_this(),
			command));
}

/// Called when the connection was done.
void Connection::Connected() {
	if (!m_isConnected) {
		if (!m_engine.OnConnectionConnected(shared_from_this())) {
			return;
		}

		m_isConnected = true;
		BeginReadCommand();
	}
}

/// Called when the connection was failed.
void Connection::Failed() {
	m_engine.OnConnectionFailed();
}

/// Close the socket.
void Connection::DoClose(const boost::system::error_code &error) {
	if (m_isConnected) {
		m_engine.OnConnectionClosed(shared_from_this(), error);
		m_isConnected = false;
		m_socket.close();
	}
}

/// Send the asynchronous command read order.
void Connection::BeginReadCommand() {
	shared_ptr<Command> command(new Command);

	boost::asio::async_read(m_socket,
		boost::asio::buffer(&command->GetHeader(), sizeof(CommandHeader)),
		boost::asio::transfer_all(),
		boost::bind(
			&Connection::HandleReadCommandHeader, shared_from_this(),
			command, boost::asio::placeholders::error));
}

/// Called after the command header reading.
void Connection::HandleReadCommandHeader(shared_ptr<Command> command,
										 const boost::system::error_code &error) {
	if (!error) {
		// Read the command data, if exists.
		if (command->GetDataSize() > 0) {
			command->ResizeData();

			boost::asio::async_read(m_socket,
				boost::asio::buffer(command->GetImplData()),
				boost::asio::transfer_all(),
				boost::bind(
					&Connection::HandleReadCommandData, shared_from_this(),
					command, boost::asio::placeholders::error));
		}
		else {
			HandleReadCommandData(command, error);
		}
	}
	else {
		DoClose(error);
	}
}

/// Called after the command data reading.
void Connection::HandleReadCommandData(shared_ptr<Command> command,
									   const boost::system::error_code &error) {
	if (!error) {
		m_engine.OnRemoteCommand(*command);

		// Prepare for the new command.
		BeginReadCommand();
	}
	else {
		DoClose(error);
	}
}

/// Do the asynchronous command write.
void Connection::DoWriteCommand(const Command &command) {
	bool isProgress = !m_writeCommandQueue.empty();
	m_writeCommandQueue.push(command);

	if (!isProgress) {
		BeginWriteCommand(m_writeCommandQueue.front());
	}
}

/// Send the asynchronous command write order.
void Connection::BeginWriteCommand(const Command &command) {
	EchoCommand(command);

	if (command.GetDataSize() == 0) {
		// Write command header with the deleting command memory.
		boost::asio::async_write(m_socket,
			boost::asio::buffer(&command.GetHeader(), sizeof(CommandHeader)),
			boost::asio::transfer_all(),
			boost::bind(
				&Connection::HandleWriteCommand, shared_from_this(),
				true, boost::asio::placeholders::error));
	}
	else {
		// Write command header without the deleting command memory.
		boost::asio::async_write(m_socket,
			boost::asio::buffer(&command.GetHeader(), sizeof(CommandHeader)),
			boost::asio::transfer_all(),
			boost::bind(
				&Connection::HandleWriteCommand, shared_from_this(),
				false, boost::asio::placeholders::error));

		boost::asio::async_write(m_socket,
			boost::asio::buffer(command.GetImplData()),
			boost::asio::transfer_all(),
			boost::bind(
				&Connection::HandleWriteCommand, shared_from_this(),
				true, boost::asio::placeholders::error));
	}
}

/// It's called after the end of writing command.
/// The command memory is deleted if possible.
void Connection::HandleWriteCommand(bool deleteCommand,
									const boost::system::error_code& error) {
	if (!error) {
		if (deleteCommand) {
			m_writeCommandQueue.pop();

			// Begin the new write order.
			if (!m_writeCommandQueue.empty()) {
				BeginWriteCommand(m_writeCommandQueue.front());
			}
		}
	}
	else {
		DoClose(error);
	}
}

} // end of namespace net
} // end of namespace lldebug
