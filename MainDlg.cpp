// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#define __D3DRM_H_
#include "QDshowRecordToMp4.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "CMp4Encoder.h"
#include "QCamsetDlg.h"

UINT AVMuxEncodeDealFunc(LPVOID lpVoid);

UINT AVMuxEncodeDealFunc(LPVOID lpVoid)
{
	CMainDlg* pMange = (CMainDlg*)lpVoid;
	if (pMange)
	{
		pMange->AVMuxEncodeDeal();
	}

	return 0;
}

// CMainDlg dialog

IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMainDlg::IDD, pParent)
{
	m_asVideoDeviceInfo.RemoveAll();
	m_asAudioDeviceInfo.RemoveAll();
	m_arrCamResolutionArr.RemoveAll();
	m_pGraphBuilder = NULL;
	m_pCapture = NULL;
	m_pMediaControl = NULL;
	m_pVW = NULL;
	m_pVideoFilter = NULL;
	m_pAudioFilter = NULL;
	m_pGrabberFilter = NULL;
	m_pAudioGrabberFilter = NULL;
	m_pAudioGrabber = NULL;
	m_hShowWnd = NULL;
	m_bIsVideoOpen = FALSE;
	m_bIsGetDevice = FALSE;
	m_bInit = FALSE;
	m_bRendered = FALSE;
	m_pGrabber = NULL;
	m_strEncodedFullName = _T("");
	m_hManageExitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_bCanEncode = FALSE;
	m_bFirst = FALSE;
	m_dLastVSampleTime = 0.0;
	m_bEndRecord = FALSE;
	m_nSourceVideoType = -1;

	m_pParam = NULL;
	m_pPic_in = NULL;
	m_pPic_out = NULL;
	m_pHandle = NULL;
	m_nFrameIndex = 0;

	m_hFaacEncHandle = NULL;
	m_faacConfigurePtr = NULL;
	m_nInputSamples = 0;    //输入采样大小
	m_nMaxOutputBytes = 0;	//输出大小
}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CBO_DSHOW_DEVICE, m_cbxCtrl);
	DDX_Control(pDX, IDC_CBO_AUDIO_DEVICE, m_cbxAudioCtrl);
	DDX_Control(pDX, IDC_CBO_VIDEO_RESOLUTION, m_cbxResolutionCtrl);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_PATH, &CMainDlg::OnBnClickedBtnPath)
	ON_BN_CLICKED(IDC_BTN_GETDEVICE, &CMainDlg::OnBnClickedBtnGetdevice)
	ON_BN_CLICKED(IDC_BTN_PREVIEW, &CMainDlg::OnBnClickedBtnPreview)
	ON_BN_CLICKED(IDC_BTN_INIT, &CMainDlg::OnBnClickedBtnInit)
	ON_BN_CLICKED(IDC_BTN_BEGIN_ENCODE, &CMainDlg::OnBnClickedBtnBeginEncode)
	ON_BN_CLICKED(IDC_BTN_END_ENCODE, &CMainDlg::OnBnClickedBtnEndEncode)
	ON_BN_CLICKED(IDC_BTN_STOP, &CMainDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_CAM_SET, &CMainDlg::OnBnClickedBtnCamSet)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_OPENCAM, &CMainDlg::OnBnClickedBtnOpencam)
END_MESSAGE_MAP()


// CMainDlg message handlers


BOOL CMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString strSavePath = theApp.Key_ReadString(_T("DShowRecord"), _T("SavePath"), _T("C:\\"));

	GetDlgItem(IDC_EDT_PATH)->SetWindowText(strSavePath);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMainDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// Stop media playback
	if(m_pMediaControl)
	{
		m_pMediaControl->Stop();
	}

	if(m_pVW)
	{
		m_pVW->put_Visible(OAFALSE);
		m_pVW->put_Owner(NULL);
	}
		
	SAFE_RELEASE(m_pCapture);
	SAFE_RELEASE(m_pMediaControl);
	SAFE_RELEASE(m_pGraphBuilder);
	SAFE_RELEASE(m_pVideoFilter);
	SAFE_RELEASE(m_pVW);
	SAFE_RELEASE(m_pAudioFilter);
	SAFE_RELEASE(m_pGrabberFilter);
	SAFE_RELEASE(m_pAudioGrabberFilter);
	SAFE_RELEASE(m_pGrabber);
	SAFE_RELEASE(m_pAudioGrabber);
		
}


//枚举视频采集设备
BOOL CMainDlg::ListVideoCaptureDevices()
{
	int count = 0;

	ImgDeviceInfo sDevice;
	// enumerate all video capture devices
	ICreateDevEnum *pCreateDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	IEnumMoniker *pEm = NULL;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) 
		return 0;

	ULONG cFetched;
	IMoniker *pM = NULL;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		sDevice.nDeviceIndex = count;

		LPOLESTR strPidvid = NULL;
		hr = pM->GetDisplayName(0, 0, &strPidvid);  //获取ID信息
		if(SUCCEEDED(hr))
		{
			//这里获取了一下设备的ID
			USES_CONVERSION; //OLE2T
			CString sPidvid = strPidvid;
			string str = T2A(sPidvid);
			string result;
			static const regex re("(vid_[0-9a-f]{4}&pid_[0-9a-f]{4})",regex::icase);
			smatch match;
			if (regex_search(str, match, re) && match.size() > 1) 
			{
				result = match.str(1);
			} 
			else 
			{
				result = string("");
			} 
			CString strPid(result.c_str());
			strPid.MakeUpper(); //全部大写
			sDevice.strDevicePidVid = strPid;

			IPropertyBag *pBag=0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
				if(hr == NOERROR)
				{
					//获取设备名称	
					char camera_name[1024]; 
					WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
					CString str(camera_name);
					sDevice.strDeviceName = str;
					m_asVideoDeviceInfo.Add(sDevice);
					m_cbxCtrl.AddString(str);

					SysFreeString(var.bstrVal);				
				}
				pBag->Release();
			}
		}
		pM->Release();

		count++;
	}

	if (m_cbxCtrl.GetCount() > 0)
	{
		m_cbxCtrl.SetCurSel(0);
	}

	pEm->Release();
	pCreateDevEnum->Release();

	return 1;
}

//枚举音频采集设备
BOOL CMainDlg::ListAudioCaptureDevices()
{
	int count = 0;

	ImgDeviceInfo sDevice;
	// enumerate all Audio capture devices
	ICreateDevEnum *pCreateDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	IEnumMoniker *pEm = NULL;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) 
		return 0;

	ULONG cFetched;
	IMoniker *pM = NULL;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		sDevice.nDeviceIndex = count;

		LPOLESTR strPidvid = NULL;
		hr = pM->GetDisplayName(0, 0, &strPidvid);  //获取ID信息
		if(SUCCEEDED(hr))
		{
			USES_CONVERSION; //OLE2T
			CString sPidvid = strPidvid;
			string str = T2A(sPidvid);
			string result;
			static const regex re("(vid_[0-9a-f]{4}&pid_[0-9a-f]{4})",regex::icase);
			smatch match;
			if (regex_search(str, match, re) && match.size() > 1) 
			{
				result = match.str(1);
			} 
			else 
			{
				result = string("");
			} 
			CString strPid(result.c_str());
			strPid.MakeUpper(); //全部大写
			sDevice.strDevicePidVid = strPid;

			IPropertyBag *pBag=0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
				if(hr == NOERROR)
				{
					//获取设备名称	
					char camera_name[1024]; 
					WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
					CString str(camera_name);
					sDevice.strDeviceName = str;
					m_asAudioDeviceInfo.Add(sDevice);
					m_cbxAudioCtrl.AddString(str);

					SysFreeString(var.bstrVal);				
				}
				pBag->Release();
			}
		}
		pM->Release();

		count++;
	}

	if (m_cbxAudioCtrl.GetCount() > 0)
	{
		m_cbxAudioCtrl.SetCurSel(0);
	}

	pEm->Release();
	pCreateDevEnum->Release();

	return 1;
}

//创建视频采集过滤器图表
HRESULT CMainDlg::InitCaptureGraphBuilder(IGraphBuilder **ppGraph, ICaptureGraphBuilder2 **ppBuild)
{
	if (!ppGraph || !ppBuild)
	{
		return E_POINTER;
	}
	IGraphBuilder *pGraph = NULL;
	ICaptureGraphBuilder2 *pBuild = NULL;
	// Create the Capture Graph Builder.
	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
	if (SUCCEEDED(hr))
	{
		// Create the Filter Graph Manager.
		hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
		if (SUCCEEDED(hr))
		{
			// Initialize the Capture Graph Builder.
			pBuild->SetFiltergraph(pGraph);  //将过滤器图表添加到管理器中

			// Return both interface pointers to the caller.
			*ppBuild = pBuild;
			*ppGraph = pGraph; // The caller must release both interfaces.

			pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
			pGraph->QueryInterface(IID_IVideoWindow, (void **)&m_pVW);

			return S_OK;
		}
		else
		{
			pBuild->Release();
		}
	}

	return hr; // Failed
}

//选择设备，并获取Video Capture Filter
void CMainDlg::CreateVideoFilter(CString strSelectedDevice, IBaseFilter **pBaseFilter)
{
	ICreateDevEnum *pCreateDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	IEnumMoniker *pEm = NULL;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) 
		return;

	ULONG cFetched;
	IMoniker *pM = NULL;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		IPropertyBag *pBag=0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
			if(hr == NOERROR)
			{
				//获取设备名称	
				char camera_name[1024]; 
				WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
				CString str(camera_name);
				SysFreeString(var.bstrVal);	
				if (strSelectedDevice == str)
				{
					hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pBaseFilter);
					if (FAILED(hr))
					{
						OutputDebugString(_T("BindToObject Failed"));
					}
				}
			}
			pBag->Release();
		}
		pM->Release();
	}

	pEm->Release();
	pCreateDevEnum->Release();
}

void CMainDlg::CreateAudioFilter(CString strSelectedDevice, IBaseFilter **pBaseFilter)
{
	ICreateDevEnum *pCreateDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	IEnumMoniker *pEm = NULL;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) 
		return;

	ULONG cFetched;
	IMoniker *pM = NULL;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		IPropertyBag *pBag=0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
			if(hr == NOERROR)
			{
				//获取设备名称	
				char camera_name[1024]; 
				WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1, camera_name, sizeof(camera_name) ,"",NULL);
				CString str(camera_name);
				SysFreeString(var.bstrVal);	
				if (strSelectedDevice == str)
				{
					hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pBaseFilter);
					if (FAILED(hr))
					{
						OutputDebugString(_T("BindToObject Failed"));
					}
				}
			}
			pBag->Release();
		}
		pM->Release();
	}

	pEm->Release();
	pCreateDevEnum->Release();
}

void CMainDlg::GetVideoResolution()
{
	if (m_pCapture)
	{
		m_arrCamResolutionArr.RemoveAll();
		m_cbxResolutionCtrl.ResetContent();
		IAMStreamConfig *pConfig = NULL;  
		//&MEDIATYPE_Video,如果包括其他媒体类型，第二个参数设置为0
		HRESULT hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
			m_pVideoFilter, IID_IAMStreamConfig, (void **)&pConfig);

		int iCount = 0, iSize = 0;
		hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
		// Check the size to make sure we pass in the correct structure.
		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
		{
			// Use the video capabilities structure.
			for (int iFormat = 0; iFormat < iCount; iFormat++)
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE *pmtConfig = NULL;
				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					//(pmtConfig->subtype == MEDIASUBTYPE_RGB24) &&
					if ((pmtConfig->majortype == MEDIATYPE_Video) &&
						(pmtConfig->formattype == FORMAT_VideoInfo) &&
						(pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
						(pmtConfig->pbFormat != NULL))
					{
						VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
						// pVih contains the detailed format information.
						LONG lWidth = pVih->bmiHeader.biWidth;
						LONG lHeight = pVih->bmiHeader.biHeight;
						BOOL bFind = FALSE;
						//是否已经存在这个分辨率，不存在就加入array
						for (int n=0; n < m_arrCamResolutionArr.GetSize(); n++)
						{
							CamResolutionInfo sInfo = m_arrCamResolutionArr.GetAt(n);
							if (sInfo.nWidth == lWidth && sInfo.nHeight == lHeight)
							{
								bFind = TRUE;
								break;
							}
						}
						if (!bFind)
						{
							CamResolutionInfo camInfo;
							camInfo.nResolutionIndex = iFormat;
							camInfo.nWidth = lWidth;
							camInfo.nHeight = lHeight;
							m_arrCamResolutionArr.Add(camInfo);

							CString strSubType = _T("");
							if (MEDIASUBTYPE_RGB24 == pmtConfig->subtype)
							{
								strSubType = _T("RGB24");
							}
							else if (MEDIASUBTYPE_RGB555 == pmtConfig->subtype)
							{
								strSubType = _T("RGB555");
							}
							else if (MEDIASUBTYPE_RGB32 == pmtConfig->subtype)
							{
								strSubType = _T("RGB32");
							}
							else if (MEDIASUBTYPE_RGB565 == pmtConfig->subtype)
							{
								strSubType = _T("RGB565");
							}
							else if (MEDIASUBTYPE_RGB8 == pmtConfig->subtype)
							{
								strSubType = _T("RGB8");
							}
							else if (MEDIASUBTYPE_IJPG == pmtConfig->subtype)
							{
								strSubType = _T("IJPG");
							}
							else if (MEDIASUBTYPE_YUY2 == pmtConfig->subtype)
							{
								strSubType = _T("YUY2");
							}
							else if (MEDIASUBTYPE_YUYV == pmtConfig->subtype)
							{
								strSubType = _T("YUYV");
							}
							else if (MEDIASUBTYPE_H264 == pmtConfig->subtype)
							{
								strSubType = _T("H264");
							}
							else if (MEDIASUBTYPE_MJPG == pmtConfig->subtype)
							{
								strSubType = _T("MJPG");
							}
							else if (MEDIASUBTYPE_Y41P == pmtConfig->subtype)
							{
								strSubType = _T("Y41P");
							}
							else
							{
								strSubType = _T("其他");
							}
							CString strFormat = _T("");
							strFormat.Format(_T("%d * %d , %s, 采样大小:%ld"), lWidth, lHeight, strSubType, pmtConfig->lSampleSize);
							m_cbxResolutionCtrl.AddString(strFormat);
						}
					}

					// Delete the media type when you are done.
					FreeMediaType(pmtConfig);
				}
			}
		}
		if (m_cbxResolutionCtrl.GetCount() > 0)
		{
			m_cbxResolutionCtrl.SetCurSel(0);
		}
	}
}

void CMainDlg::FreeMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt == NULL)
	{
		return;
	}

	if (pmt->cbFormat != 0) 
	{
		CoTaskMemFree((PVOID)pmt->pbFormat);
		// Strictly unnecessary but tidier
		pmt->cbFormat = 0;
		pmt->pbFormat = NULL;
	}

	if (pmt->pUnk != NULL) 
	{
		pmt->pUnk->Release();
		pmt->pUnk = NULL;
	}
} 

void CMainDlg::GetAudioInfo()
{
	if (m_pCapture)
	{
		IAMStreamConfig *pConfig = NULL;  
		//&MEDIATYPE_Video,如果包括其他媒体类型，第二个参数设置为0
		HRESULT hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, 
			m_pAudioFilter, IID_IAMStreamConfig, (void **)&pConfig);

		int iCount = 0, iSize = 0;
		hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
		// Check the size to make sure we pass in the correct structure.
		if (iSize == sizeof(AUDIO_STREAM_CONFIG_CAPS))
		{
			// Use the video capabilities structure.
			for (int iFormat = 0; iFormat < iCount; iFormat++)
			{
				AUDIO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE *pmtConfig = NULL;
				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					WAVEFORMATEX *pVih = (WAVEFORMATEX*)pmtConfig->pbFormat;

					// Delete the media type when you are done.
					FreeMediaType(pmtConfig);
				}
			}
		}
	}
}


void CMainDlg::OnBnClickedBtnPath()
{
	TCHAR pszPath[MAX_PATH];  
	BROWSEINFO bi;   
	bi.hwndOwner      = this->GetSafeHwnd();  
	bi.pidlRoot       = NULL;  
	bi.pszDisplayName = NULL;   
	bi.lpszTitle      = TEXT("请选择文件夹");   
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;  
	bi.lpfn           = NULL;   
	bi.lParam         = 0;  
	bi.iImage         = 0;   

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);  
	if (pidl == NULL)  
	{  
		return;  
	}  

	if (SHGetPathFromIDList(pidl, pszPath))  
	{  
		GetDlgItem(IDC_EDT_PATH)->SetWindowText(pszPath);
		theApp.Key_WriteString(_T("DShowRecord"), _T("SavePath"), pszPath);
	}
}


void CMainDlg::OnBnClickedBtnGetdevice()
{
	m_cbxCtrl.ResetContent();
	m_cbxAudioCtrl.ResetContent();
	m_asVideoDeviceInfo.RemoveAll();
	m_asAudioDeviceInfo.RemoveAll();
	ListVideoCaptureDevices();
	ListAudioCaptureDevices();

	m_bIsGetDevice = TRUE;
}

void CMainDlg::OnBnClickedBtnInit()
{
	if (m_bInit)
	{
		return;
	}
	if (!m_bIsGetDevice)
	{
		MessageBox(_T("请先获取设备信息"), _T("提示"));
		return;
	}
	//1 选择视频采集设备
	int nIndex  = m_cbxCtrl.GetCurSel();
	if (nIndex < 0)
	{
		MessageBox(_T("请选择视频设备"), _T("提示"));
		return;
	}

	//2 选择音频设备
	int nAIndex  = m_cbxAudioCtrl.GetCurSel();
	if (nAIndex < 0)
	{
		MessageBox(_T("请选择音频设备"), _T("提示"));
		return;
	}

	CString strDeviceName = _T("");
	m_cbxCtrl.GetLBText(nIndex, strDeviceName);
	if (strDeviceName.IsEmpty())
	{
		MessageBox(_T("获取视频设备名称为空"), _T("提示"));
		return;
	}
	//选择设备，并获取Video Capture Filter
	CreateVideoFilter(strDeviceName, &m_pVideoFilter);
	if (m_pVideoFilter == NULL)
	{
		MessageBox(_T("获取m_pVideoFilter失败"), _T("提示"));
		return;
	}

	CString strAudioDeviceName = _T("");
	m_cbxAudioCtrl.GetLBText(nAIndex, strAudioDeviceName);
	if (strAudioDeviceName.IsEmpty())
	{
		MessageBox(_T("获取音频设备名称为空"), _T("提示"));
		return;
	}
	//选择设备，并获取Audio Capture Filter
	CreateAudioFilter(strAudioDeviceName, &m_pAudioFilter);
	if (m_pAudioFilter == NULL)
	{
		MessageBox(_T("获取m_pAudioFilter失败"), _T("提示"));
		return;
	}

	//创建采集过滤器图表
	HRESULT hr = InitCaptureGraphBuilder(&m_pGraphBuilder, &m_pCapture);
	if (S_OK != hr || m_pGraphBuilder == NULL)
	{
		MessageBox(_T("创建IGraphBuilder和ICaptureGraphBuilder2接口失败"), _T("提示"));
		return;
	}

	//创建视频捕捉实例 
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_pGrabberFilter); 
	if (m_pGrabberFilter == NULL)
	{
		MessageBox(_T("获取m_pGrabberFilter失败"), _T("提示"));
		return;
	}

	//创建音频捕捉实例
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_pAudioGrabberFilter); 
	if (m_pAudioGrabberFilter == NULL)
	{
		MessageBox(_T("获取m_pAudioGrabberFilter失败"), _T("提示"));
		return;
	}

	//将获取到的采集过滤器加入图表
	hr = m_pGraphBuilder->AddFilter(m_pVideoFilter, L"Video Capture Filter");
	if (S_OK != hr)
	{
		MessageBox(_T("把m_pVideoFilter添加到IGraphBuilder失败"), _T("提示"));
		return;
	}

	//关闭自动曝光  
	IAMCameraControl *m_pCtrl = NULL;  
	long nValue = -5, nFlag = 1;
	m_pVideoFilter->QueryInterface(IID_IAMCameraControl, (void **)&m_pCtrl );
	if (m_pCtrl)
	{
		m_pCtrl->Get(CameraControl_Exposure, &nValue, &nFlag ); 
		m_pCtrl->Set(CameraControl_Exposure, nValue, CameraControl_Flags_Manual ); 
	}

	hr = m_pGraphBuilder->AddFilter(m_pAudioFilter, L"Audio Capture Filter");
	if (S_OK != hr)
	{
		MessageBox(_T("把m_pAudioFilter添加到IGraphBuilder失败"), _T("提示"));
		return;
	}

	//将视频捕捉过滤器加入图表
	hr = m_pGraphBuilder->AddFilter(m_pGrabberFilter, L"Video Grabber");
	if(S_OK != hr)
	{
		MessageBox(_T("Fail to put sample grabber in graph"));
		return;
	}

	//将音频捕捉过滤器加入图表
	hr = m_pGraphBuilder->AddFilter(m_pAudioGrabberFilter, L"Audio Grabber");
	if(S_OK != hr)
	{
		MessageBox(_T("Fail to put m_pAudioGrabberFilter in graph"));
		return;
	}

	GetVideoResolution();
	//GetAudioInfo();
	m_bInit = TRUE;
}

void CMainDlg::OnBnClickedBtnOpencam()
{
	if (!m_bInit)
	{
		MessageBox(_T("请先初始化！"), _T("提示"));
		return;
	}

	if (m_bIsVideoOpen)
	{
		MessageBox(_T("相机已经打开！"), _T("提示"));
		return;
	}

	if (!m_bRendered)
	{
		//选择一个分辨率
		int nSel = m_cbxResolutionCtrl.GetCurSel();
		if (nSel < 0)
		{
			MessageBox(_T("请选择一个分辨率"), _T("提示"));
			return;
		}
		CString strResolution = _T("");
		m_cbxResolutionCtrl.GetLBText(nSel, strResolution);
		if (strResolution.IsEmpty())
		{
			MessageBox(_T("请选择一个有值分辨率"), _T("提示"));
			return;
		}

		int nSetWidth = -1;
		int nSetHeight = -1;
		int nFindValue = strResolution.Find(_T("*"));
		if (nFindValue > 0)
		{
			nSetWidth = _ttoi(strResolution.Left(nFindValue));

			CString strTempValue = strResolution.Mid(nFindValue + 1);
			nFindValue = strTempValue.Find(_T(","));

			nSetHeight = _ttoi(strTempValue.Left(nFindValue));
		}
		int nResolutionIndex = 0; //默认为0，即当前默认分辨率
		for (int i = 0; i < m_arrCamResolutionArr.GetSize(); i++)
		{
			CamResolutionInfo camInfo = m_arrCamResolutionArr.GetAt(i);
			if (camInfo.nWidth = nSetWidth && camInfo.nHeight == nSetHeight)
			{
				nResolutionIndex = camInfo.nResolutionIndex;
				break;
			}
		}
		m_nFrameWidth = nSetWidth;
		m_nFrameHeight = nSetHeight;

		//设置视频分辨率、格式
		IAMStreamConfig *pConfig = NULL;  
		m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
			m_pVideoFilter, IID_IAMStreamConfig, (void **) &pConfig);

		AM_MEDIA_TYPE *pmt = NULL; 
		VIDEO_STREAM_CONFIG_CAPS scc;
		pConfig->GetStreamCaps(nResolutionIndex, &pmt, (BYTE*)&scc); //nResolutionIndex就是选择的分辨率序号

		//这里仅以采集源中的两种做例子（YUY2和RGB），一般的摄像头都是支持YUY2的
		if (pmt->subtype == MEDIASUBTYPE_YUY2) 
		{
			m_nSourceVideoType = 1;
		}
		else  //如果不是YUY2，则默认为RGB24，需要摄像头支持RGB24，否则只能针对支持的类型做处理
		{
			m_nSourceVideoType = 0;

			pmt->majortype = MEDIATYPE_Video;	
			pmt->subtype = MEDIASUBTYPE_RGB24;  //抓取RGB24
			pmt->formattype = FORMAT_VideoInfo;

			pConfig->SetFormat(pmt);
		}

		m_pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void **)&m_pGrabber);
		HRESULT hr = m_pGrabber->SetMediaType(pmt);
		if(FAILED(hr))
		{
			AfxMessageBox(_T("Fail to set media type!"));
			return;
		}
		//是否缓存数据，缓存的话，可以给后面做其他处理，不缓存的话，图像处理就放在回调中
		m_pGrabber->SetBufferSamples(FALSE); 
		m_pGrabber->SetOneShot(FALSE);
		//mCB.lWidth = nSetWidth;
		//mCB.lHeight = nSetHeight;
		//设置回调，在回调中处理每一帧
		m_pGrabber->SetCallback(&mCB, 1);

		//设置音频抓取数据
		m_pAudioGrabberFilter->QueryInterface(IID_ISampleGrabber, (void **)&m_pAudioGrabber);

		//获取音频采集源的相关参数
		IAMStreamConfig *pAudioConfig = NULL;  
		m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, 
			m_pAudioFilter, IID_IAMStreamConfig, (void **) &pAudioConfig);

		AM_MEDIA_TYPE *audiPmt = NULL; 
		AUDIO_STREAM_CONFIG_CAPS ascc;
		pAudioConfig->GetStreamCaps(0, &audiPmt, (BYTE*)&ascc);

		WAVEFORMATEX *pVih = (WAVEFORMATEX*)audiPmt->pbFormat;
		//pVih->nSamplesPerSec = 4096;
		//这里获取到音频采样率、通道数
		//m_audioCB.m_nChannels = pVih->nChannels;
		//m_audioCB.m_nSamplesPerSec = pVih->nSamplesPerSec;
		//m_audioCB.m_wBitsPerSample = pVih->wBitsPerSample;

		//-----------
		m_nChannels = pVih->nChannels;
		m_nSamplesPerSec = pVih->nSamplesPerSec;
		m_wBitsPerSample = pVih->wBitsPerSample;
		//----------------

		//audiPmt->cbFormat = sizeof(WAVEFORMATEX);
		//audiPmt->pbFormat = (BYTE*)pVih;
		audiPmt->majortype = MEDIATYPE_Audio;//MEDIATYPE_Video
		audiPmt->subtype = MEDIASUBTYPE_PCM;//MEDIASUBTYPE_RGB24
		audiPmt->formattype = FORMAT_WaveFormatEx;//视频采集时没设置这一项

		pAudioConfig->SetFormat(audiPmt);
		hr = m_pAudioGrabber->SetMediaType(audiPmt);
		if(FAILED(hr))
		{
			AfxMessageBox(_T("Fail to set audio media type!"));
			return;
		}

		m_pAudioGrabber->SetBufferSamples(FALSE); 
		m_pAudioGrabber->SetOneShot(FALSE);
		m_pAudioGrabber->SetCallback(&m_audioCB, 1);

		hr = m_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pVideoFilter, m_pGrabberFilter, NULL);
		if( FAILED(hr))
		{
			AfxMessageBox(_T("Video RenderStream failed"));
			return;
		}
		hr = m_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, m_pAudioFilter, m_pAudioGrabberFilter, NULL);
		if( FAILED(hr))
		{
			AfxMessageBox(_T("Audio RenderStream failed"));
			return;
		}

		FreeMediaType(pmt);
		FreeMediaType(audiPmt);

		//m_pVW->put_Visible(OATRUE);

		//如果不想要视频预览，则请注释上方显示的代码，使用下面这样的设置
		m_pVW->put_AutoShow(OAFALSE);
		m_pVW->put_Visible(OAFALSE);

		m_bRendered = TRUE;
	}
	//开始预览
	HRESULT hr = m_pMediaControl->Run();
	if(FAILED(hr))
	{
		AfxMessageBox(_T("Couldn't run the graph!"));
		return;
	}
	m_bIsVideoOpen = TRUE;

	MessageBox(_T("相机打开成功！"), _T("提示"));
}


void CMainDlg::OnBnClickedBtnPreview()
{
	if (!m_bIsVideoOpen)
	{
		MessageBox(_T("请打开相机后再预览！"), _T("提示"));
		return;
	}

	//设置视频显示窗口
	m_hShowWnd = GetDlgItem(IDC_STC_SHOW)->m_hWnd ;		//picture控件的句柄
	HRESULT hr = m_pVW->put_Owner((OAHWND)m_hShowWnd);
	if (FAILED(hr)) 
		return;
	hr = m_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr)) 
		return;

	long nFrameWidth = 0, nFrameHeight = 0;
	m_pVW->get_Width(&nFrameWidth);
	m_pVW->get_Height(&nFrameHeight);

	//图像显示位置,下面的写法是为了让视频显示在当前picture控件的正中央
	if (m_pVW)
	{
		CRect rc;
		::GetClientRect(m_hShowWnd, &rc);
		double fWndWidth = rc.Width();
		double fWndHeight = rc.Height();
		double fWndScale = fWndWidth/fWndHeight;

		double fFrameWidth = nFrameWidth;
		double fFrameHeight = nFrameHeight;
		double fFrameScale = fFrameWidth/fFrameHeight;

		int nShowWidth = fWndWidth;
		int nShowHeight = fWndHeight;
		int xPos = 0, yPos = 0;
		if (fWndScale >= fFrameScale)  //控件窗口宽高比例比视频的宽高大
		{
			if (fWndHeight <= fFrameHeight)
			{
				nShowHeight = fWndHeight;
				nShowWidth = nFrameWidth*fWndHeight/nFrameHeight;
			}
			else
			{
				nShowHeight = fFrameHeight;
				nShowWidth = nFrameWidth;
			}
		}
		else
		{
			if (fWndWidth <= fFrameWidth)
			{
				nShowWidth = fWndWidth;
				nShowHeight = fWndWidth*nFrameHeight/nFrameWidth;
			}
			else
			{
				nShowHeight = fFrameHeight;
				nShowWidth = nFrameWidth;
			}
		}

		xPos = (fWndWidth - nShowWidth)/2;
		yPos = (fWndHeight - nShowHeight)/2;

		m_pVW->SetWindowPosition(xPos, yPos, nShowWidth, nShowHeight);

		m_pVW->put_Visible(OATRUE);
	} 
}

void CMainDlg::OnBnClickedBtnBeginEncode()
{
	if (!m_bIsVideoOpen)
	{
		MessageBox(_T("请打开相机后再开始采集！"), _T("提示"));
		return;
	}

	CString strPath = _T("");
	GetDlgItem(IDC_EDT_PATH)->GetWindowText(strPath);
	if (strPath.IsEmpty())
	{
		AfxMessageBox(_T("请选择存储路径"));
		return;
	}
	if (strPath.Right(1) != _T("\\"))
	{
		strPath += _T("\\");
	}

	CTime time = CTime::GetCurrentTime();
	CString szTime = time.Format("%Y%m%d_%H%M%S");
	m_strEncodedFullName = _T("");
	m_strEncodedFullName.Format(_T("%s%s.mp4"), strPath, szTime);

	m_audioCB.m_bBeginEncode = TRUE;
	mCB.m_bBeginEncode = TRUE;

	m_bCanEncode = FALSE;
	m_bEndRecord = FALSE;

	AfxBeginThread(AVMuxEncodeDealFunc, this);
}


void CMainDlg::OnBnClickedBtnEndEncode()
{
	mCB.m_bBeginEncode = FALSE;
	m_audioCB.m_bBeginEncode = FALSE;

	m_bEndRecord = TRUE;

	SetEvent(m_hManageExitEvent);
}


void CMainDlg::OnBnClickedBtnStop()
{
	if (m_pMediaControl)
	{
		//停止
		m_pMediaControl->Stop();
	}
	m_bIsVideoOpen = FALSE;
}


void CMainDlg::OnBnClickedBtnCamSet()
{
	if (!m_bIsVideoOpen)
	{
		MessageBox(_T("请先打开相机！"), _T("提示"));
		return;
	}

	QCamsetDlg camsetDlg;
	camsetDlg.m_pTempVideoFilter = m_pVideoFilter;
	camsetDlg.DoModal();
}


//处理线程
void CMainDlg::AVMuxEncodeDeal()
{
	//------------------------------视频编码初始化------------------------------------
	//--------------------------------------------------------------------------------
	int csp = X264_CSP_I420;
	int width = m_nFrameWidth;    
	int height = m_nFrameHeight;
	int y_size = width * height;
	ULONG nYUVLen = width * height + (width * height)/2;   //每帧YUV420数据的大小，这么写是为了后面使用方便

	m_pParam = (x264_param_t*)malloc(sizeof(x264_param_t));

	//初始化，是对不正确的参数进行修改,并对各结构体参数和cabac编码,预测等需要的参数进行初始化
	x264_param_default(m_pParam);

	//如果有编码延迟，即时编码设置为zerolatency
	x264_param_default_preset(m_pParam, x264_preset_names[2], "zerolatency"); 

	m_pParam->i_width = width;        //帧宽
	m_pParam->i_height = height;      //帧高
	m_pParam->i_csp = X264_CSP_I420;  //表示用来编码的源视频帧为YUV420的

	//针对本程序，b_annexb必须为1，编出来的NALU前4个字节为分隔用的前缀码，后面mp4v2在判断NALU类型的时候要用到，设置为0则表示NALU的长度，会导致无法判断是什么类型的帧
	m_pParam->b_annexb = 1;         // 值为true，则NALU之前是4字节前缀码0x00000001；
	// 值为false，则NALU之前的4个字节为NALU长度
	m_pParam->b_repeat_headers = 1; //每个关键帧前都发送sps和pps  
	m_pParam->b_cabac = 1;			//自适应上下文算术编码，baseline不支持  
	m_pParam->i_threads = 1;		//处理线程个数
	m_pParam->i_keyint_max = 50;    //设置IDR关键帧的间隔
	m_pParam->i_fps_den=1;			//帧率
	m_pParam->i_fps_num=25;			//帧率 
	m_pParam->rc.b_mb_tree=0;		//不为0导致编码延时帧，所以最好为0  
	m_pParam->rc.i_rc_method = X264_RC_CRF; //码率控制参数，CQP：恒定质量，CRF：恒定码率，ABR：平均码率
	m_pParam->rc.f_rf_constant = 25.0;		//CRF下调整此参数会影响编码速度和图像质量，比如为20，数据速率为2500kbps左右，设置30数据速率1520kbps左右
	m_pParam->rc.f_rf_constant_max = 45.0;  
	m_pParam->i_level_idc = 30;				//表示编码复杂度,具体多复杂，还需再研究

	//设置Profile，这里有7种级别（编码出来的码流规格），级别越高，清晰度越高，耗费资源越大
	//如果用baseline则没有B帧，但baseline通用性很好，所以请斟酌选择
	x264_param_apply_profile(m_pParam, x264_profile_names[1]);

	//x264_picture_t存储压缩编码前的像素数据
	m_pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	m_pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));

	x264_picture_init(m_pPic_out);

	//为图像结构体x264_picture_t分配内存
	x264_picture_alloc(m_pPic_in, csp, m_pParam->i_width, m_pParam->i_height);

	//打开编码器
	m_pHandle = x264_encoder_open(m_pParam);
	if (m_pHandle == NULL)
	{
		MessageBox(_T("视频编码器打开失败！"));
		return;
	}

	m_nFrameIndex = 0;
	//------------------------------------------------------------------------------------------------

	//-------------------------------------------音频编码初始化开始------------------------------------
	//-----------------------------------------------------------------------------------------------
	//这里的m_nSamplesPerSec和m_nChannels都是通过dshow根据实际的音频设备获取到的，不要乱写
	//m_nInputSamples是m_nChannels个通道的采样数，
	m_hFaacEncHandle = faacEncOpen(m_nSamplesPerSec, m_nChannels, &m_nInputSamples, &m_nMaxOutputBytes);
	if(m_hFaacEncHandle == NULL)
	{
		printf("[ERROR] Failed to call faacEncOpen()\n");
		MessageBox(_T("音频编码器打开失败！"));
		return;
	}

	//获取配置
	m_faacConfigurePtr = faacEncGetCurrentConfiguration(m_hFaacEncHandle);
	m_faacConfigurePtr->inputFormat = FAAC_INPUT_16BIT;
	// 0 = Raw,1 = ADTS
	m_faacConfigurePtr->outputFormat = 0;   //裸流还是前面加ADTS
	m_faacConfigurePtr->aacObjectType = LOW;
	m_faacConfigurePtr->allowMidside = 0;
	m_faacConfigurePtr->useLfe = 0;

	//设置配置
	faacEncSetConfiguration(m_hFaacEncHandle, m_faacConfigurePtr);

	unsigned char *chDecodeInfo = NULL;
	GetDecoderSpecificInfo(chDecodeInfo);

	//目标mp4的全路径
	USES_CONVERSION;
	string sFullPathName = T2CA(m_strEncodedFullName);

	//注意参数的传递
	CMp4Encoder *pMp4Encoder = new CMp4Encoder();
	pMp4Encoder->m_sFilePathName = sFullPathName;
	pMp4Encoder->m_vWidth = m_nFrameWidth;      //帧宽
	pMp4Encoder->m_vHeight = m_nFrameHeight;	//帧高
	bool bSuc = pMp4Encoder->InitMp4Encoder(chDecodeInfo, m_nInputSamples/m_nChannels, m_nSamplesPerSec);
	if (!bSuc)
	{
		MessageBox(_T("编码器初始化失败！"));
		return;
	}

	CString strLog = _T("");
	strLog.Format(_T("\n -----Audio m_nInputSamples:%ld,m_nChannels:%d,m_nSamplesPerSec:%ld,m_nMaxOutputBytes:%ld \n"), m_nInputSamples, m_nChannels, m_nSamplesPerSec, m_nMaxOutputBytes);
	OutputDebugString(strLog);

	double videoSampletime = 0.0;
	double dTempTakeTime = 0.0;
	m_bFirst = TRUE;

	while (1)
	{
		DWORD dwRet = WaitForSingleObject(m_hManageExitEvent, 5);  
		if(dwRet == WAIT_OBJECT_0)
		{
			if (theApp.m_arrGrabData.GetSize() <= 0)
			{
				break;
			}
		}
		//因为theApp.m_arrGrabData一直在接收数据，这里要加锁
		theApp.m_mxGlobalMutex.Lock();
		int nCount = theApp.m_arrGrabData.GetSize();
		if(nCount<=0)
		{
			theApp.m_mxGlobalMutex.Unlock();
			continue;
		}

		//每次都取第一个
		GrabDataInfo sDataInfo = theApp.m_arrGrabData.GetAt(0);
		theApp.m_arrGrabData.RemoveAt(0);
		theApp.m_mxGlobalMutex.Unlock();

		if (sDataInfo.nType == 0) //视频编码
		{
			if (!m_bCanEncode)  
			{
				//如果音频还没开始编，则此音频前的所有视频帧都抛弃
				delete[] sDataInfo.pData;
				continue;
			}

			//视频的采样时间，用来做同步
			m_dLastVSampleTime = sDataInfo.dSampleTime;

			strLog.Format(_T("\n -----#####Video sampletime:%f \n"), sDataInfo.dSampleTime);
			OutputDebugString(strLog);

			if (m_bFirst)
			{
				m_bFirst = FALSE;
				//这样做就是为了给videoSampletime赋个初值，只能在sDataInfo获取到后做
				videoSampletime = sDataInfo.dSampleTime;
			}

			//每一帧大小
			BYTE * yuvByte = new BYTE[nYUVLen];
			//每一帧大小中Y、U、V的数据大小
			BYTE * yByte = new BYTE[y_size];
			BYTE * uByte = new BYTE[y_size/4];
			BYTE * vByte = new BYTE[y_size/4];
			//先初始化一下
			memset(yuvByte, 0, nYUVLen);
			memset(yByte, 0, y_size);
			memset(uByte, 0, y_size/4);
			memset(vByte, 0, y_size/4);

			if (m_nSourceVideoType == 1) //来源是YUY2
			{
				//YUY2转YUV420
				YUV422To420(sDataInfo.pData, yByte, uByte, vByte, width, height);
			}
			else   //来源是RGB24
			{
				//把RGB24转为YUV420
				RGB2YUV(sDataInfo.pData, width, height, yuvByte, &nYUVLen);
			}

			//转换后，就用不到原始数据了，可以先释放
			delete[] sDataInfo.pData;

			int iNal = 0;

			//x264_nal_t存储压缩编码后的码流数据
			x264_nal_t* pNals = NULL;

			if (m_nSourceVideoType == 1) 
			{
				//注意，这里是YUV420（或叫I420），注意写的起始位置和大小，前y_size是Y的数据，然后y_size/4是U的数据，最后y_size/4是V的数据
				memcpy(m_pPic_in->img.plane[0], yByte, y_size);		//先写Y
				memcpy(m_pPic_in->img.plane[1], uByte, y_size/4);	//再写U
				memcpy(m_pPic_in->img.plane[2], vByte, y_size/4);	//再写V
			}
			else
			{
				//注意，这里是YUV420（或叫I420），注意写的起始位置和大小，前y_size是Y的数据，然后y_size/4是U的数据，最后y_size/4是V的数据
				memcpy(m_pPic_in->img.plane[0], yuvByte, y_size);						//先写Y
				memcpy(m_pPic_in->img.plane[1], yuvByte + y_size, y_size/4);			//再写U
				memcpy(m_pPic_in->img.plane[2], yuvByte + y_size + y_size/4, y_size/4); //再写V
			}

			m_pPic_in->i_pts = m_nFrameIndex++; //时钟

			//编码一帧图像，pNals为返回的码流数据，iNal是返回的pNals中的NAL单元的数目
			int ret = x264_encoder_encode(m_pHandle, &pNals, &iNal, m_pPic_in, m_pPic_out);
			if (ret < 0)
			{
				OutputDebugString(_T("\n x264_encoder_encode err"));
				delete[] yuvByte;
				delete[] yByte;
				delete[] uByte;
				delete[] vByte;

				continue;
			}

			//计算两帧之间的间隔时间，单位为毫秒
			dTempTakeTime = (sDataInfo.dSampleTime - videoSampletime)*1000;

			//把本帧的采样时间暂存
			videoSampletime = sDataInfo.dSampleTime;

			//写入，注意这里虽然循环多次写入，但只是写入一帧，千万不要在循环里加入时间片间隔，所以j == 0时将间隔时间置为0，否则会出现花屏现象
			for (int j = 0; j < iNal; ++j)
			{
				pMp4Encoder->Mp4VEncode(pNals[j].p_payload, pNals[j].i_payload, dTempTakeTime);

				if (j == 0)  //仅此帧第一次写入的片段需要加上与前一帧的时间间隔，本帧后面的片段是不需要间隔的
				{
					dTempTakeTime = 0.0;
				}
			}

			//用完要释放
			delete[] yuvByte; 
			delete[] yByte;
			delete[] uByte;
			delete[] vByte;
		}
		else if (sDataInfo.nType == 1)//音频
		{
			BOOL bFlag = FALSE;
			//采样时间比刚刚编过码的视频帧小，说明当前音频帧已经过时，需要往后面找合适的音频帧
			if(m_bCanEncode && sDataInfo.dSampleTime - m_dLastVSampleTime < 0.0) 
			{
				bFlag = TRUE;  //后面数据释放与否的标志

				//当前过时的帧可以直接释放
				delete [] sDataInfo.pData;

				//往后找可以用的音频帧
				int nIndex = -1;
				while (nIndex == -1)
				{
					nIndex = -1;
					theApp.m_mxGlobalMutex.Lock();
					int nCount = theApp.m_arrGrabData.GetSize();
					for (int k=0;k<nCount;k++)
					{
						sDataInfo = theApp.m_arrGrabData.GetAt(k);
						//必须找到第一个比上一帧视频帧的采样时间大的音频帧
						if (sDataInfo.nType == 1 && sDataInfo.dSampleTime - m_dLastVSampleTime >= 0.0)
						{
							nIndex = k;  //找到，退出while循环
							break;
						}
					}
					theApp.m_mxGlobalMutex.Unlock();

					if (m_bEndRecord) //
					{
						break;
					}

					Sleep(300);
				}

				//如果用户已经点击了结束采集，则确保theApp.m_arrGrabData里已经没有剩余帧
				if (m_bEndRecord && nIndex == -1)
				{
					continue;
				}
			}

			//设定每次能编码的数据大小
			int nPCMBufferSize = m_nInputSamples*m_wBitsPerSample / 8;

			//编码前后的buffer数据，后面编码时用
			BYTE* pbPCMBuffer = new BYTE[nPCMBufferSize];
			BYTE *pbAACBuffer = new BYTE [m_nMaxOutputBytes];

			ULONG ulTotalEncode = 0;
			int nTime = 0;

			while (1)
			{
				//pBuffer大小为BufferLen，远大于编码能力nPCMBufferSize，所以这里多分几次编
				//每次从pBuffer中取出nPCMBufferSize的大小，直到取完
				memcpy(pbPCMBuffer, sDataInfo.pData+ulTotalEncode, nPCMBufferSize);
				ulTotalEncode += nPCMBufferSize;
				nTime++;
				int nRet = faacEncEncode(m_hFaacEncHandle, (int*) pbPCMBuffer, m_nInputSamples, pbAACBuffer, m_nMaxOutputBytes);
				if (nRet <= 0) //faac一般需要3、4个样本缓存，所以相当于丢弃
				{
					break;
				}
				m_bCanEncode = TRUE;  //等音频能正式用了，视频再正式编码

				strLog.Format(_T("\n -----@@@@@Audio encode sampletime:%f \n"), sDataInfo.dSampleTime);
				OutputDebugString(strLog);

				//写入音频数据
				pMp4Encoder->Mp4AEncode(pbAACBuffer, nRet);

				//取到最后一次要注意，大小不是nPCMBufferSize了，而是BufferLen - ulTotalEncode
				if (sDataInfo.nDataSize < ulTotalEncode + nPCMBufferSize) 
				{
					int nEndDataSize = sDataInfo.nDataSize - ulTotalEncode;
					if (nEndDataSize > 0) //剩余的
					{
						delete[] pbPCMBuffer;
						pbPCMBuffer = new BYTE[nEndDataSize];
						memcpy(pbPCMBuffer, sDataInfo.pData+ulTotalEncode, nEndDataSize);

						//要修改一下输入采样
						int nInputSamples = nEndDataSize / (m_wBitsPerSample/8);
						//对剩余的数据编码
						nRet = faacEncEncode(m_hFaacEncHandle, (int*)pbPCMBuffer, nInputSamples, pbAACBuffer, m_nMaxOutputBytes);
						if (nRet <= 0)
						{
							break;
						}

						pMp4Encoder->Mp4AEncode(pbAACBuffer, nRet);
					}
					break;
				}
			}

			//释放
			delete [] pbPCMBuffer;
			delete [] pbAACBuffer;
			if(!bFlag)
				delete [] sDataInfo.pData;
		}
	}
	//关闭音频编码器
	faacEncClose(m_hFaacEncHandle);
	m_hFaacEncHandle = NULL;

	//释放内存
	x264_picture_clean(m_pPic_in);

	//关闭编码器
	x264_encoder_close(m_pHandle);
	m_pHandle = NULL;

	free(m_pPic_in);
	m_pPic_in = NULL;
	free(m_pPic_out);
	m_pPic_out = NULL;

	free(m_pParam);
	m_pParam = NULL;

	//释放
	pMp4Encoder->CloseMp4Encoder();
	delete pMp4Encoder;

}

unsigned int CMainDlg::GetDecoderSpecificInfo(unsigned char * & apInfo)
{
	if ( m_hFaacEncHandle == NULL )
	{
		return 0;
	}

	unsigned long uLen = 0;
	faacEncGetDecoderSpecificInfo(m_hFaacEncHandle, &apInfo, &uLen);
	return uLen;
}

//RGB24转YUV420
BOOL CMainDlg::RGB2YUV(LPBYTE RgbBuf, UINT nWidth, UINT nHeight, LPBYTE yuvBuf, unsigned long *len)  
{
	if (RgbBuf == NULL)
	{
		return FALSE;
	}

	int i, j;  
	unsigned char* bufY = NULL, *bufU = NULL, *bufV = NULL, *bufRGB = NULL;
	memset(yuvBuf,0,(unsigned int )*len);  
	bufY = yuvBuf;  
	bufV = yuvBuf + nWidth * nHeight;  
	bufU = bufV + (nWidth * nHeight* 1/4);  
	*len = 0;   
	unsigned char y, u, v, r, g, b;  
	unsigned int ylen = nWidth * nHeight;  
	unsigned int ulen = (nWidth * nHeight)/4;  
	unsigned int vlen = (nWidth * nHeight)/4;   
	for (j = 0; j<nHeight;j++)  
	{  
		bufRGB = RgbBuf + nWidth * (nHeight - 1 - j) * 3 ;  
		for (i = 0;i<nWidth;i++)  
		{  
			int pos = nWidth * i + j;  
			r = *(bufRGB++);  
			g = *(bufRGB++);  
			b = *(bufRGB++);  
			y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;            
			u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;            
			v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;  
			*(bufY++) = max( 0, min(y, 255 ));  
			if (j%2==0&&i%2 ==0)  
			{  
				if (u>255)  
				{  
					u=255;  
				}  
				if (u<0)  
				{  
					u = 0;  
				}  
				*(bufU++) =u;  
				//存u分量  
			}  
			else  
			{  
				//存v分量  
				if (i%2==0)  
				{  
					if (v>255)  
					{  
						v = 255;  
					}  
					if (v<0)  
					{  
						v = 0;  
					}  
					*(bufV++) =v;  
				}  
			}  
		}  
	}  
	*len = nWidth * nHeight+(nWidth * nHeight)/2; 

	return TRUE;  
}

//YUY2转YUV420
BOOL CMainDlg::YUV422To420(BYTE* pYUV, BYTE* pY, BYTE* pU, BYTE* pV, LONG lWidth, LONG lHeight)
{
	int i,j;

	BYTE* pYUVTemp = pYUV;
	BYTE* pYUVTempNext = pYUV+lWidth*2;

	for(i=0; i<lHeight; i+=2)
	{
		for(j=0; j<lWidth; j+=2)
		{
			pY[j] = *pYUVTemp++;
			pY[j+lWidth] = *pYUVTempNext++;

			pU[j/2] =(*(pYUVTemp) + *(pYUVTempNext))/2;
			pYUVTemp++;
			pYUVTempNext++;

			pY[j+1] = *pYUVTemp++;
			pY[j+1+lWidth] = *pYUVTempNext++;

			pV[j/2] =(*(pYUVTemp) + *(pYUVTempNext))/2;
			pYUVTemp++;
			pYUVTempNext++;
		}
		pYUVTemp+=lWidth*2;
		pYUVTempNext+=lWidth*2;
		pY+=lWidth*2;
		pU+=lWidth/2;
		pV+=lWidth/2;
	}

	return TRUE;
}


