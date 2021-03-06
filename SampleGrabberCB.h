#pragma once
#define __D3DRM_H__

#include "qedit.h"


class CSampleGrabberCB :
	public ISampleGrabberCB
{
public:
	CSampleGrabberCB(void);
	virtual ~CSampleGrabberCB(void);

public:
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv);
	HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);

public:
	BOOL m_bBeginEncode;

};

