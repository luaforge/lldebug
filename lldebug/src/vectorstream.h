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

/**
 * @brief Handle 'vector' as a basic_streambuf class.
 */
template <class Ch, class Tr=std::char_traits<Ch>, class Alloc=std::allocator<Ch> >
class basic_vector_streambuf : public std::basic_streambuf<Ch,Tr> {
public:
	typedef std::vector<Ch,Alloc> container_type;
	typedef std::basic_streambuf<Ch,Tr> base_type;
	typedef typename base_type::int_type int_type;
	typedef typename base_type::char_type char_type;
	typedef typename base_type::traits_type traits_type;

public:
	explicit basic_vector_streambuf(const container_type &data)
		: m_buffer(data) {
		if (!m_buffer.empty()) {
			Ch *ptr = &*m_buffer.begin();
			this->setg(ptr, ptr, ptr + m_buffer.size());
		}
	}

	explicit basic_vector_streambuf()
		: m_buffer(256) {
		Ch *ptr = &*m_buffer.begin();
		this->setp(ptr, ptr + m_buffer.size());
	}

	virtual ~basic_vector_streambuf() {
	}

	/// Get the container object.
	container_type container() {
		return container_type(this->pbase(), this->pptr());
	}

protected:
	virtual int_type overflow(int c = Tr::eof()) {
		if (c != Tr::eof()) {
			if (this->pptr() >= this->epptr()) {
				extend_buffer();
			}

			*this->pptr() = Tr::to_char_type(c);
			this->pbump(1);
			return Tr::not_eof(c);
		}
		else {
			return Tr::eof();
		}
	}

	virtual int_type underflow() {
		if (this->gptr() >= this->egptr()){
			return Tr::eof();
		}

		return Tr::to_int_type(*this->gptr());
	}

private:
	/// Extend the buffer size.
	void extend_buffer() {
		if (this->pbase() == this->pptr()) {
			return;
		}

		// Resize the buffer.
		size_t prevpos = (this->pptr() - this->pbase());
		size_t newsize = (this->epptr() - this->pbase()) * 2;
		m_buffer.resize(newsize);

		// Set pointer.
		Ch *ptr = &*m_buffer.begin();
		this->setp(ptr, ptr + m_buffer.size());
		this->pbump((int)prevpos);
	}

private:
	container_type m_buffer;
};


/**
 * @brief Handle 'vector' as a basic_istream class.
 */
template <class Ch, class Tr=std::char_traits<Ch>, class Alloc=std::allocator<Ch> >
class basic_vector_istream : public std::basic_istream<Ch,Tr> {
public:
	typedef basic_vector_streambuf<Ch,Tr,Alloc> buffer_type;
	typedef typename buffer_type::char_type char_type;
	typedef typename buffer_type::container_type container_type;

public:
	explicit basic_vector_istream(const container_type &data) 
		: std::basic_istream<Ch,Tr>(NULL), m_buf(data) {
		this->init(&m_buf);
	}

	~basic_vector_istream() {
	}

private:
	buffer_type m_buf;
};


/**
 * @brief Handle 'vector' as a basic_ostream class.
 */
template <class Ch, class Tr=std::char_traits<Ch>, class Alloc=std::allocator<Ch> >
class basic_vector_ostream : public std::basic_ostream<Ch,Tr> {
public:
	typedef basic_vector_streambuf<Ch,Tr,Alloc> buffer_type;
	typedef typename buffer_type::char_type char_type;
	typedef typename buffer_type::container_type container_type;

public:
	explicit basic_vector_ostream()
		: std::basic_ostream<Ch,Tr>(NULL) {
		this->init(&m_buf);
	}

	~basic_vector_ostream() {
	}

	/// Get the container object.
	container_type container() {
		return m_buf.container();
	}

private:
	buffer_type m_buf;
};

typedef basic_vector_istream<char> vector_istream;
typedef basic_vector_ostream<char> vector_ostream;
typedef basic_vector_istream<wchar_t> vector_wistream;
typedef basic_vector_ostream<wchar_t> vector_wostream;

} // end of namespace lldebug

#endif
