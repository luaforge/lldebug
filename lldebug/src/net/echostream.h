/*
 *
 */

#ifndef __LLDEBUG_ECHOSTREAM__
#define __LLDEBUG_ECHOSTREAM__

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <vector>
#include <cassert>

namespace lldebug {
namespace net {

class WinsockInitializer {
	static int ms_count;

public:
	static int Start() {
		if (ms_count == 0) {
			WSADATA data;
			WORD version = MAKEWORD(2, 0);
			if (WSAStartup(version, &data) != 0) {
				throw std::runtime_error("Failed to call WSAStartup.");
			}
		}

		++ms_count;
		return 0;
	}

	static void End() {
		if (ms_count <= 0) {
			return;
		}
		if (--ms_count == 0) {
			WSACleanup();
		}
	}

	static bool Initialized() {
		return (ms_count > 0);
	}
};

int WinsockInitializer::ms_count = 0;

class Socket {
public:
	explicit Socket()
		: m_socket(INVALID_SOCKET) {
		WinsockInitializer::Start();
	}

	virtual ~Socket() {
		if (m_socket != INVALID_SOCKET) {
			closesocket(m_socket);
		}

		WinsockInitializer::End();
	}

	int Initialize(const std::string &hostname, const std::string &port) {
		addrinfo hints;
		addrinfo *res;

		if (hostname.empty() || port.empty()) {
			return -1;
		}

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res) != 0) {
			return -1;
		}

		SOCKET sock = INVALID_SOCKET;
		for (addrinfo *it = res; it != NULL; it = it->ai_next) {
			sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
			if (sock == INVALID_SOCKET) {
				continue;
			}

			if (connect(sock, it->ai_addr, (int)it->ai_addrlen) != 0) {
				closesocket(sock);
				sock = INVALID_SOCKET;
				continue;
			}

			break;
		}

		freeaddrinfo(res);

		if (sock == INVALID_SOCKET) {
			return -1;
		}

		// success!!
		if (m_socket != INVALID_SOCKET) {
			closesocket(m_socket);
		}
		m_socket = sock;
		return 0;
	}

	bool Initialized() const {
		return (m_socket != INVALID_SOCKET);
	}

	size_t Send(const char *buffer, size_t maxsize) {
		if (m_socket == INVALID_SOCKET) {
			return 0;
		}

		if (buffer == NULL || maxsize == 0) {
			return 0;
		}

		size_t pos = 0;
		while (pos < maxsize) {
			int size = send(m_socket, &buffer[pos], (int)(maxsize - pos), 0);
			if (size == SOCKET_ERROR) {
				return pos;
			}

			pos += (size_t)size;
		}

		return pos;
	}

	size_t Recv(char *buffer, size_t maxsize) {
		if (m_socket == INVALID_SOCKET) {
			return 0;
		}

		if (buffer == NULL || maxsize == 0) {
			return 0;
		}

		size_t pos = 0;
		while (pos < maxsize) {
			int size = recv(m_socket, &buffer[pos], (int)(maxsize - pos), 0);
			if (size == SOCKET_ERROR) {
				return pos;
			}

			pos += (size_t)size;
		}

		return pos;
	}

private:
	SOCKET m_socket;
};


/**
 * @brief Echo client.
 */
template <class Ch, class Tr=std::char_traits<Ch> >
class basic_echo_ostreambuf : public std::basic_streambuf<Ch,Tr> {
public:
	typedef typename std::basic_streambuf<Ch,Tr> base_type;
	typedef typename base_type::int_type int_type;

public:
	explicit basic_echo_ostreambuf()
		: m_prev_cr(false) {
		setbuf(0, 0); // suppress buffering
	}

	virtual ~basic_echo_ostreambuf() {
		flush_internal(false);
	}

	bool open(const std::string &hostname, const std::string &port) {
		if (m_socket.Initialize(hostname, port) != 0) {
			return false;
		}

		return true;
	}

	bool is_open() const {
		return m_socket.Initialized();
	}

protected:
	virtual std::streampos seekoff(std::streamoff off,
								   std::ios::seek_dir dir, 
								   int mode = std::ios::in | std::ios::out) {
		return Tr::eof();
	}

	virtual std::streampos seekpos(std::streampos pos,
								   int mode = std::ios::in | std::ios::out) {
		return Tr::eof();
	}

	int flush_internal(bool force) {
		if (!force && m_buffer.empty()) {
			return 0;
		}

		m_buffer.push_back('\r');
		m_buffer.push_back('\n');

		size_t size = m_socket.Send(&m_buffer[0], m_buffer.size());
		if (size == 0) {
			return -1;
		}

		m_socket.Recv(&m_buffer[0], size);
		m_buffer.clear();
		return 0;
	}

	virtual int_type overflow(int c = Tr::eof()) {
		if (m_prev_cr) {
			m_prev_cr = false;
			flush_internal(true);

			if (Tr::to_char_type(c) == '\r') {
				m_prev_cr = true;
			}
			else if (Tr::to_char_type(c) == '\n') {
				return 0;
			}
			else {
				m_buffer.push_back(Tr::to_char_type(c));
			}
		}
		else {
			if (Tr::to_char_type(c) == '\r') {
				m_prev_cr = true;
			}
			else if (Tr::to_char_type(c) == '\n') {
				flush_internal(true);
			}
			else {
				m_buffer.push_back(Tr::to_char_type(c));
			}
		}

		return 0;
	}

	virtual int_type sync() {
		return flush_internal(false);
	}

private:
	Socket m_socket;
	std::vector<Ch> m_buffer;
	bool m_prev_cr;
};

/**
 * @brief Echo client.
 */
template <class Ch,class Tr=std::char_traits<Ch> >
class basic_echo_ostream : public std::basic_ostream<Ch,Tr> {
public:
	explicit basic_echo_ostream(const std::string &hostname="localhost", const std::string &port="7")
		: std::basic_ostream<Ch,Tr>(NULL) {
		this->init(&m_buf);
		open(hostname, port);
	}

	~basic_echo_ostream() {
	}

	bool open(const std::string &hostname="localhost", const std::string &port="7") {
		return m_buf.open(hostname, port);
	}

	bool is_open() const {
		return m_buf.is_open();
	}

private:
	basic_echo_ostreambuf<Ch> m_buf;
};

typedef basic_echo_ostream<char> echo_ostream;
typedef basic_echo_ostream<wchar_t> echo_wostream;

} // end of namespace net
} // end of namespace lldebug

#endif
