
INCLUDES = -I../../include -I../../src/lua/src

noinst_PROGRAMS = test1

test1_SOURCES = ../../test/test1.cpp

test1_LDFLAGS = -L$(libdir) \
				-L../lldebug \
				-L../babel \
				-L../treelistctrl \
				-L../wxscintilla

test1_LDADD = $(libadd) \
				liblldebug.a -lbabel -ltreelistctrl -lwxscintilla \
				-lboost_thread-mt \
				-lboost_filesystem-mt \
				-lboost_serialization-mt \
				@WXLIBS@

