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

package dk.bearware;

public class WebRTCAudioPreprocessor {

    public class Preamplifier {
        public boolean bEnable;
        public float fFixedGainFactor;
    }

    public Preamplifier preamplifier = new Preamplifier();

    public class EchoCanceller {
        public boolean bEnable;
    }

    public EchoCanceller echocanceller = new EchoCanceller();

    public class GainController2 {
        public boolean bEnable;

        public class FixedDigital {
            public float fGainDB;
            public FixedDigital() {
            }
        }

        public class AdaptiveDigital {
            public boolean bEnable;
            public float fHeadRoomDB;
            public float fMaxGainDB;
            public float fInitialGainDB;
            public float fMaxGainChangeDBPerSecond;
            public float fMaxOutputNoiseLevelDBFS;

            public AdaptiveDigital() {
            }
        }
        public FixedDigital fixeddigital = new FixedDigital();
        public AdaptiveDigital adaptivedigital = new AdaptiveDigital();
    }

    public GainController2 gaincontroller2 = new GainController2();

    public class NoiseSuppression {
        public boolean bEnable;
        public int nLevel;
    }

    public NoiseSuppression noisesuppression = new NoiseSuppression();

    public WebRTCAudioPreprocessor() {
    }

    public WebRTCAudioPreprocessor(boolean setDefaults) {
        if (setDefaults) {

            preamplifier.bEnable = WebRTCConstants.DEFAULT_WEBRTC_PREAMPLIFIER_ENABLE;
            preamplifier.fFixedGainFactor = WebRTCConstants.DEFAULT_WEBRTC_PREAMPLIFIER_GAINFACTOR;

            echocanceller.bEnable = WebRTCConstants.DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE;

            noisesuppression.bEnable = WebRTCConstants.DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE;
            noisesuppression.nLevel = WebRTCConstants.DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL;

            gaincontroller2.bEnable = WebRTCConstants.DEFAULT_WEBRTC_GAINCTL_ENABLE;
            gaincontroller2.fixeddigital.fGainDB = WebRTCConstants.DEFAULT_WEBRTC_GAINDB;

            gaincontroller2.adaptivedigital.bEnable = WebRTCConstants.DEFAULT_WEBRTC_SAT_PROT_ENABLE;
            gaincontroller2.adaptivedigital.fHeadRoomDB = WebRTCConstants.DEFAULT_WEBRTC_HEADROOM_DB;
            gaincontroller2.adaptivedigital.fMaxGainDB = WebRTCConstants.DEFAULT_WEBRTC_MAXGAIN_DB;
            gaincontroller2.adaptivedigital.fInitialGainDB = WebRTCConstants.DEFAULT_WEBRTC_INITIAL_GAIN_DB;
            gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = WebRTCConstants.DEFAULT_WEBRTC_MAXGAIN_DBSEC;
            gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = WebRTCConstants.DEFAULT_WEBRTC_MAX_OUT_NOISE;
        }
    }
}
