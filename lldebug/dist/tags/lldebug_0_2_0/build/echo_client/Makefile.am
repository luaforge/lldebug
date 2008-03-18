
INCLUDES =	-I../../src \
			-I../../extralib/boost_asio_0_3_9

noinst_PROGRAMS = echo_client

echo_client_CPPFLAGS = $(cppflags) -Wall
echo_client_LDADD =	 $(libadd) \
					../boost_system/libboost_system.a \
					-lboost_thread-mt

echo_client_SOURCES = \
	../../src/net/echostream.cpp \
	../../src/echo/echoclient.cpp

