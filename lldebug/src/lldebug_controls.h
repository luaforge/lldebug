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

#ifndef __LLDEBUG_CONTROLS_H__
#define __LLDEBUG_CONTROLS_H__

#include "lldebug_mediator.h"
#include "lldebug_event.h"

namespace lldebug {

class MainFrame;
class SourceView;
class InteractView;
class WatchView;
class TracebackView;

enum {
	LLDEBUG_FRAMESIZE = 128,

	ID_MAINFRAME = wxID_HIGHEST + 2560,
	ID_WINDOWHOLDER,
	ID_SOURCEVIEW,
	ID_OUTPUTVIEW,
	ID_INTERACTVIEW,
	ID_LOCALWATCHVIEW,
	ID_GLOBALWATCHVIEW,
	ID_REGISTRYWATCHVIEW,
	ID_ENVIRONWATCHVIEW,
	ID_STACKWATCHVIEW,
	ID_WATCHVIEW,
	ID_BACKTRACEVIEW,
};

inline std::string wxConvToUTF8(const wxString &str) {
	return std::string(wxConvUTF8.cWC2MB(str.c_str()));
}

inline wxString wxConvFromUTF8(const std::string &str) {
	return wxString(wxConvUTF8.cMB2WC(str.c_str()));
}

}

#endif
