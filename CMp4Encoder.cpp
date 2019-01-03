#include "stdafx.h"
#define __D3DRM_H_

#include "CMp4Encoder.h"
//
//
CMp4Encoder::CMp4Encoder()
	: m_vWidth(640),
	  m_vHeight(480),
	  m_vFrateR(25),
	  m_vTimeScale(90000),
	  m_mp4FHandle(NULL),
	  m_aTrackId(MP4_INVALID_TRACK_ID),
	  m_vTrackId(MP4_INVALID_TRACK_ID)
{
	m_sFilePathName = "";
}

bool CMp4Encoder::InitMp4Encoder(unsigned char * pchar, int nSampleNum, int nSampleRate)
{
	if (m_sFilePathName.length() <= 0)
	{
		return false;
	}

	//-------- file handle--------------------
	m_mp4FHandle = MP4Create(m_sFilePathName.c_str());
	if (m_mp4FHandle == MP4_INVALID_FILE_HANDLE)
	{
		MessageBoxA(NULL,"mp4fileHandle Error!","ERROR",MB_OK);
		return false;  
	}
	MP4SetTimeScale(m_mp4FHandle, m_vTimeScale);
	//-----------------------------audio track----------------------------------------
	m_aTrackId = MP4AddAudioTrack(m_mp4FHandle, nSampleRate, nSampleNum, MP4_MPEG4_AUDIO_TYPE);
	if (m_aTrackId == MP4_INVALID_TRACK_ID)
	{
		MessageBoxA(NULL,"AudioTrack Error!","ERROR",MB_OK);
		return false;
	}
	//
	MP4SetAudioProfileLevel(m_mp4FHandle, 0x2);                                                         
	MP4SetTrackESConfiguration(m_mp4FHandle, m_aTrackId, pchar, 2);
	//-------------------------------------------------------------------------------------
	return true;
}


//------------------------------h264码流分析------------------------------------------
//	h264 NALU:  0x00 00 00 01 | nalu_type(1字节)| nalu_data (N 字节) | 0x00 00 00 01 | ...
//				起始码(4字节)          类型             数据           下一个NALU起始码
//  nalu_type有以下种类，长度为1个字节，8位分别表示：【 1比特 禁止位，恒为0 | 2比特 重要性指示位  | 5比特 类型 】
//	0x67 (0 11 00111) SPS		非常重要			
//	0x68 (0 11 01000) PPS		非常重要			
//	0x65 (0 11 00101) IDR帧		非常重要，关键帧
//	0x61 (0 11 00001) I帧		重要	，非IDR的关键帧
//	0x41 (0 10 00001) P帧		重要				 
//	0x01 (0 00 00001) B帧		不重要				
//	0x06 (0 00 00110) SEI		不重要				
// 这里对每个NAL单元的前4或5个字节进行解析，从而查找是什么类型的NAL单元
// 第一帧 SPS【0 0 0 1 0x67】 PPS【0 0 0 1 0x68】 SEI【0 0 1 0x6】 IDR【0 0 1 0x65】
// p帧      P【0 0 0 1 0x41】
// I帧    SPS【0 0 0 1 0x67】 PPS【0 0 0 1 0x68】 IDR【0 0 1 0x65】
// 除非编码器和解码器进行特定的语法协商，解码器一般不对SEI包进行解析，所以这里不处理SEI数据
//
// MP4WriteSample接收I、P帧 nalu,该nalu需要用4字节的数据大小头替换原有的起始头，并且数据大小为big-endian格式（即将高字节存储在起始地址）
//-------------------------------------------------------------------------------------------------

void CMp4Encoder::Mp4VEncode(BYTE* _naluData,int _naluSize, double nTakeTime)
{
	int index = -1;
	//
	if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x67)
	{
		index = 0x67;
	}
	if(index!=0x67 && m_vTrackId==MP4_INVALID_TRACK_ID)
	{
		return;
	}
	if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x68)
	{
		index = 0x68;
	}
	if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==1 && _naluData[3]==0x65)
	{
		index = 0x65;
	}
	if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x41)
	{
		index = 0x41;
	}
	if (_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x01)
	{
		index = 0x01;
	}
	//
	switch(index)
	{
	case 0x67:        //SPS 
		if(m_vTrackId == MP4_INVALID_TRACK_ID)
		{
			m_vTrackId = MP4AddH264VideoTrack  
				(m_mp4FHandle,   
				m_vTimeScale,   
				m_vTimeScale / m_vFrateR,  
				m_vWidth,     // width  
				m_vHeight,    // height  
				_naluData[5], // sps[1] AVCProfileIndication  
				_naluData[6], // sps[2] profile_compat  
				_naluData[7], // sps[3] AVCLevelIndication  
				3);           // 4 bytes length before each NAL unit  
			if (m_vTrackId == MP4_INVALID_TRACK_ID)  
			{  
				MessageBoxA(NULL,"add video track failed.\n","ERROR!",MB_OK);  
				return;  
			} 
			MP4SetVideoProfileLevel(m_mp4FHandle, 0x7F);
		}

		MP4AddH264SequenceParameterSet(m_mp4FHandle,m_vTrackId,_naluData+4,_naluSize-4);  
		break;
	case 0x68:  //PPS
		MP4AddH264PictureParameterSet(m_mp4FHandle,m_vTrackId,_naluData+4,_naluSize-4);  
		break;
	case 0x65:  //I
		{
			BYTE* data = new BYTE[_naluSize+1];
			data[0] = (_naluSize-3)>>24;  
			data[1] = (_naluSize-3)>>16;  
			data[2] = (_naluSize-3)>>8;  
			data[3] = (_naluSize-3)&0xff;  
			memcpy(data+4,_naluData+3,_naluSize-3);
 
			//注意这里第五个参数，是指两个视频帧之间的tick数目，nTakeTime是时间间隔（毫秒），90000/1000是指每秒的tick数
			if(!MP4WriteSample(m_mp4FHandle, m_vTrackId, data, _naluSize+1, nTakeTime*90000/1000, 0, 1))
			{  
				return;  
			} 

			delete [] data;
			break;
		}
	case 0x41: //P
		{
			_naluData[0] = (_naluSize-4) >>24;  
			_naluData[1] = (_naluSize-4) >>16;  
			_naluData[2] = (_naluSize-4) >>8;  
			_naluData[3] = (_naluSize-4) &0xff; 

			//注意这里第五个参数，是指两个视频帧之间的tick数目，nTakeTime是时间间隔（毫秒），90000/1000是指每秒的tick数
			if(!MP4WriteSample(m_mp4FHandle, m_vTrackId, _naluData, _naluSize, nTakeTime*90000/1000, 0, 1))
			{  
				return;  
			} 

			break;
		}
	case 0x01:
		{

			break;
		}
	}
}

void CMp4Encoder::Mp4AEncode(BYTE* _aacData,int _aacSize)
{
	if(m_vTrackId == MP4_INVALID_TRACK_ID)
	{
		return;
	}
	//音频只要和采样时间相近的视频帧一同写入即可
	MP4WriteSample(m_mp4FHandle, m_aTrackId, _aacData, _aacSize , MP4_INVALID_DURATION, 0, 1);
}

void CMp4Encoder::CloseMp4Encoder()
{
	if(m_mp4FHandle)
	{  
		MP4Close(m_mp4FHandle);  
		m_mp4FHandle = NULL;  
	}
}

