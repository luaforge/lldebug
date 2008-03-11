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
#include "net/echostream.h"

namespace lldebug {
namespace echo {

/// Echo client main.
static int ClientMain(const std::string &hostName,
					  const std::string &serviceName) {
	try {
		echo_ostream echo(hostName, serviceName);

		if (!echo.is_open()) {
			std::cerr << "Couldn't open the echo stream." << std::endl;
			return -1;
		}

		std::cout << "echo client that sends \'localhost:" << serviceName << "' address" << std::endl;
		std::cout << "waiting for your input ..." << std::endl;
	
		for(;;) {
			std::string buffer;
			std::getline(std::cin, buffer);

			// End if input is empty.
			if (buffer.empty()) {
				break;
			}

			echo << buffer << std::endl;
		}
	}
	catch (std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}

} // end of namespace echo
} // end of namespace lldebug

int main(int argc, char *argv[]) {
	std::string hostName = "localhost";
	std::string serviceName = "42598";

	if (argc > 1) {
		hostName = argv[1];
	}

	if (argc > 2) {
		serviceName = argv[2];
	}

	return lldebug::echo::ClientMain(hostName, serviceName);
}

