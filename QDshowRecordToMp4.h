
// QDshowRecordToMp4.h : main header file for the PROJECT_NAME application
//
#define __D3DRM_H__

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CQDshowRecordToMp4App:
// See QDshowRecordToMp4.cpp for the implementation of this class
//

class CQDshowRecordToMp4App : public CWinApp
{
public:
	CQDshowRecordToMp4App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

public:
	BOOL Key_WriteString(LPCTSTR lpSubKey, LPCTSTR lpValueName, LPCTSTR strValue);
	CString Key_ReadString(LPCTSTR lpSubKey, LPCTSTR lpValueName, LPCTSTR lpszDefaultValue);
	CMutex  m_mxGlobalMutex;
	ASGrabDataInfoArray m_arrGrabData;
};

extern CQDshowRecordToMp4App theApp;