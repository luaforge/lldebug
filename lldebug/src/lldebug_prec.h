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

#ifndef __LLDEBUG_PREC_H__
#define __LLDEBUG_PREC_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>

#define BOOST_SYSTEM_NO_LIB
#define NOMINMAX
#define _WIN32_WINDOWS 0x400
#include "boost/asio.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/functional.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>

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
	using boost::weak_ptr;

	//typedef boost::archive::binary_oarchive serialize_oarchive;
	//typedef boost::archive::binary_iarchive serialize_iarchive;
	typedef boost::archive::xml_oarchive serialize_oarchive;
	typedef boost::archive::xml_iarchive serialize_iarchive;

	using boost::thread;
	using boost::condition;
	typedef boost::recursive_mutex mutex;
	typedef boost::recursive_mutex::scoped_lock scoped_lock;

	typedef std::vector<std::string> string_array;

	template<class Ty>
	const Ty &median(const Ty &x, const Ty &min_value, const Ty &max_value) {
		return std::max(min_value, std::min(x, max_value));
	}
}


#ifdef LLDEBUG_FRAME

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include <wx/textctrl.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/log.h>

#ifdef LLDEBUG_BUILD_DLL
    #define WXDLLIMPEXP_LLDEBUG WXEXPORT
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_LLDEBUG WXIMPORT
#else // not making nor using DLL
    #define WXDLLIMPEXP_LLDEBUG
#endif

namespace lldebug {
	typedef std::vector<wxString> StringArray;
}

#else // LLDEBUG_FRAME

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#endif // LLDEBUG_FRAME

#endif
