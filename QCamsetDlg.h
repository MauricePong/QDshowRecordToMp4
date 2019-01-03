#pragma once
#define __D3DRM_H__

#include "afxcmn.h"


// QCamsetDlg dialog

class QCamsetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(QCamsetDlg)

public:
	QCamsetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~QCamsetDlg();

// Dialog Data
	enum { IDD = IDD_CAMSET_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedChkAutoexp();
	afx_msg void OnBnClickedChkAutoWbalence();
	afx_msg void OnBnClickedApplyBtn();

public:
	IBaseFilter* m_pTempVideoFilter;
	CSliderCtrl m_exposureCtrl;
	CSliderCtrl m_whitebalenceCtrl;
	CSliderCtrl m_lightnessCtrl;
	CSliderCtrl m_contackCtrl;
	CSliderCtrl m_saturationCtrl;


};
