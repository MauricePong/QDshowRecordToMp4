
// QDshowRecordToMp4Dlg.h : header file
//

#pragma once

#define __D3DRM_H__




// CQDshowRecordToMp4Dlg dialog
class CQDshowRecordToMp4Dlg : public CDialogEx
{
// Construction
public:
	CQDshowRecordToMp4Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_QDSHOWRECORDTOMP4_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();

public:
	 CWinThread *m_pThread;
	
	
};
