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

const char FRAME_NAME[] = "lldebug_frame";

/// Execute file with port number.
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

#elif defined(__linux__)
#include <stdlib.h>

static int LLDebugExecuteFile(const std::string &filename,
							  unsigned short port) {
	char commandline[512];
	snprintf(commandline, sizeof(commandline),
		"%s %d > /dev/null &",
		filename.c_str(), port);

	int ret = system(commandline);
	if (ret == -1) {
		return -1;
	}
	
	return 0;
}

#else
#include <unistd.h>

static void catch_SIGCHLD(int signo) {
	pid_t child_pid = 0;
      
	// Call 'wait' for all killed processes.
	do {
		int child_ret;

		// If there is some exited child processes, 'waitpid' returns one's id.
		// If not, it returns 0 because of WNOHANG option.
		child_pid = waitpid(-1, &child_ret, WNOHANG);
	} while (child_pid > 0);
}

/// Setup the signal handler called when a child process is exited.
/** It's necessary to avoid a zombie process.
 */
static void setup_SIGCHLD() {
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = catch_SIGCHLD;
	act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
	sigemptyset(&act.sa_mask);  // No additional signal mask.

	// Set a signal handler.
	sigaction(SIGCHLD, &act, NULL);
}

static int exec_main(const std::string &filename,
					 unsigned short port) {
	char portstr[16];
	snprintf(portstr, sizeof(portstr), "%d", port);
	
	return execlp(filename.c_str(), filename.c_str(), portstr, NULL);
}

static int LLDebugExecuteFile(const std::string &filename,
							  unsigned short port) {
	setup_SIGCHLD();

	switch (fork()) {
	case -1: // Failed
		return -1;
	case 0: // Child process
		exit(exec_main(filename, port));
		break;
	default: // Parent process
		break;
	}
	
	return 0;
}

#endif

namespace lldebug {
namespace context {

int ExecuteFrame(unsigned short port) {
	// Execute the file with some arguments.
	return LLDebugExecuteFile(FRAME_NAME, port);
}

} // end of namespace context
} // end of namespace lldebug
