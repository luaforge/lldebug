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

/**
 * @brief Change the locale temporary.
 */
class scoped_locale {
public:
	explicit scoped_locale(const std::locale &loc) {
		m_prev = std::locale::global(loc);
	}

	~scoped_locale() {
		std::locale::global(m_prev);
	}

private:
	std::locale m_prev;
};


/**
 * @brief Break point object for the debugger.
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

/**
 * @brief Break point list.
 */
class BreakpointList {
public:
	explicit BreakpointList(shared_ptr<RemoteEngine> engine);
	virtual ~BreakpointList();

	/// Find the breakpoint from key and line.
	Breakpoint Find(const std::string &key, int line);

	/// Find the first breakpoint of the key.
	Breakpoint First(const std::string &key);

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
		ar & LLDEBUG_MEMBER_NVP(set);
	}

private:
	shared_ptr<RemoteEngine> m_engine;

	typedef std::set<Breakpoint> ImplSet;
	ImplSet m_set;
};

/**
 * @brief Source title, path, and contents etc.
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
	explicit SourceManager(shared_ptr<RemoteEngine> engine);
	~SourceManager();

	/// Get the source infomation from key.
	const Source *Get(const std::string &key);

	/// Get the string source by the first part of the key.
	const Source *GetString(const std::string &key);

	/// Get the string source
	std::list<Source> GetList();

	/// Add a source.
	int AddSource(const Source &source);

#ifdef LLDEBUG_CONTEXT
	/// Add a source.
	int Add(const std::string &key, const std::string &src);

	/// Save a source.
	int Save(const std::string &key, const string_array &source);
#endif

private:
	shared_ptr<RemoteEngine> m_engine;

	typedef std::map<std::string, Source> ImplMap;
	ImplMap m_sourceMap;
	int m_textCounter;
};


/**
 * @brief The type of the log message.
 */
enum LogType {
	LOGTYPE_MESSAGE, ///< Normal message.
	LOGTYPE_ERROR, ///< Error message.
	LOGTYPE_WARNING, ///< Warning message.
	LOGTYPE_TRACE, ///< Trace message.
};

/**
 * @brief Log info that contains message and file infomation.
 */
class LogData {
public:
	explicit LogData(LogType type, const std::string &msg,
					 const std::string &key=std::string(""), int line=-1)
		: m_type(type), m_message(msg), m_key(key), m_line(line)
		, m_isRemote(false) {
	}

	explicit LogData()
		: m_type(LOGTYPE_MESSAGE), m_line(-1)
		, m_isRemote(false) {
	}

	~LogData() {
	}

	/// Get the log type.
	LogType GetType() const {
		return m_type;
	}

	/// Get the log message.
	const std::string &GetLog() const {
		return m_message;
	}

	/// Get the source key that is associated with 'Source' class. It may be invalid.
	const std::string &GetKey() const {
		return m_key;
	}

	/// Get the source number. If it's invalid, it returns minus value.
	int GetLine() const {
		return m_line;
	}

	/// Was this log sent through the network ?
	bool IsRemote() const {
		return m_isRemote;
	}

	/// Set this the remote log.
	void SetRemote() {
		m_isRemote = true;
	}

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int) {
		ar & LLDEBUG_MEMBER_NVP(type);
		ar & LLDEBUG_MEMBER_NVP(message);
		ar & LLDEBUG_MEMBER_NVP(key);
		ar & LLDEBUG_MEMBER_NVP(line);
		ar & LLDEBUG_MEMBER_NVP(isRemote);
	}

private:
	LogType m_type;
	std::string m_message;
	std::string m_key;
	int m_line;
	bool m_isRemote;
};

} // end of namespace lldebug

#endif
