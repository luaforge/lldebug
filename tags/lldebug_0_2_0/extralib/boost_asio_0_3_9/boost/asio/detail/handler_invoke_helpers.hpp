//
// handler_invoke_helpers.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2007 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HANDLER_INVOKE_HELPERS_HPP
#define BOOST_ASIO_DETAIL_HANDLER_INVOKE_HELPERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/handler_invoke_hook.hpp>

// Calls to asio_handler_invoke must be made from a namespace that does not
// contain overloads of this function. The asio_handler_invoke_helpers
// namespace is defined here for that purpose.
namespace asio_handler_invoke_helpers {

template <typename Function, typename Context>
inline void invoke(const Function& function, Context* context)
{
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
  Function tmp(function);
  tmp();
#else
  using namespace boost::asio;
  asio_handler_invoke(function, context);
#endif
}

} // namespace asio_handler_invoke_helpers

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_HANDLER_INVOKE_HELPERS_HPP
