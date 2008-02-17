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

#ifndef __LLDEBUG_QUEUE_MT_H__
#define __LLDEBUG_QUEUE_MT_H__

#include <deque>

namespace lldebug {
namespace net {

/**
 * @brief It's a thread safe queue class.
 */
template<class Ty, class Container=std::deque<Ty> >
class queue_mt {
public:
	typedef typename Container::value_type      value_type;
	typedef typename Container::size_type       size_type;
	typedef typename Container::pointer         pointer;
	typedef typename Container::const_pointer   const_pointer;
	typedef typename Container::reference       reference;
	typedef typename Container::const_reference const_reference;

	typedef typename Container                         container_type;
	typedef typename Container::iterator               iterator;
	typedef typename Container::const_iterator         const_iterator;
	typedef typename Container::reverse_iterator       reverse_iterator_type;
	typedef typename Container::const_reverse_iterator const_reverse_iterator_type;

public:
	explicit queue_mt() {
	}

	~queue_mt() {
	}

	void push(const value_type &val) {
		scoped_lock lock(m_mutex);
		this->c.push_back(val);
	}

	void pop() {
		scoped_lock lock(m_mutex);
		this->c.pop_front();
	}

	bool empty() const {
		scoped_lock lock(m_mutex);
		return this->c.empty();
	}

	size_type size() const {
		scoped_lock lock(m_mutex);
		return this->c.size();
	}

	reference back() {
		scoped_lock lock(m_mutex);
		return this->c.back();
	}

	const_reference back() const {
		scoped_lock lock(m_mutex);
		return this->c.back();
	}

	reference front() {
		scoped_lock lock(m_mutex);
		return this->c.front();
	}

	const_reference front() const {
		scoped_lock lock(m_mutex);
		return this->c.front();
	}

public:
	Container c;

private:
	mutable mutex m_mutex;
};

} // end of namespace net
} // end of namespace lldebug

#endif
