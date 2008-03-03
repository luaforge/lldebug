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
#include <string>
#include <vector>
#include <iostream>

using namespace boost::asio::ip;

/// Echo client main.
static int ClientMain(const std::string &hostName,
					  const std::string &serviceName) {
	try {
		boost::asio::io_service service;
		udp::resolver resolver(service);
		udp::resolver_query query(udp::v4(), hostName, serviceName);
		udp::endpoint endpoint = *resolver.resolve(query);
		udp::socket sock(service, udp::endpoint());

		std::cout << "echo client that sends \'localhost:" << serviceName << "' address" << std::endl;
		std::cout << "waiting for your input ..." << std::endl;
	
		for(;;) {
			std::string buffer;
			std::getline(std::cin, buffer);

			if (buffer.empty()) {
				buffer += '\0';
			}

			udp::endpoint senderPoint = endpoint;
			sock.send_to(
				boost::asio::buffer(buffer),
				senderPoint);

			std::vector<char> recvbuf(buffer.size() + 1);
			size_t size = sock.receive_from(
				boost::asio::buffer(recvbuf),
				senderPoint);

			std::cout.write(&recvbuf[0], (std::streamsize)size) << std::endl;
		}
	}
	catch (std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	std::string hostName = "localhost";
	std::string serviceName = "42598";

	if (argc > 1) {
		hostName = argv[1];
	}

	if (argc > 2) {
		serviceName = argv[2];
	}

	return ClientMain(hostName, serviceName);
}
