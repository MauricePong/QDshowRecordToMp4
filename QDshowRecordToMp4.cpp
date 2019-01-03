
// QDshowRecordToMp4.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#define __D3DRM_H_

#include "QDshowRecordToMp4.h"
#include "QDshowRecordToMp4Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CQDshowRecordToMp4App

BEGIN_MESSAGE_MAP(CQDshowRecordToMp4App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CQDshowRecordToMp4App construction

CQDshowRecordToMp4App::CQDshowRecordToMp4App()
	:m_mxGlobalMutex(FALSE, _T("GlobalMutex"))
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	m_arrGrabData.RemoveAll();
	// Place all significant initialization in InitInstance
}


// The one and only CQDshowRecordToMp4App object

CQDshowRecordToMp4App theApp;


// CQDshowRecordToMp4App initialization

BOOL CQDshowRecordToMp4App::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	//_CrtSetBreakAlloc(363);

	CQDshowRecordToMp4Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CString CQDshowRecordToMp4App::Key_ReadString(LPCTSTR lpSubKey, LPCTSTR lpValueName, LPCTSTR lpszDefaultValue)
{
	HKEY hcpl;
	CString	strCombSubKey;
	if( lpSubKey == NULL )
		strCombSubKey = CString( "Software\\DShowTool" );
	else
		strCombSubKey.Format( _T("Software\\DShowTool\\%s"), lpSubKey );
	CString	strValue = _T("");
	if( lpszDefaultValue )
		strValue = CString( lpszDefaultValue );
	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, strCombSubKey, 0, KEY_READ, &hcpl ) == ERROR_SUCCESS ) 
	{
		DWORD	dwType = REG_SZ;
		DWORD	dwSize = 512;
		TCHAR	strRead[512];
		long ret = RegQueryValueEx( hcpl, lpValueName, NULL, &dwType, (LPBYTE)strRead, &dwSize );
		RegCloseKey(hcpl);
		if( ret == ERROR_SUCCESS )
			strValue = CString(strRead);
	}
	return strValue;
}

BOOL CQDshowRecordToMp4App::Key_WriteString(LPCTSTR lpSubKey, LPCTSTR lpValueName, LPCTSTR strValue)
{
	HKEY hcpl;
	CString	strCombSubKey;
	if( lpSubKey == NULL )
		strCombSubKey = CString("Software\\DShowTool");
	else
		strCombSubKey.Format( _T("Software\\DShowTool\\%s"), lpSubKey );
	DWORD dwDisposition;
	if( RegCreateKeyEx( HKEY_LOCAL_MACHINE, strCombSubKey, 0, _T("DSHOWTOOL"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hcpl, &dwDisposition ) == ERROR_SUCCESS ) 
	{
		long ret = RegSetValueEx( hcpl, lpValueName, NULL, REG_SZ, (BYTE*)strValue, (DWORD)(_tcslen(strValue)+1)*sizeof(TCHAR) );
		RegCloseKey(hcpl);
		if( ret == ERROR_SUCCESS )
			return TRUE;
	}
	return FALSE;
}