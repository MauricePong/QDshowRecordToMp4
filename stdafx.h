
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


#include <stdio.h>  
#include <stdlib.h>  
//#include "inttypes.h"
#include "stdint.h"

#include <fstream>
#include <string>
#include <regex>
using namespace std;

//DShow库加载
#include "Wmcodecdsp.h"
#pragma comment(lib,"wmcodecdspuuid.lib") 

#include "Dshow.h"
#include <qedit.h>
#include "Dmodshow.h"
#include "dmo.h"

#ifdef _DEBUG
	#pragma comment(lib,"strmbasd.lib")
#else
	#pragma comment(lib,"STRMBASE.lib")
#endif
#pragma comment(lib,"strmiids.lib")
#pragma comment(lib,"quartz.lib")
#pragma comment(lib,"dmoguids.lib")   


//要支持YUV420,也就是WMMEDIASUBTYPE_I420必须添加此头文件
#include <wmsdkidl.h>

//X264
extern "C"
{
#include "x264.h"  
};
#pragma comment(lib,"libx264.lib")

//mp4v2
#include "mp4v2\mp4v2.h"
#ifdef _DEBUG
	#pragma comment(lib,"libmp4v2_debug.lib")
#else
	#pragma comment(lib,"libmp4v2.lib")
#endif

//faac
extern "C"
{
#include "faac.h"
};
#ifdef _DEBUG
	#pragma comment(lib,"libfaacd.lib")
#else
	#pragma comment(lib,"libfaac.lib")
#endif

#include <sys/timeb.h>

//用来存储视音频数据
struct GrabDataInfo
{
	BYTE *pData;
	int nDataSize;
	double dSampleTime;
	int nType;     //0为视频，1为音频

	GrabDataInfo()
	{
		pData = NULL;
		nDataSize = 0;
		dSampleTime = 0.0;
		nType = -1;
	};

	GrabDataInfo(const GrabDataInfo &other)
	{
		*this = other;
	};

	GrabDataInfo& operator = (const GrabDataInfo& other)
	{
		pData = other.pData;
		nDataSize = other.nDataSize;
		dSampleTime = other.dSampleTime;
		nType = other.nType;
		return *this;
	};
};
typedef CArray <GrabDataInfo, GrabDataInfo&> ASGrabDataInfoArray;

//相机设备信息
struct ImgDeviceInfo
{
	CString strDevicePidVid;		//设备PIDVID
	CString strDeviceName;			//设备名称
	int nDeviceIndex;				//设备序号

	ImgDeviceInfo()
	{
		strDevicePidVid = _T("");
		strDeviceName = _T("");
		nDeviceIndex = -1;
	};

	ImgDeviceInfo(const ImgDeviceInfo &other)
	{
		*this = other;
	};

	ImgDeviceInfo& operator = (const ImgDeviceInfo& other)
	{
		strDevicePidVid = other.strDevicePidVid;
		strDeviceName = other.strDeviceName;
		nDeviceIndex = other.nDeviceIndex;
		return *this;
	};
};
typedef CArray <ImgDeviceInfo, ImgDeviceInfo&> ASImgDeviceInfoArray;

//分辨率
struct CamResolutionInfo
{
	int nWidth;				//分辨率宽
	int nHeight;			//分辨率高
	int nResolutionIndex;	//分辨率序号
	CString strSubType;     //视频子类型

	CamResolutionInfo()
	{
		nWidth = 640;
		nHeight = 480;
		nResolutionIndex = -1;
		strSubType = _T("");
	};

	CamResolutionInfo(const CamResolutionInfo &other)
	{
		*this = other;
	};

	CamResolutionInfo& operator = (const CamResolutionInfo& other)
	{
		nWidth = other.nWidth;
		nHeight = other.nHeight;
		nResolutionIndex = other.nResolutionIndex;
		strSubType = other.strSubType;
		return *this;
	};
};
typedef CArray <CamResolutionInfo, CamResolutionInfo&> ASCamResolutionInfoArray;


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


