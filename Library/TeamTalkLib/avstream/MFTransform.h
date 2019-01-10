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

typedef std::unique_ptr<class MFTransform> mftransform_t;
class MFTransform
{
public:
    virtual ~MFTransform() {}
    static mftransform_t Create(IMFMediaType* pInputType, const GUID& dest_videoformat);
    static mftransform_t Create(const media::VideoFormat& inputfmt, media::FourCC outputfmt);

    virtual bool SubmitSample(CComPtr<IMFSample>& pInSample) = 0;
    virtual CComPtr<IMFSample> RetrieveSample() = 0;

    virtual bool SubmitSample(const media::VideoFrame& frame) = 0;
    virtual ACE_Message_Block* RetrieveMBSample() = 0;

};
#endif
