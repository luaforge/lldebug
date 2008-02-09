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

#ifndef __LLDEBUG_SERIALIZATION__
#define __LLDEBUG_SERIALIZATION__

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
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

#include "lldebug_vectorstream.h"

namespace lldebug {

//typedef boost::archive::xml_oarchive serialize_oarchive;
//typedef boost::archive::xml_iarchive serialize_iarchive;
typedef boost::archive::text_oarchive serialize_oarchive;
typedef boost::archive::text_iarchive serialize_iarchive;

typedef std::vector<char> container_type;

/**
 * @brief Serializer class
 */
struct Serializer {
	template<class T0>
	static container_type ToData(const T0 &value0) {
		vector_ostream stream;
		serialize_oarchive ar(stream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		stream.flush();
		return stream.container();
	}

	template<class T0, class T1>
	static container_type ToData(const T0 &value0, const T1 &value1) {
		vector_ostream stream;
		serialize_oarchive ar(stream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		ar << BOOST_SERIALIZATION_NVP(value1);
		stream.flush();
		return stream.container();
	}

	template<class T0, class T1, class T2>
	static container_type ToData(const T0 &value0, const T1 &value1, const T2 &value2) {
		vector_ostream stream;
		serialize_oarchive ar(stream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		ar << BOOST_SERIALIZATION_NVP(value1);
		ar << BOOST_SERIALIZATION_NVP(value2);
		stream.flush();
		return stream.container();
	}

	template<class T0, class T1, class T2, class T3>
	static container_type ToData(const T0 &value0, const T1 &value1, const T2 &value2, const T3 &value3) {
		vector_ostream stream;
		serialize_oarchive ar(stream);

		ar << BOOST_SERIALIZATION_NVP(value0);
		ar << BOOST_SERIALIZATION_NVP(value1);
		ar << BOOST_SERIALIZATION_NVP(value2);
		ar << BOOST_SERIALIZATION_NVP(value3);
		stream.flush();
		return stream.container();
	}

	template<class T0>
	static void ToValue(const container_type &data, T0 &value0) {
		vector_istream stream(data);
		serialize_iarchive ar(stream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
	}

	template<class T0, class T1>
	static void ToValue(const container_type &data, T0 &value0, T1 &value1) {
		vector_istream stream(data);
		serialize_iarchive ar(stream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
		ar >> BOOST_SERIALIZATION_NVP(value1);
	}

	template<class T0, class T1, class T2>
	static void ToValue(const container_type &data, T0 &value0, T1 &value1, T2 &value2) {
		vector_istream stream(data);
		serialize_iarchive ar(stream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
		ar >> BOOST_SERIALIZATION_NVP(value1);
		ar >> BOOST_SERIALIZATION_NVP(value2);
	}

	template<class T0, class T1, class T2, class T3>
	static void ToValue(const container_type &data, T0 &value0, T1 &value1, T2 &value2, T3 &value3) {
		vector_istream stream(data);
		serialize_iarchive ar(stream);

		ar >> BOOST_SERIALIZATION_NVP(value0);
		ar >> BOOST_SERIALIZATION_NVP(value1);
		ar >> BOOST_SERIALIZATION_NVP(value2);
		ar >> BOOST_SERIALIZATION_NVP(value3);
	}
};

}

#endif
