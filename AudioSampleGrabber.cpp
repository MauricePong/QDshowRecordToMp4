#include "stdafx.h"
#define __D3DRM_H_

#include "QDshowRecordToMp4.h"
#include "AudioSampleGrabber.h"

CAudioSampleGrabber::CAudioSampleGrabber(void)
{
	m_bBeginEncode = FALSE; 

}

CAudioSampleGrabber::~CAudioSampleGrabber(void)
{
}

ULONG STDMETHODCALLTYPE CAudioSampleGrabber::AddRef() 
{ 
	return 2; 
}

ULONG STDMETHODCALLTYPE CAudioSampleGrabber::Release() 
{ 
	return 1; 
}

HRESULT STDMETHODCALLTYPE CAudioSampleGrabber::QueryInterface(REFIID riid, void ** ppv)
{
	if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown )
	{ 
		*ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
		return NOERROR;
	} 
        
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE CAudioSampleGrabber::SampleCB(double SampleTime, IMediaSample *pSample)
{

	return 0;
}
        
HRESULT STDMETHODCALLTYPE CAudioSampleGrabber::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	if(m_bBeginEncode)
	{
		//timeb curTime;
		//ftime(&curTime);

		BYTE * pcmData = new BYTE[BufferLen];
		memcpy(pcmData, pBuffer, BufferLen);
		GrabDataInfo sData;
		sData.pData = pcmData;
		sData.nDataSize = BufferLen;
		sData.dSampleTime = SampleTime; //curTime.time + ((double)(curTime.millitm+500) / 1000.0);
		sData.nType = 1;

		theApp.m_mxGlobalMutex.Lock();
		theApp.m_arrGrabData.Add(sData);
		theApp.m_mxGlobalMutex.Unlock();

		CString str;
		str.Format(_T("\n Audio--BufferLen:%ld, SampleTime:%f "), BufferLen, sData.dSampleTime);
		OutputDebugString(str);
	}

	return 0;
}
