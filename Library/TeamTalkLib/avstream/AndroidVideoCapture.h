/*
 * Copyright (c) 2005-2026, BearWare.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 */

#if !defined(ANDROIDVIDEOCAPTURE_H)
#define ANDROIDVIDEOCAPTURE_H

#include "VideoCapture.h"

#include <mutex>

namespace vidcap {

    class AndroidVideoCapture : public VideoCapture
    {
    public:
        AndroidVideoCapture();
        ~AndroidVideoCapture() override;

        vidcap_devices_t GetDevices() override;

        bool InitVideoCapture(const tstring& deviceid,
                              const media::VideoFormat& vidfmt) override;

        bool StartVideoCapture() override;

        void StopVideoCapture() override;

        media::VideoFormat GetVideoCaptureFormat() override;

        bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc) override;
        void UnregisterVideoFormat(media::FourCC fcc) override;

        bool PushVideoFrame(const char* frame_data, int frame_len,
                            int width, int height) override;

    private:
        std::mutex m_mutex;
        vidcap_devices_t m_devices;
        media::VideoFormat m_curfmt;
        VideoCaptureCallback m_callback;
        bool m_running = false;
    };

} // namespace vidcap

#endif /* ANDROIDVIDEOCAPTURE_H */
