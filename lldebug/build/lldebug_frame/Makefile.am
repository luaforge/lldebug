
INCLUDES =	-I../../include \
			-I../../src \
			-I../../extralib/boost_asio_0_3_9 \
			-I../../extralib/treelistctrl/include \
			-I../../extralib/wxscintilla/include

noinst_PROGRAMS = lldebug_frame

lldebug_frame_CPPFLAGS = -DLLDEBUG_VISUAL \
						-Wall $(WXCPPFLAGS)

lldebug_frame_LDADD = $(libadd) \
						../treelistctrl/libtreelistctrl.a \
						../wxscintilla/libwxscintilla.a \
						../boost_system/libboost_system.a \
						-lboost_thread-mt \
						-lboost_filesystem-mt \
						-lboost_serialization-mt \
						$(WXLIBS)

lldebug_frame_SOURCES = \
	../../src/configfile.cpp \
	../../src/sysinfo.cpp \
	../../src/luainfo.cpp \
	../../src/net/command.cpp \
	../../src/net/connection.cpp \
	../../src/net/echostream.cpp \
	../../src/net/netutils.cpp \
	../../src/net/remoteengine.cpp \
	../../src/visual/application.cpp \
	../../src/visual/backtraceview.cpp \
	../../src/visual/event.cpp \
	../../src/visual/interactiveview.cpp \
	../../src/visual/langsettings.cpp \
	../../src/visual/mainframe.cpp \	
	../../src/visual/mediator.cpp \
	../../src/visual/outputview.cpp \
	../../src/visual/sourceview.cpp \
	../../src/visual/strutils.cpp \
	../../src/visual/watchview.cpp

