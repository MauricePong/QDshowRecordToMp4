#pragma once
#define __D3DRM_H__

#include "MainDlg.h"


// CMyThread

class CMyThread : public CWinThread
{
	DECLARE_DYNCREATE(CMyThread)

protected:
	CMyThread();           // protected constructor used by dynamic creation
	virtual ~CMyThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()

public:
	CMainDlg *pMainDlg;

};


