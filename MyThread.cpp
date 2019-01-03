// MyThread.cpp : implementation file
//

#include "stdafx.h"
#define __D3DRM_H_

#include "QDshowRecordToMp4.h"
#include "MyThread.h"
#include "MainDlg.h"

// CMyThread

IMPLEMENT_DYNCREATE(CMyThread, CWinThread)

CMyThread::CMyThread()
{
	pMainDlg = NULL;
}

CMyThread::~CMyThread()
{
}

BOOL CMyThread::InitInstance()
{
	pMainDlg = new CMainDlg();
	pMainDlg->Create(IDD_MAIN_DLG);
	pMainDlg->ShowWindow(SW_SHOW);

	m_pMainWnd = pMainDlg;

	return TRUE;
}

int CMyThread::ExitInstance()
{
	if (pMainDlg)
	{
		pMainDlg->DestroyWindow();
		delete pMainDlg;
		pMainDlg = NULL;
	}

	

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CMyThread, CWinThread)
END_MESSAGE_MAP()


// CMyThread message handlers
