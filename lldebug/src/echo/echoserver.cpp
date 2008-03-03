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

#include <boost/config.hpp>

#define BOOST_SYSTEM_NO_LIB
#ifdef BOOST_WINDOWS
	#define NOMINMAX
	#define _WIN32_WINDOWS 0x400
#endif
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <iostream>

using namespace boost::asio::ip;

#ifdef BOOST_WINDOWS
static const char *title = "echo server";
static HWND consoleWindow = NULL;

static void initialize() {
	SetConsoleTitleA(title);
}
#else
static void initialize() {
}
#endif

/// Write str with newline.
static void WriteLine(const char *str, size_t length) {
	std::cout.write(str, (std::streamsize)length) << std::endl;

#ifdef BOOST_WINDOWS
	if (consoleWindow == NULL) {
		consoleWindow = FindWindowA(NULL, title);
	}
	if (consoleWindow != NULL) {
		// Move console to the toplevel.
		SetWindowPos(consoleWindow, HWND_TOP, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION);
	}
#endif
}

/// Echo server main.
static int ServerMain(unsigned short port) {
	initialize();

	try {
		boost::asio::io_service service;
		udp::endpoint endpoint(udp::v4(), port);
		udp::socket sock(service, endpoint);

		std::cout << "echo server addressed \'localhost:" << port << "'" << std::endl;
		for(;;) {
			try {
				static char data[1024 * 10];

				udp::endpoint senderPoint;
				size_t size = sock.receive_from(
					boost::asio::buffer(data, sizeof(data)),
					senderPoint);

				WriteLine(data, size);

				sock.send_to(
					boost::asio::buffer(data, size),
					senderPoint);
			}
			catch (boost::system::system_error &e) {
				const boost::system::error_code &ec = e.code();
				if (ec == boost::asio::error::eof
					|| ec == boost::asio::error::connection_aborted
					|| ec == boost::asio::error::connection_reset) {
				}
				else {
					throw;
				}
			}
		}
	}
	catch (std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	std::string serviceName = "42598";

	if (argc >= 2) {
		serviceName = argv[1];
	}

	return ServerMain(atoi(serviceName.c_str()));
}
