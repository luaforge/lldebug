
INCLUDES =	-I../../include \
			-I../../src \
			-I../../extralib/babel \
			-I../../extralib/treelistctrl/include \
			-I../../extralib/wxscintilla/include

lib_LTLIBRARIES = liblldebug.la

liblldebug_la_CPPFLAGS = -DLLDEBUG_CONTEXT \
						-Wall $(WXCPPFLAGS)

liblldebug_la_LDFLAGS = $(LDFLAGS)

liblldebug_la_LIBADD = $(libadd) \
						../babel/libbabel.a \
						../treelistctrl/libtreelistctrl.a \
						../wxscintilla/libwxscintilla.a \
						-llua51 \
						-lboost_thread-mt \
						-lboost_filesystem-mt \
						-lboost_serialization-mt \
						$(WXLIBS)

liblldebug_la_SOURCES = \
	../../src/sysinfo.cpp \
	../../src/luainfo.cpp \
	../../src/configfile.cpp \
	../../src/net/command.cpp \
	../../src/net/connection.cpp \
	../../src/net/netutils.cpp \
	../../src/net/remoteengine.cpp \
	../../src/context/codeconv.cpp \
	../../src/context/context.cpp \
	../../src/context/contextutils.cpp \
	../../src/context/lldebug.cpp \
	../../src/context/luaiterate.cpp \

	../../src/visual/application.cpp \
	../../src/visual/backtraceview.cpp \
	../../src/visual/event.cpp \
	../../src/visual/interactview.cpp \
	../../src/visual/langsettings.cpp \
	../../src/visual/mainframe.cpp \	
	../../src/visual/mediator.cpp \
	../../src/visual/outputview.cpp \
	../../src/visual/sourceview.cpp \
	../../src/visual/watchview.cpp
