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
#include "sysinfo.h"
#include "net/remoteengine.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <fstream>
#include <sstream>

namespace lldebug {

Breakpoint::Breakpoint(const std::string &key, int line,
					   bool isInternal, bool isTemp)
	: m_key(key), m_line(line)
	, m_isInternal(isInternal), m_isTemp(isTemp) {
}

Breakpoint::~Breakpoint() {
}

BreakpointList::BreakpointList(net::RemoteEngine *engine)
	: m_engine(engine) {
}

BreakpointList::~BreakpointList() {
}

Breakpoint BreakpointList::Find(const std::string &key, int line) {
	Breakpoint bp(key, line);

	ImplSet::const_iterator it = m_breakPoints.find(bp);
	if (it == m_breakPoints.end()) {
		return Breakpoint();
	}

	return *it;
}

Breakpoint BreakpointList::First(const std::string &key) {
	// Find the breakpoint which has the least line number.
	Breakpoint tmp(key, -1);
	ImplSet::const_iterator it = m_breakPoints.lower_bound(tmp);
	if (it == m_breakPoints.end()) {
		return Breakpoint();
	}

	// Check whether the keys are same.
	if (it->GetKey() != key) {
		return Breakpoint();
	}

	return *it;
}

Breakpoint BreakpointList::Next(const Breakpoint &bp) {
	// Increment line number and find next.
	Breakpoint tmp(bp.GetKey(), bp.GetLine() + 1);
	ImplSet::const_iterator it = m_breakPoints.lower_bound(tmp);
	if (it == m_breakPoints.end()) {
		return Breakpoint();
	}

	// Check whether the keys are same.
	if (it->GetKey() != bp.GetKey()) {
		return Breakpoint();
	}

	return *it;
}

void BreakpointList::Set(const Breakpoint &bp) {
	if (!bp.IsOk()) {
		return;
	}

#ifdef LLDEBUG_VISUAL
	m_engine->SetBreakpoint(bp);
#else
	std::pair<ImplSet::iterator,bool> it = m_breakPoints.insert(bp);
	if (!it.second) {
		*(it.first) = bp;
	}
	m_engine->ChangedBreakpointList(*this);
#endif
}

void BreakpointList::Remove(const Breakpoint &bp) {
	ImplSet::iterator it = m_breakPoints.find(bp);
	if (it == m_breakPoints.end()) {
		return;
	}

#ifdef LLDEBUG_VISUAL
	m_engine->RemoveBreakpoint(bp);
#else
	m_breakPoints.erase(it);
	m_engine->ChangedBreakpointList(*this);
#endif
}

void BreakpointList::Toggle(const std::string &key, int line) {
	Breakpoint bp(key, line);

	// erase if it exists, add if it isn't
	ImplSet::iterator it = m_breakPoints.find(bp);
	if (it == m_breakPoints.end()) {
		Set(bp);
	}
	else {
		Remove(*it);
	}
}


/*-----------------------------------------------------------------*/
Source::Source(const std::string &key, const std::string &title,
			   const string_array &sources, const std::string &path) {
	// Convert sources to utf8.
	/*string_array array;
	for (string_array::size_type i = 0; i < sources.size(); ++i) {
		wxString str(wxConvLocal.cMB2WX(sources[i].c_str()));

		array.push_back(wxConvToUTF8(str));
	}*/

	m_key = key;
	m_title = title;
	m_path = path;
	m_sources = sources;
	//m_sourceEncoding = enc;
}

Source::Source() {
}

Source::~Source() {
}


/*-----------------------------------------------------------------*/
SourceManager::SourceManager(RemoteEngine *engine)
	: m_engine(engine), m_textCounter(0) {
}

SourceManager::~SourceManager() {
}

std::list<Source> SourceManager::GetList() {
	scoped_lock lock(m_mutex);
	std::list<Source> result;

	ImplMap::iterator it;
	for (it = m_sourceMap.begin(); it != m_sourceMap.end(); ++it) {
		result.push_back(it->second);
	}

	return result;
}

const Source *SourceManager::Get(const std::string &key) {
	scoped_lock lock(m_mutex);

	ImplMap::iterator it = m_sourceMap.find(key);
	if (it == m_sourceMap.end()) {
		return NULL;
	}

	return &(it->second);
}

const Source *SourceManager::GetString(const std::string &key) {
	scoped_lock lock(m_mutex);
	std::list<Source> result;

	ImplMap::iterator it;
	for (it = m_sourceMap.begin(); it != m_sourceMap.end(); ++it) {
		if (it->second.GetKey().find(key) == 0) {
			return &(it->second);
		}
	}

	return NULL;
}

static string_array split(std::istream &stream) {
	string_array array;
	char buffer[2048];

	// Split each line.
	while (! stream.eof()) {
		stream.getline(buffer, sizeof(buffer));
		array.push_back(buffer);
	}

	return array;
}

int SourceManager::Add(const std::string &key) {
	scoped_lock lock(m_mutex);

	if (key.empty()) {
		return -1;
	}

	if (m_sourceMap.find(key) != m_sourceMap.end()) {
		return 0;
	}

	// It's a file if the beginning char is '@'.
	if (key[0] == '@') {
		boost::filesystem::path path(&key[1]);
		path = boost::filesystem::complete(path);
		path = path.normalize();
		std::string pathstr = path.native_file_string();

		std::ifstream ifs(pathstr.c_str());
		if (ifs.fail()) {
			return -1;
		}

		Source src(key, std::string(key, 1), split(ifs), pathstr);
		m_sourceMap.insert(std::make_pair(key, src));
		m_engine->AddedSource(src);
	}
	else {
		// We make the original source title and don't use the key,
		// because it may be too long.
		std::stringstream title;
		title << "[string " << m_textCounter++ << "]";
		title.flush();

		std::stringstream sstream(key);
		Source src(key, title.str(), split(sstream));
		m_sourceMap.insert(std::make_pair(key, src));
		m_engine->AddedSource(src);
	}
	
	return 0;
}

int SourceManager::Add(const Source &source) {
	scoped_lock lock(m_mutex);

	m_sourceMap.insert(std::make_pair(source.GetKey(), source));
	return 0;
}

int SourceManager::Save(const std::string &key, const string_array &source) {
	scoped_lock lock(m_mutex);

	// Find the source from key.
	ImplMap::iterator it = m_sourceMap.find(key);
	if (it == m_sourceMap.end()) {
		return -1;
	}

	Source &src = it->second;
	if (src.GetPath().empty()) {
		return -1;
	}

	// Save the new source.
	std::ofstream fp(src.GetPath().c_str());
	if (fp.fail()) {
		return -1;
	}

	for (string_array::size_type i = 0; i < source.size(); ++i) {
		const std::string &line = source[i];

		if (i > 0) {
			fp << std::endl;
		}

		fp << line; //ConvFromUTF8(line, src.GetSourceEncoding());
	}

	return 0;
}

}
