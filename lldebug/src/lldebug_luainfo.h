//---------------------------------------------------------------------------
//
// Name:        Project1App.h
// Author:      雅博
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_LUAINFO_H__
#define __LLDEBUG_LUAINFO_H__

namespace lldebug {

std::string LuaToString(lua_State *L, int idx);

enum VarRootType {
	VARROOT_GLOBAL,
	VARROOT_LOCAL,
	VARROOT_REGISTRY,
	VARROOT_ENVIRON,
	VARROOT_STACK,
};

/**
 * @brief luaの変数情報を保持します。
 */
class LuaVar {
public:
	explicit LuaVar();
	LuaVar(lua_State *L, VarRootType type, int level = -1);
	LuaVar(shared_ptr<LuaVar> parent, const std::string &name, int valueIdx);
	LuaVar(shared_ptr<LuaVar> parent, int keyIdx, int valueIdx);
	virtual ~LuaVar();

	/// このオブジェクトが有効かどうか取得します。
	bool IsOk() const {
		return (m_parent != NULL);
	}

	/// この変数があるlua_State *を取得します。
	lua_State *GetLua() const {
		return m_L;
	}

	/// 基底のテーブルタイプを取得します。
	VarRootType GetRootType() const {
		return m_rootType;
	}

	/// 基底テーブルがローカルだったときのスタックフレームレベルを取得します。
	int GetLevel() const {
		return m_level;
	}

	/// 変数名orテーブルのキー名を取得します。
	const std::string &GetName() const {
		return m_name;
	}

	/// 変数の値を文字列で取得します。
	const std::string &GetValue() const {
		return m_value;
	}

	/// 変数の値型を取得します。
	int GetValueType() const {
		return m_valueType;
	}

	/// 変数の値型を文字列で取得します。
	std::string GetValueTypeName() const {
		return lua_typename(NULL, m_valueType);
	}

	/// 変数がフィールドを持つかどうかを取得します。
	bool HasFields() const {
		return m_hasFields;
	}

	/// 変数がフィールドを持つかどうかを設定します。
	void SetHasFields(bool hasFields) {
		m_hasFields = hasFields;
	}
	/// 親を取得します。
	shared_ptr<LuaVar> GetParent() const {
		return m_parent;
	}

	friend bool operator==(const LuaVar &x, const LuaVar &y) {
		bool xhasp = (x.m_parent == NULL);
		bool yhasp = (y.m_parent == NULL);
		if (xhasp != yhasp) {
			return false;
		}

		if (x.m_L != y.m_L || x.m_name != y.m_name) {
			return false;
		}

		if (x.m_parent == NULL) {
			return (
				x.m_rootType == y.m_rootType &&
				x.m_level == y.m_level);
		}
		else {
			return (*x.m_parent == *y.m_parent);
		}
	}

	friend bool operator!=(const LuaVar &x, const LuaVar &y) {
		return !(x == y);
	}

private:
	lua_State *m_L;
	VarRootType m_rootType;
	int m_level;  ///< 基底がローカルだった場合のスタックフレームレベルです。
	shared_ptr<LuaVar> m_parent;

	std::string m_name;
	std::string m_value;
	int m_valueType;
	bool m_hasFields;
};


/**
 * @brief 関数の呼び出し履歴を保持します。
 */
class LuaBackTraceInfo {
public:
	explicit LuaBackTraceInfo(lua_State *L = NULL, int level = 0,
							  lua_Debug *ar = NULL,
							  const std::string &sourceTitle = std::string(""));
	~LuaBackTraceInfo();

	/// このコールがなされたlua_Stateです。
	lua_State *GetLua() const {
		return m_lua;
	}

	/// 関数のスタックレベルです。
	int GetLevel() const {
		return m_level;
	}

	/// このコールがなされたソースの識別子です。
	const std::string &GetKey() const {
		return m_key;
	}

	/// このコールがなされたソースのタイトルです。
	const std::string &GetTitle() const {
		return m_sourceTitle;
	}

	/// このコールがなされたソースの行番号です。
	int GetLine() const {
		return m_line;
	}

	/// 呼び出し元の関数の名前です。
	const std::string &GetFuncName() const {
		return m_funcName;
	}

public:
	lua_State *m_lua;
	int m_level;

	std::string m_key;
	std::string m_sourceTitle;
	int m_line;
	std::string m_funcName;
};

typedef std::vector<LuaVar> LuaVarList;
typedef std::vector<LuaVar> LuaStackList;
typedef std::vector<LuaBackTraceInfo> LuaBackTrace;

}

#endif
