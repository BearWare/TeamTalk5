/*
* Copyright (c) 2005-2018, BearWare.dk
*
* Contact Information:
*
* Bjoern D. Rasmussen
* Kirketoften 5
* DK-8260 Viby J
* Denmark
* Email: contact@bearware.dk
* Phone: +45 20 20 54 59
* Web: http://www.bearware.dk
*
* This source code is part of the TeamTalk SDK owned by
* BearWare.dk. Use of this file, or its compiled unit, requires a
* TeamTalk SDK License Key issued by BearWare.dk.
*
* The TeamTalk SDK License Agreement along with its Terms and
* Conditions are outlined in the file License.txt included with the
* TeamTalk SDK distribution.
*
*/

#if !defined(MFTRANSFORM_H)
#define MFTRANSFORM_H

#include <codec/MediaUtil.h>

#include <mfapi.h>
#include <atlbase.h>

#include <memory>

media::FourCC ConvertSubType(const GUID& native_subtype);
const GUID& ConvertFourCC(media::FourCC fcc);
ACE_TString FourCCToString(media::FourCC fcc);
media::VideoFormat ConvertVideoMediaType(IMFMediaType* pInputType);
// returns WAVEFORMATEX-struct
WAVEFORMATEX* MediaTypeToWaveFormatEx(IMFMediaType* pMediaType, std::vector<char>& buf);
CComPtr<IMFMediaType> ConvertAudioFormat(const media::AudioFormat& format);
ACE_Message_Block* ConvertVideoSample(IMFSample* pSample, const media::VideoFormat& fmt);
ACE_Message_Block* ConvertAudioSample(IMFSample* pSample, const media::AudioFormat& fmt);
std::vector<ACE_Message_Block*> ConvertRawSample(IMFSample* pSample);
CComPtr<IMFSample> CreateSample(const media::AudioFrame& frame);

enum TransformState
{
    TRANSFORM_SUBMITTED     = 0x1,
    TRANSFORM_ERROR         = 0x2,
    TRANSFORM_OUTPUTREADY   = 0x4,

    TRANSFORM_IO_SUCCESS    = TRANSFORM_SUBMITTED | TRANSFORM_OUTPUTREADY,
    TRANSFORM_INPUT_BLOCKED = TRANSFORM_ERROR | TRANSFORM_OUTPUTREADY,
};

typedef std::unique_ptr<class MFTransform> mftransform_t;

typedef CComPtr<IMFSample> imfsample_t;
typedef std::vector< CComPtr<IMFSample> > imfsamples_t;

class MFTransform
{
public:
    virtual ~MFTransform() {}
    static mftransform_t Create(IMFMediaType* pInputType, const GUID& dest_videoformat);
    static mftransform_t Create(const media::VideoFormat& inputfmt, media::FourCC outputfmt);
    static mftransform_t Create(media::AudioFormat inputfmt, media::AudioFormat outputfmt, int output_samples);
    static mftransform_t CreateMP3(const media::AudioFormat& inputfmt, UINT uBitrate, const ACE_TCHAR* szOutputFilename = nullptr);
    static mftransform_t CreateWMA(const media::AudioFormat& inputfmt, UINT uBitrate, const ACE_TCHAR* szOutputFilename = nullptr);
    static mftransform_t CreateAAC(const media::AudioFormat& inputfmt, UINT uBitrate, const ACE_TCHAR* szOutputFilename = nullptr);

    virtual CComPtr<IMFMediaType> GetInputType() = 0;
    virtual CComPtr<IMFMediaType> GetOutputType() = 0;

    virtual TransformState SubmitSample(CComPtr<IMFSample>& pInSample) = 0;
    virtual std::vector< CComPtr<IMFSample> > RetrieveSample() = 0;

    virtual bool Drain() = 0;

    virtual TransformState SubmitSample(const media::VideoFrame& frame) = 0;
    virtual TransformState SubmitSample(const media::AudioFrame& frame) = 0;
    virtual std::vector<ACE_Message_Block*> RetrieveVideoFrames() = 0;
    virtual std::vector<ACE_Message_Block*> RetrieveAudioFrames() = 0;
    virtual std::vector<ACE_Message_Block*> RetrieveRawFrames() = 0;

    virtual std::vector< CComPtr<IMFSample> > ProcessSample(CComPtr<IMFSample>& pInSample) = 0;
    // return ACE_Message_Block containing media::AudioFrame or media::VideoFrame
    virtual std::vector<ACE_Message_Block*> ProcessMBSample(CComPtr<IMFSample>& pInSample) = 0;
    // return ACE_Message_Block containing media::AudioFrame
    virtual std::vector<ACE_Message_Block*> ProcessAudioResampler(const media::AudioFrame& sample) = 0;
    virtual std::vector<ACE_Message_Block*> ProcessAudioEncoder(const media::AudioFrame& sample, bool bEraseOutput) = 0;

};
#endif
