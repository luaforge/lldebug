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
#include "net/netutils.h"
#include "net/echostream.h"

#include <fstream>

namespace lldebug {
namespace net {

/// 
static std::ostream &operator<<(std::ostream &os, const Command &command) {
	os << "type:      " << command.GetType() << std::endl;
	os << "commandId: " << command.GetCommandId() << std::endl;
	os << "datasize:  " << command.GetDataSize() << std::endl;
	if (command.GetDataSize() != 0) {
		os << "data:" << std::endl;
		os << command.ToString() << std::endl;
	}
	return os;
}

void EchoCommand(const Command &command) {
#if 1 //ndef NDEBUG
	echo_ostream echo("localhost");

	if (echo.is_open()) {
		echo << command << std::endl;
	}
#endif
}

void SaveCommand(const std::string &filename, const Command &command) {
	std::ofstream ofs(filename.c_str());
	if (ofs.is_open()) {
		ofs << command << std::endl;
	}
}

} // end of namespace net
} // end of namespace lldebug
