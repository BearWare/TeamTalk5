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

    class GainController2 {
        public boolean bEnable = false;

        class FixedDigital {
            public float fGainDB = 0.0f;
            public FixedDigital() {
            }
        }

        class AdaptiveDigital {
            public boolean bEnable = false;
            public AdaptiveDigital() {
            }
        }
        public FixedDigital fixeddigital = new FixedDigital();
        public AdaptiveDigital adaptivedigital = new AdaptiveDigital();
    }

    public GainController2 gaincontroller2 = new GainController2();

    class NoiseSuppression {
        public boolean bEnable = false;
        public int nLevel = 1;
    }

    public NoiseSuppression noisesuppression = new NoiseSuppression();

    public WebRTCAudioPreprocessor() {
    }
    
}
