// QCamsetDlg.cpp : implementation file
//

#include "stdafx.h"
#define __D3DRM_H_

#include "QDshowRecordToMp4.h"
#include "QCamsetDlg.h"
#include "afxdialogex.h"


// QCamsetDlg dialog

IMPLEMENT_DYNAMIC(QCamsetDlg, CDialogEx)

QCamsetDlg::QCamsetDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(QCamsetDlg::IDD, pParent)
{
	m_pTempVideoFilter = NULL;
}

QCamsetDlg::~QCamsetDlg()
{
}

void QCamsetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_EXPOSURE, m_exposureCtrl);
	DDX_Control(pDX, IDC_SLIDER_WHITEBALENCE, m_whitebalenceCtrl);
	DDX_Control(pDX, IDC_SLIDER_LIGHTNESS, m_lightnessCtrl);
	DDX_Control(pDX, IDC_SLIDER_CONTACT, m_contackCtrl);
	DDX_Control(pDX, IDC_SLIDER_SATURATION, m_saturationCtrl);
}


BEGIN_MESSAGE_MAP(QCamsetDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHK_AUTOEXP, &QCamsetDlg::OnBnClickedChkAutoexp)
	ON_BN_CLICKED(IDC_CHK_AUTO_WBALENCE, &QCamsetDlg::OnBnClickedChkAutoWbalence)
	ON_BN_CLICKED(IDB_APPLY_BTN, &QCamsetDlg::OnBnClickedApplyBtn)
END_MESSAGE_MAP()


// QCamsetDlg message handlers


BOOL QCamsetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_pTempVideoFilter)
	{
		long Min, Max, Step, Default, Flags, Val;

		IAMCameraControl *m_pCtrl;  
		m_pTempVideoFilter->QueryInterface(IID_IAMCameraControl, (void **)&m_pCtrl );  
		m_pCtrl->GetRange(CameraControl_Exposure, &Min, &Max, &Step, &Default, &Flags);
		m_pCtrl->Get(CameraControl_Exposure, &Val, &Flags); 
		m_exposureCtrl.SetRange(Min, Max);
		m_exposureCtrl.SetPos(Val);
		if (Flags == CameraControl_Flags_Auto)
		{
			((CButton*)GetDlgItem(IDC_CHK_AUTOEXP))->SetCheck(1);
		}
		else
		{
			((CButton*)GetDlgItem(IDC_CHK_AUTOEXP))->SetCheck(0);
		}

		CString strShow = _T("");
		strShow.Format(_T("(%d - %d)"), Min, Max);
		GetDlgItem(IDC_STC_EXPOSURE)->SetWindowText(strShow);

		CComPtr <IAMVideoProcAmp> pProcAmp;
		HRESULT hr = m_pTempVideoFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
		if(SUCCEEDED(hr))
		{
			//白平衡
			hr = pProcAmp->GetRange(VideoProcAmp_WhiteBalance, &Min, &Max, &Step,
				&Default, &Flags);
			if(SUCCEEDED(hr))
			{
				hr = pProcAmp->Get(VideoProcAmp_WhiteBalance, &Val, &Flags);
				//Val = Min+(Max-Min)*nBrightNess/100;
				//pProcAmp->Set(VideoProcAmp_Brightness, Val, Flags);

				m_whitebalenceCtrl.SetRange(Min, Max);
				m_whitebalenceCtrl.SetPos(Val);

				strShow.Format(_T("(%d - %d)"), Min, Max);
				GetDlgItem(IDC_STC_WHITE_BALENCE)->SetWindowText(strShow);
			}

			//亮度
			hr = pProcAmp->GetRange(VideoProcAmp_Brightness, &Min, &Max, &Step,
				&Default, &Flags);
			if(SUCCEEDED(hr))
			{
				hr = pProcAmp->Get(VideoProcAmp_Brightness, &Val, &Flags);
				//Val = Min+(Max-Min)*nBrightNess/100;
				//pProcAmp->Set(VideoProcAmp_Brightness, Val, Flags);

				m_lightnessCtrl.SetRange(Min, Max);
				m_lightnessCtrl.SetPos(Val);

				strShow.Format(_T("(%d - %d)"), Min, Max);
				GetDlgItem(IDC_STC_BRIGHTNESS)->SetWindowText(strShow);
			}

			//对比度nContrast
			hr = pProcAmp->GetRange(VideoProcAmp_Contrast, &Min, &Max, &Step,
				&Default, &Flags);
			if(SUCCEEDED(hr))
			{
				hr = pProcAmp->Get(VideoProcAmp_Contrast, &Val, &Flags);
				//Val = Min+(Max-Min)*nContrast/100;
				//pProcAmp->Set(VideoProcAmp_Contrast, Val, Flags);

				m_contackCtrl.SetRange(Min, Max);
				m_contackCtrl.SetPos(Val);

				strShow.Format(_T("(%d - %d)"), Min, Max);
				GetDlgItem(IDC_STC_CONTACT)->SetWindowText(strShow);
			}
			
			//饱和度nSaturation
			hr = pProcAmp->GetRange(VideoProcAmp_Saturation, &Min, &Max, &Step,
				&Default, &Flags);
			if(SUCCEEDED(hr))
			{
				hr = pProcAmp->Get(VideoProcAmp_Saturation, &Val, &Flags);
				//Val = Min+(Max-Min)*nSaturation/100;
				//pProcAmp->Set(VideoProcAmp_Saturation, Val, Flags);

				m_saturationCtrl.SetRange(Min, Max);
				m_saturationCtrl.SetPos(Val);

				strShow.Format(_T("(%d - %d)"), Min, Max);
				GetDlgItem(IDC_STC_SARATION)->SetWindowText(strShow);
			}
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void QCamsetDlg::OnOK()
{
	OnBnClickedApplyBtn();

	CDialogEx::OnOK();
}


void QCamsetDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnCancel();
}


void QCamsetDlg::OnBnClickedChkAutoexp()
{
	int nCheck = ((CButton*)GetDlgItem(IDC_CHK_AUTOEXP))->GetCheck();

	long Val, Flags;
	IAMCameraControl *m_pCtrl;  
	m_pTempVideoFilter->QueryInterface(IID_IAMCameraControl, (void **)&m_pCtrl); 
	m_pCtrl->Get(CameraControl_Exposure, &Val, &Flags); 

	if (nCheck == 1) //自动曝光
	{
		m_pCtrl->Set(CameraControl_Exposure, Val, CameraControl_Flags_Auto);
	}
	else
	{
		m_pCtrl->Set(CameraControl_Exposure, Val, CameraControl_Flags_Manual);
	}
}


void QCamsetDlg::OnBnClickedChkAutoWbalence()
{
	int nCheck = ((CButton*)GetDlgItem(IDC_CHK_AUTO_WBALENCE))->GetCheck();

	long Min, Max, Step, Default, Flags, Val;

	CComPtr <IAMVideoProcAmp> pProcAmp;
	m_pTempVideoFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
	//白平衡
	pProcAmp->GetRange(VideoProcAmp_WhiteBalance, &Min, &Max, &Step, &Default, &Flags);
	pProcAmp->Get(VideoProcAmp_Brightness, &Val, &Flags);

	if (nCheck == 1)
	{
		pProcAmp->Set(VideoProcAmp_Brightness, Val, VideoProcAmp_Flags_Auto);
	}
	else
	{
		pProcAmp->Set(VideoProcAmp_Brightness, Val, VideoProcAmp_Flags_Manual);
	}
}


void QCamsetDlg::OnBnClickedApplyBtn()
{
	int nCheck1 = ((CButton*)GetDlgItem(IDC_CHK_AUTOEXP))->GetCheck();
	int nCheck2 = ((CButton*)GetDlgItem(IDC_CHK_AUTO_WBALENCE))->GetCheck();

	if (nCheck1 == 0)
	{
		int nValue = m_exposureCtrl.GetPos();
		IAMCameraControl *m_pCtrl;  
		m_pTempVideoFilter->QueryInterface(IID_IAMCameraControl, (void **)&m_pCtrl); 
		m_pCtrl->Set(CameraControl_Exposure, nValue, CameraControl_Flags_Manual);
	}

	CComPtr <IAMVideoProcAmp> pProcAmp;
	m_pTempVideoFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
	if (nCheck2 == 0)
	{
		int nValue = m_whitebalenceCtrl.GetPos();
		pProcAmp->Set(VideoProcAmp_Brightness, nValue, VideoProcAmp_Flags_Manual);
	}

	int nValue = m_lightnessCtrl.GetPos();
	pProcAmp->Set(VideoProcAmp_Brightness, nValue, VideoProcAmp_Flags_Manual);

	nValue = m_contackCtrl.GetPos();
	pProcAmp->Set(VideoProcAmp_Contrast, nValue, VideoProcAmp_Flags_Manual);

	nValue = m_saturationCtrl.GetPos();
	pProcAmp->Set(VideoProcAmp_Saturation, nValue, VideoProcAmp_Flags_Manual);
}
