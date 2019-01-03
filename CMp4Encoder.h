//
#pragma once  
#define __D3DRM_H__

#include "stdafx.h"


class CMp4Encoder
{
public:
	CMp4Encoder();
	bool InitMp4Encoder(unsigned char * pchar, int nSampleNum, int nSampleRate);
	void Mp4VEncode(BYTE*, int, double);
	void Mp4AEncode(BYTE*, int);
	void CloseMp4Encoder();

public:
	int m_vWidth,m_vHeight,m_vFrateR,m_vTimeScale;
	MP4FileHandle m_mp4FHandle;
	MP4TrackId m_vTrackId,m_aTrackId;
	string m_sFilePathName;
};