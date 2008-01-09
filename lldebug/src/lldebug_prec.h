//---------------------------------------------------------------------------
//
// Name:        MainFrame.h
// Author:      ‰ë”Ž
// Created:     2007/11/23 0:05:33
// Description: MainFrame class declaration
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_PREC_H__
#define __LLDEBUG_PREC_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>
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

#define LUA_CORE
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

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

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

namespace lldebug {
	using boost::shared_ptr;
	using boost::shared_static_cast;
	using boost::shared_dynamic_cast;
	using boost::shared_polymorphic_cast;
	using boost::shared_polymorphic_downcast;
	using boost::static_pointer_cast;
	using boost::weak_ptr;

	using boost::thread;
	using boost::condition;
	typedef boost::recursive_mutex mutex;
	typedef boost::recursive_mutex::scoped_lock scoped_lock;

	typedef std::vector<std::string> string_array;
	typedef std::vector<wxString> StringArray;

	template<class Ty>
	const Ty &median(const Ty &x, const Ty &min_value, const Ty &max_value) {
		return std::max(min_value, std::min(x, max_value));
	}
}

#define LLDEBUG_MEMBER_NVP(name) \
	boost::serialization::make_nvp( \
		BOOST_STRINGIZE(name), BOOST_PP_CAT(m_, name))

#endif

