
INCLUDES =	-I../../include \
			-I../../src \
			-I../../src/lua/src \
			-I../../extralib/babel \
			-I../../extralib/treelistctrl/include \
			-I../../extralib/wxscintilla/include

#lib_LTLIBRARIES = liblldebug.la
noinst_LIBRARIES = liblldebug.a

#MYLIBS="-Wl,-E -ldl -lreadline -lhistory -lncurses"
#
#AM_LDFLAGS = @LDFLAGS@

liblldebug_a_CPPFLAGS = -DLUA_USE_LINUX \
						-DLLDEBUG_BUILD_DLL \
						 @WXCPPFLAGS@

#liblldebug_la_LIBADD = $(libadd) \
#					   -L../babel \
#					   -L../treelistctrl \
#					   -L../wxscintilla \
#					   -lbabel -ltreelistctrl -lwxscintilla \
#					   -lboost_thread-mt \
#					   -lboost_filesystem-mt \
#					   -lboost_serialization-mt \
#					   @WXLIBS@

liblldebug_a_SOURCES = \
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
	../../src/lldebug_mainframe.cpp \
	../../src/lldebug_prec.cpp \
	../../src/lldebug_sourceview.cpp \
	../../src/lldebug_sysinfo.cpp \
	../../src/lldebug_watchview.cpp \
	../../src/lldebug_context.cpp


