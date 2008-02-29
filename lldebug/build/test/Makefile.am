
INCLUDES = -I../../include \
			`lua-config --include`

noinst_PROGRAMS = test1

LUA_LIBS=`lua-config --libs`

test1_SOURCES = ../../test/test1.cpp
test1_CPPFLAGS = -Wall
test1_LDFLAGS = -L$(libdir) $(LUA_LIBS)
test1_LDADD = $(libadd) \
			../lldebug/liblldebug.a \
			../boost_system/libboost_system.a \
			-lboost_thread-mt \
			-lboost_filesystem-mt \
			-lboost_serialization-mt
			

