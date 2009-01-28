
INCLUDES = -I../../include `lua-config --include`

noinst_PROGRAMS = lua_debug

LUA_LIBS=`lua-config --libs`

lua_debug_SOURCES = ../../src/lua_debug/lua_debug.c
lua_debug_CPPFLAGS = -Wall
lua_debug_LDFLAGS = -L$(libdir) $(LUA_LIBS)
lua_debug_LDADD = $(libadd) \
			../lldebug/liblldebug.a \
			../boost_system/libboost_system.a \
			-lboost_thread-mt \
			-lboost_filesystem-mt \
			-lboost_serialization-mt
