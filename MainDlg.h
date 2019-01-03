#pragma once
#define __D3DRM_H__

#include "SampleGrabberCB.h"
#include "AudioSampleGrabber.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x ) \
	if ( NULL != x ) \
{ \
	x->Release(); \
	x = NULL; \
}
#endif

// CMainDlg dialog

class CMainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMainDlg();

// Dialog Data
	enum { IDD = IDD_MAIN_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

public:
	afx_msg void OnBnClickedBtnPath();
	afx_msg void OnBnClickedBtnGetdevice();
	afx_msg void OnBnClickedBtnInit();
	afx_msg void OnBnClickedBtnOpencam();
	afx_msg void OnBnClickedBtnPreview();
	afx_msg void OnBnClickedBtnBeginEncode();
	afx_msg void OnBnClickedBtnEndEncode();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnCamSet();
	afx_msg void OnDestroy();
	BOOL ListVideoCaptureDevices();
	BOOL ListAudioCaptureDevices();
	HRESULT InitCaptureGraphBuilder(IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppBuild);
	void CreateVideoFilter(CString strSelectedDevice, IBaseFilter **pBaseFilter);
	void CreateAudioFilter(CString strSelectedDevice, IBaseFilter **pBaseFilter);
	void GetVideoResolution();
	void GetAudioInfo();
	void FreeMediaType(AM_MEDIA_TYPE *pmt);
	void AVMuxEncodeDeal();
	BOOL RGB2YUV(LPBYTE RgbBuf, UINT nWidth, UINT nHeight, LPBYTE yuvBuf, unsigned long *len);
	BOOL YUV422To420(BYTE* pYUV, BYTE* pY, BYTE* pU, BYTE* pV, LONG lWidth, LONG lHeight);
	unsigned int GetDecoderSpecificInfo(unsigned char * & apInfo);

public:
	CComboBox m_cbxCtrl;
	CComboBox m_cbxAudioCtrl;
	CComboBox m_cbxResolutionCtrl;
	ASImgDeviceInfoArray m_asVideoDeviceInfo;
	ASImgDeviceInfoArray m_asAudioDeviceInfo;
	ASCamResolutionInfoArray m_arrCamResolutionArr;
	IGraphBuilder *m_pGraphBuilder;
	ICaptureGraphBuilder2* m_pCapture;
	IMediaControl  *m_pMediaControl;
	IVideoWindow* m_pVW;
	IBaseFilter* m_pVideoFilter;
	IBaseFilter* m_pAudioFilter;
	IBaseFilter* m_pGrabberFilter;
	IBaseFilter* m_pAudioGrabberFilter;
	HWND m_hShowWnd;
	BOOL m_bIsVideoOpen;
	BOOL m_bIsGetDevice;
	BOOL m_bInit;
	BOOL m_bRendered;
	ISampleGrabber* m_pGrabber;
	ISampleGrabber* m_pAudioGrabber;
	CSampleGrabberCB mCB;
	CAudioSampleGrabber m_audioCB;
	CString m_strEncodedFullName;
	HANDLE m_hManageExitEvent;
	BOOL m_bCanEncode;
	BOOL m_bFirst;
	double m_dLastVSampleTime;
	BOOL m_bEndRecord;
	int m_nSourceVideoType;
	//X264编码参数
	x264_param_t* m_pParam;
	x264_picture_t* m_pPic_in;
	x264_picture_t* m_pPic_out;
	x264_t* m_pHandle;
	int m_nFrameIndex;
	int m_nFrameWidth;
	int m_nFrameHeight;
	//FAAC编码参数
	faacEncHandle m_hFaacEncHandle;
	faacEncConfigurationPtr m_faacConfigurePtr;
	ULONG m_nInputSamples;		//输入采样大小
	ULONG m_nMaxOutputBytes;	//输出大小
	int m_nChannels;			//通道
	ULONG m_nSamplesPerSec;		//采样率
	int m_wBitsPerSample;		//位数
	
};
