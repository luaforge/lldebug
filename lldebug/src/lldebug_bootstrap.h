//---------------------------------------------------------------------------
//
// Name:        Project1App.h
// Author:      雅博
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_BOOTSTRAP_H__
#define __LLDEBUG_BOOTSTRAP_H__

namespace lldebug {

/**
 * @brief wxWidgetsを別スレッドで開始するためのクラスです。
 * 
 * wxWidgetsを別スレッドから開始するときに問題になる点は
 * ・wxEntryを呼び出した後でないとスレッドが使用できない
 * ・wxEntryStartとwxApp::OnRunを同じスレッドから呼び出さないといけない
 * などです。
 * 
 * つまり最初だけは、wxThread以外のスレッド機構を用いる必要があります。
 */
class Bootstrap {
public:
	explicit Bootstrap();
	virtual ~Bootstrap();

	virtual int Initialize();
	virtual void Exit();
	virtual void WaitExit();

protected:
	friend class Application;
	static Bootstrap *Get();
	virtual void Success(bool flag);

private:
	/// Initializerの状態を識別します。
	enum InitState {
		INITSTATE_INITIAL,
		INITSTATE_INITED,
		INITSTATE_EXITED,
	};

	static Bootstrap *ms_instance;
	shared_ptr<thread> m_thread;
	mutex m_mutex;
	condition m_cond;
	InitState m_state;
};

}

#endif
