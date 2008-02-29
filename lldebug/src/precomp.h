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

#ifndef __LLDEBUG_PRECOMP_H__
#define __LLDEBUG_PRECOMP_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <boost/functional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/serialization.hpp>

#define BOOST_SYSTEM_NO_LIB
#ifdef BOOST_WINDOWS_API
	#define NOMINMAX
	#ifndef _WIN32_WINDOWS
		#define _WIN32_WINDOWS 0x400
	#endif
#endif
#include "boost/asio/io_service.hpp"

#if defined(LLDEBUG_CONTEXT)
	extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
	}
#elif defined(LLDEBUG_VISUAL)
	#include <wx/wxprec.h>
	#ifndef WX_PRECOMP
		#include <wx/wx.h>
	#endif
#endif

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

#define LLDEBUG_MEMBER_NVP(name) \
	boost::serialization::make_nvp( \
		BOOST_STRINGIZE(name), BOOST_PP_CAT(m_, name))

namespace lldebug {
	using boost::shared_ptr;
	using boost::shared_static_cast;
	using boost::shared_dynamic_cast;
	using boost::shared_polymorphic_cast;
	using boost::shared_polymorphic_downcast;
	using boost::static_pointer_cast;

	using boost::thread;
	using boost::condition;
	typedef boost::recursive_mutex mutex;
	typedef boost::recursive_mutex::scoped_lock scoped_lock;

	typedef std::vector<std::string> string_array;

	namespace net {
		class RemoteEngine;
		class Command;
	}
	using namespace net;

	template<class Ty>
	const Ty &median(const Ty &x, const Ty &min_value, const Ty &max_value) {
		return std::max(min_value, std::min(x, max_value));
	}
}

#endif
