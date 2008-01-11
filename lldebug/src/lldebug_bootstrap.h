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
