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
#include "execute.h"

/// Execute file with args.
static int LLDebugExecuteFile(const std::string &filename,
							  unsigned short port);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
static int LLDebugExecuteFile(const std::string &filename,
							  unsigned short port) {
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;

	// Initialize the infomation.
	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;

	// Execution string. This must be mutable.
	char commandline[_MAX_PATH * 2];
	snprintf(commandline, sizeof(commandline), "\"%s\" %d",
		filename.c_str(), port);
	
	// Execute file.
	BOOL result = CreateProcessA(
		NULL, // filename
		commandline, // arguments
		NULL, // process attributes
		NULL, // thread attributes
		FALSE, // is inherit handle
		NORMAL_PRIORITY_CLASS, // attributes
		NULL, // pointer of environmental path
		NULL, // curret directory
		&si, // startup info
		&pi // process and thread info
		);

	if (!result) {
		LPTSTR lpBuffer;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			GetLastError(),
			LANG_USER_DEFAULT,
			(LPTSTR)&lpBuffer,
			0,
			NULL);
		MessageBox(NULL, lpBuffer, TEXT("error message"), MB_ICONHAND|MB_OK);
		LocalFree(lpBuffer);
		return -1;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return 0;
}

#else
#include <stdlib.h>

static int LLDebugExecuteFile(const std::string &filename,
							  unsigned short port) {
	char commandline[256];
	snprintf(commandline, sizeof(commandline), "%s %d",
		filename.c_str(), port);

	system(commandline);
}

#endif

namespace lldebug {
namespace context {

int ExecuteFile(const std::string &filename, unsigned short port) {
	if (filename.empty()) {
		return -1;
	}

	// Execute the file with some arguments.
	return LLDebugExecuteFile(filename, port);
}

} // end of namespace context
} // end of namespace lldebug
