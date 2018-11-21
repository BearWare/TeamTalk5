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

#include "VideoCapture.h"

#if defined(ENABLE_LIBVIDCAP)
#include "LibVidCap.h"
#define VIDCAP_INST VCSingleton::instance()
#elif defined(ENABLE_QTKIT)
#include "QTVidCap.h"
#define VIDCAP_INST QTKitInst::instance()
#elif defined(ENABLE_AVF)
#include "AVFCapture.h"
#define VIDCAP_INST AVFSingleton::instance()
#elif defined(ENABLE_V4L2)
#include "V4L2Capture.h"
#define VIDCAP_INST V4L2Singleton::instance()
#else
#error No video capture library specified
#endif

vidcap::VideoCapture* GetVideoCapture()
{
    return VIDCAP_INST;
}

