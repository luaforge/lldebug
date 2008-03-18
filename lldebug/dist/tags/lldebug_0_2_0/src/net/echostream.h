/*
 * Copyright (c) 2005-2008  cielacanth <cielacanth AT s60.xrea.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __LLDEBUG_ECHOSTREAM__
#define __LLDEBUG_ECHOSTREAM__

#include <boost/asio/ip/udp.hpp>

#include <iostream>

namespace lldebug {
namespace net {

/**
 * @brief Thread functions of basic_echo_ostreambuf.
 * 
 * basic_echo_ostreambuf is a template class,
 * so if I include this class in it, I can't put in a cpp file.
 */
struct echo_thread {
public:
	~echo_thread();

	/// Get the io_service object.
	static echo_thread *get_instance() {
		return &ms_instance;
	}

	/// Get the io_service object.
	boost::asio::io_service &get_service() {
		scoped_lock lock(m_mutex);
		return m_service;
	}

	/// Add the echo request to the queue.
	void add_request(const boost::asio::ip::udp::endpoint &endpoint,
					 const std::vector<char> &buffer);

private:
	explicit echo_thread();
	void exit_thread();
	void thread_main();

	struct request;
	void add_request(shared_ptr<request> req);
	shared_ptr<request> get_request();

private:
	static echo_thread ms_instance;
	shared_ptr<boost::thread> m_thread;
	boost::asio::io_service m_service;
	boost::asio::ip::udp::socket m_socket;
	mutex m_mutex;
	condition m_cond;
	bool m_is_exit_thread;
	std::queue<shared_ptr<request> > m_request_queue;
};

/**
 * @brief Echo client.
 */
template <class Ch, class Tr=std::char_traits<Ch> >
class basic_echobuf : public std::basic_streambuf<Ch,Tr> {
public:
	typedef typename std::basic_streambuf<Ch,Tr> base_type;
	typedef typename base_type::int_type int_type;

public:
	explicit basic_echobuf()
		: m_prev_cr(false) {
	}

	virtual ~basic_echobuf() {
		flush_internal(false);
	}

	/// Open the tcp connection.
	bool open(const std::string &hostname, const std::string &servicename) {
		try {
			using namespace boost::asio::ip;
			echo_thread *instance = echo_thread::get_instance();
			if (instance == NULL) {
				return false;
			}

			udp::resolver resolver(instance->get_service());
			udp::resolver_query query(udp::v4(), hostname, servicename);
			udp::resolver_iterator it;
			for (it = resolver.resolve(query); 
				it != udp::resolver_iterator();
				++it) {
				break;
			}

			// Not found.
			if (it == udp::resolver_iterator()) {
				return false;
			}

			m_endpoint = *it;
			return true;
		}
		catch (std::exception &) {
		}

		return false;
	}

	/// Is the tcp connection opened ?
	bool is_open() const {
		return (m_endpoint != boost::asio::ip::udp::endpoint());
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

	/// Flush the data.
	int flush_internal(bool force) {
		if (!is_open()) {
			m_buffer.clear();
			return -1;
		}

		if (!force && m_buffer.empty()) {
			return 0;
		}

		// Do echo.
		echo_thread *instance = echo_thread::get_instance();
		if (instance != NULL) {
			instance->add_request(m_endpoint, m_buffer);
		}
		m_buffer.clear();
		return 0;
	}

private:
	boost::asio::ip::udp::endpoint m_endpoint;
	std::vector<Ch> m_buffer;
	bool m_prev_cr;
};

/**
 * @brief Echo client.
 */
template <class Ch,class Tr=std::char_traits<Ch> >
class basic_echo_ostream : public std::basic_ostream<Ch,Tr> {
public:
	explicit basic_echo_ostream(const std::string &hostname="localhost",
								const std::string &servicename="12532")
		: std::basic_ostream<Ch,Tr>(NULL) {
		this->init(&m_buf);
		open(hostname, servicename);
	}

	~basic_echo_ostream() {
	}

	/// Open the tcp connection.
	bool open(const std::string &hostname="localhost",
			  const std::string &servicename="12532") {
		return m_buf.open(hostname, servicename);
	}

	/// Is the tcp connection opened.
	bool is_open() const {
		return m_buf.is_open();
	}

private:
	basic_echobuf<Ch,Tr> m_buf;
};


typedef basic_echo_ostream<char> echo_ostream;
typedef basic_echo_ostream<wchar_t> echo_wostream;

} // end of namespace net
} // end of namespace lldebug

#endif

