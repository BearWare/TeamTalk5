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

#include "SoundSystem.h"


namespace soundsystem {

    class AudioTbox : public SoundSystem
    {
        AudioTbox();
        virtual ~AudioTbox();
    public:
        static std::shared_ptr<AudioTbox> getInstance();
        bool RestartSoundSystem();

        //sound group members
        int NewSoundGroup();
        void RemoveSoundGroup(int sndgrpid);

        bool GetDefaultDevices(int& inputdeviceid,
                               int& outputdeviceid);
        bool GetDefaultDevices(SoundAPI sndsys,
                               int& inputdeviceid,
                               int& outputdeviceid);

        //input members
        bool OpenInputStream(StreamCapture* capture, int inputdeviceid, 
                             int sndgrpid, int samplerate, int channels,
                             int framesize);
        bool CloseInputStream(StreamCapture* capture);

        //output members
        bool OpenOutputStream(StreamPlayer* player, int outputdeviceid,
                              int sndgrpid, int samplerate, int channels, 
                              int framesize);
        bool CloseOutputStream(StreamPlayer* player);

        bool StartStream(StreamPlayer* player);
        bool StopStream(StreamPlayer* player);
        bool IsStreamStopped(StreamPlayer* player);
        bool IsStreamActive(StreamPlayer* player);

        //duplex members
        bool OpenDuplexStream(StreamDuplex* duplex, int inputdeviceid,
                              int outputdeviceid, int sndgrpid,
                              int samplerate, int input_channels, 
                              int output_channels, int framesize) { return false; }
        bool CloseDuplexStream(StreamDuplex* duplex)  { return false; }
        bool AddDuplexOutputStream(StreamDuplex* duplex,
                                   StreamPlayer* player) { return false; }
        bool RemoveDuplexOutputStream(StreamDuplex* duplex,
                                      StreamPlayer* player) { return false; }
    protected:
        bool Init();
        void Close();
        bool CanRestart();
        void FillDevices(sounddevices_t& sounddevs);

        SoundGroup* GetSoundGroup(int sndgrpid);
        InputStreamer* GetInputStreamer(StreamCapture* capture);
        OutputStreamer* GetOutputStreamer(StreamPlayer* player);
        void GetPlayers(int sndgrpid, std::vector<StreamPlayer*>& players);

        typedef std::map<int, SoundGroup> soundgroups_t;
        soundgroups_t m_sndgrps;

        typedef std::map<StreamPlayer*, struct ATOutputStreamer*> players_t;
        typedef std::map<StreamCapture*, struct ATInputStreamer*> recorders_t;

        players_t m_players;
        recorders_t m_recorders;
    };

}
