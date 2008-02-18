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

#include "precomp.h"
#include "net/command.h"
#include "net/vectorstream.h"

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

namespace lldebug {
namespace net {

//typedef boost::archive::xml_oarchive serialize_oarchive;
//typedef boost::archive::xml_iarchive serialize_iarchive;
typedef boost::archive::text_oarchive serialize_oarchive;
typedef boost::archive::text_iarchive serialize_iarchive;


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


/*-----------------------------------------------------------------*/
CommandData::CommandData() {
}

CommandData::CommandData(const std::vector<char> &data)
	: m_data(data) {
}

CommandData::~CommandData() {
}

void CommandData::Get_ChangedState(bool &isBreak) const {
	Serializer::ToValue(m_data, isBreak);
}
void CommandData::Set_ChangedState(bool isBreak) {
	m_data = Serializer::ToData(isBreak);
}

void CommandData::Get_UpdateSource(std::string &key, int &line,
										 int &updateCount) const {
	Serializer::ToValue(m_data, key, line, updateCount);
}
void CommandData::Set_UpdateSource(const std::string &key, int line,
										 int updateCount) {
	m_data = Serializer::ToData(key, line, updateCount);
}

void CommandData::Get_AddedSource(Source &source) const {
	Serializer::ToValue(m_data, source);
}
void CommandData::Set_AddedSource(const Source &source) {
	m_data = Serializer::ToData(source);
}

void CommandData::Get_SaveSource(std::string &key,
									   string_array &sources) const {
	Serializer::ToValue(m_data, key, sources);
}
void CommandData::Set_SaveSource(const std::string &key,
									   const string_array &sources) {
	m_data = Serializer::ToData(key, sources);
}

void CommandData::Get_SetUpdateCount(int &updateCount) const {
	Serializer::ToValue(m_data, updateCount);
}
void CommandData::Set_SetUpdateCount(int updateCount) {
	m_data = Serializer::ToData(updateCount);
}

void CommandData::Get_SetBreakpoint(Breakpoint &bp) const {
	Serializer::ToValue(m_data, bp);
}
void CommandData::Set_SetBreakpoint(const Breakpoint &bp) {
	m_data = Serializer::ToData(bp);
}

void CommandData::Get_RemoveBreakpoint(Breakpoint &bp) const {
	Serializer::ToValue(m_data, bp);
}
void CommandData::Set_RemoveBreakpoint(const Breakpoint &bp) {
	m_data = Serializer::ToData(bp);
}

void CommandData::Get_ChangedBreakpointList(BreakpointList &bps) const {
	Serializer::ToValue(m_data, bps);
}
void CommandData::Set_ChangedBreakpointList(const BreakpointList &bps) {
	m_data = Serializer::ToData(bps);
}

void CommandData::Get_OutputLog(LogType &type, std::string &str,
									  std::string &key, int &line) const {
	Serializer::ToValue(m_data, type, str, key, line);
}
void CommandData::Set_OutputLog(LogType type, const std::string &str,
									  const std::string &key, int line) {
	m_data = Serializer::ToData(type, str, key, line);
}

void CommandData::Get_Eval(std::string &str,
								 LuaStackFrame &stackFrame) const {
	Serializer::ToValue(m_data, str, stackFrame);
}
void CommandData::Set_Eval(const std::string &str,
								 const LuaStackFrame &stackFrame) {
	m_data = Serializer::ToData(str, stackFrame);
}

void CommandData::Get_RequestFieldVarList(LuaVar &var) const {
	Serializer::ToValue(m_data, var);
}
void CommandData::Set_RequestFieldVarList(const LuaVar &var) {
	m_data = Serializer::ToData(var);
}

void CommandData::Get_RequestLocalVarList(LuaStackFrame &stackFrame) const {
	Serializer::ToValue(m_data, stackFrame);
}
void CommandData::Set_RequestLocalVarList(const LuaStackFrame &stackFrame) {
	m_data = Serializer::ToData(stackFrame);
}

void CommandData::Get_RequestEnvironVarList(LuaStackFrame &stackFrame) const {
	Serializer::ToValue(m_data, stackFrame);
}
void CommandData::Set_RequestEnvironVarList(const LuaStackFrame &stackFrame) {
	m_data = Serializer::ToData(stackFrame);
}

void CommandData::Get_RequestEvalVarList(string_array &array,
											   LuaStackFrame &stackFrame) const {
	Serializer::ToValue(m_data, array, stackFrame);
}
void CommandData::Set_RequestEvalVarList(const string_array &array,
											   const LuaStackFrame &stackFrame) {
	m_data = Serializer::ToData(array, stackFrame);
}

void CommandData::Get_RequestSource(std::string &key) {
	Serializer::ToValue(m_data, key);
}
void CommandData::Set_RequestSource(const std::string &key) {
	m_data = Serializer::ToData(key);
}

void CommandData::Get_ValueString(std::string &str) const {
	Serializer::ToValue(m_data, str);
}
void CommandData::Set_ValueString(const std::string &str) {
	m_data = Serializer::ToData(str);
}

void CommandData::Get_ValueSource(Source &source) const {
	Serializer::ToValue(m_data, source);
}
void CommandData::Set_ValueSource(const Source &source) {
	m_data = Serializer::ToData(source);
}

void CommandData::Get_ValueVarList(LuaVarList &vars) const {
	Serializer::ToValue(m_data, vars);
}
void CommandData::Set_ValueVarList(const LuaVarList &vars) {
	m_data = Serializer::ToData(vars);
}

void CommandData::Get_ValueBacktraceList(LuaBacktraceList &backtraces) const {
	Serializer::ToValue(m_data, backtraces);
}
void CommandData::Set_ValueBacktraceList(const LuaBacktraceList &backtraces) {
	m_data = Serializer::ToData(backtraces);
}

} // end of namespace net
} // end of namespace lldebug
