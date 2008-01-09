//---------------------------------------------------------------------------
//
// Name:        Project1App.h
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_CONTROLS_H__
#define __LLDEBUG_CONTROLS_H__

#include "lldebug_event.h"

namespace lldebug {

class Context;
class ManagerFrame;
class MainFrame;
class SourceView;
class InteractView;
class WatchView;
class TracebackView;

enum {
	LLDEBUG_FRAMESIZE = 128,

	ID_MANAGER_FRAME = wxID_HIGHEST + 2560,
	ID_MAINFRAME,
	ID_MAINFRAME_END = ID_MAINFRAME + LLDEBUG_FRAMESIZE,
	ID_WINDOWHOLDER,
	ID_WINDOWHOLDER_END = ID_WINDOWHOLDER + LLDEBUG_FRAMESIZE,
	ID_SOURCEVIEW,
	ID_SOURCEVIEW_END = ID_SOURCEVIEW + LLDEBUG_FRAMESIZE,
	ID_INTERACTVIEW,
	ID_INTERACTVIEW_END = ID_INTERACTVIEW + LLDEBUG_FRAMESIZE,
	ID_LOCALWATCHVIEW,
	ID_LOCALWATCHVIEW_END = ID_LOCALWATCHVIEW + LLDEBUG_FRAMESIZE,
	ID_GLOBALWATCHVIEW,
	ID_GLOBALWATCHVIEW_END = ID_GLOBALWATCHVIEW + LLDEBUG_FRAMESIZE,
	ID_REGISTRYWATCHVIEW,
	ID_REGISTRYWATCHVIEW_END = ID_REGISTRYWATCHVIEW + LLDEBUG_FRAMESIZE,
	ID_ENVIRONWATCHVIEW,
	ID_ENVIRONWATCHVIEW_END = ID_ENVIRONWATCHVIEW + LLDEBUG_FRAMESIZE,
	ID_STACKWATCHVIEW,
	ID_STACKWATCHVIEW_END = ID_STACKWATCHVIEW + LLDEBUG_FRAMESIZE,
	ID_WATCHVIEW,
	ID_WATCHVIEW_END = ID_WATCHVIEW + LLDEBUG_FRAMESIZE,
	ID_BACKTRACEVIEW,
	ID_BACKTRACEVIEW_END = ID_BACKTRACEVIEW + LLDEBUG_FRAMESIZE,
	ID_CONTROLS_END,
};

inline std::string wxConvToUTF8(const wxString &str) {
	return std::string(wxConvUTF8.cWC2MB(str.c_str()));
}

inline wxString wxConvFromUTF8(const std::string &str) {
	return wxString(wxConvUTF8.cMB2WC(str.c_str()));
}

void SendCreateFrameEvent(Context *ctx);
void SendDestroyedFrameEvent(Context *ctx);

}

#endif
