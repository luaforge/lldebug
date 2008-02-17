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

#ifndef __LLDEBUG_VECTORSTREAM_H__
#define __LLDEBUG_VECTORSTREAM_H__

#include <vector>
#include <iostream>

namespace lldebug {
namespace net {

/**
 * @brief Handle 'vector' as a basic_istreambuf class.
 */
template <class Ch, class Tr=std::char_traits<Ch> >
class basic_vector_istreambuf : public std::basic_streambuf<Ch,Tr> {
public:
	explicit basic_vector_istreambuf(const std::vector<Ch> &data)
		: m_data(data), m_pos(0) {
		setbuf(0,0); // suppress buffering
	}

	virtual ~basic_vector_istreambuf() {
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

	virtual std::streamsize xsgetn(char_type* s, std::streamsize n) {
		std::streamsize end = std::min(
			m_pos + n,
			static_cast<std::streamsize>(m_data.size()));
		std::streamsize size = end - m_pos;

		if (size == 0) {
			return Tr::eof(); 
		}
		else {
			std::copy(&m_data[m_pos], &m_data[end], s);
			m_pos += size;
			return size;
		}
	}

	virtual int_type uflow() {
        if (m_pos >= static_cast<std::streamsize>(m_data.size())) {
			return Tr::eof();
		}
		
		return Tr::to_int_type(m_data[m_pos++]);
	}

	virtual int_type underflow() {
		if (m_pos >= static_cast<std::streamsize>(m_data.size())) {
			return Tr::eof();
		}
		
		return Tr::to_int_type(m_data[m_pos]);
	}

	virtual int_type pbackfail(int_type c = Tr::eof()) {
		if (m_pos <= 0 || m_data[m_pos-1] != c) {
			return Tr::eof();
		}

		--m_pos;
		return !Tr::eof(); // any value except eof is ok
	}

private:
	const std::vector<Ch> &m_data;
	typename std::streamsize m_pos;
};

/**
 * @brief Handle 'vector' as a basic_istream class.
 */
template <class Ch,class Tr=std::char_traits<Ch> >
class basic_vector_istream : public std::basic_istream<Ch,Tr> {
public:
	explicit basic_vector_istream(const std::vector<Ch> &data) 
		: std::basic_istream<Ch,Tr>(NULL), m_buf(data) {
		this->init(&m_buf);
	}

	~basic_vector_istream() {
	}

private:
	basic_vector_istreambuf<Ch> m_buf;
};


/**
 * @brief Handle 'vector' as a basic_ostreambuf class.
 */
template <class Ch, class Tr=std::char_traits<Ch> >
class basic_vector_ostreambuf : public std::basic_streambuf<Ch,Tr> {
public:
	explicit basic_vector_ostreambuf() {
		setbuf(0, 0); // suppress buffering
	}

	virtual ~basic_vector_ostreambuf() {
	}

	std::vector<Ch> container() {
		return m_data;
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

	virtual int overflow(int c = Tr::eof()) {
		m_data.push_back(Tr::to_char_type(c));
		return 0;
	}

private:
	std::vector<Ch> m_data;
};

/**
 * @brief Handle 'vector' as a basic_ostream class.
 */
template <class Ch,class Tr=std::char_traits<Ch> >
class basic_vector_ostream : public std::basic_ostream<Ch,Tr> {
public:
	explicit basic_vector_ostream()
		: std::basic_ostream<Ch,Tr>(NULL) {
		this->init(&m_buf);
	}

	~basic_vector_ostream() {
	}

	std::vector<Ch> container() {
		return m_buf.container();
	}

private:
	basic_vector_ostreambuf<Ch> m_buf;
};


typedef basic_vector_istream<char> vector_istream;
typedef basic_vector_ostream<char> vector_ostream;
typedef basic_vector_istream<wchar_t> vector_wistream;
typedef basic_vector_ostream<wchar_t> vector_wostream;

} // end of namespace net
} // end of namespace lldebug

#endif
