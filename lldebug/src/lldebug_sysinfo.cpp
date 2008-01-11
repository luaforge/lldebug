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

#include "lldebug_prec.h"
#include "lldebug_sysinfo.h"
#include "lldebug_codeconv.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <fstream>
#include <sstream>

namespace lldebug {

std::string GetConfigFileName(const std::string &filename) {
	using namespace boost::filesystem;
	wxASSERT(!filename.empty());
	wxString dir = wxStandardPaths().GetUserConfigDir();

	path basePath(wxConvLocal.cWC2MB(dir.c_str()));
	path configPath = basePath / "lldebug";
	if (!exists(configPath)) {
		try {
			if (!create_directory(configPath)) {
				configPath = basePath;
			}
		}
		catch (filesystem_error &) {
			configPath = basePath;
		}
	}

	configPath /= filename;
	return configPath.native_file_string();
}

Command::Command(Type type)
	: m_type(type) {
}

Command::~Command() {
}

CommandQueue::CommandQueue() {
}

CommandQueue::~CommandQueue() {
}

bool CommandQueue::Wait(size_t maxSec) {
	scoped_lock lock(m_mutex);

	if (m_queue.empty()) {
		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += maxSec;

		return m_cond.timed_wait(lock, xt);
	}

	return true;
}

const Command &CommandQueue::Get() {
	scoped_lock lock(m_mutex);
	return m_queue.front();
}

int CommandQueue::Push(const Command &cmd) {
	scoped_lock lock(m_mutex);
	m_queue.push_back(cmd);
	m_cond.notify_all();
	return 0;
}

int CommandQueue::PushCommand(Command::Type type) {
	return Push(Command(type));
}

void CommandQueue::Pop() {
	scoped_lock lock(m_mutex);
	m_queue.pop_front();
}

bool CommandQueue::IsEmpty() {
	scoped_lock lock(m_mutex);
	return m_queue.empty();
}


/*-----------------------------------------------------------------*/
BreakPoint::BreakPoint(const std::string &key, int line,
					   bool isInternal, bool isTemp)
	: m_key(key), m_line(line)
	, m_isInternal(isInternal), m_isTemp(isTemp) {
}

BreakPoint::~BreakPoint() {
}

BreakPointList::BreakPointList() {
}

BreakPointList::~BreakPointList() {
}

const BreakPoint &BreakPointList::Get(size_t i) {
	scoped_lock lock(m_mutex);

	return m_breakPoints[i];
}

size_t BreakPointList::GetSize() {
	scoped_lock lock(m_mutex);

	return m_breakPoints.size();
}

const BreakPoint *BreakPointList::Find(const std::string &key, int line) {
	scoped_lock lock(m_mutex);

	for (ImplList::iterator it = m_breakPoints.begin();
		it != m_breakPoints.end();
		++it) {
		if (it->GetKey() == key && it->GetLine() == line) {
			return &(*it);
		}
	}

	return NULL;
}

void BreakPointList::Add(const BreakPoint &bp) {
	scoped_lock lock(m_mutex);

	if (Find(bp.GetKey(), bp.GetLine()) == NULL) {
		m_breakPoints.push_back(bp);
	}
}

void BreakPointList::Toggle(const std::string &key, int line) {
	scoped_lock lock(m_mutex);

	for (ImplList::iterator it = m_breakPoints.begin();
		it != m_breakPoints.end();
		++it) {
		if (it->GetKey() == key && it->GetLine() == line) {
			m_breakPoints.erase(it);
			return;
		}
	}

	// 見つからなかったらブレークポイントを追加します。
	m_breakPoints.push_back(BreakPoint(key, line, false, false));
}


/*-----------------------------------------------------------------*/
Source::Source(const std::string &key, const std::string &title,
			   const string_array &sources, const std::string &path) {
	char buffer[1024 * 4];

	// sizeof(buffer)分の文字列を抽出します。
	string_array::size_type pos = 0;
	for (string_array::size_type i = 0; i < sources.size() && pos < sizeof(buffer) - 1; ++i) {
		const std::string &str = sources[i];

		size_t nextpos = std::min(pos + str.length(), sizeof(buffer) - 1);
		memcpy(&buffer[pos], str.c_str(), nextpos - pos);
		pos = nextpos;
	}
	buffer[pos] = '\0';

	// ソースのエンコーディングを識別します。
	int enc = GetEncoding(buffer);

	// エンコーディングをUTF8に変換します。
	string_array array;
	for (string_array::size_type i = 0; i < sources.size(); ++i) {
		const std::string &str = sources[i];
		array.push_back(ConvToUTF8From(str, enc));
	}

	m_key = key;
	m_title = ConvToUTF8(title);
	m_path = path;
	m_sources = array;
	m_sourceEncoding = enc;
}

Source::~Source() {
}


SourceManager::SourceManager()
	: m_textCounter(0) {
}

SourceManager::~SourceManager() {
}

const Source *SourceManager::Get(const std::string &key) {
	scoped_lock lock(m_mutex);

	ImplMap::iterator it = m_sourceMap.find(key);
	if (it == m_sourceMap.end()) {
		return NULL;
	}

	return &(it->second);
}

static string_array split(std::istream &stream) {
	string_array array;
	char buffer[2048];

	// 行ごとに分解します。
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

	// 先頭が'@'ならそれはファイル名です。
	if (key[0] == '@') {
		// パスのエンコーディングは変換しません。
		boost::filesystem::path path(&key[1]);
		boost::filesystem::complete(path);
		std::string filepath = path.native_file_string();

		std::ifstream ifs(filepath.c_str());
		if (ifs.fail()) {
			return -1;
		}

		Source src(key, std::string(key, 1), split(ifs), filepath);
		m_sourceMap.insert(std::make_pair(key, src));
	}
	else {
		// ソースがテキストの場合は長すぎる可能性があるので、タイトルを別に作成します。
		std::stringstream title;
		title << "<string" << m_textCounter++ << ">";
		title.flush();

		std::stringstream sstream(key);
		Source src(key, title.str(), split(sstream));
		m_sourceMap.insert(std::make_pair(key, src));
	}

	return 0;
}

int SourceManager::Save(const std::string &key, const string_array &source) {
	scoped_lock lock(m_mutex);

	ImplMap::iterator it = m_sourceMap.find(key);
	if (it == m_sourceMap.end()) {
		return -1;
	}

	// ソースファイルではありません。
	Source &src = it->second;
	if (src.GetPath().empty()) {
		return -1;
	}

	std::ofstream fp(src.GetPath().c_str());
	if (fp.fail()) {
		return -1;
	}

	for (string_array::size_type i = 0; i < source.size(); ++i) {
		const std::string &line = source[i];

		// 最後の行に改行を入れないようにします。
		if (i > 0) {
			fp << std::endl;
		}

		fp << ConvFromUTF8(line, src.GetSourceEncoding());
	}

	return 0;
}

}
