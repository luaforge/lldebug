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

#ifndef __LLDEBUG_SYSINFO_H__
#define __LLDEBUG_SYSINFO_H__

namespace lldebug {

std::string GetConfigFileName(const std::string &filename);

/**
 * @brief スクリプトやその実行に対して命令を下すクラスです。
 */
class Command {
public:
	/**
	 * @brief コマンドを識別します。
	 */
	enum Type {
		TYPE_NONE,
		TYPE_PAUSE,
		TYPE_RESTART,
		TYPE_TOGGLE,
		TYPE_STEPOVER,
		TYPE_STEPINTO,
		TYPE_STEPRETURN,
		TYPE_QUIT,
	};

public:
	explicit Command(Type type);
	virtual ~Command();

	/// コマンドの種類を取得します。
	Type GetType() const {
		return m_type;
	}

private:
	Type m_type;
};

/**
 * @brief コマンドキューです。
 */
class CommandQueue {
public:
	explicit CommandQueue();
	virtual ~CommandQueue();

	/// コマンドを最大maxSec秒間待ちます。
	virtual bool Wait(size_t maxSec);

	/// コマンドを取得します。
	virtual const Command &Get();

	/// コマンドを一つ追加します。
	virtual int Push(const Command &cmd);

	/// コマンドを一つ追加します。
	virtual int PushCommand(Command::Type type);

	/// コマンドを一つ削除します。
	virtual void Pop();

	/// コマンドが空かどうか調べます。
	virtual bool IsEmpty();

private:
	typedef std::list<Command> QueueImpl;
	QueueImpl m_queue;

	mutex m_mutex;
	condition m_cond;
};


/**
 * @brief デバッガのブレイクポイントオブジェクトです。
 */
class Breakpoint {
public:
	explicit Breakpoint(const std::string &key = std::string(""), int line = 0,
						bool isInternal = false, bool isTemp = false);
	~Breakpoint();

	/// Is this valid ?
	bool IsOk() const {
		return !m_key.empty();
	}

	/// Get the key of source file.
	const std::string &GetKey() const {
		return m_key;
	}

	/// Get the line number.
	int GetLine() const {
		return m_line;
	}

	/// Is this a internal breakpoint ? (user can't see it)
	bool IsInternal() const {
		return m_isInternal;
	}

	/// Is this a temporary breakpoint ?
	bool IsTemp() const {
		return m_isTemp;
	}

	friend bool operator <(const Breakpoint &x, const Breakpoint &y) {
		return (
			(x.GetKey() < y.GetKey()) ||
			(x.GetKey() == y.GetKey() && x.GetLine() < y.GetLine()));
	}

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int) {
		ar & LLDEBUG_MEMBER_NVP(key);
		ar & LLDEBUG_MEMBER_NVP(line);
	}

private:
	std::string m_key;
	int m_line;
	bool m_isInternal;
	bool m_isTemp;
};

class RemoteEngine;

/**
 * @brief ブレイクポイントのリストを取得します。
 */
class BreakpointList {
public:
	explicit BreakpointList(RemoteEngine *engine);
	virtual ~BreakpointList();

	/// Find the breakpoint from key and line.
	Breakpoint Find(const std::string &key, int line);

	/// Find the next breakpoint (same key and bigger line).
	Breakpoint Next(const Breakpoint &bp);

	/// Set the new breakpoint.
	void Set(const Breakpoint &bp);

	/// Remove the breakpoint.
	void Remove(const Breakpoint &bp);

	/// Toggle on/off of the breakpoint.
	void Toggle(const std::string &key, int line);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int) {
		ar & LLDEBUG_MEMBER_NVP(breakPoints);
	}

private:
	RemoteEngine *m_engine;

	typedef std::set<Breakpoint> ImplSet;
	ImplSet m_breakPoints;
};

/**
 * @brief デバッグ時に表示されるソースファイルなどを管理します。
 */
class Source {
public:
	explicit Source(const std::string &key,
					const std::string &title,
					const string_array &sources,
					const std::string &path = std::string(""));
	explicit Source();
	~Source();

	/// Get the source identifier.
	const std::string &GetKey() const {
		return m_key;
	}

	/// Get the source title with utf8.
	const std::string &GetTitle() const {
		return m_title;
	}

	/// Get the path of the source with local encoding, if any.
	const std::string &GetPath() const {
		return m_path;
	}

	/// Get the source contents with utf8.
	const string_array &GetSources() const {
		return m_sources;
	}

	/// Get the line string of the source with utf8.
	string_array::size_type GetLineCount() const {
		return m_sources.size();
	}

	/// Get the number of the source lines.
	const std::string &GetSourceLine(string_array::size_type l) const {
		return m_sources[l];
	}

	/// 読み込み時のソースのエンコーディングを取得します。
	/*int GetSourceEncoding() const {
		return m_sourceEncoding;
	}*/

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int) {
		ar & LLDEBUG_MEMBER_NVP(key);
		ar & LLDEBUG_MEMBER_NVP(title);
		ar & LLDEBUG_MEMBER_NVP(path);
		ar & LLDEBUG_MEMBER_NVP(sources);
	}

private:
	std::string m_key;
	std::string m_title;
	std::string m_path;
	string_array m_sources;
};

/**
 * @brief The manager of the source files displayed when debugging.
 */
class SourceManager {
public:
	explicit SourceManager(RemoteEngine *engine);
	~SourceManager();

	/// Get the source list.
	std::list<Source> GetList();

	/// Get the source infomation from key.
	const Source *Get(const std::string &key);

	/// Add a source.
	int Add(const std::string &key);

	/// Add a source.
	int Add(const Source &source);

	/// Save a source.
	int Save(const std::string &key, const string_array &source);

private:
	mutex m_mutex;
	RemoteEngine *m_engine;

	typedef std::map<std::string, Source> ImplMap;
	ImplMap m_sourceMap;
	int m_textCounter;
};

}

#endif
