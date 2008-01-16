
INCLUDES =	-I../../include \
			-I../../src \
			-I../../src/lua/src \
			-I../../extralib/babel \
			-I../../extralib/treelistctrl/include \
			-I../../extralib/wxscintilla/include

lib_LTLIBRARIES = liblldebug.la

liblldebug_la_CPPFLAGS = -DLUA_USE_LINUX \
						-DLLDEBUG_BUILD_DLL \
						-DLUA_USE_APICHECK \
						-Wall -ldl $(WXCPPFLAGS)

liblldebug_la_LDFLAGS = $(LDFLAGS)

liblldebug_la_LIBADD = $(libadd) \
						../babel/libbabel.a \
						../treelistctrl/libtreelistctrl.a \
						../wxscintilla/libwxscintilla.a \
						-lboost_thread-mt \
						-lboost_filesystem-mt \
						-lboost_serialization-mt \
						-lreadline -lhistory -lncurses \
						$(WXLIBS)

liblldebug_la_SOURCES = \
	../../src/lua/src/lapi.c \
	../../src/lua/src/lauxlib.c \
	../../src/lua/src/lbaselib.c \
	../../src/lua/src/lcode.c \
	../../src/lua/src/ldblib.c \
	../../src/lua/src/ldebug.c \
	../../src/lua/src/ldo.c \
	../../src/lua/src/ldump.c \
	../../src/lua/src/lfunc.c \
	../../src/lua/src/lgc.c \
	../../src/lua/src/linit.c \
	../../src/lua/src/liolib.c \
	../../src/lua/src/llex.c \
	../../src/lua/src/lmathlib.c \
	../../src/lua/src/lmem.c \
	../../src/lua/src/loadlib.c \
	../../src/lua/src/lobject.c \
	../../src/lua/src/lopcodes.c \
	../../src/lua/src/loslib.c \
	../../src/lua/src/lparser.c \
	../../src/lua/src/lstate.c \
	../../src/lua/src/lstring.c \
	../../src/lua/src/lstrlib.c \
	../../src/lua/src/ltable.c \
	../../src/lua/src/ltablib.c \
	../../src/lua/src/ltm.c \
	../../src/lua/src/lundump.c \
	../../src/lua/src/lvm.c \
	../../src/lua/src/lzio.c \
	../../src/lldebug.cpp \
	../../src/lldebug_application.cpp \
	../../src/lldebug_backtraceview.cpp \
	../../src/lldebug_bootstrap.cpp \
	../../src/lldebug_codeconv.cpp \
	../../src/lldebug_event.cpp \
	../../src/lldebug_interactview.cpp \
	../../src/lldebug_luainfo.cpp \
	../../src/lldebug_langsettings.cpp \
	../../src/lldebug_mainframe.cpp \
	../../src/lldebug_sourceview.cpp \
	../../src/lldebug_sysinfo.cpp \
	../../src/lldebug_watchview.cpp \
	../../src/lldebug_context.cpp


