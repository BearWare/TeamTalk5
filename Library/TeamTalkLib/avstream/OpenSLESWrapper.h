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

#ifndef OPENSLESWRAPPER_H
#define OPENSLESWRAPPER_H

#include "SoundSystemBase.h"

#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>

#include <map>
#include <vector>
#include <set>
#include <mutex>

#define ANDROID_INPUT_BUFFERS 3
#define ANDROID_OUTPUT_BUFFERS 3

namespace soundsystem {

    struct SLInputStreamer : InputStreamer
    {
        SLObjectItf recorderObject = nullptr;
        SLRecordItf recorderRecord = nullptr;
        SLAndroidSimpleBufferQueueItf recorderBufferQueue = nullptr;
        std::recursive_mutex mutex;

        std::vector<short> buffers[ANDROID_INPUT_BUFFERS];
        uint32_t buf_index = 0;
        SLInputStreamer(StreamCapture* r, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : InputStreamer(r, sg, fs, sr, chs, sndsys, devid) { }
    };

    struct SLOutputStreamer : OutputStreamer
    {
        SLObjectItf playerObject = nullptr;
        SLPlayItf playerPlay = nullptr;
        SLAndroidSimpleBufferQueueItf playerBufferQueue = nullptr;
        std::recursive_mutex mutex;

        std::vector<short> buffers[ANDROID_OUTPUT_BUFFERS];
        uint32_t buf_index = 0;
        SLOutputStreamer(StreamPlayer* p, int sg, int fs, int sr, int chs, SoundAPI sndsys, int devid)
            : OutputStreamer(p, sg, fs, sr, chs, sndsys, devid) { }

    };

    struct SLSoundGroup : SoundGroup
    {
        SLObjectItf outputMixObject = nullptr;
        std::recursive_mutex mutex;
        int refCount = 0;
        SLSoundGroup() { }
        ~SLSoundGroup() { assert(!outputMixObject); assert(refCount == 0);}
    };

    typedef SoundSystemBase< SLSoundGroup, SLInputStreamer, SLOutputStreamer, DuplexStreamer > SSB;
    class OpenSLESWrapper : public SSB
    {
    private:
        OpenSLESWrapper();
        OpenSLESWrapper(const OpenSLESWrapper& aud);
        const OpenSLESWrapper& operator = (const OpenSLESWrapper& aud);

    protected:

        bool Init();
        void Close();
        void FillDevices(sounddevices_t& sounddevs);

        soundgroup_t NewSoundGroup();
        void RemoveSoundGroup(soundgroup_t sndgrp);

        // input
        inputstreamer_t NewStream(StreamCapture* capture,
                                  int inputdeviceid, int sndgrpid,
                                  int samplerate, int channels,
                                  int framesize);
        bool StartStream(inputstreamer_t streamer);
        bool StopStream(inputstreamer_t streamer);
        void CloseStream(inputstreamer_t streamer);
        bool IsStreamStopped(inputstreamer_t streamer);
        bool UpdateStreamCaptureFeatures(inputstreamer_t streamer);

        bool SetEchoCancellation(inputstreamer_t streamer, bool enable);
        bool IsEchoCancelling(inputstreamer_t streamer);
        bool SetAGC(inputstreamer_t streamer, bool enable);
        bool IsAGC(inputstreamer_t streamer);
        bool SetDenoising(inputstreamer_t streamer, bool enable);
        bool IsDenoising(inputstreamer_t streamer);

        // output
        outputstreamer_t NewStream(StreamPlayer* player, int outputdeviceid,
                                   int sndgrpid, int samplerate, int channels,
                                   int framesize);
        void CloseStream(outputstreamer_t streamer);

        bool StartStream(outputstreamer_t streamer);
        bool StopStream(outputstreamer_t streamer);
        bool IsStreamStopped(outputstreamer_t streamer);

        // duplex
        duplexstreamer_t NewStream(StreamDuplex* duplex, int inputdeviceid,
                                   int outputdeviceid, int sndgrpid,
                                   int samplerate, int input_channels,
                                   int output_channels, int framesize)  { return duplexstreamer_t(); }
        void CloseStream(duplexstreamer_t streamer) { }
        bool StartStream(duplexstreamer_t streamer) { return false; }
        bool IsStreamStopped(duplexstreamer_t streamer) { return true; }
        bool AddDuplexOutputStream(StreamDuplex* duplex,
                                   StreamPlayer* player)  { return false; }
        bool RemoveDuplexOutputStream(StreamDuplex* duplex,
                                      StreamPlayer* player)  { return false; }


    public:

        virtual ~OpenSLESWrapper();
        static std::shared_ptr<OpenSLESWrapper> getInstance();

        bool GetDefaultDevices(int& inputdeviceid,
                               int& outputdeviceid);
        bool GetDefaultDevices(SoundAPI sndsys,
                               int& inputdeviceid,
                               int& outputdeviceid);
    private:

        SLObjectItf InitOutputMixObject(soundgroup_t& sndgrp);
        void CloseOutputMixObject(soundgroup_t& sndgrp);

        // engine interfaces
        SLObjectItf m_engineObject;
        SLEngineItf m_engineEngine;
    };

    typedef SSB::soundgroup_t soundgroup_t;
    typedef SSB::inputstreamer_t inputstreamer_t;
    typedef SSB::outputstreamer_t outputstreamer_t;
    typedef SSB::duplexstreamer_t duplexstreamer_t;

}

#endif
