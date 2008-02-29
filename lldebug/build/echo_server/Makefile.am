
INCLUDES =	-I../../src \
			-I../../extralib/boost_asio_0_3_9

noinst_PROGRAMS = echo_server

echo_server_CPPFLAGS = $(cppflags) -Wall -O0
echo_server_LDADD =	 $(libadd) \
					../boost_system/libboost_system.a \
					-lboost_thread-mt
echo_server_SOURCES = ../../src/echo/echoserver.cpp
