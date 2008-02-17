/*
 */

#ifndef __LLDEBUG_QUEUE_H__
#define __LLDEBUG_QUEUE_H__

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
