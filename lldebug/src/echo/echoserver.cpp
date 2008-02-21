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

#define _WIN32_WINNT 0x500
#include "precomp.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "net/echostream.h"

namespace lldebug {
namespace net {

using namespace boost::asio::ip;

static mutex s_consoleMutex;

/// Write str with newline.
static void WriteLine(const std::string &str) {
	scoped_lock lock(s_consoleMutex);
	std::cout << str << std::endl;

#ifdef BOOST_WINDOWS_API
	{
	static HWND consoleWindow = NULL;
	static bool first = true;
	const char *title = "Echo Server";
	if (first) {
		SetConsoleTitleA(title);
		first = false;
	}
	if (consoleWindow == NULL) {
		consoleWindow = FindWindowA(NULL, title);
	}
	if (consoleWindow != NULL) {
		// Move console to the toplevel.
		SetWindowPos(consoleWindow, HWND_TOP, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION);
	}
	}
#endif
}

/// Do new socket session.
static void Session(shared_ptr<tcp::socket> sock) {
	try {
		while (true) {
			boost::asio::streambuf streamBuf;
			boost::system::error_code error;

			size_t size = boost::asio::read_until(*sock,
				streamBuf, "\r\n", error);
			if (error == boost::asio::error::eof) {
				break;
			}
			else if (error) {
				throw boost::system::system_error(error);
			}

			// Use vector as a buffer.
			// (stringstream split the input texts by the space)
			std::istream stream(&streamBuf);
			std::vector<char> vec(size);
			stream.read(&vec[0], (std::streamsize)vec.size());

			// Create string object to save the instance.
			std::string str("");
			if (vec.size() >= 2) {
				str = std::string(vec.begin(), vec.end() - 2);
			}
			WriteLine(str);

			str += "\r\n";
			sock->write_some(boost::asio::buffer(str), error);
			if (error == boost::asio::error::eof) {
				break;
			}
			else if (error) {
				throw boost::system::system_error(error);
			}
		}
	}
	catch (std::exception &ex) {
		scoped_lock lock(s_consoleMutex);
		std::cerr << ex.what() << std::endl;
	}
	catch (...) {
		scoped_lock lock(s_consoleMutex);
		std::cerr << "Unknown exception !!!" << std::endl;
	}
}

/// Echo server main.
static int ServerMain(const std::string &serviceName) {
	try {
		boost::asio::io_service service;

		tcp::resolver resolver(service);
		tcp::resolver_query query("localhost", serviceName);
		tcp::endpoint endpoint = *resolver.resolve(query);
	
		tcp::acceptor acceptor(service);
		acceptor.open(endpoint.protocol());
		acceptor.set_option(tcp::acceptor::reuse_address(true));
		acceptor.bind(endpoint);
		acceptor.listen();
	
		while (true) {
			shared_ptr<tcp::socket> sock(new tcp::socket(service));
			acceptor.accept(*sock);
		
			boost::thread th(boost::bind(&Session, sock));
		}
	}
	catch (std::exception &ex) {
		scoped_lock lock(s_consoleMutex);
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}

} // end of namespace net
} // end of namespace lldebug

int main(int argc, char *argv[]) {
	std::string serviceName = "7";

	if (argc >= 2) {
		serviceName = argv[1];
	}

	return lldebug::ServerMain(serviceName);

/*	boost::asio::io_service service;
	boost::asio::ip::tcp::socket sock(service);

	boost::asio::ip::tcp::resolver resolver(service);
	boost::asio::ip::tcp::resolver_query query("localhost", "7");
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

	sock.connect(endpoint);

	while (1) {
		std::vector<char> vec(1000);
		sock.write_some(
			boost::asio::buffer("testtesttesttesttesttesttesttesttesttesttesttest"));
//		sock.read_some(
//			boost::asio::buffer(vec));
	}*/
}
