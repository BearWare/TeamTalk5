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
            public float fInitialSaturationMarginDB;
            public float fExtraSaturationMarginDB;
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

            echocanceller.bEnable = WebRTCConstants.DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE;

            gaincontroller2.bEnable = WebRTCConstants.DEFAULT_WEBRTC_GAINCTL_ENABLE;
            gaincontroller2.fixeddigital.fGainDB = WebRTCConstants.DEFAULT_WEBRTC_GAINDB;

            gaincontroller2.adaptivedigital.bEnable = WebRTCConstants.DEFAULT_WEBRTC_SAT_PROT_ENABLE;
            gaincontroller2.adaptivedigital.fInitialSaturationMarginDB = WebRTCConstants.DEFAULT_WEBRTC_INIT_SAT_MARGIN_DB;
            gaincontroller2.adaptivedigital.fExtraSaturationMarginDB = WebRTCConstants.DEFAULT_WEBRTC_EXTRA_SAT_MARGIN_DB;
            gaincontroller2.adaptivedigital.fMaxGainChangeDBPerSecond = WebRTCConstants.DEFAULT_WEBRTC_MAXGAIN_DBSEC;
            gaincontroller2.adaptivedigital.fMaxOutputNoiseLevelDBFS = WebRTCConstants.DEFAULT_WEBRTC_MAX_OUT_NOISE;

            noisesuppression.bEnable = WebRTCConstants.DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE;
            noisesuppression.nLevel = WebRTCConstants.DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL;
        }
    }
}
