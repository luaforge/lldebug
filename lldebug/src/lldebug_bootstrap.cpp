//---------------------------------------------------------------------------
//
// Name:        Project1App.cpp
// Author:      雅博
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#include "lldebug_prec.h"
#include "lldebug_bootstrap.h"
#include <boost/function.hpp>

namespace lldebug {

Bootstrap *Bootstrap::ms_instance = NULL;

Bootstrap *Bootstrap::Get() {
	return ms_instance;
}

Bootstrap::Bootstrap()
	: m_state(INITSTATE_INITIAL) {
	wxASSERT(ms_instance == NULL);
	ms_instance = this;
}

Bootstrap::~Bootstrap() {
	Exit();
	WaitExit();
	ms_instance = NULL;
}

void Bootstrap::Success(bool flag) {
	scoped_lock lock(m_mutex);

	if (m_state != INITSTATE_INITIAL) {
		return;
	}

	// initedをtrueにしたのち、condの変化を通知します。
	// この時点でcond.waitを呼んでいる箇所のみそのウェイトが解除されます。
	//
	// 例
	// void thread1() {
	//   scoped_lock lock(mutex);
	//   inited = true;
	//   cond.notify_all();
	// }
	// 
	// void thread2() {
	//   scoped_lock lock(mutex);
	//   while (! inited)
	//     cond.wait(mutex); // waitと同時にmutexのロックを解除。
	// }
	// 
	// 1, notify が wait の前に呼ばれた場合
	//   mutexで変数を保護した後、initedをtrueにしそれを知らせます。
	//   wait側ではmutexのロックが解除された後はinitedがtrueなので
	//   滞りなく処理が終了します。
	// 
	// 2, notify が wait の後に呼ばれた場合
	//   mutexで変数を保護した後、inited値を調べ、falseならwait。
	//   wait内部ではすぐにmutexのロックを解除します。
	//   その後、thread1からnotify_allが呼ばれinitedの値を調べ処理が終了します。
	//
	m_state = (flag ? INITSTATE_INITED : INITSTATE_EXITED);
	m_cond.notify_all();
}
class ThreadObj {
public:
	explicit ThreadObj(int argc, char **argv)
		: m_argc(argc), m_argv(argv) {
	}
	void operator()() {
		wxEntry(m_argc, m_argv);
	}
private:
	int m_argc;
	char **m_argv;
};

int Bootstrap::Initialize() {
	scoped_lock lock(m_mutex);

	if (m_thread != NULL || m_state != INITSTATE_INITIAL) {
		return -1;
	}

	boost::function0<void> fn = ThreadObj(0, NULL);
	shared_ptr<thread> th(new thread(fn));
	if (th == NULL) {
		return -1;
	}

	// まずApplicationクラスの初期化を待ちます。
	while (true) {
		switch (m_state) {
		case INITSTATE_INITIAL:
			break;
		case INITSTATE_INITED:
			goto loop_quit;
		case INITSTATE_EXITED:
			th->join();
			return -1;
		}

		m_cond.wait(lock);
	}
loop_quit:;

	// 終了時に使われるwxTheApp->ExitはwxAppBaseクラスが持つ
	// MainLoopManualオブジェクトに対して終了処理を出しますが、
	// メインループが始まっていないとこのオブジェクト自体が
	// 作られていない可能性が出てきます。
	boost::xtime end;
	boost::xtime_get(&end, boost::TIME_UTC);
	end.sec += 10;
	while (wxApp::GetInstance() == NULL || !wxTheApp->IsMainLoopRunning()) {
		boost::xtime current;
		boost::xtime_get(&current, boost::TIME_UTC);

		// １０秒以上たったらあきらめます。
		if (boost::xtime_cmp(current, end) >= 0) break;
		boost::thread::yield();
		
	}

	m_thread = th;
	return 0;
}

void Bootstrap::Exit() {
	if (wxApp::GetInstance() != NULL) {
		wxTheApp->Exit();
		wxWakeUpIdle();
	}
}

void Bootstrap::WaitExit() {
	scoped_lock lock(m_mutex);
	if (m_thread == NULL) {
		return;
	}

	m_thread->join();
	m_thread.reset();
	m_state = INITSTATE_EXITED;
}

}
