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

#ifndef __LLDEBUG_CONNECTION_H__
#define __LLDEBUG_CONNECTION_H__

#include "net/command.h"

#include <boost/asio/ip/tcp.hpp>

namespace lldebug {
namespace net {

class Connection;

/**
 * @brief TCP connection maker used by server size.
 */
/*class Connector {
	: public boost::enable_shared_from_this<Connector> {
public:
	enum State {
		STATE_TRYING,
		STATE_SUCCESS,
		STATE_FAIL,
	};

public:
	explicit Connector(RemoteEngine &engine);
	virtual ~Connector();

	/// Get the remote engine.
	RemoteEngine &GetEngine() {
		return m_engine;
	}

	/// Get the connection object.
	shared_ptr<Connection> GetConnection() {
		return m_connection;
	}

	bool IsTrying() const {
		return (m_state == STATE_TRYING);
	}

	bool IsSuccess() const {
		return (m_state == STATE_SUCCESS);
	}

	bool IsFail() const {
		return (m_state == STATE_FAIL);
	}

protected:
	void BeginConfirmCommand();
	void HandleConfirmCommand(shared_ptr<CommandHeader> header,
							  const boost::system::error_code &error);
	void Succeeded();
	void Failed();

private:
	RemoteEngine &m_engine;
	shared_ptr<Connection> m_connection;
	State m_state;
	int m_handleCommandCount;
};*/


/**
 * @brief TCP connection maker used by server size.
 */
class ServerConnector
	: public boost::enable_shared_from_this<ServerConnector> {
public:
	explicit ServerConnector(RemoteEngine &engine);
	virtual ~ServerConnector();

	/// Start the server connection.
	void Start(unsigned short port);

private:
	void HandleAccept(const boost::system::error_code &error);
	void HandleCommand(shared_ptr<CommandHeader> header,
					   const boost::system::error_code &error);

private:
	RemoteEngine &m_engine;
	boost::asio::ip::tcp::acceptor m_acceptor;
	shared_ptr<Connection> m_connection;
	int m_handleCommandCount;
};

/**
 * @brief TCP connection maker used by client size.
 */
class ClientConnector
	: public boost::enable_shared_from_this<ClientConnector> {
public:
	explicit ClientConnector(RemoteEngine &engine);
	virtual ~ClientConnector();

	/// Start the client connection.
	void Start(const std::string &hostName, const std::string &serviceName);

private:
	void HandleResolve(boost::asio::ip::tcp::resolver_iterator nextEndpoint,
					   const boost::system::error_code &error);
	void HandleConnect(boost::asio::ip::tcp::resolver_iterator nextEndpoint,
					   const boost::system::error_code &error);
	void HandleCommand(shared_ptr<CommandHeader> header,
					   const boost::system::error_code &error);

private:
	RemoteEngine &m_engine;
	boost::asio::ip::tcp::resolver m_resolver;
	shared_ptr<Connection> m_connection;
	int m_handleCommandCount;
};

/**
 * @brief TCP connection
 */
class Connection
	: public boost::enable_shared_from_this<Connection> {
public:
	virtual ~Connection();

	/// Close this socket.
	void Close();
	
	/// Write command data.
	void WriteCommand(const CommandHeader &header,
					  const CommandData &data);
	
	/// Get the socket object.
	boost::asio::ip::tcp::socket &GetSocket() {
		return m_socket;
	}

private:
	friend class ClientConnector;
	friend class ServerConnector;
	explicit Connection(RemoteEngine &engine);
	void Connected();
	void Failed();

private:
	void DoClose(const boost::system::error_code &error);

	void BeginReadCommand();
	void HandleReadCommandHeader(shared_ptr<Command> command,
								 const boost::system::error_code &error);
	void HandleReadCommandData(shared_ptr<Command> command,
							   const boost::system::error_code &error);

	void DoWriteCommand(const Command &command);						
	void BeginWriteCommand(const Command &command);
	void HandleWriteCommand(bool deleteCommand,
							const boost::system::error_code& error);

private:
	RemoteEngine &m_engine;
	boost::asio::io_service &m_service;
	boost::asio::ip::tcp::socket m_socket;
	bool m_isConnected;

	typedef std::queue<Command> WriteCommandQueue;
	/// Reserved write command queue.
	/** 
	 * Because the command memory must be kept until the end of the writing.
	 */
	WriteCommandQueue m_writeCommandQueue;
};

} // end of namespace net
} // end of namespace lldebug

#endif
