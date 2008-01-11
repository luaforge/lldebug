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
class BreakPoint {
public:
	explicit BreakPoint(const std::string &key = std::string(""), int line = 0,
						bool isInternal = false, bool isTemp = false);
	~BreakPoint();

	/// 設定されているファイルの識別子を取得します。
	const std::string &GetKey() const {
		return m_key;
	}

	/// 設定されている行数を取得します。
	int GetLine() const {
		return m_line;
	}

	/// 内部で使うブレークポイントか？
	bool IsInternal() const {
		return m_isInternal;
	}

	/// 一時的なブレークポイントか？
	bool IsTemp() const {
		return m_isTemp;
	}

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
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
 * @brief ブレイクポイントのリストを取得します。
 */
class BreakPointList {
public:
	explicit BreakPointList();
	virtual ~BreakPointList();

	/// ブレイクポイントを取得します(iは0origin)。
	const BreakPoint &Get(size_t i);

	/// ブレイクポイントの合計サイズを取得します。
	size_t GetSize();

	/// 所定の位置にあるブレイクポイントを探し出します。
	const BreakPoint *Find(const std::string &key, int line);

	/// ブレイクポイントを新規に追加します。
	void Add(const BreakPoint &bp);

	/// ブレイクポイントのオン／オフを切り替えます。
	void Toggle(const std::string &key, int line);

private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & LLDEBUG_MEMBER_NVP(breakPoints);
	}

private:
	mutex m_mutex;

	typedef std::vector<BreakPoint> ImplList;
	ImplList m_breakPoints;
};


/**
 * @brief デバッグ時に表示されるソースファイルなどを管理します。
 */
class Source {
public:
	explicit Source(const std::string &key = std::string(""),
					const std::string &title = std::string(""),
					const string_array &sources = string_array(),
					const std::string &path = std::string(""));
	~Source();

	/// ソースの識別子を取得します。
	const std::string &GetKey() const {
		return m_key;
	}

	/// ソースのタイトルをUTF8形式で取得します。
	const std::string &GetTitle() const {
		return m_title;
	}

	/// ソースのパスがあればそれをネイティブのエンコーディング形式で取得します。
	const std::string &GetPath() const {
		return m_path;
	}

	/// ソースの文字列をUTF8形式で取得します。
	const string_array &GetSources() const {
		return m_sources;
	}

	/// ソースの行数を取得します。
	string_array::size_type GetNumberOfLines() const {
		return m_sources.size();
	}

	/// ソースある行の文字列をUTF8形式で取得します。
	const std::string &GetSourceLine(string_array::size_type l) const {
		return m_sources[l];
	}

	/// 読み込み時のソースのエンコーディングを取得します。
	int GetSourceEncoding() const {
		return m_sourceEncoding;
	}

private:
	std::string m_key;
	std::string m_title;
	std::string m_path;
	string_array m_sources;
	int m_sourceEncoding;
};

/**
 * @brief デバッグ時に表示されるソースファイルなどを管理します。
 */
class SourceManager {
public:
	explicit SourceManager();
	~SourceManager();

	/// ソースファイルをUTF8形式で取得します。
	const Source *Get(const std::string &key);

	/// ソースをキーから追加します。
	/// (keyは'@dir/filename.ext'かソースそのままの文字列で渡されます。)
	int Add(const std::string &key);

	/// ソースをセーブします。
	int Save(const std::string &key, const string_array &source);

private:
	typedef std::map<std::string, Source> ImplMap;
	ImplMap m_sourceMap;
	int m_textCounter;

	mutex m_mutex;
};

}

#endif
