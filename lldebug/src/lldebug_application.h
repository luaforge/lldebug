//---------------------------------------------------------------------------
//
// Name:        Project1App.h
// Author:      雅博
// Created:     2007/11/23 0:05:32
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __LLDEBUG_APPLICATION_H__
#define __LLDEBUG_APPLICATION_H__

#include "lldebug_controls.h"

namespace lldebug {

class ManagerFrame : public wxFrame {
public:
	explicit ManagerFrame();
	virtual ~ManagerFrame();

	void DestroyedFrame(Context *ctx);

private:
	void OnCreateFrame(wxEvent &event);
	void OnDestroyFrame(wxEvent &event);

	DECLARE_EVENT_TABLE();

private:
	int m_frameCounter;
};

/**
 * @brief アプリケーションクラスです。
 */
class Application : public wxApp {
public:
	explicit Application();
	virtual ~Application();

	ManagerFrame *GetFrame() {
		return m_frame;
	}

public:
	virtual bool OnInit();
	virtual int OnExit();

private:
	ManagerFrame *m_frame;
};

}

#endif
